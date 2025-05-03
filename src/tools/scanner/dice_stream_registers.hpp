#ifndef DICE_STREAM_REGISTERS_HPP
#define DICE_STREAM_REGISTERS_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "scanner.hpp" // For FireWireDevice definition

namespace FWA::SCANNER
{

	/**
	 * @brief Helper to read TX stream parameters using progressive discovery
	 *
	 * Uses a progressive discovery approach to detect valid TX streams:
	 * 1. Starts with reported stream count (if available) or explores up to a reasonable limit
	 * 2. Probes each stream with critical registers first to validate its existence
	 * 3. Reads all registers for valid streams
	 * 4. Stops after multiple consecutive failures
	 * 5. Updates device.txStreamCount based on actual discovered streams
	 *
	 * @param deviceInterface Pointer to FireWire device interface
	 * @param service FireWire service
	 * @param device Device information to update
	 * @param discoveredDiceBase Base address for DICE registers
	 * @param generation FireWire bus generation
	 * @param txStreamSizeQuadlets Size of each TX stream in quadlets
	 */
	void readDiceTxStreamRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation,
																 uint32_t txStreamSizeQuadlets);

	/**
	 * @brief Helper to read RX stream parameters using progressive discovery
	 *
	 * Uses a progressive discovery approach to detect valid RX streams:
	 * 1. Starts with reported stream count (if available) or explores up to a reasonable limit
	 * 2. Probes each stream with critical registers first to validate its existence
	 * 3. Reads all registers for valid streams
	 * 4. Stops after multiple consecutive failures
	 * 5. Updates device.rxStreamCount based on actual discovered streams
	 *
	 * @param deviceInterface Pointer to FireWire device interface
	 * @param service FireWire service
	 * @param device Device information to update
	 * @param discoveredDiceBase Base address for DICE registers
	 * @param generation FireWire bus generation
	 * @param rxStreamSizeQuadlets Size of each RX stream in quadlets
	 */
	void readDiceRxStreamRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
																 FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation,
																 uint32_t rxStreamSizeQuadlets);

} // namespace FWA::SCANNER

#endif // DICE_STREAM_REGISTERS_HPP