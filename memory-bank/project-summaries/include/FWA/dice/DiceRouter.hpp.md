# Summary for include/FWA/dice/DiceRouter.hpp

This C++ header file defines the `FWA::Dice::DiceRouter` class. This class is specifically designed to manage and control the internal signal routing matrix found in TC Electronic's DICE-based FireWire audio devices. It interacts with the device, typically via the Extended Audio Protocol (EAP), to query and modify routing connections.

**Key Declarations and Components:**

-   **Namespace `FWA::Dice`:** The class is defined within this nested namespace.

-   **Includes:**
    -   `"../DiceEAP.hpp"`: For the `DiceEAP` class, which is used to send EAP commands for routing control.
    -   `"DiceDefines.hpp"`: For DICE-specific constants, EAP command IDs related to routing, and potentially structures like `RoutingEntry` or `RoutingTableDimensions`.
    -   `../../Error.h`: For `IOKitError`.
    -   `<vector>`: For `std::vector` (e.g., to represent the routing matrix).
    -   `<map>`: Potentially for representing sparse routing tables or named channels.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<expected>`: For `std::expected` error handling.
    -   `<spdlog/spdlog.h>`: For logging.

-   **Public Nested Struct `RoutingTable` (or similar):**
    -   This structure would be defined to represent the state of the routing matrix.
    -   It might be a `std::vector<std::vector<bool>>` where `true` indicates a connection between an input and output index.
    -   Alternatively, it could be a list of `RoutingEntry` structs (where `RoutingEntry` might contain `sourceID` and `destinationID`).
    -   It might also store the dimensions of the matrix (number of routable inputs and outputs).

-   **Class `DiceRouter`:**
    -   **Public Interface:**
        -   **Constructor:**
            -   `DiceRouter(std::shared_ptr<DiceEAP> eap, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the router with a `DiceEAP` object (to send routing-related EAP commands) and a logger.
        -   **Routing Table Access:**
            -   `std::expected<RoutingTable, IOKitError> getRoutingTable() const;`
                -   This method uses the `_eap` object to send an EAP command (e.g., `EAP_CMD_GET_ROUTING_TABLE` from `DiceDefines.hpp`) to the DICE device to fetch the current state of its routing matrix.
                -   The EAP response payload, containing the routing data, is then parsed and transformed into the `RoutingTable` struct.
                -   The current routing table might be cached internally after a successful fetch.
        -   **Modifying Connections:**
            -   `std::expected<void, IOKitError> setConnection(const RoutingEntry& entry) const;`
                -   Takes a `RoutingEntry` struct (or individual source/destination IDs and a boolean connect/disconnect flag).
                -   Uses the `_eap` object to send an EAP command (e.g., `EAP_CMD_SET_ROUTING_ENTRY`) to the DICE device to establish or break the specified connection in the matrix.
        -   **Querying Connections:**
            -   `bool isConnected(uint16_t sourceID, uint16_t destinationID) const;`
                -   Checks if a specific route between a source and destination is currently active, typically by querying the cached `_routingTable`. May trigger a `getRoutingTable()` if the cache is considered stale.
        -   **Routing Capabilities (Conceptual):**
            -   Methods to discover the number of routable inputs and outputs, and potentially their names or types, e.g.:
                -   `std::expected<uint32_t, IOKitError> getNumberOfRouterInputs() const;`
                -   `std::expected<uint32_t, IOKitError> getNumberOfRouterOutputs() const;`
                -   These would also use specific EAP commands.

    -   **Private Members:**
        -   `std::shared_ptr<DiceEAP> _eap;`: The EAP communication handler.
        -   `std::shared_ptr<spdlog::logger> _logger;`: The logger instance.
        -   `mutable RoutingTable _cachedRoutingTable;`: (Optional) For caching the last known state of the routing table to avoid excessive EAP communication for queries. `mutable` if `isConnected` is `const`.
        -   `mutable std::mutex _cacheMutex;`: (Optional) If caching is used, a mutex to protect access to `_cachedRoutingTable`.

**Overall Role:**
The `DiceRouter` class provides a dedicated abstraction for managing the signal routing capabilities of DICE-based FireWire audio interfaces.
-   It encapsulates the EAP commands necessary to query and modify the device's internal routing matrix.
-   It offers a higher-level API to `DiceAudioDevice` for common routing operations like getting the current matrix state or setting up specific connections.
-   This separation of concerns keeps the `DiceAudioDevice` class cleaner and focuses `DiceRouter` on the specifics of routing control via EAP.
