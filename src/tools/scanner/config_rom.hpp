#ifndef CONFIG_ROM_HPP
#define CONFIG_ROM_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER
// Define missing Config ROM keys (previously in FWStandardHeaders.h)
// Source: IEEE 1394-1995 / IEEE 1212r
#define kConfigDirectoryKey_Unit 0x17
#define kConfigDirectoryKey_Unit_Spec_ID 0x1C
#define kConfigDirectoryKey_Unit_SW_Version 0x1D
#define kConfigDirectoryKey_Model 0x07
#define kConfigDirectoryKey_Unit_Dependent 0xC1
// Note: 0xD1 is vendor-specific, handled elsewhere if needed.
{

	/**
	 * @brief Parses the device's Configuration ROM to find the base address of a directory specified by a target key.
	 *
	 * Reads the Config ROM starting from the standard base (0xFFFFF0000400), follows the
	 * root directory pointer, searches the root directory for the Unit Directory (key 0x17),
	 * then searches the Unit Directory for the specified targetKey (e.g., 0xC1 for Unit_Dependent,
	 * or vendor-specific keys like 0xD1).
	 *
	 * @param deviceInterface Pointer-to-pointer to an opened IOFireWireDeviceInterface.
	 * @param service The io_service_t representing the FireWire nub.
	 * @param targetKey The Config ROM directory key to search for within the Unit Directory (e.g., kConfigDirectoryKey_Unit_Dependent).
	 * @param generation The current bus generation number (obtained via GetBusGeneration).
	 * @return The absolute 64-bit base address of the directory associated with the targetKey if found,
	 *         otherwise returns FWA::DICE::DICE_INVALID_OFFSET (typically 0).
	 */
	uint64_t parseConfigRom(IOFireWireDeviceInterface **deviceInterface,
													io_service_t service,
													uint32_t targetKey,
													UInt32 generation);

} // namespace FWA::SCANNER

#endif // CONFIG_ROM_HPP