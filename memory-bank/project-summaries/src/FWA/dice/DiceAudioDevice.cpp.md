# Summary for src/FWA/dice/DiceAudioDevice.cpp

This C++ file implements the `FWA::DiceAudioDevice` class. This class is a specialized version of `FWA::AudioDevice`, designed specifically to handle FireWire audio devices that are based on TC Electronic's DICE (Digital Interface Communication Engine) chipset. It inherits from `AudioDevice` and extends its functionality to support DICE-specific features and control mechanisms.

**Key Functionalities (Extending/Overriding `AudioDevice`):**

-   **Constructor (`DiceAudioDevice::DiceAudioDevice`):**
    -   Takes the same parameters as the `AudioDevice` constructor (`IOFireWireLibDeviceRef`, GUID, name, vendor, logger).
    -   Calls the base `AudioDevice` constructor.
    -   Initializes DICE-specific member variables (e.g., `_diceEAP`, `_diceRouter`).

-   **DICE-Specific Initialization (`init` - overridden or extended):**
    -   Calls the base class `AudioDevice::init()`.
    -   **DICE EAP (Extended Audio Protocol) Setup:**
        -   Creates an instance of `DiceEAP`, passing it the `CommandInterface` (obtained from the base class). The `DiceEAP` object is responsible for handling DICE's proprietary EAP messages, which are used for advanced control and status reporting (e.g., clock source, sample rate, routing, mixer levels).
        -   May register for asynchronous EAP notifications from the device.
    -   **DICE Router Setup:**
        -   Creates an instance of `DiceRouter`, also passing the `CommandInterface` (or the `DiceEAP` object if routing commands go via EAP). The `DiceRouter` manages the device's internal signal routing matrix.
    -   **Read DICE-Specific Information:**
        -   May read DICE-specific registers or use EAP commands to query:
            -   Detailed clock source information (names, current selection).
            -   Current sample rate.
            -   Routing matrix state.
            -   Mixer control parameters (if applicable and exposed).
            -   Other vendor-specific settings available on DICE chips.

-   **Extended Audio Protocol (EAP) Interaction:**
    -   Provides methods to send EAP commands and receive EAP status/data via the `_diceEAP` object.
    -   Examples: `sendEAPCommand(command, params)`, `getEAPStatus(statusID)`.

-   **Routing Control:**
    -   Provides methods to interact with the `_diceRouter` object:
        -   `getRouterMatrix()`: Retrieves the current state of the device's routing matrix.
        -   `setRoutingConnection(uint16_t outputChannel, uint16_t inputChannel, bool connect)`: Makes or breaks a connection in the routing matrix.
        -   `getAvailableRouterInputs()`, `getAvailableRouterOutputs()`: Lists available sources and destinations for routing.

-   **Clock Source Management (DICE-Specific):**
    -   Overrides or implements methods like:
        -   `getAvailableClockSources()`: Queries the device (likely via EAP) for a list of available clock sources (e.g., Internal, Word Clock, ADAT In, S/PDIF In).
        -   `getCurrentClockSource()`: Gets the currently selected clock source.
        -   `setClockSource(uint32_t sourceID)`: Sets a new clock source using DICE-specific commands.

-   **Sample Rate Control (DICE-Specific):**
    -   Overrides or implements methods like:
        -   `getAvailableSampleRates()`: Queries for supported sample rates.
        -   `getCurrentSampleRate()`: Gets the current operational sample rate.
        -   `setSampleRate(double rate)`: Sets a new sample rate.

-   **Vendor-Specific Controls:**
    -   May implement methods to control other features unique to DICE-based devices, such as phantom power toggles, input pads, specific DSP block parameters, etc. These are typically controlled via EAP messages or direct memory-mapped register access (if exposed via AV/C vendor-unique commands).

-   **JSON Serialization (`getDeviceInfoAsJson()` - overridden):**
    -   Calls the base class `AudioDevice::getDeviceInfoAsJson()` to get the common device information.
    -   Appends DICE-specific information to the JSON object, such as:
        -   Detailed clock source list and current selection.
        -   Routing matrix information.
        -   EAP version or status.

**Overall Role:**
The `DiceAudioDevice` class extends the generic `AudioDevice` to provide specialized support for the advanced features and control protocols found on DICE-based FireWire audio interfaces. By encapsulating DICE-specific logic (EAP, routing), it allows the FWA library to offer more comprehensive control and monitoring for these popular devices.
