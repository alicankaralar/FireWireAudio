# Summary for src/Isoch/utils/AmdtpHelpers.cpp

This C++ file implements a set of helper functions within the `FWA::Isoch::AmdtpHelpers` namespace. These functions are specifically designed to assist with the creation, parsing, and manipulation of AMDT-P (Audio and Music Data Transmission Protocol) packets and their encapsulating CIP (Common Isochronous Packet) headers, as defined by the IEC 61883-6 standard.

**Key Functionalities:**

-   **CIP Header Field Extraction (Getters):**
    -   These functions take a pointer to the raw byte data of a CIP header (`const uint8_t* cipHeader`) and extract specific fields:
        -   `uint8_t getSourceNodeID(const uint8_t* cipHeader)`: Extracts the source node ID (though this is part of the general isochronous packet header, not CIP specific).
        -   `uint8_t getDataLength(const uint8_t* cipHeader)`: Extracts the data length field from the isochronous packet header.
        -   `uint8_t getTag(const uint8_t* cipHeader)`: Extracts the tag field.
        -   `uint8_t getChannel(const uint8_t* cipHeader)`: Extracts the isochronous channel number.
        -   `uint8_t getTCode(const uint8_t* cipHeader)`: Extracts the transaction code.
        -   `uint8_t getSy(const uint8_t* cipHeader)`: Extracts the synchronization code.
        -   `uint16_t getSyt(const uint8_t* cipHeader)`: Extracts the 16-bit SYT (System Time) timestamp value from the CIP header. Handles byte order if necessary.
        -   `uint8_t getDbs(const uint8_t* cipHeader)`: Extracts the Data Block Size (DBS).
        -   `bool getSph(const uint8_t* cipHeader)`: Extracts the Source Packet Header (SPH) flag.
        -   `uint8_t getDbc(const uint8_t* cipHeader)`: Extracts the Data Block Counter (DBC).
        -   `uint8_t getFdfFormat(const uint8_t* cipHeader)`: Extracts the format part of the Format Dependent Field (FDF).
        -   `uint8_t getFdfSpecific(const uint8_t* cipHeader)`: Extracts the format-specific part of the FDF. For AMDT-P audio, this field contains the event number and audio channel count.

-   **CIP Header Field Construction (Setters):**
    -   These functions take a pointer to a mutable CIP header byte array (`uint8_t* cipHeader`) and a value, and set the corresponding field in the header:
        -   `void setSyt(uint8_t* cipHeader, uint16_t syt)`
        -   `void setDbs(uint8_t* cipHeader, uint8_t dbs)`
        -   `void setSph(uint8_t* cipHeader, bool sph)`
        -   `void setDbc(uint8_t* cipHeader, uint8_t dbc)`
        -   `void setFdfFormat(uint8_t* cipHeader, uint8_t fdfFormat)`
        -   `void setFdfSpecific(uint8_t* cipHeader, uint8_t fdfSpecific)`

-   **AMDT-P Specific FDF Field Manipulation (Audio):**
    -   `uint8_t packAmdtpAudioFdfSpecific(uint8_t event, uint8_t channels)`: Packs the AMDT-P event number (e.g., sample count) and number of audio channels into the 8-bit FDF specific field according to the AMDT-P audio specification.
    -   `void unpackAmdtpAudioFdfSpecific(uint8_t fdfSpecific, uint8_t& outEvent, uint8_t& outChannels)`: Unpacks the event number and channel count from the FDF specific field.

-   **Data Block Size (DBS) Calculation:**
    -   `uint8_t calculateDBS(const AudioStreamFormat& format)`:
        -   Calculates the Data Block Size (DBS) in bytes required for AMDT-P based on the provided `AudioStreamFormat`.
        -   DBS = `(bitDepth / 8) * numChannels`. For example, for 24-bit stereo, DBS = (24/8) * 2 = 6 bytes.
        -   Ensures the bit depth is a multiple of 8.

-   **AMDT-P Packet Validation (Conceptual):**
    -   While not explicitly shown as separate functions, the logic within `AmdtpReceiver` and `AmdtpTransmitter` would use these helper functions to validate incoming packets (e.g., check FDF for expected type, verify DBC sequence) and ensure outgoing packets are correctly formed.

**Overall Role:**
The `AmdtpHelpers` namespace provides a crucial set of low-level utilities for working with the bit-level details of CIP headers and AMDT-P packets. These functions are heavily used by:
-   `AmdtpReceiver`: To parse incoming CIP headers, extract SYT timestamps, DBC values, FDF fields, and then correctly interpret the AMDT-P payload.
-   `AmdtpTransmitter`: To construct valid CIP headers for outgoing AMDT-P packets, embedding correct SYT, DBC, FDF, DBS, and SPH values.
-   Other components in the `FWA::Isoch` module that might need to inspect or manipulate AMDT-P packet data at a low level.
By centralizing these operations, `AmdtpHelpers` ensures consistency and correctness in implementing the IEC 61883-6 standard.
