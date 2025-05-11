# Summary for include/FWA/Enums.hpp

This C++ header file defines several enumerations (`enum class`) within the `FWA` namespace. These enumerations are used throughout the FireWire Audio (FWA) library to represent various states, types, directions, and codes in a type-safe and readable manner.

**Key Enumerations Defined:**

1.  **`FWA::SubunitType : uint8_t`**:
    -   Represents the type of an AV/C subunit.
    -   **Values:**
        -   `Unit = 0x00`
        -   `Audio = 0x01`
        -   `Music = 0x02` (typically for MIDI)
        -   `VendorUnique = 0x03`
        -   `Unknown = 0xFF` (or some other distinct value)

2.  **`FWA::DescriptorType : uint8_t`**:
    -   Represents the type of an AV/C descriptor.
    -   **Values (examples, actual values depend on AV/C spec):**
        -   `SubunitIdentifier = 0x00`
        -   `UnitInfo = 0x01` (or a specific value for unit information)
        -   `AudioSubunitInfo = 0x02` (or a specific value for audio subunit info)
        -   `MusicSubunitInfo = 0x03` (or a specific value for music subunit info)
        -   `AudioPlugInfo = 0x10` (example value)
        -   `MIDIPlugInfo = 0x11` (example value)
        -   `StreamFormatInfo = 0x20` (example value)
        -   `InfoBlock = 0x0C`
        -   `VendorUniqueDescriptor = 0xFF` (or a range)
        -   `UnknownDescriptor`

3.  **`FWA::PlugDirection : uint8_t`**:
    -   Indicates the direction of an audio or MIDI plug.
    -   **Values:**
        -   `Input = 0x00`
        -   `Output = 0x01`
        -   `UnknownDirection = 0xFF`

4.  **`FWA::StreamDirection : uint8_t`**:
    -   Indicates the direction of an isochronous audio stream.
    -   **Values:**
        -   `InputStream = 0x00` (data flowing from device to host)
        -   `OutputStream = 0x01` (data flowing from host to device)

5.  **`FWA::AVCCResponseType : uint8_t`**:
    -   Represents the response codes for AV/C commands, as defined in the AV/C specifications.
    -   **Values (standard AV/C responses):**
        -   `NOT_IMPLEMENTED = 0x08`
        -   `ACCEPTED = 0x09`
        -   `REJECTED = 0x0A`
        -   `IN_TRANSITION = 0x0B`
        -   `STABLE = 0x0C` (often used for STATUS commands)
        -   `CHANGED = 0x0D` (often used for NOTIFY responses)
        -   `INTERIM = 0x0F` (often used for CONTROL commands awaiting completion)

6.  **`FWA::SampleFrequencyCode : uint8_t` (SFC):**
    -   Represents standard sample frequency codes used in AV/C Stream Format Descriptors.
    -   **Values (mapping to actual rates):**
        -   `SFC_32000Hz = 0x00`
        -   `SFC_44100Hz = 0x01`
        -   `SFC_48000Hz = 0x02`
        -   `SFC_88200Hz = 0x03`
        -   `SFC_96000Hz = 0x04`
        -   `SFC_176400Hz = 0x05`
        -   `SFC_192000Hz = 0x06`
        -   (Other codes for different rates or variable rates might be included).

7.  **`FWA::BitDepthCode : uint8_t`**:
    -   Represents common bit depths for audio samples, often used in AV/C Stream Format Descriptors.
    -   **Values (mapping to actual bit depths and types):**
        -   `BD_16BIT_LINEAR = 0x01` (example)
        -   `BD_20BIT_LINEAR = 0x02`
        -   `BD_24BIT_LINEAR = 0x03`
        -   `BD_32BIT_FLOAT = 0x04` (or a specific code for float)
        -   (Other codes for different bit depths or compressed formats).

**Overall Role:**
This header file is crucial for improving the type safety, readability, and maintainability of the FWA library. By defining these strongly-typed enumerations:
-   It prevents the use of "magic numbers" (raw integer values) for representing these concepts.
-   It allows the C++ compiler to perform type checking, catching potential errors at compile time.
-   It makes the code self-documenting, as the enum names clearly indicate the meaning of the values.
These enums are used extensively in class interfaces, data structures (like `AudioStreamFormat`, `DescriptorSpecifier`), and internal logic throughout the FWA library.
