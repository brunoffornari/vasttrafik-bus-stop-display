import SwiftUI
import UniformTypeIdentifiers

@main
struct BusStopUpdaterApp: App {
    init() {
        // Test mode without clicking: BusStopUpdater --run <file.ino> <display address>
        let arguments = CommandLine.arguments
        if let flag = arguments.firstIndex(of: "--run"), arguments.count > flag + 2 {
            Task { @MainActor in
                let updater = Updater()
                updater.echoLogToConsole = true
                updater.chooseSketch(URL(fileURLWithPath: arguments[flag + 1]))
                updater.deviceAddress = arguments[flag + 2]
                await updater.start()
                print("RESULT: \(updater.message?.text ?? "(no message)")")
                exit(updater.message?.isError == false ? 0 : 1)
            }
        }
    }

    var body: some Scene {
        WindowGroup {
            ContentView()
        }
        .windowResizability(.contentSize)
    }
}

struct ContentView: View {
    @StateObject private var updater = Updater()

    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            header

            card(number: "1", title: "Sketch") { sketchPicker }
            card(number: "2", title: "Display") { devicePicker }

            Button {
                Task { await updater.start() }
            } label: {
                Label(updater.isRunning ? "Updating..." : "Update display", systemImage: "arrow.up.circle.fill")
                    .font(.title3.weight(.semibold))
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 6)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
            .disabled(!updater.canStart)

            if updater.hasStarted { progressView }
            if let message = updater.message { messageView(message) }

            DisclosureGroup("Details") { logView }
            DisclosureGroup("Advanced") { advanced }
        }
        .padding(24)
        .frame(width: 540)
        .onDrop(of: [.fileURL], isTargeted: $updater.dropTargeted) { providers in
            guard let provider = providers.first else { return false }
            provider.loadItem(forTypeIdentifier: UTType.fileURL.identifier, options: nil) { item, _ in
                if let data = item as? Data, let url = URL(dataRepresentation: data, relativeTo: nil) {
                    Task { @MainActor in updater.chooseSketch(url) }
                }
            }
            return true
        }
        .overlay {
            if updater.dropTargeted {
                RoundedRectangle(cornerRadius: 14).strokeBorder(Color.accentColor, style: StrokeStyle(lineWidth: 3, dash: [8]))
                    .padding(6)
            }
        }
    }

    // MARK: Pieces

    private var header: some View {
        HStack(spacing: 12) {
            Image(systemName: "bus.fill")
                .font(.system(size: 26, weight: .semibold))
                .foregroundStyle(.white)
                .frame(width: 48, height: 48)
                .background(LinearGradient(colors: [Color(red: 0.18, green: 0.44, blue: 0.93), Color(red: 0.10, green: 0.25, blue: 0.63)],
                                           startPoint: .top, endPoint: .bottom),
                            in: RoundedRectangle(cornerRadius: 12))
            VStack(alignment: .leading, spacing: 2) {
                Text("Bus Stop Updater").font(.title2.weight(.bold))
                Text("Compile and send your code over WiFi").foregroundStyle(.secondary)
            }
        }
    }

    private func card<Content: View>(number: String, title: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack(spacing: 8) {
                Text(number)
                    .font(.caption.weight(.bold))
                    .frame(width: 20, height: 20)
                    .background(Color.accentColor, in: Circle())
                    .foregroundStyle(.white)
                Text(title).font(.headline)
            }
            content()
        }
        .padding(14)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.quaternary.opacity(0.5), in: RoundedRectangle(cornerRadius: 12))
    }

    private var sketchPicker: some View {
        HStack {
            Image(systemName: "doc.text").foregroundStyle(.secondary)
            if let url = updater.sketchURL {
                Text(url.lastPathComponent).fontWeight(.medium)
                Text(url.deletingLastPathComponent().path)
                    .font(.caption).foregroundStyle(.secondary)
                    .lineLimit(1).truncationMode(.head)
            } else {
                Text("No file chosen. Choose it, or drop it here.").foregroundStyle(.secondary)
            }
            Spacer()
            Button("Choose .ino...") { chooseFile() }
                .disabled(updater.isRunning)
        }
    }

    private var devicePicker: some View {
        HStack {
            Image(systemName: "wifi").foregroundStyle(.secondary)
            TextField("Display IP address, e.g. 192.168.1.89", text: $updater.deviceAddress)
                .textFieldStyle(.roundedBorder)
                .disabled(updater.isRunning)
            Button {
                Task { await updater.findDevice() }
            } label: {
                if updater.isFinding { ProgressView().controlSize(.small) } else { Text("Find") }
            }
            .frame(minWidth: 44)
            .disabled(updater.isRunning || updater.isFinding)
            .help("Looks for busstop.local on your WiFi")
        }
    }

    private var progressView: some View {
        VStack(alignment: .leading, spacing: 8) {
            ForEach(updater.steps) { step in
                HStack(spacing: 10) {
                    icon(for: step.status)
                    Text(step.title)
                        .foregroundStyle(step.status == .pending ? .secondary : .primary)
                    Spacer()
                    if step.id == 1, step.status == .running, let progress = updater.uploadProgress {
                        Text("\(Int(progress * 100))%").monospacedDigit().foregroundStyle(.secondary)
                    }
                }
                if step.id == 1, step.status == .running, let progress = updater.uploadProgress {
                    ProgressView(value: progress)
                }
            }
        }
        .padding(14)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(.quaternary.opacity(0.5), in: RoundedRectangle(cornerRadius: 12))
    }

    @ViewBuilder
    private func icon(for status: StepStatus) -> some View {
        switch status {
        case .pending: Image(systemName: "circle").foregroundStyle(.tertiary)
        case .running: ProgressView().controlSize(.small).frame(width: 16, height: 16)
        case .done: Image(systemName: "checkmark.circle.fill").foregroundStyle(.green)
        case .failed: Image(systemName: "xmark.circle.fill").foregroundStyle(.red)
        }
    }

    private func messageView(_ message: UserMessage) -> some View {
        HStack(alignment: .top, spacing: 10) {
            Image(systemName: message.isError ? "exclamationmark.triangle.fill" : "checkmark.seal.fill")
                .foregroundStyle(message.isError ? .orange : .green)
            Text(message.text).fixedSize(horizontal: false, vertical: true)
        }
        .padding(12)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background((message.isError ? Color.orange : Color.green).opacity(0.14), in: RoundedRectangle(cornerRadius: 10))
    }

    private var logView: some View {
        ScrollView {
            Text(updater.log.isEmpty ? "Nothing yet." : updater.log)
                .font(.system(size: 11, design: .monospaced))
                .foregroundStyle(.secondary)
                .frame(maxWidth: .infinity, alignment: .leading)
                .textSelection(.enabled)
        }
        .frame(height: 160)
        .padding(8)
        .background(.quaternary.opacity(0.4), in: RoundedRectangle(cornerRadius: 8))
        .padding(.top, 6)
    }

    private var advanced: some View {
        VStack(alignment: .leading, spacing: 8) {
            LabeledContent("Board") {
                TextField("", text: $updater.board).textFieldStyle(.roundedBorder)
            }
            LabeledContent("Update password") {
                SecureField("The one you set on the setup page", text: $updater.passwordOverride).textFieldStyle(.roundedBorder)
            }
            Button("Reset board") { updater.board = Updater.defaultBoard }
                .controlSize(.small)
        }
        .padding(.top, 6)
    }

    private func chooseFile() {
        let panel = NSOpenPanel()
        panel.title = "Choose your .ino file"
        panel.allowedContentTypes = [UTType(filenameExtension: "ino") ?? .data]
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        if let current = updater.sketchURL { panel.directoryURL = current.deletingLastPathComponent() }
        if panel.runModal() == .OK, let url = panel.url { updater.chooseSketch(url) }
    }
}
