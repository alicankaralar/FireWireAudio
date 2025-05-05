#ifndef CONFIG_ROM_HPP
#define CONFIG_ROM_HPP

#include <cstdint>
#include <map> // Added for std::map

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER
// Define missing Config ROM keys (previously in FWStandardHeaders.h)
// Source: IEEE 1394-1995 / IEEE 1212r
#define kConfigDirectoryKey_Unit 0x17
#define kConfigDirectoryKey_Unit_Spec_ID                                       \
  0x1C // Note: These seem incorrect based on 1212r, should be 0x03, 0x0C? Check
       // later.
#define kConfigDirectoryKey_Unit_SW_Version 0x1D
#define kConfigDirectoryKey_Model 0x07
#define kConfigDirectoryKey_Unit_Dependent 0xC1
// Note: 0xD1 is vendor-specific, handled elsewhere if needed.
{

/**
 * @brief Parses the device's Configuration ROM to find potential base addresses
 * indicated by vendor-specific keys.
 *
 * Reads the Config ROM starting from the standard base (0xFFFFF0000400),
 * follows the root directory pointer, searches the root directory for the Unit
 * Directory (key 0x17), then iterates through the Unit Directory, looking for
 * vendor-specific keys (0xD0 - 0xFF). For each vendor key found, it calculates
 * and stores the absolute address pointed to by the key's value.
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface.
 * @param service The io_service_t representing the FireWire nub.
 * @param generation The current bus generation number (obtained via
 * GetBusGeneration).
 * @return A map where keys are the vendor-specific Config ROM keys (0xD0-0xFF)
 * found, and values are the corresponding absolute 64-bit addresses they point
 * to. Returns an empty map if no vendor keys are found or if errors occur.
 */
std::map<uint32_t, uint64_t>
parseConfigRomVendorKeys(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, UInt32 generation);

} // namespace FWA::SCANNER

#endif // CONFIG_ROM_HPP
