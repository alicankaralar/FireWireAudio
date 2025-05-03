#ifndef DICE_HELPERS_HPP
#define DICE_HELPERS_HPP

#include <cstdint>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice struct

namespace FWA::SCANNER
{

	/**
	 * @brief Reads DICE-specific registers from the device.
	 *
	 * Attempts to discover the DICE register base address via Config ROM parsing.
	 * Reads standard DICE registers (Version, Offsets, Counts, Sizes, etc.) relative to the discovered base.
	 * Attempts to read EAP capabilities and configuration if available.
	 * Reads per-stream parameters based on discovered/default stream counts.
	 * Stores successfully read raw (Big Endian) register values in device.diceRegisters.
	 * Updates device.txStreamCount, device.rxStreamCount, and device.diceChipType based on reads or defaults.
	 *
	 * @param deviceInterface Pointer-to-pointer to an opened IOFireWireDeviceInterface.
	 * @param service The io_service_t representing the FireWire nub.
	 * @param device Reference to the FireWireDevice struct to populate.
	 */
	void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
												 io_service_t service,
												 FireWireDevice &device);

	/**
	 * @brief Sets default TX/RX stream counts based on the detected chip type.
	 *
	 * Used as a fallback if EAP or standard register reads fail.
	 *
	 * @param device Reference to the FireWireDevice struct to update.
	 */
	void setDefaultDiceConfig(FireWireDevice &device);

} // namespace FWA::SCANNER

#endif // DICE_HELPERS_HPP