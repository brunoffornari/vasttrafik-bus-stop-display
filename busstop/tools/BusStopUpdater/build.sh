#!/bin/bash
# Builds "Bus Stop Updater.app" (needs the Xcode command line tools).
set -e
cd "$(dirname "$0")"

APP="Bus Stop Updater.app"
rm -rf "$APP" .build && mkdir -p .build "$APP/Contents/MacOS" "$APP/Contents/Resources"

echo "Making the icon..."
swiftc -o .build/make_icon make_icon.swift -framework AppKit
.build/make_icon .build/AppIcon.iconset
iconutil -c icns .build/AppIcon.iconset -o "$APP/Contents/Resources/AppIcon.icns"

echo "Compiling the app..."
swiftc -O -swift-version 5 -parse-as-library -target arm64-apple-macosx13.0 \
  -o "$APP/Contents/MacOS/BusStopUpdater" Sources/*.swift

cat > "$APP/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>Bus Stop Updater</string>
  <key>CFBundleDisplayName</key><string>Bus Stop Updater</string>
  <key>CFBundleIdentifier</key><string>com.brunofornari.busstopupdater</string>
  <key>CFBundleExecutable</key><string>BusStopUpdater</string>
  <key>CFBundleIconFile</key><string>AppIcon</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleShortVersionString</key><string>1.0</string>
  <key>CFBundleVersion</key><string>1</string>
  <key>LSMinimumSystemVersion</key><string>13.0</string>
  <key>NSHighResolutionCapable</key><true/>
  <key>NSLocalNetworkUsageDescription</key><string>Bus Stop Updater sends your code to the display on your WiFi.</string>
</dict>
</plist>
PLIST

codesign --force --sign - "$APP" >/dev/null 2>&1 || true
echo "Done: $PWD/$APP"
