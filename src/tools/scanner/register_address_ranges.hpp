#ifndef REGISTER_ADDRESS_RANGES_HPP
#define REGISTER_ADDRESS_RANGES_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace FWA::SCANNER {
/**
 * @brief Represents a group of registers within a contiguous address range
 */
struct RegisterAddressRange {
  uint64_t baseAddress; ///< Starting address of the range
  std::vector<std::pair<uint64_t, uint32_t>>
      registers; ///< Registers in this range
};

/**
 * @brief Groups registers by address proximity for more effective string
 * analysis
 *
 * This function takes a map of register addresses to values and groups them
 * into contiguous ranges based on address proximity. Registers within 16 bytes
 * of each other are considered part of the same group.
 *
 * @param registers Map of register addresses to register values
 * @return Vector of RegisterAddressRange structs containing grouped registers
 */
std::vector<RegisterAddressRange>
groupRegistersByAddressRange(const std::map<uint64_t, uint32_t> &registers);

/**
 * @brief Processes each address range to extract coherent strings
 *
 * Analyzes each register group to find both quadlet-level strings
 * (where each register contains a complete ASCII string) and byte-level strings
 * (where ASCII characters are spread across individual bytes in registers).
 *
 * @param ranges Vector of RegisterAddressRange structs to process
 */
void processAddressRangesForStrings(
    const std::vector<RegisterAddressRange> &ranges);

} // namespace FWA::SCANNER

#endif // REGISTER_ADDRESS_RANGES_HPP
