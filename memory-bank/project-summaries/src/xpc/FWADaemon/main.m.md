# Summary for src/xpc/FWADaemon/main.m

This Objective-C file (`main.m`) serves as the main entry point for the `FWADaemon` XPC service. Its primary responsibility is to set up and start the `NSXPCListener` that will listen for and accept incoming XPC connections from clients.

**Key Components and Workflow:**

1.  **Imports:**
    -   `Foundation/Foundation.h`: For core Foundation classes, including XPC types.
    -   `shared/xpc/FWADaemonControlProtocol.h`: Imports the Objective-C protocol that defines the interface the daemon exports to its clients (e.g., methods the driver or GUI can call on the daemon).
    -   `FWADaemon.h`: Imports the header for the `FWADaemon` class, which is the actual object that implements the `FWADaemonControlProtocol` and handles client requests.

2.  **`FWADaemonService` Class:**
    -   **Purpose:** This is a small helper class that acts as the delegate for the `NSXPCListener`.
    -   **Conformance:** Implements the `NSXPCListenerDelegate` protocol.
    -   **`- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection` method:**
        -   This delegate method is invoked by the `NSXPCListener` whenever a new client attempts to connect to the `net.mrmidi.FWADaemon` service.
        -   **Exported Interface:** It configures the `newConnection` by setting its `exportedInterface` to `[NSXPCInterface interfaceWithProtocol:@protocol(FWADaemonControlProtocol)]`. This informs the XPC system (and the connecting client) about the set of methods (defined in `FWADaemonControlProtocol`) that the daemon makes available.
        -   **Exported Object:** Crucially, it sets `newConnection.exportedObject = [FWADaemon sharedService];`. This assigns the singleton instance of the `FWADaemon` class as the object that will actually handle the incoming XPC messages conforming to the `FWADaemonControlProtocol`.
        -   **Connection Handlers:** Sets up `interruptionHandler` and `invalidationHandler` blocks for the `newConnection` to log when a client connection is unexpectedly lost or deliberately invalidated.
        -   `[newConnection resume];`: Activates the connection, allowing it to start processing messages.
        -   Returns `YES` to indicate that the new connection is accepted.

3.  **`main()` Function:**
    -   This is the C standard entry point for the executable.
    -   **Autorelease Pool:** Standard Objective-C practice.
    -   **Create `NSXPCListener`:**
        -   `NSXPCListener *listener = [[NSXPCListener alloc] initWithMachServiceName:@"net.mrmidi.FWADaemon"];`
        -   This creates the XPC listener that will listen on the globally registered Mach service name "net.mrmidi.FWADaemon". This name must match the one specified in the daemon's entitlements and the one used by clients to connect.
    -   **Set Delegate:**
        -   An instance of `FWADaemonService` is created and assigned as the `delegate` of the `listener`.
    -   **Resume Listener:**
        -   `[listener resume];` starts the listener, making it active and ready to accept incoming client connections.
    -   **Initialize Daemon Singleton:**
        -   `[FWADaemon sharedService];`
        -   This line explicitly calls the class method to get (and thus initialize if not already done) the `FWADaemon` singleton. This is important because the `FWADaemon`'s `init` method performs critical setup, such as creating the shared memory segment (`setupSharedMemory`). Initializing it here ensures these resources are ready before any client connects or tries to use them.
    -   **Run the Run Loop:**
        -   `[[NSRunLoop currentRunLoop] run];`
        -   This call starts the main run loop for the daemon process. The XPC listener and its connections operate within this run loop. The `run` call will block, keeping the daemon process alive and responsive to XPC events until the service is terminated.

**Overall Role:**
`main.m` is the bootstrap code for the `FWADaemon`. It sets up the XPC listening infrastructure, specifies how incoming connections should be handled (by delegating to the `FWADaemon` singleton), and then enters a run loop to keep the service operational. It ensures that the core daemon logic is initialized and ready to serve client requests.
