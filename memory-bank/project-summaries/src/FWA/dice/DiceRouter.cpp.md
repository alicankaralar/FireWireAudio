# Summary for src/FWA/dice/DiceRouter.cpp

This C++ file implements the `FWA::DiceRouter` class. This class is specifically designed to manage and control the internal signal routing matrix found in DICE-based FireWire audio devices. DICE chipsets often feature a flexible routing system that allows users to connect various inputs (physical inputs, computer playback channels) to various outputs (physical outputs, computer recording channels).

**Key Functionalities:**

-   **Constructor (`DiceRouter::DiceRouter`):**
    -   Takes a `std::shared_ptr<DiceEAP>` (which is used to send DICE Extended Audio Protocol commands for routing control) and an `std::shared_ptr<spdlog::logger>`.
    -   Stores these dependencies. The `DiceEAP` object is crucial as routing on DICE devices is typically managed via EAP messages.

-   **Fetching Routing Table (`fetchRoutingTable`):**
    -   `std::expected<RoutingTable, IOKitError> fetchRoutingTable()`:
        -   This method queries the DICE device (via EAP commands sent through the `_diceEAP` object) to retrieve the current state of its entire routing matrix.
        -   The `RoutingTable` is likely a struct or class that can represent the matrix, perhaps as a 2D array of booleans (connected/disconnected) or a list of active connections.
        -   The EAP command for this might be `GET_ROUTING_TABLE` or similar.
        -   The response from the device contains the routing data, which is then parsed and stored in the `RoutingTable` object.
        -   This method might be called during initialization to get the initial state and can be called later to refresh the view.
        -   The current state is often cached internally in `_routingTable`.

-   **Modifying Routing Connections (`setConnection`):**
    -   `std::expected<void, IOKitError> setConnection(uint16_t outputChannelID, uint16_t inputChannelID, bool connect)`:
        -   This is the primary method for making or breaking a connection in the routing matrix.
        -   **Parameters:**
            -   `outputChannelID`: The ID of the destination channel in the matrix (e.g., a physical output, a computer record stream channel).
            -   `inputChannelID`: The ID of the source channel in the matrix (e.g., a physical input, a computer playback stream channel).
            -   `connect`: `true` to establish the connection, `false` to break it.
        -   It constructs and sends an EAP command (e.g., `SET_ROUTING_ENTRY`) via the `_diceEAP` object to instruct the DICE chip to change the routing.
        -   After a successful command, it might call `fetchRoutingTable()` again to update its internal cache or rely on the device to confirm the change.

-   **Querying Routing State:**
    -   `const RoutingTable& getCachedRoutingTable() const`: Returns a reference to the internally cached routing table.
    -   `bool isConnected(uint16_t outputChannelID, uint16_t inputChannelID) const`: Checks the cached routing table to see if a specific route is currently active.

-   **Discovering Routing Capabilities (Conceptual):**
    -   The class might also include methods (or rely on `DiceEAP` to provide data for) discovering the dimensions of the routing matrix:
        -   Number of available input sources for routing.
        -   Number of available output destinations for routing.
        -   Names or types associated with these routable channels (e.g., "Analog In 1", "Playback Ch 1-2", "Main Out L/R", "Record Ch 1-2"). This information is often also retrieved via EAP commands.

**Overall Role:**
The `DiceRouter` class provides a high-level abstraction for the complex task of managing the signal routing within a DICE-based audio interface. It hides the specifics of the EAP commands required for routing control and presents a more intuitive API (e.g., `setConnection`). This class is a key component used by `DiceAudioDevice` to expose routing capabilities to applications or higher-level control logic. It allows users to customize how audio signals flow through the device.
