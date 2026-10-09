import AppKit

// Draws the app icon (blue rounded square with a white bus) into an .iconset folder.
let outputDirectory = CommandLine.arguments[1]
try? FileManager.default.createDirectory(atPath: outputDirectory, withIntermediateDirectories: true)

func whiteSymbol(_ name: String, pointSize: CGFloat) -> NSImage {
    let configuration = NSImage.SymbolConfiguration(pointSize: pointSize, weight: .semibold)
    let base = NSImage(systemSymbolName: name, accessibilityDescription: nil)!.withSymbolConfiguration(configuration)!
    let image = NSImage(size: base.size)
    image.lockFocus()
    base.draw(at: .zero, from: .zero, operation: .sourceOver, fraction: 1)
    NSColor.white.set()
    NSRect(origin: .zero, size: base.size).fill(using: .sourceAtop)
    image.unlockFocus()
    return image
}

func render(size: Int) -> Data {
    let pixels = CGFloat(size)
    let bitmap = NSBitmapImageRep(bitmapDataPlanes: nil, pixelsWide: size, pixelsHigh: size, bitsPerSample: 8,
                                  samplesPerPixel: 4, hasAlpha: true, isPlanar: false, colorSpaceName: .deviceRGB,
                                  bytesPerRow: 0, bitsPerPixel: 0)!
    NSGraphicsContext.saveGraphicsState()
    NSGraphicsContext.current = NSGraphicsContext(bitmapImageRep: bitmap)

    let inset = pixels * 0.06
    let rect = NSRect(x: inset, y: inset, width: pixels - 2 * inset, height: pixels - 2 * inset)
    let shape = NSBezierPath(roundedRect: rect, xRadius: rect.width * 0.225, yRadius: rect.width * 0.225)
    NSGradient(starting: NSColor(red: 0.20, green: 0.46, blue: 0.95, alpha: 1),
               ending: NSColor(red: 0.09, green: 0.22, blue: 0.58, alpha: 1))!.draw(in: shape, angle: -90)

    let symbol = whiteSymbol("bus.fill", pointSize: pixels * 0.5)
    let target = NSRect(x: (pixels - symbol.size.width) / 2, y: (pixels - symbol.size.height) / 2,
                        width: symbol.size.width, height: symbol.size.height)
    symbol.draw(in: target)

    NSGraphicsContext.restoreGraphicsState()
    return bitmap.representation(using: .png, properties: [:])!
}

let sizes: [(String, Int)] = [
    ("icon_16x16", 16), ("icon_16x16@2x", 32), ("icon_32x32", 32), ("icon_32x32@2x", 64),
    ("icon_128x128", 128), ("icon_128x128@2x", 256), ("icon_256x256", 256), ("icon_256x256@2x", 512),
    ("icon_512x512", 512), ("icon_512x512@2x", 1024),
]
for (name, size) in sizes {
    try! render(size: size).write(to: URL(fileURLWithPath: "\(outputDirectory)/\(name).png"))
}
