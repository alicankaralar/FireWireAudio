#ifndef STRING_EXTRACTION_HPP
#define STRING_EXTRACTION_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "endianness_helpers.hpp" // For DeviceEndianness enum

namespace FWA::SCANNER {
/**
 * @brief Represents a string found in memory with its address and type
 */
struct StringMatch {
  std::string text; ///< The extracted string text
  uint64_t address; ///< The memory address where the string was found
  bool isByteLevel; ///< Whether the string was found at the byte level or
                    ///< quadlet level
};

/**
 * @brief Extracts strings from a map of register values
 *
 * This function analyzes register values to find both quadlet-level strings
 * (where each register contains a complete ASCII string) and byte-level strings
 * (where ASCII characters are spread across individual bytes in registers).
 *
 * @param registers Map of register addresses to register values
 * @param endianness The detected endianness of the device
 * @return Vector of StringMatch objects containing the extracted strings
 */
std::vector<StringMatch> extractStringsFromMemory(
    const std::map<uint64_t, uint32_t> &registers,
    DeviceEndianness endianness = DeviceEndianness::UNKNOWN_ENDIAN);

/**
 * @brief Extracts printable strings from a byte array with improved termination
 * detection
 *
 * This function analyzes a byte array to find printable ASCII strings, with
 * enhanced termination detection and optional structure-aware parsing. It can
 * handle both null-terminated strings and strings terminated by non-printable
 * characters.
 *
 * @param bytes Pointer to the byte array
 * @param length Length of the byte array in bytes
 * @param structureSize Optional size of each string structure (e.g., 16, 32, 64
 * bytes)
 * @param minLength Minimum length for a string to be considered valid (default:
 * 3)
 * @return Vector of extracted strings
 */
std::vector<std::string>
extractPrintableStringsFromBytes(const uint8_t *bytes, size_t length,
                                 size_t structureSize = 0,
                                 size_t minLength = 3);

} // namespace FWA::SCANNER

#endif // STRING_EXTRACTION_HPP
