#ifndef EAP_HELPERS_HPP
#define EAP_HELPERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER
{

	/**
	 * @brief Reads DICE EAP (Extended Audio Protocol) capability registers.
	 *
	 * Reads the EAP capability space offset, then reads General, Router, and Mixer capability registers.
	 * Updates device.diceChipType based on the General capability register.
	 * Stores raw capability register values in device.diceRegisters.
	 * Assumes the DICE base address for reading the EAP offset is FWA::DICE::DICE_REGISTER_BASE.
	 *
	 * @param deviceInterface Pointer-to-pointer to an opened IOFireWireDeviceInterface.
	 * @param service The io_service_t representing the FireWire nub.
	 * @param device Reference to the FireWireDevice struct to populate.
	 * @param generation The current bus generation number.
	 * @return true if EAP capability space offset and General capability were read successfully, false otherwise.
	 */
	bool readDiceEAPCapabilities(IOFireWireDeviceInterface **deviceInterface,
															 io_service_t service,
															 FireWireDevice &device,
															 UInt32 generation);

	/**
	 * @brief Reads DICE EAP current configuration registers (e.g., stream counts).
	 *
	 * Reads the EAP current configuration space offset, then reads TX/RX stream counts
	 * from the appropriate configuration block (currently hardcoded to LOW).
	 * Updates device.txStreamCount and device.rxStreamCount.
	 * Stores raw count register values in device.diceRegisters.
	 * Assumes the DICE base address for reading the EAP offset is FWA::DICE::DICE_REGISTER_BASE.
	 *
	 * @param deviceInterface Pointer-to-pointer to an opened IOFireWireDeviceInterface.
	 * @param service The io_service_t representing the FireWire nub.
	 * @param device Reference to the FireWireDevice struct to populate.
	 * @param generation The current bus generation number.
	 * @return true if EAP current config space offset and TX count were read successfully, false otherwise.
	 */
	bool readDiceEAPCurrentConfig(IOFireWireDeviceInterface **deviceInterface,
																io_service_t service,
																FireWireDevice &device,
																UInt32 generation);

} // namespace FWA::SCANNER

#endif // EAP_HELPERS_HPP