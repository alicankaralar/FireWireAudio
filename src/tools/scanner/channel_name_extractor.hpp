#ifndef CHANNEL_NAME_EXTRACTOR_HPP
#define CHANNEL_NAME_EXTRACTOR_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "endianness_helpers.hpp" // For DeviceEndianness enum
#include "scanner.hpp"            // For FireWireDevice struct
#include "string_extraction.hpp"  // For StringMatch struct

namespace FWA::SCANNER {
/**
 * @brief Collects registers in the channel names area
 *
 * Scans a range of memory around the channel names base address and
 * collects all readable registers.
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param channelNamesBaseAddr The base address for channel names
 * @param generation The FireWire bus generation
 * @param preRange Number of quadlets to scan before the base address
 * @param postRange Number of quadlets to scan after the base address
 * @return Map of register addresses to register values
 */
std::map<UInt64, UInt32>
collectChannelRegisters(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, UInt64 channelNamesBaseAddr,
                        UInt32 generation, int preRange = 256,
                        int postRange = 1024);

/**
 * @brief Processes registers to extract a byte array
 *
 * Converts a map of register values to a byte array for structure-aware
 * parsing. Handles endianness correctly.
 *
 * @param registers Map of register addresses to register values
 * @param deviceEndianness The detected endianness of the device
 * @return Vector of bytes representing the register contents
 */
std::vector<UInt8>
processChannelRegisters(const std::map<UInt64, UInt32> &registers,
                        DeviceEndianness deviceEndianness);

/**
 * @brief Combines results from different string extraction methods
 *
 * Combines strings extracted using both quadlet-level and byte-level methods,
 * avoiding duplicates.
 *
 * @param quadletStrings Strings extracted at the quadlet level
 * @param byteStrings Strings extracted at the byte level
 * @return Combined vector of unique StringMatch objects
 */
std::vector<StringMatch>
combineStringResults(const std::vector<StringMatch> &quadletStrings,
                     const std::vector<std::string> &byteStrings);

/**
 * @brief Extracts channel names from device memory
 *
 * Collects registers, processes them, and extracts strings using both
 * quadlet-level and byte-level methods.
 *
 * @param deviceInterface The FireWire device interface
 * @param service The IO service
 * @param device The FireWireDevice struct to store results
 * @param generation The FireWire bus generation
 * @return Vector of StringMatch objects containing the extracted strings
 */
std::vector<StringMatch>
extractChannelNames(IOFireWireDeviceInterface **deviceInterface,
                    io_service_t service, FireWireDevice &device,
                    UInt32 generation);

} // namespace FWA::SCANNER

#endif // CHANNEL_NAME_EXTRACTOR_HPP
