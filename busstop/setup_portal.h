// ============================================================================
// setup_portal.h - Phone setup page (WiFi + bus stop)
// ============================================================================
#pragma once

// Starts the "BusStop-Setup" hotspot and web page. Saves the choices and
// restarts the device. Also restarts after a few minutes without activity.
// Does not return.
[[noreturn]] void runSetupPortal();
