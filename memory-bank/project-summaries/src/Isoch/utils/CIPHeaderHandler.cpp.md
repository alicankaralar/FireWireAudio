# Summary for src/Isoch/utils/CIPHeaderHandler.cpp

This C++ file implements the `FWA::Isoch::CIPHeaderHandler` class. This class is a utility focused on parsing fields from and constructing Common Isochronous Packet (CIP) headers. CIP headers are a fundamental part of the IEC 61883 standard for transmitting time-sensitive data (like audio and video) over IEEE 1394 (FireWire). AMDT-P packets are encapsulated within CIP packets.

**Key Functionalities:**

-   **`CIPHeader` Struct (Likely defined in `CIPHeaderHandler.hpp`):**
    -   This struct would hold the parsed fields of a CIP header, such as:
        -   `sph` (Source Packet Header flag - boolean)
        -   `dbc` (Data Block Counter - uint8_t)
        -   `fmt` (Format ID - uint8_t, e.g., AMDT_AUDIO, AMDT_MIDI)
        -   `fdf` (Format Dependent Field - uint8_t, content depends on `fmt`)
        -   `syt` (System Time timestamp - uint16_t, present if `sph` is true)
        -   (It might also include fields from the general isochronous packet header like source_ID, data_length, tag, channel, tcode, sy, if processed together).

-   **Parsing Incoming CIP Headers (`parseHeader`):**
    -   `CIPHeader parseHeader(const uint8_t* headerData)`:
        -   Takes a pointer `headerData` to the raw bytes of an incoming isochronous packet (at least the 8 bytes of the CIP header, and potentially the preceding isochronous packet header quadlets).
        -   **Field Extraction:** It carefully extracts each field from the raw bytes according to the bit layout defined in the IEC 61883-1 (CIP) and IEC 61883-6 (AMDT-P) specifications. This involves:
            -   Reading the first quadlet of the isochronous packet (if included) for `data_length`, `tag`, `channel`, `tcode`, `sy`.
            -   Reading the second quadlet (first quadlet of CIP header proper) for `source_ID_hi`, `sph`, `dbc`, `fmt`, `fdf_format_part`.
            -   Reading the third quadlet (second quadlet of CIP header proper) for `fdf_specific_part` and `syt` (if `sph` is true).
            -   Byte swapping (e.g., for `syt`) might be necessary depending on the endianness of the FireWire bus versus the host CPU.
        -   Populates and returns a `CIPHeader` struct with these extracted values.

-   **Constructing Outgoing CIP Headers (`constructHeader`):**
    -   `void constructHeader(uint8_t* headerBuffer, const CIPHeader& headerInfo)`:
        -   Takes a pointer `headerBuffer` to a memory location (at least 8 bytes) where the CIP header will be constructed, and a `CIPHeader` struct containing the desired values for each field.
        -   **Field Packing:** It meticulously packs each field from the `headerInfo` struct into the `headerBuffer` according to the correct bit positions and sizes. This involves:
            -   Setting the SPH bit.
            -   Placing the DBC value.
            -   Setting the FMT field (e.g., to indicate AMDT-P audio).
            -   Setting the FDF field (e.g., with event count and channel information for AMDT-P audio).
            -   If `headerInfo.sph` is true, it embeds the `headerInfo.syt` timestamp into the last two bytes of the CIP header.
            -   (It might also construct the preceding isochronous packet header quadlet if this function is responsible for the full 8-byte isochronous header + 8-byte CIP header).

-   **Utility Functions (from `AmdtpHelpers.cpp` which this might supersede or complement):**
    -   The functions previously attributed to `AmdtpHelpers` for getting/setting individual CIP fields (like `getSyt`, `setSyt`, `getDbc`, `setDbc`, etc.) are essentially the core operations performed within `parseHeader` and `constructHeader` of this `CIPHeaderHandler` class. This class centralizes that logic.

**Overall Role:**
The `CIPHeaderHandler` class is a vital utility for any component dealing directly with isochronous packets on the FireWire bus, particularly those implementing AMDT-P.
-   **For Receivers (e.g., `AmdtpReceiver`, `IsochPacketProcessor`):** It's used to dissect incoming raw packet data, making sense of the CIP header to determine packet type, sequence, timing (SYT), and how to interpret the subsequent AMDT-P payload.
-   **For Transmitters (e.g., `AmdtpTransmitter`, `IsochPacketProcessor`):** It's used to build valid CIP headers for outgoing AMDT-P packets, ensuring all fields are correctly set according to the standard and the data being transmitted.
By encapsulating the bit-level details of CIP header manipulation, it simplifies the implementation of higher-level protocols like AMDT-P and promotes correctness in handling isochronous data.
