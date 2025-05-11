# Summary for src/Isoch/core/IsochPacketProcessor.cpp

This C++ file implements the `FWA::Isoch::IsochPacketProcessor` class. This class serves as a central processing unit for isochronous packets, handling both incoming (receive) and outgoing (transmit) AMDT-P (Audio and Music Data Transmission Protocol) data. It deals with the intricacies of CIP (Common Isochronous Packet) headers and the AMDT-P payload.

**Key Functionalities:**

-   **Constructor (`IsochPacketProcessor::IsochPacketProcessor`):**
    -   Takes dependencies such as:
        -   A shared pointer to an `AmdtpReceiver` instance (for handling depacketization of received data).
        -   A shared pointer to an `AmdtpTransmitter` instance (for handling packetization of data to be transmitted).
        -   A shared pointer to a `CIPHeaderHandler` instance (for parsing and constructing CIP headers).
        -   An `spdlog::logger`.
    -   Initializes member variables with these dependencies.

-   **Processing Received Packets (`processReceivedPacket` or similar):**
    -   `void processReceivedPacket(const uint8_t* packetData, size_t packetLength)`:
        -   This method is called when a raw isochronous packet (a full CIP packet) is received from the IOKit layer (e.g., via a DCL completion callback managed by `AmdtpReceiver` or `IsochDCLManager`).
        -   **CIP Header Parsing:** Uses the `_cipHeaderHandler->parseHeader(packetData)` to extract information from the CIP header, such as:
            -   Data Block Size (DBS)
            -   Source Packet Header (SPH) flag
            -   Data Block Counter (DBC)
            -   Format Dependent Field (FDF)
            -   SYT (timestamp), if present.
        -   **Validation:** Performs basic validation of the CIP header and AMDT-P structure.
        -   **AMDT-P Depacketization:** If the FDF indicates an AMDT-P packet (e.g., audio or MIDI data):
            -   It passes the relevant parts of the `packetData` (the AMDT-P payload) along with the parsed CIP header information (especially SYT and DBC) to the `_amdtpReceiver->processPacket(...)` method.
            -   The `AmdtpReceiver` then handles the detailed depacketization of the AMDT-P data blocks and extraction of the actual audio/MIDI samples.
        -   Handles different AMDT-P FDF types (e.g., audio, MIDI, label/no-data).

-   **Preparing Packets for Transmission (`getNextTransmitPacket` or similar):**
    -   `bool getNextTransmitPacket(std::vector<uint8_t>& outPacketBuffer, uint64_t currentTimestamp)`:
        -   This method is called by a component that feeds packets to the IOKit transmit mechanism (e.g., `IsochPacketProvider`).
        -   **Request Data from Transmitter:** It requests the next chunk of audio/MIDI data to be packetized from the `_amdtpTransmitter->getNextAudioData(...)` or a similar method. The `_amdtpTransmitter` would have been fed this data from an upstream source (like `ShmIsochBridge` via `AmdtpTransmitStreamProcessor`).
        -   **AMDT-P Packet Construction:** If data is available (or if a "no data" packet needs to be sent for timing):
            -   The `_amdtpTransmitter` formats the data into AMDT-P data blocks.
            -   **CIP Header Generation:** The `IsochPacketProcessor` (or the `AmdtpTransmitter` itself, using `_cipHeaderHandler`) constructs the CIP header for the outgoing packet. This includes:
                -   Setting SPH, DBS, FDF appropriately.
                -   Incrementing and setting the DBC.
                -   Calculating and embedding the SYT timestamp based on `currentTimestamp` and the SYT interval.
            -   The CIP header and AMDT-P data blocks are combined into `outPacketBuffer`.
        -   Returns `true` if a packet was successfully generated, `false` otherwise (e.g., no data and no need to send a timing packet).

-   **Synchronization and Timestamp Management:**
    -   For transmission, it ensures that SYT timestamps are correctly generated and embedded in outgoing CIP headers, crucial for receiver synchronization.
    -   For reception, it extracts SYT timestamps and passes them to the `AmdtpReceiver`, which can use them for clock recovery (`AudioClockPLL`) or to timestamp the received audio data.

-   **State Management:**
    -   May maintain state related to the current stream, such as sequence numbers (DBC) for detecting packet loss on receive, or managing SYT generation intervals on transmit.

**Overall Role:**
The `IsochPacketProcessor` acts as a bidirectional processing hub for AMDT-P over CIP. It sits between the raw isochronous packet layer (interfacing with IOKit via DCLs) and the more specialized AMDT-P packetizer (`AmdtpTransmitter`) and depacketizer (`AmdtpReceiver`). It leverages the `CIPHeaderHandler` to deal with the complexities of the Common Isochronous Packet format, ensuring that data is correctly encapsulated for transmission and correctly parsed upon reception according to the IEC 61883 standards.
