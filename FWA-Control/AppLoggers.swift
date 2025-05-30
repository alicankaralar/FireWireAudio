import Logging

/// Centralized factory for creating Logger instances with consistent labels.
struct AppLoggers {
    static let app             = Logger(label: "net.mrmidi.fwa-control.App")
    static let deviceManager   = Logger(label: "net.mrmidi.fwa-control.DeviceManager")
    static let xpcManager      = Logger(label: "net.mrmidi.fwa-control.XPCManager")
    static let xpcHandler      = Logger(label: "net.mrmidi.fwa-control.XPCNotificationHandler")
    static let driverInstaller = Logger(label: "net.mrmidi.fwa-control.DriverInstaller")
    static let settings        = Logger(label: "net.mrmidi.fwa-control.Settings")
}
