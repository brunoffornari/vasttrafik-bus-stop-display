import Foundation
import SwiftUI

/// Collects text from a background process safely.
final class OutputBox: @unchecked Sendable {
    private let lock = NSLock()
    private var value = ""
    func add(_ text: String) { lock.lock(); value += text; lock.unlock() }
    var text: String { lock.lock(); defer { lock.unlock() }; return value }
}

enum StepStatus {
    case pending, running, done, failed
}

struct UpdateStep: Identifiable {
    let id: Int
    let title: String
    var status: StepStatus = .pending
}

struct UserMessage {
    let text: String
    let isError: Bool
}

/// Compiles the sketch with arduino-cli and sends it to the device over WiFi (espota).
@MainActor
final class Updater: ObservableObject {
    static let defaultBoard = "esp32:esp32:esp32:PartitionScheme=min_spiffs"
    static let defaultDeviceName = "busstop.local"

    @Published var sketchURL: URL?
    @Published var deviceAddress: String
    @Published var board: String
    @Published var passwordOverride = ""
    @Published var steps = Updater.freshSteps()
    @Published var uploadProgress: Double?
    @Published var log = ""
    @Published var isRunning = false
    @Published var isFinding = false
    @Published var message: UserMessage?
    @Published var dropTargeted = false

    private let defaults = UserDefaults.standard
    var echoLogToConsole = false

    init() {
        deviceAddress = defaults.string(forKey: "deviceAddress") ?? ""
        board = defaults.string(forKey: "board") ?? Updater.defaultBoard
        if let path = defaults.string(forKey: "sketchPath"), FileManager.default.fileExists(atPath: path) {
            sketchURL = URL(fileURLWithPath: path)
        }
    }

    var canStart: Bool {
        sketchURL != nil && !deviceAddress.trimmingCharacters(in: .whitespaces).isEmpty && !isRunning
    }

    var hasStarted: Bool {
        steps.contains { $0.status != .pending }
    }

    private static func freshSteps() -> [UpdateStep] {
        [UpdateStep(id: 0, title: "Compile the code"),
         UpdateStep(id: 1, title: "Send it to the display"),
         UpdateStep(id: 2, title: "Wait for the display to restart")]
    }

    // MARK: - Choosing the sketch

    /// Accepts a .ino file, or a sketch folder that contains <folder>.ino.
    func chooseSketch(_ url: URL) {
        var isDirectory: ObjCBool = false
        FileManager.default.fileExists(atPath: url.path, isDirectory: &isDirectory)

        var ino = url
        if isDirectory.boolValue {
            ino = url.appendingPathComponent(url.lastPathComponent + ".ino")
        }
        guard ino.pathExtension == "ino", FileManager.default.fileExists(atPath: ino.path) else {
            message = UserMessage(text: "Please choose a .ino file.", isError: true)
            return
        }
        sketchURL = ino
        defaults.set(ino.path, forKey: "sketchPath")
        message = nil
    }

    // MARK: - Finding the device

    func findDevice() async {
        isFinding = true
        defer { isFinding = false }
        message = nil

        let output = OutputBox()
        _ = await Updater.run("/sbin/ping", ["-c", "1", "-t", "3", Updater.defaultDeviceName]) { output.add($0) }
        if let address = Updater.firstMatch(in: output.text, pattern: #"\(([0-9]{1,3}(?:\.[0-9]{1,3}){3})\)"#) {
            deviceAddress = address
            defaults.set(address, forKey: "deviceAddress")
            message = UserMessage(text: "Found the display at \(address).", isError: false)
        } else {
            message = UserMessage(
                text: "Could not find the display. Is it on, and on the same WiFi as this Mac? You can type its IP address instead.",
                isError: true)
        }
    }

    // MARK: - Update

    func start() async {
        guard let sketch = sketchURL else {
            message = UserMessage(text: "Choose your .ino file first.", isError: true)
            return
        }
        let address = deviceAddress.trimmingCharacters(in: .whitespaces)
        defaults.set(address, forKey: "deviceAddress")
        defaults.set(board, forKey: "board")

        guard let cli = Updater.findArduinoCLI() else {
            message = UserMessage(text: "Arduino IDE was not found. Please install it from arduino.cc.", isError: true)
            return
        }
        guard let espota = Updater.findEspota() else {
            message = UserMessage(text: "The ESP32 board package is missing. Open Arduino IDE once and install \"esp32\".", isError: true)
            return
        }

        let sketchDir = sketch.deletingLastPathComponent()
        if sketchDir.lastPathComponent != sketch.deletingPathExtension().lastPathComponent {
            message = UserMessage(
                text: "The folder \"\(sketchDir.lastPathComponent)\" must have the same name as \(sketch.lastPathComponent).",
                isError: true)
            return
        }

        steps = Updater.freshSteps()
        uploadProgress = nil
        log = ""
        message = nil
        isRunning = true
        defer { isRunning = false }

        let password = passwordOverride.isEmpty ? (Updater.readUpdatePassword(in: sketchDir) ?? "") : passwordOverride
        let outputDir = NSTemporaryDirectory() + "BusStopUpdater-" + UUID().uuidString
        defer { try? FileManager.default.removeItem(atPath: outputDir) }

        // 1. Compile
        setStep(0, .running)
        append("$ arduino-cli compile \(sketch.lastPathComponent)\n")
        let compileCode = await Updater.run(cli, ["compile", "--fqbn", board, "--output-dir", outputDir, sketchDir.path]) {
            [weak self] text in Task { @MainActor in self?.append(text) }
        }
        let binary = outputDir + "/" + sketch.lastPathComponent + ".bin"
        guard compileCode == 0, FileManager.default.fileExists(atPath: binary) else {
            setStep(0, .failed)
            message = UserMessage(text: "The code did not compile. Open \"Details\" below to see why.", isError: true)
            return
        }
        setStep(0, .done)

        // 2. Send
        setStep(1, .running)
        uploadProgress = 0
        append("\n$ sending to \(address)\n")
        var espotaArguments = [espota, "-i", address, "-p", "3232", "-f", binary, "-r"]
        if !password.isEmpty { espotaArguments += ["-a", password] }
        let uploadCode = await Updater.run("/usr/bin/python3", espotaArguments) { [weak self] text in
            Task { @MainActor in
                self?.append(text)
                if let percent = Updater.lastPercent(in: text) { self?.uploadProgress = Double(percent) / 100 }
            }
        }
        guard uploadCode == 0 else {
            setStep(1, .failed)
            uploadProgress = nil
            message = UserMessage(
                text: "Could not send the code. Check the IP address, that the display is on and on the same WiFi, and that the update password is right.",
                isError: true)
            return
        }
        uploadProgress = 1
        setStep(1, .done)

        // 3. Wait for the display to come back
        setStep(2, .running)
        try? await Task.sleep(nanoseconds: 4_000_000_000)
        var backOnline = false
        for _ in 0..<25 {
            let code = await Updater.run("/sbin/ping", ["-c", "1", "-t", "1", address]) { _ in }
            if code == 0 { backOnline = true; break }
            try? await Task.sleep(nanoseconds: 1_000_000_000)
        }
        if backOnline {
            setStep(2, .done)
            message = UserMessage(text: "Done! The display is running the new code.", isError: false)
        } else {
            setStep(2, .failed)
            message = UserMessage(text: "The code was sent, but the display did not answer yet. Give it a moment and look at the screen.", isError: true)
        }
    }

    // MARK: - Helpers

    private func setStep(_ index: Int, _ status: StepStatus) {
        steps[index].status = status
    }

    private func append(_ text: String) {
        if echoLogToConsole { print(text, terminator: "") }
        log += text.replacingOccurrences(of: "\r", with: "\n")
        if log.count > 40_000 { log = String(log.suffix(30_000)) }
    }

    static func findArduinoCLI() -> String? {
        [
            "/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli",
            "/opt/homebrew/bin/arduino-cli",
            "/usr/local/bin/arduino-cli",
        ].first { FileManager.default.isExecutableFile(atPath: $0) }
    }

    static func findEspota() -> String? {
        let base = NSHomeDirectory() + "/Library/Arduino15/packages/esp32/hardware/esp32"
        guard let versions = try? FileManager.default.contentsOfDirectory(atPath: base) else { return nil }
        for version in versions.sorted(by: { $0.compare($1, options: .numeric) == .orderedDescending }) {
            let path = "\(base)/\(version)/tools/espota.py"
            if FileManager.default.fileExists(atPath: path) { return path }
        }
        return nil
    }

    /// Looks for `kOtaPassword = "..."` in the sketch's .h files.
    static func readUpdatePassword(in directory: URL) -> String? {
        guard let files = try? FileManager.default.contentsOfDirectory(at: directory, includingPropertiesForKeys: nil) else { return nil }
        for file in files where file.pathExtension == "h" {
            guard let text = try? String(contentsOf: file, encoding: .utf8) else { continue }
            if let password = firstMatch(in: text, pattern: #"kOtaPassword\s*=\s*"([^"]*)""#) { return password }
        }
        return nil
    }

    static func firstMatch(in text: String, pattern: String) -> String? {
        guard let regex = try? NSRegularExpression(pattern: pattern),
              let match = regex.firstMatch(in: text, range: NSRange(text.startIndex..., in: text)),
              match.numberOfRanges > 1,
              let range = Range(match.range(at: 1), in: text) else { return nil }
        return String(text[range])
    }

    static func lastPercent(in text: String) -> Int? {
        guard let regex = try? NSRegularExpression(pattern: #"(\d{1,3})%"#) else { return nil }
        let matches = regex.matches(in: text, range: NSRange(text.startIndex..., in: text))
        guard let last = matches.last, let range = Range(last.range(at: 1), in: text) else { return nil }
        return Int(text[range]).map { min($0, 100) }
    }

    /// Runs a program and streams its output. Returns the exit code (-1 if it could not start).
    nonisolated static func run(_ executable: String, _ arguments: [String],
                                onOutput: @escaping @Sendable (String) -> Void) async -> Int32 {
        await withCheckedContinuation { continuation in
            let process = Process()
            process.executableURL = URL(fileURLWithPath: executable)
            process.arguments = arguments
            var environment = ProcessInfo.processInfo.environment
            environment["PATH"] = "/usr/bin:/bin:/usr/sbin:/sbin:/opt/homebrew/bin"
            process.environment = environment

            let pipe = Pipe()
            process.standardOutput = pipe
            process.standardError = pipe
            pipe.fileHandleForReading.readabilityHandler = { handle in
                let data = handle.availableData
                if !data.isEmpty, let text = String(data: data, encoding: .utf8) { onOutput(text) }
            }
            process.terminationHandler = { finished in
                pipe.fileHandleForReading.readabilityHandler = nil
                let rest = pipe.fileHandleForReading.readDataToEndOfFile()
                if !rest.isEmpty, let text = String(data: rest, encoding: .utf8) { onOutput(text) }
                continuation.resume(returning: finished.terminationStatus)
            }
            do {
                try process.run()
            } catch {
                onOutput("Could not start \(executable): \(error.localizedDescription)\n")
                continuation.resume(returning: -1)
            }
        }
    }
}
