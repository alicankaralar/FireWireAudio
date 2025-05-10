#ifndef FIREWIRE_SCANNER_HPP
#define FIREWIRE_SCANNER_HPP

#include <atomic>
#include <cstdint>
#include <map>
#include <setjmp.h>
#include <string>
#include <vector>

#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <memory>          // For std::shared_ptr
#include <spdlog/spdlog.h> // For spdlog::logger

#include "FWA/dice/DiceAbsoluteAddresses.hpp" // Include for enums/constants

namespace FWA::SCANNER {
// Forward declaration of DeviceEndianness enum class
enum class DeviceEndianness;

// Define FireWireDevice struct first
struct FireWireDevice {
  UInt64 guid;
  std::string name;
  std::string vendor;
  DiceChipType diceChipType = DiceChipType::Unknown;
  std::map<UInt64, UInt32>
      diceRegisters; // Stores raw values as read from device

  // Device endianness (detected during scan)
  DeviceEndianness deviceEndianness;

  // Config ROM vendor keys (used for endianness detection)
  std::map<UInt32, UInt64> configRomVendorKeys;

  // TX/RX stream configuration
  UInt32 txStreamCount = 0;
  UInt32 rxStreamCount = 0;

  // Configuration based on sample rate
  DiceConfig currentConfig = DiceConfig::Unknown;
  // Configuration based on sample rate

  // New fields for detailed device information
  FWA::DICE::ClockSourceEnum syncSource = FWA::DICE::ClockSourceEnum::Unknown;
  RouterFrameSyncMode routerFsMode = RouterFrameSyncMode::Unknown;
  bool aesLocked = false;
  UInt8 mixerRxChannels = 0;
  UInt8 avsRxChannelId = 0;
  UInt8 avsRxDataBlockSize = 0;
  UInt8 avsTxDataBlockSize = 0;
  AvsSystemMode avsTxSystemMode = AvsSystemMode::Unknown;

  // Determined DICE base addresses
  UInt64 diceGlobalBase = DICE_INVALID_OFFSET; // Use constexpr variable
  UInt64 diceTxBase = DICE_INVALID_OFFSET;     // Use constexpr variable
  UInt64 diceRxBase = DICE_INVALID_OFFSET;     // Use constexpr variable
  std::string diceBaseDeterminationMethod = "Unknown";

  // Channel names base address (dynamically discovered)
  UInt64 channelNamesBaseAddr = DICE_INVALID_OFFSET;
};

// Forward declare the main class
class FireWireScanner;

// Helper function declarations (implementations in separate .cpp files)
// Now they can use the fully defined FireWireDevice struct
FireWireDevice getDeviceInfo(io_service_t device); // Changed return type
UInt64 parseConfigRom(IOFireWireDeviceInterface **deviceInterface,
                      io_service_t service, UInt32 targetKey,
                      UInt32 generation);
void readDiceRegisters(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    FireWireDevice &device); // Removed FireWireScanner:: prefix
void printDeviceInfo(
    const FireWireDevice &device); // Removed FireWireScanner:: prefix
void exploreDiceMemoryLayout(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    FireWireDevice &device, UInt32 generation,
    UInt64 baseAddr = DICE_REGISTER_BASE); // Use constexpr variable
void exploreChannelNamesArea(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    FireWireDevice &device,
    UInt32 generation); // Detailed exploration of channel names
std::string interpretAsASCII(UInt32 value);
// EAP functions are declared in eap_helpers.hpp now
// bool readDiceEAPCapabilities(IOFireWireDeviceInterface **deviceInterface,
// io_service_t service, FireWireDevice &device, UInt32 generation); bool
// readDiceEAPCurrentConfig(IOFireWireDeviceInterface **deviceInterface,
// io_service_t service, FireWireDevice &device, UInt32 generation);
void setDefaultDiceConfig(
    FireWireDevice &device); // Removed FireWireScanner:: prefix
void segfaultHandler(int signal);

// Global variables for signal handling (declared extern here, defined in
// utils.cpp)
extern std::atomic<bool> g_segfaultOccurred;
extern jmp_buf g_jmpBuf;

// Global logger instance (defined in main.cpp)
extern std::shared_ptr<spdlog::logger> logger_;

// Define the main scanner class after helpers are declared
class FireWireScanner {
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
