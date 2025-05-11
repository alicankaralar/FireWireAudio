# Summary for src/FWA/Helpers.cpp

This C++ file implements a collection of general-purpose utility functions within the `FWA::Helpers` namespace. These functions are used by various other classes within the FWA library to perform common tasks related to data conversion, IOKit property access, and string manipulation.

**Key Functionalities:**

-   **GUID Conversion:**
    -   `std::string guidToString(uint64_t guid)`:
        -   Converts a 64-bit Global Unique Identifier (GUID) into its hexadecimal string representation (e.g., "0x0011223344556677"). Uses `std::ostringstream` for formatting.

-   **IOKit Object Property Access:**
    -   These functions simplify retrieving properties from IOKit `io_service_t` objects. They handle the CoreFoundation (CF) types and memory management.
    -   `std::optional<std::string> getStringProperty(io_service_t service, CFStringRef key)`:
        -   Retrieves a string property associated with the given `key` from the `service` object's registry properties.
        -   Uses `IORegistryEntryCreateCFProperty` to get the `CFStringRef`.
        -   Converts the `CFStringRef` to `std::string`.
        -   Returns `std::optional<std::string>` to handle cases where the property might not exist.
    -   `std::optional<uint32_t> getUInt32Property(io_service_t service, CFStringRef key)`:
        -   Retrieves a 32-bit unsigned integer property. Gets a `CFNumberRef` and converts it to `uint32_t`.
    -   `std::optional<uint64_t> getUInt64Property(io_service_t service, CFStringRef key)`:
        -   Retrieves a 64-bit unsigned integer property.

-   **Byte Swapping (Endian Conversion):**
    -   `template<typename T> T swapBytes(T value)`:
        -   A template function to reverse the byte order of an integral type `T`.
        -   Uses `__builtin_bswap16`, `__builtin_bswap32`, `__builtin_bswap64` (compiler intrinsics for efficient byte swapping) based on the size of `T`.
        -   This is crucial for handling data from FireWire devices, which may use a different endianness than the host system.
    -   (Specific instantiations or usage for `uint16_t`, `uint32_t`, `uint64_t` are implied by its use in data extraction).

-   **Data Extraction from Byte Vectors:**
    -   These functions extract multi-byte integer values from a `std::vector<uint8_t>` (raw data buffer), typically read from a device descriptor or AV/C command response.
    -   `uint16_t extractUInt16(const std::vector<uint8_t>& data, size_t offset, bool swap = false)`:
        -   Extracts a `uint16_t` starting at `offset`.
        -   Optionally performs byte swapping if `swap` is true.
    -   `uint32_t extractUInt32(const std::vector<uint8_t>& data, size_t offset, bool swap = false)`: Similar for `uint32_t`.
    -   `uint64_t extractUInt64(const std::vector<uint8_t>& data, size_t offset, bool swap = false)`: Similar for `uint64_t`.

-   **String Manipulation:**
    -   `std::string trim(const std::string& str)`:
        -   Removes leading and trailing whitespace characters (spaces, tabs, newlines, etc.) from a string.
    -   `std::vector<std::string> split(const std::string& s, char delimiter)`:
        -   Splits a string `s` into a vector of substrings based on the given `delimiter` character.

**Overall Role:**
The `FWA::Helpers` namespace provides a suite of common, low-level utility functions that abstract away repetitive or boilerplate code. These helpers are used extensively throughout the FWA library for tasks such as:
-   Interacting with IOKit to get device properties.
-   Parsing raw byte data received from devices (e.g., from descriptors or command responses), including handling potential endianness differences.
-   Formatting data for display or logging (e.g., GUIDs).
-   Cleaning up string data.
They contribute to code clarity, reduce redundancy, and centralize common operations.
