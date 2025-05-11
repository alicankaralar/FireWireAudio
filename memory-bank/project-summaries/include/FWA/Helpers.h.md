# Summary for include/FWA/Helpers.h

This C++ header file defines the `FWA::Helpers` namespace, which provides a collection of general-purpose utility functions. These functions are designed to simplify common tasks encountered throughout the FWA (FireWire Audio) library, such as data conversion, IOKit property access, string manipulation, and byte operations.

**Key Declarations and Components:**

-   **Namespace `FWA::Helpers`:** All utility functions are declared within this namespace.

-   **Includes:**
    -   `<IOKit/IOKitLib.h>`: For IOKit types like `io_service_t`.
    -   `<CoreFoundation/CoreFoundation.h>`: For CoreFoundation types like `CFStringRef`, `CFNumberRef`, `CFDataRef`.
    -   `<string>`: For `std::string`.
    -   `<vector>`: For `std::vector`.
    -   `<optional>`: For `std::optional`.
    -   `<sstream>`: For `std::ostringstream`.
    -   `<iomanip>`: For stream manipulators like `std::hex` and `std::setw`.
    -   `<algorithm>`: For algorithms like `std::find_if_not`.
    -   `<cctype>`: For character classification functions like `std::isspace`.

-   **Utility Functions Declared:**

    -   **GUID Conversion:**
        -   `std::string guidToString(uint64_t guid);`: Converts a 64-bit Global Unique Identifier into its hexadecimal string representation (e.g., "0x0011223344556677").

    -   **IOKit Object Property Access:**
        -   These functions simplify retrieving properties from IOKit `io_service_t` objects.
        -   `std::optional<std::string> getStringProperty(io_service_t service, CFStringRef key);`: Retrieves a string property.
        -   `std::optional<uint32_t> getUInt32Property(io_service_t service, CFStringRef key);`: Retrieves a 32-bit unsigned integer property.
        -   `std::optional<uint64_t> getUInt64Property(io_service_t service, CFStringRef key);`: Retrieves a 64-bit unsigned integer property.

    -   **Byte Swapping (Endian Conversion):**
        -   `template<typename T> T swapBytes(T value);`: A template function to reverse the byte order of an integral type `T`. This is crucial for handling data from FireWire devices, which are typically big-endian, on little-endian host systems (like x86 Macs). The implementation likely uses compiler intrinsics (`__builtin_bswap16`, `__builtin_bswap32`, `__builtin_bswap64`) for efficiency.

    -   **Data Extraction from Byte Vectors:**
        -   These functions extract multi-byte integer values from a `std::vector<uint8_t>` (raw data buffer), with an option for byte swapping.
        -   `uint16_t extractUInt16(const std::vector<uint8_t>& data, size_t offset, bool swap = false);`
        -   `uint32_t extractUInt32(const std::vector<uint8_t>& data, size_t offset, bool swap = false);`
        -   `uint64_t extractUInt64(const std::vector<uint8_t>& data, size_t offset, bool swap = false);`

    -   **String Manipulation:**
        -   `std::string trim(const std::string& str);`: Removes leading and trailing whitespace characters from a string.
        -   `std::vector<std::string> split(const std::string& s, char delimiter);`: Splits a string `s` into a vector of substrings based on the given `delimiter` character.

    -   **Hexadecimal Conversion:**
        -   `std::string bytesToHexString(const std::vector<uint8_t>& bytes);`: Converts a vector of bytes into a string of hexadecimal characters (e.g., for logging or display of raw descriptor data).

**Overall Role:**
The `FWA::Helpers` namespace provides a centralized collection of reusable utility functions that address common, low-level tasks within the FWA library. By encapsulating these operations:
-   It reduces code duplication across different modules.
-   It improves code readability by providing clearly named functions for specific tasks.
-   It centralizes potentially complex or error-prone operations (like IOKit property access with CF type management, or byte swapping) into well-tested utilities.
These helpers are likely used by many classes in the FWA library, including descriptor parsers, device management classes, and data handling components.
