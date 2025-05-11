# Summary for src/xpc/FWADaemon/Info.plist

This file is the property list (`Info.plist`) for the `FWADaemon` XPC service. It contains essential metadata that identifies and configures the daemon for the macOS system.

**Key Information Defined:**

-   **`CFBundleIdentifier` (Bundle Identifier):**
    -   Value: `net.mrmidi.FWADaemon`
    -   This is a crucial unique identifier for the XPC service. Client applications (like `FWA-Control` or the FWA library itself via `XPCBridge`) use this string to look up and connect to the `FWADaemon` service.

-   **`CFBundleName` (Bundle Name):**
    -   Value: `FWADaemon`
    -   A human-readable name for the daemon.

-   **`CFBundleVersion` (Bundle Version):**
    -   Value: `1.0`
    -   The version number of the daemon bundle.

-   **`CFBundleShortVersionString` (Bundle Short Version String):**
    -   Value: `1.0`
    -   A user-friendly version string.

-   **`LSMinimumSystemVersion` (Minimum System Version):**
    -   Value: `15.0`
    -   Specifies the minimum version of macOS required to run this daemon.

-   **Comments on Configuration (Important):**
    -   `<!-- Removed XPCService and MachServices keys -->`
    -   `<!-- Removed CFBundlePackageType or set to APPL -->`
    -   These comments indicate that some standard keys for XPC services might have been intentionally omitted or were previously present.
        -   **`XPCService` Dictionary:** For an XPC service managed by `launchd` (Apple's service management framework), this dictionary is typically required. It would define keys like `ServiceType` (e.g., "Application", "System", or "UserAgent") and `RunLoopType` (e.g., "NSRunLoop", "DispatchMain"). Its absence suggests that this XPC service might be launched directly by an application (e.g., `FWA-Control` embedding and launching it) rather than being a system-wide service managed by `launchd` based on its bundle identifier alone. However, the `main.m` for the daemon *does* set up an `NSXPCListener` with the Mach service name `net.mrmidi.FWADaemon`, and the entitlements allow global Mach lookup, which is more typical of a `launchd`-managed service. This might indicate an inconsistency or a specific launch strategy.
        -   **`CFBundlePackageType`**: For an XPC service bundle, this is usually `XPC!`. If it were `APPL`, it would be treated more like a standalone application.

**Overall Role:**
The `Info.plist` provides the basic identity and versioning for the `FWADaemon`. While it defines the crucial `CFBundleIdentifier` used for XPC service lookup, the comments about removed keys suggest that its configuration as a standard, `launchd`-managed XPC service might be incomplete or follow a non-standard launch pattern. The actual behavior will depend on how the daemon is packaged and launched (e.g., as part of an application bundle or via a separate `launchd.plist` file that references this bundle identifier).
