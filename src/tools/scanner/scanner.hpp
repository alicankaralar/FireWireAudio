#ifndef FIREWIRE_SCANNER_HPP
#define FIREWIRE_SCANNER_HPP

#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <atomic>
#include <setjmp.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>

#include "FWA/DiceDefines.hpp" // Include relative to project root

namespace FWA::SCANNER
{
	// Define FireWireDevice struct first
	struct FireWireDevice
	{
		uint64_t guid;
		std::string name;
		std::string vendor;
		FWA::DICE::DiceChipType diceChipType = FWA::DICE::DiceChipType::Unknown;
		std::map<uint64_t, uint32_t> diceRegisters; // Stores raw Big Endian values

		// TX/RX stream configuration
		uint32_t txStreamCount = 0;
		uint32_t rxStreamCount = 0;

		// Configuration based on sample rate
		FWA::DICE::DiceConfig currentConfig = FWA::DICE::DiceConfig::Unknown;
		// Configuration based on sample rate

		// New fields for detailed device information
		FWA::DICE::ClockSource syncSource = FWA::DICE::ClockSource::Unknown;
		FWA::DICE::RouterFrameSyncMode routerFsMode = FWA::DICE::RouterFrameSyncMode::Unknown;
		bool aesLocked = false;
		uint8_t mixerRxChannels = 0;
		uint8_t avsRxChannelId = 0;
		uint8_t avsRxDataBlockSize = 0;
		uint8_t avsTxDataBlockSize = 0;
		FWA::DICE::AvsSystemMode avsTxSystemMode = FWA::DICE::AvsSystemMode::Unknown;
	};

	// Forward declare the main class
	class FireWireScanner;

	// Helper function declarations (implementations in separate .cpp files)
	// Now they can use the fully defined FireWireDevice struct
	FireWireDevice getDeviceInfo(io_service_t device); // Changed return type
	IOReturn safeReadQuadlet(IOFireWireDeviceInterface **deviceInterface, io_service_t service, uint64_t absoluteAddr, UInt32 &value, UInt32 generation);
	IOReturn readQuadlet(IOFireWireDeviceInterface **deviceInterface, io_service_t service, uint64_t absoluteAddr, UInt32 &value, UInt32 generation);
	uint64_t parseConfigRom(IOFireWireDeviceInterface **deviceInterface, io_service_t service, uint32_t targetKey, UInt32 generation);
	void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device);																																	// Removed FireWireScanner:: prefix
	void printDeviceInfo(const FireWireDevice &device);																																																																	// Removed FireWireScanner:: prefix
	void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device, UInt32 generation, uint64_t baseAddr = DICE_REGISTER_BASE); // Removed FireWireScanner:: prefix
	void exploreChannelNamesArea(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device, UInt32 generation);																					// Detailed exploration of channel names
	std::string interpretAsASCII(UInt32 value);
	// EAP functions are declared in eap_helpers.hpp now
	// bool readDiceEAPCapabilities(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device, UInt32 generation);
	// bool readDiceEAPCurrentConfig(IOFireWireDeviceInterface **deviceInterface, io_service_t service, FireWireDevice &device, UInt32 generation);
	void setDefaultDiceConfig(FireWireDevice &device); // Removed FireWireScanner:: prefix
	void segfaultHandler(int signal);

	// Global variables for signal handling (declared extern here, defined in utils.cpp)
	extern std::atomic<bool> g_segfaultOccurred;
	extern jmp_buf g_jmpBuf;

	// Define the main scanner class after helpers are declared
	class FireWireScanner
	{
	public:
		FireWireScanner();
		~FireWireScanner();

		std::vector<FireWireDevice> scanDevices();

	private:
		mach_port_t masterPort_;

		// No private helpers needed here anymore
	};

} // namespace FWA::SCANNER

#endif // FIREWIRE_SCANNER_HPP