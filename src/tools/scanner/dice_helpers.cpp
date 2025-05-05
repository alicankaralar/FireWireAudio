#include "dice_helpers.hpp"
#include "config_rom.hpp" // For parseConfigRom
#include "dice_stream_registers.hpp" // For readDiceTxStreamRegisters, readDiceRxStreamRegisters
#include "eap_helpers.hpp" // Include EAP functions
#include "io_helpers.hpp"  // For safeReadQuadlet, interpretAsASCII
#include "log.hpp"         // For logging
#include "scanner.hpp"     // For FireWireDevice, DiceDefines.hpp constants
#include "utils_explore_channels.hpp" // For exploreChannelNamesArea
#include "utils_explore_general.hpp" // For exploreDiceMemoryLayout, extractCoherentRegisterStrings
#include "utils_memory.hpp" // For findVendorKeyAddress (if not in config_rom.hpp)
#include "utils_string.hpp" // For extractStringsFromMemory, discoverChannelNamesAddress, validateChannelNumbers

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <map>
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireFamilyCommon.h> // For kConfigDirectoryKey_Unit_Dependent (FWStandardHeaders.h is deprecated/moved)
#include <IOKit/firewire/IOFireWireLib.h>

// Define default offsets if not already globally available
#ifndef DEFAULT_DICE_TX_OFFSET
#define DEFAULT_DICE_TX_OFFSET 0x400 // Default offset in BYTES from global base
#endif
#ifndef DEFAULT_DICE_RX_OFFSET
#define DEFAULT_DICE_RX_OFFSET 0x800 // Default offset in BYTES from global base
#endif

namespace FWA::SCANNER {

// --- Static Helper Functions (Internal to this file) ---

namespace { // Anonymous namespace for internal linkage

// Helper to read common global DICE registers
static void readDiceGlobalRegistersInternal(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation) {
  std::map<uint64_t, std::string> registersToRead = {
      // Base Parameter Space Offsets/Sizes (read these first)
      {DICE_REGISTER_GLOBAL_PAR_SPACE_OFF,
       "Global Parameter Space Offset (Quadlets)"},
      {DICE_REGISTER_GLOBAL_PAR_SPACE_SZ,
       "Global Parameter Space Size (Quadlets)"},
      {DICE_REGISTER_TX_PAR_SPACE_OFF, "TX Parameter Space Offset (Quadlets)"},
      {DICE_REGISTER_TX_PAR_SPACE_SZ, "TX Parameter Space Size (Quadlets)"},
      {DICE_REGISTER_RX_PAR_SPACE_OFF, "RX Parameter Space Offset (Quadlets)"},
      {DICE_REGISTER_RX_PAR_SPACE_SZ, "RX Parameter Space Size (Quadlets)"},
      // Global Registers (offsets relative to DISCOVERED base)
      {DICE_REGISTER_GLOBAL_OWNER, "Owner"},
      {DICE_REGISTER_GLOBAL_NOTIFICATION, "Notification"},
      {DICE_REGISTER_GLOBAL_NICK_NAME,
       "Nick Name Base"}, // Note: This is base, actual name needs block read
      {DICE_REGISTER_GLOBAL_CLOCK_SELECT, "Clock Select"},
      {DICE_REGISTER_GLOBAL_ENABLE, "Enable"},
      {DICE_REGISTER_GLOBAL_STATUS, "Status"},
      {DICE_REGISTER_GLOBAL_EXTENDED_STATUS, "Extended Status"},
      {DICE_REGISTER_GLOBAL_SAMPLE_RATE, "Sample Rate"},
      {DICE_REGISTER_GLOBAL_VERSION,
       "Version"}, // Already read, but read again for consistency here? Or
                   // skip? Skip for now.
      {DICE_REGISTER_GLOBAL_CLOCKCAPABILITIES, "Clock Capabilities"},
      {DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES,
       "Clock Source Names Base"}, // Note: Base address
  };

  std::cerr << "Debug [DICE]: Reading global registers relative to base 0x"
            << std::hex << discoveredDiceBase << std::dec << std::endl;

  for (const auto &pair : registersToRead) {
    uint64_t regOffset = pair.first;
    const std::string &regName = pair.second;
    uint64_t fullAddr = discoveredDiceBase + regOffset;
    UInt32 value = 0;

    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, fullAddr, value, generation);
    if (status == kIOReturnSuccess) {
      device.diceRegisters[fullAddr] = value; // Store raw BE value
      std::cerr << "Debug [DICE]: Read success for Global " << regName << " (0x"
                << std::hex << fullAddr << ") -> 0x"
                << CFSwapInt32LittleToHost(value) << std::dec << std::endl;
    } else {
      std::cerr << "Warning [DICE]: Read failed for Global " << regName
                << " (0x" << std::hex << fullAddr << ") (status: " << status
                << ")" << std::dec << std::endl;
    }
  }
}

} // Anonymous namespace

// --- Targeted Register Reading Functions ---

static void readGpcsrRegisters(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, FireWireDevice &device,
                               uint64_t discoveredDiceBase, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading GPCSR registers..." << std::endl;
  uint64_t gpcsrBase = DICE_REGISTER_GPCSR_BASE;

  // Read GPCSR_CHIP_ID
  uint64_t chipIdAddr = gpcsrBase + DICE_REGISTER_GPCSR_CHIP_ID;
  UInt32 chipIdValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, chipIdAddr, chipIdValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[chipIdAddr] = chipIdValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(chipIdValue);
    uint32_t chipType = (hostValue & DICE_GPCSR_CHIP_ID_CHIP_TYPE_MASK) >>
                        DICE_GPCSR_CHIP_ID_CHIP_TYPE_SHIFT;
    // uint32_t chipId = (hostValue & DICE_GPCSR_CHIP_ID_CHIP_ID_MASK) >>
    // DICE_GPCSR_CHIP_ID_CHIP_ID_SHIFT; // Not currently stored

    switch (chipType) {
    case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceII):
      device.diceChipType = FWA::DICE::DiceChipType::DiceII;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE II" << std::endl;
      break;
    case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceMini):
      device.diceChipType = FWA::DICE::DiceChipType::DiceMini;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE Mini" << std::endl;
      break;
    case static_cast<uint32_t>(FWA::DICE::DiceChipType::DiceJr):
      device.diceChipType = FWA::DICE::DiceChipType::DiceJr;
      std::cerr << "Debug [DICE]: Detected Chip Type: DICE Jr" << std::endl;
      break;
    default:
      device.diceChipType = FWA::DICE::DiceChipType::Unknown;
      std::cerr << "Warning [DICE]: Unknown Chip Type: " << chipType
                << std::endl;
      break;
    }
  } else {
    std::cerr << "Warning [DICE]: Failed to read GPCSR_CHIP_ID (0x" << std::hex
              << chipIdAddr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  // Read GPCSR_AUDIO_SELECT
  uint64_t audioSelectAddr = gpcsrBase + DICE_REGISTER_GPCSR_AUDIO_SELECT;
  UInt32 audioSelectValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, audioSelectAddr, audioSelectValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[audioSelectAddr] =
        audioSelectValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(audioSelectValue);
    std::cerr << "Debug [DICE]: Read GPCSR_AUDIO_SELECT (0x" << std::hex
              << audioSelectAddr << "): 0x" << hostValue << std::dec
              << std::endl;
    // TODO: Interpret bitfields and store relevant info in DiscoveredDevice
    // (e.g., flags for enabled interfaces)
  } else {
    std::cerr << "Warning [DICE]: Failed to read GPCSR_AUDIO_SELECT (0x"
              << std::hex << audioSelectAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

static void
readClockControllerRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             uint64_t discoveredDiceBase, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading Clock Controller registers..."
            << std::endl;
  uint64_t clockControllerBase = DICE_REGISTER_CLOCK_CONTROLLER_BASE;

  // Read SYNC_CTRL
  uint64_t syncCtrlAddr =
      clockControllerBase + DICE_REGISTER_CLOCK_CONTROLLER_SYNC_CTRL;
  UInt32 syncCtrlValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, syncCtrlAddr, syncCtrlValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[syncCtrlAddr] = syncCtrlValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(syncCtrlValue);
    uint32_t syncSrc =
        (hostValue & DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_MASK) >>
        DICE_CLOCK_CONTROLLER_SYNC_CTRL_SYNC_SRC_SHIFT;

    switch (syncSrc) {
    case 0:
      device.syncSource = FWA::DICE::ClockSource::AES0;
      break;
    case 1:
      device.syncSource = FWA::DICE::ClockSource::AES1;
      break;
    case 2:
      device.syncSource = FWA::DICE::ClockSource::AES2;
      break;
    case 3:
      device.syncSource = FWA::DICE::ClockSource::AES3;
      break;
    case 4:
      device.syncSource = FWA::DICE::ClockSource::SlaveInputs;
      break;
    case 5:
      device.syncSource = FWA::DICE::ClockSource::HPLL;
      break;
    case 6:
      device.syncSource = FWA::DICE::ClockSource::Internal;
      break;
    default:
      device.syncSource = FWA::DICE::ClockSource::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read SYNC_CTRL (0x" << std::hex << syncCtrlAddr
              << "): Sync Source = " << static_cast<int>(device.syncSource)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_SYNC_CTRL (0x"
              << std::hex << syncCtrlAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }

  // Read DOMAIN_CTRL
  uint64_t domainCtrlAddr =
      clockControllerBase + DICE_REGISTER_CLOCK_CONTROLLER_DOMAIN_CTRL;
  UInt32 domainCtrlValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, domainCtrlAddr, domainCtrlValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[domainCtrlAddr] =
        domainCtrlValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(domainCtrlValue);
    uint32_t rtrFs =
        (hostValue & DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_MASK) >>
        DICE_CLOCK_CONTROLLER_DOMAIN_CTRL_RTR_FS_SHIFT;

    switch (rtrFs) {
    case 0:
      device.routerFsMode = FWA::DICE::RouterFrameSyncMode::BaseRate;
      break;
    case 1:
      device.routerFsMode = FWA::DICE::RouterFrameSyncMode::DoubleRate;
      break;
    case 2:
      device.routerFsMode = FWA::DICE::RouterFrameSyncMode::QuadRate;
      break;
    default:
      device.routerFsMode = FWA::DICE::RouterFrameSyncMode::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read DOMAIN_CTRL (0x" << std::hex
              << domainCtrlAddr
              << "): Router FS Mode = " << static_cast<int>(device.routerFsMode)
              << std::dec << std::endl;
  } else {
    std::cerr
        << "Warning [DICE]: Failed to read CLOCK_CONTROLLER_DOMAIN_CTRL (0x"
        << std::hex << domainCtrlAddr << ") (status: " << status << ")"
        << std::dec << std::endl;
  }
}

static void
readAesReceiverRegisters(IOFireWireDeviceInterface **deviceInterface,
                         io_service_t service, FireWireDevice &device,
                         uint64_t discoveredDiceBase, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading AES Receiver registers..." << std::endl;
  uint64_t aesReceiverBase = DICE_REGISTER_AES_RECEIVER_BASE;

  // Read STAT_ALL
  uint64_t statAllAddr = aesReceiverBase + DICE_REGISTER_AES_RECEIVER_STAT_ALL;
  UInt32 statAllValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, statAllAddr, statAllValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[statAllAddr] = statAllValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(statAllValue);
    device.aesLocked = (hostValue & DICE_AES_RECEIVER_STAT_ALL_LOCK_BIT) != 0;
    std::cerr << "Debug [DICE]: Read AES_RECEIVER_STAT_ALL (0x" << std::hex
              << statAllAddr
              << "): Locked = " << (device.aesLocked ? "true" : "false")
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AES_RECEIVER_STAT_ALL (0x"
              << std::hex << statAllAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

static void readAudioMixerRegisters(IOFireWireDeviceInterface **deviceInterface,
                                    io_service_t service,
                                    FireWireDevice &device,
                                    uint64_t discoveredDiceBase,
                                    UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading Audio Mixer registers..." << std::endl;
  uint64_t audioMixerBase = DICE_REGISTER_AUDIO_MIXER_BASE;

  // Read MIXER_NUMOFCH
  uint64_t numOfChAddr = audioMixerBase + DICE_REGISTER_AUDIO_MIXER_NUMOFCH;
  UInt32 numOfChValue = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, numOfChAddr, numOfChValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[numOfChAddr] = numOfChValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(numOfChValue);
    device.mixerRxChannels = static_cast<uint8_t>(
        hostValue & 0xFF); // Assuming lower 8 bits for channel count
    std::cerr << "Debug [DICE]: Read AUDIO_MIXER_NUMOFCH (0x" << std::hex
              << numOfChAddr
              << "): RX Channels = " << static_cast<int>(device.mixerRxChannels)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AUDIO_MIXER_NUMOFCH (0x"
              << std::hex << numOfChAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

static void readAvsRegisters(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             uint64_t discoveredDiceBase, UInt32 generation) {
  std::cerr << "Debug [DICE]: Reading AVS registers..." << std::endl;
  uint64_t avsBase = DICE_REGISTER_AVS_SUB_SYSTEM_BASE;

  // Read AVS Audio Receiver Config (for receiver 0)
  uint64_t arxCfg0Addr = avsBase + DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG0;
  UInt32 arxCfg0Value = 0;
  IOReturn status = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, arxCfg0Addr, arxCfg0Value, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[arxCfg0Addr] = arxCfg0Value; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(arxCfg0Value);
    device.avsRxChannelId = static_cast<uint8_t>(
        (hostValue & DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_MASK) >>
        DICE_AVS_AUDIO_RECEIVER_CFG0_CHANNEL_ID_SHIFT);
    std::cerr << "Debug [DICE]: Read ARX0_CFG0 (0x" << std::hex << arxCfg0Addr
              << "): Channel ID = " << static_cast<int>(device.avsRxChannelId)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read ARX0_CFG0 (0x" << std::hex
              << arxCfg0Addr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  uint64_t arxCfg1Addr = avsBase + DICE_REGISTER_AVS_AUDIO_RECEIVER_CFG1;
  UInt32 arxCfg1Value = 0;
  status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, arxCfg1Addr,
                                         arxCfg1Value, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[arxCfg1Addr] = arxCfg1Value; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(arxCfg1Value);
    device.avsRxDataBlockSize = static_cast<uint8_t>(
        (hostValue & DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_MASK) >>
        DICE_AVS_AUDIO_RECEIVER_CFG1_SPECIFIED_DBS_SHIFT);
    std::cerr << "Debug [DICE]: Read ARX0_CFG1 (0x" << std::hex << arxCfg1Addr
              << "): Specified DBS = "
              << static_cast<int>(device.avsRxDataBlockSize) << std::dec
              << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read ARX0_CFG1 (0x" << std::hex
              << arxCfg1Addr << ") (status: " << status << ")" << std::dec
              << std::endl;
  }

  // Read AVS Audio Transmitter Config (for transmitter 0)
  uint64_t atxCfgAddr = avsBase + DICE_REGISTER_AVS_AUDIO_TRANSMITTER_CFG;
  UInt32 atxCfgValue = 0;
  status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, atxCfgAddr,
                                         atxCfgValue, generation);
  if (status == kIOReturnSuccess) {
    device.diceRegisters[atxCfgAddr] = atxCfgValue; // Store raw BE value
    uint32_t hostValue = CFSwapInt32LittleToHost(atxCfgValue);
    device.avsTxDataBlockSize = static_cast<uint8_t>(
        (hostValue & DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_MASK) >>
        DICE_AVS_AUDIO_TRANSMITTER_CFG_DATA_BLOCK_SIZE_SHIFT);
    uint32_t sysMode =
        (hostValue & DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_MASK) >>
        DICE_AVS_AUDIO_TRANSMITTER_CFG_SYS_MODE_SHIFT;

    switch (sysMode) {
    case 0:
      device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Low;
      break;
    case 1:
      device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Mid;
      break;
    case 2:
      device.avsTxSystemMode = FWA::DICE::AvsSystemMode::High;
      break;
    default:
      device.avsTxSystemMode = FWA::DICE::AvsSystemMode::Unknown;
      break;
    }
    std::cerr << "Debug [DICE]: Read ATX0_CFG (0x" << std::hex << atxCfgAddr
              << "): Data Block Size = "
              << static_cast<int>(device.avsTxDataBlockSize)
              << ", System Mode = " << static_cast<int>(device.avsTxSystemMode)
              << std::dec << std::endl;
  } else {
    std::cerr << "Warning [DICE]: Failed to read AVS_AUDIO_TRANSMITTER_CFG (0x"
              << std::hex << atxCfgAddr << ") (status: " << status << ")"
              << std::dec << std::endl;
  }
}

/**
 * @brief Scans memory around a given address for potential 64-bit pointers to
 * high memory regions.
 *
 * Reads a block of memory centered around `centerAddress` and interprets
 * consecutive quadlets (UInt32) as potential 64-bit pointers (assuming Big
 * Endian storage across two quadlets). Checks if these potential pointers fall
 * within the typical high FireWire address range (>= 0xFFFFF0000000).
 *
 * @param deviceInterface Pointer-to-pointer to an opened
 * IOFireWireDeviceInterface.
 * @param service The io_service_t representing the FireWire nub.
 * @param generation The current bus generation number.
 * @param centerAddress The 64-bit address around which to scan.
 * @param searchRadiusBytes The total number of bytes to read (centered on
 * centerAddress). Should be a multiple of 8.
 * @return A vector containing potential 64-bit high-address pointers found in
 * the scanned region.
 */
std::vector<uint64_t>
findPointersNearAddress(IOFireWireDeviceInterface **deviceInterface,
                        io_service_t service, UInt32 generation,
                        uint64_t centerAddress, size_t searchRadiusBytes) {
  std::vector<uint64_t> potentialPointers;
  if (!deviceInterface || !*deviceInterface) {
    std::cerr << "Error [findPointersNearAddress]: Invalid device interface."
              << std::endl;
    return potentialPointers;
  }
  if (searchRadiusBytes == 0 || searchRadiusBytes % 8 != 0) {
    std::cerr << "Warning [findPointersNearAddress]: searchRadiusBytes must be "
                 "a non-zero multiple of 8. Adjusting to nearest valid size or "
                 "skipping."
              << std::endl;
    // Adjust or return? Let's adjust down for now.
    searchRadiusBytes = (searchRadiusBytes / 8) * 8;
    if (searchRadiusBytes == 0)
      return potentialPointers;
  }

  // Calculate start address, avoiding underflow
  uint64_t startAddr = (centerAddress > searchRadiusBytes / 2)
                           ? (centerAddress - searchRadiusBytes / 2)
                           : 0;
  // Ensure startAddr is quadlet-aligned (though ReadBlock might handle this)
  startAddr &= ~0x3ULL;

  std::cerr << "Debug [PointerScan]: Scanning " << searchRadiusBytes
            << " bytes around 0x" << std::hex << centerAddress
            << " (starting at 0x" << startAddr << ")" << std::dec << std::endl;

  std::vector<UInt32> buffer(searchRadiusBytes / sizeof(UInt32));
  UInt32 numBytesToRead =
      static_cast<UInt32>(searchRadiusBytes); // Request this many bytes
  UInt32 originalRequestedBytes =
      numBytesToRead; // Store original request for comparison

  FWAddress fwAddr;
  fwAddr.addressHi = static_cast<UInt16>((startAddr >> 32) & 0xFFFF);
  fwAddr.addressLo = static_cast<UInt32>(startAddr & 0xFFFFFFFF);

  // Use Read, passing numBytesToRead by pointer (in/out)
  IOReturn status = (*deviceInterface)
                        ->Read(deviceInterface, service, &fwAddr, buffer.data(),
                               &numBytesToRead, kFWFailOnReset, generation);

  if (status != kIOReturnSuccess) {
    std::cerr << "Error [PointerScan]: Read failed at 0x" << std::hex
              << startAddr << " (status: " << status << ")" << std::dec
              << std::endl;
    return potentialPointers;
  }

  // Check if the actual bytes read (now in numBytesToRead) is less than
  // requested or too small
  if (numBytesToRead < 8) // Need at least 8 bytes for one 64-bit pointer
  {
    std::cerr << "Warning [PointerScan]: Read returned less than 8 bytes ("
              << numBytesToRead << "). Cannot scan for 64-bit pointers."
              << std::dec << std::endl;
    return potentialPointers;
  }
  if (numBytesToRead < originalRequestedBytes) {
    std::cerr << "Warning [PointerScan]: Read returned fewer bytes ("
              << numBytesToRead << ") than requested ("
              << originalRequestedBytes << ")." << std::dec << std::endl;
    // Continue processing with the bytes actually read
  }

  std::cerr << "Debug [PointerScan]: Read successful, read " << numBytesToRead
            << " bytes." << std::dec << std::endl;

  size_t numQuadletsRead =
      numBytesToRead / sizeof(UInt32); // Use actual bytes read
  const uint64_t POINTER_THRESHOLD = 0xFFFFF0000000ULL;

  for (size_t i = 0; i < numQuadletsRead - 1; ++i) {
    // Assume Big Endian storage: quadlet[i] is high part, quadlet[i+1] is low
    // part
    UInt32 quadletHi = buffer[i];     // Already Big Endian from device
    UInt32 quadletLo = buffer[i + 1]; // Already Big Endian from device

    // Combine into 64-bit value
    uint64_t potentialPtr =
        (static_cast<uint64_t>(quadletHi) << 32) | quadletLo;

    // Check if it looks like a high address pointer
    if (potentialPtr >= POINTER_THRESHOLD) {
      uint64_t pointerLocation = startAddr + (i * sizeof(UInt32));
      std::cerr
          << "Debug [PointerScan]: Found potential high-address pointer at 0x"
          << std::hex << pointerLocation << ": 0x" << potentialPtr << std::dec
          << std::endl;
      potentialPointers.push_back(potentialPtr);
    }
  }

  return potentialPointers;
}

// Helper function to read and validate TX/RX offsets from a given global base
static bool readDiceOffsetsInternal(IOFireWireDeviceInterface **deviceInterface,
                                    io_service_t service, UInt32 generation,
                                    uint64_t globalBase, uint64_t &txBase,
                                    uint64_t &rxBase) {
  UInt32 txOffsetBE = 0;
  UInt32 rxOffsetBE = 0;
  bool txReadSuccess = false;
  bool rxReadSuccess = false;
  const uint32_t MAX_REASONABLE_OFFSET_QUADLETS =
      0x10000; // Define a reasonable upper limit for offsets (in quadlets)

  logInfo("  Attempting to read TX/RX offsets from Global Base: 0x%llx",
          globalBase);

  IOReturn txRet = safeReadQuadlet(deviceInterface, service,
                                   globalBase + DICE_REGISTER_TX_PAR_SPACE_OFF,
                                   txOffsetBE, generation);
  if (txRet == kIOReturnSuccess) {
    uint32_t txOffsetQuadlets = CFSwapInt32LittleToHost(txOffsetBE);
    // Sanity check: offset shouldn't be zero or excessively large
    if (txOffsetQuadlets != 0 &&
        txOffsetQuadlets < MAX_REASONABLE_OFFSET_QUADLETS) {
      txBase = globalBase + (static_cast<uint64_t>(txOffsetQuadlets) *
                             4); // Offset is in quadlets
      txReadSuccess = true;
      logInfo("    Read TX Offset (Quadlets): 0x%x -> TX Base: 0x%llx",
              txOffsetQuadlets, txBase);
    } else {
      logWarning("    Read suspicious TX Offset value (Quadlets): 0x%x from "
                 "0x%llx. Ignoring.",
                 txOffsetQuadlets, globalBase + DICE_REGISTER_TX_PAR_SPACE_OFF);
    }
  } else {
    logWarning("    Read failed for TX Offset at 0x%llx (Error: 0x%x)",
               globalBase + DICE_REGISTER_TX_PAR_SPACE_OFF, txRet);
  }

  IOReturn rxRet = safeReadQuadlet(deviceInterface, service,
                                   globalBase + DICE_REGISTER_RX_PAR_SPACE_OFF,
                                   rxOffsetBE, generation);
  if (rxRet == kIOReturnSuccess) {
    uint32_t rxOffsetQuadlets = CFSwapInt32LittleToHost(rxOffsetBE);
    // Sanity check
    if (rxOffsetQuadlets != 0 &&
        rxOffsetQuadlets < MAX_REASONABLE_OFFSET_QUADLETS) {
      rxBase = globalBase + (static_cast<uint64_t>(rxOffsetQuadlets) *
                             4); // Offset is in quadlets
      rxReadSuccess = true;
      logInfo("    Read RX Offset (Quadlets): 0x%x -> RX Base: 0x%llx",
              rxOffsetQuadlets, rxBase);
    } else {
      logWarning("    Read suspicious RX Offset value (Quadlets): 0x%x from "
                 "0x%llx. Ignoring.",
                 rxOffsetQuadlets, globalBase + DICE_REGISTER_RX_PAR_SPACE_OFF);
    }
  } else {
    logWarning("    Read failed for RX Offset at 0x%llx (Error: 0x%x)",
               globalBase + DICE_REGISTER_RX_PAR_SPACE_OFF, rxRet);
  }

  // Return true only if BOTH offsets were read successfully and seem valid
  return txReadSuccess && rxReadSuccess;
}

// Function to discover DICE base addresses (Global, TX, RX)
static bool
discoverDiceBaseAddressesInternal(IOFireWireDeviceInterface **deviceInterface,
                                  io_service_t service, UInt32 generation,
                                  uint64_t &globalBase, uint64_t &txBase,
                                  uint64_t &rxBase, std::string &method) {
  logInfo("Attempting to discover DICE base addresses...");
  globalBase = DICE_INVALID_OFFSET;
  txBase = DICE_INVALID_OFFSET;
  rxBase = DICE_INVALID_OFFSET;
  method = "None";
  bool baseFound = false;
  UInt32 testValue = 0; // For verification reads

  // Method 1: Use Vendor Key from Config ROM
  uint64_t vendorKeyAddress = 0;
  if (findVendorKeyAddress(
          deviceInterface, service, generation,
          vendorKeyAddress)) // Assuming this function exists and works
  {
    logInfo("  Found Vendor Key Address in Config ROM: 0x%llx",
            vendorKeyAddress);

    // Attempt 1a: Assume Vendor Key Address IS the Global Base
    logInfo("  Attempt 1a: Assuming Vendor Key Address (0x%llx) is the Global "
            "Base.",
            vendorKeyAddress);
    if (safeReadQuadlet(deviceInterface, service,
                        vendorKeyAddress + DICE_REGISTER_GLOBAL_VERSION,
                        testValue, generation) == kIOReturnSuccess &&
        testValue != 0) {
      logInfo("    Verification read successful (Version=0x%x).",
              CFSwapInt32LittleToHost(testValue));
      globalBase = vendorKeyAddress;
      method = "Vendor Key Address";
      baseFound = true;
    } else {
      logWarning("    Verification failed assuming Vendor Key Address is "
                 "Global Base.");
    }
  } else {
    logInfo("  Vendor Key not found in Config ROM.");
  }

  // Method 2: Fallback to known addresses if Vendor Key method failed
  if (!baseFound) {
    logInfo("  Attempt 2: Testing fallback base 0x%llx...", DICE_REGISTER_BASE);
    if (safeReadQuadlet(deviceInterface, service,
                        DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_VERSION,
                        testValue, generation) == kIOReturnSuccess &&
        testValue != 0) {
      logInfo("    Verification read successful (Version=0x%x).",
              CFSwapInt32LittleToHost(testValue));
      globalBase = DICE_REGISTER_BASE;
      method = "Fallback (Verified)";
      baseFound = true;
    } else {
      logError("    Fallback base verification failed! Cannot reliably "
               "determine Global Base.");
      method = "Fallback (Failed)";
      return false; // Cannot proceed without a valid global base
    }
  }

  // If Global Base found, try to determine TX/RX Bases
  if (baseFound) {
    logInfo("  Global Base determined via %s: 0x%llx", method.c_str(),
            globalBase);
    if (readDiceOffsetsInternal(deviceInterface, service, generation,
                                globalBase, txBase, rxBase)) {
      logInfo("  Successfully read TX/RX offsets from determined Global Base.");
      return true; // Success!
    } else {
      logWarning(
          "  Failed to read valid TX/RX offsets from Global Base 0x%llx.",
          globalBase);
      logWarning("  Using default offsets (in bytes) as a last resort.");
      txBase =
          globalBase + DEFAULT_DICE_TX_OFFSET; // Default offset is in BYTES
      rxBase =
          globalBase + DEFAULT_DICE_RX_OFFSET; // Default offset is in BYTES
      logInfo("  Using default TX Base: 0x%llx, RX Base: 0x%llx", txBase,
              rxBase);
      method += " + Default Offsets";
      return true; // Return true even with default offsets, but method
                   // indicates it
    }
  }

  // Should not be reached if baseFound was true, but included for completeness
  logError("Failed to discover DICE Global Base address using any method.");
  return false;
}

// --- Public DICE Helper Function Implementations ---
// EAP functions moved to eap_helpers.cpp
void setDefaultDiceConfig(FireWireDevice &device) {
  std::cerr << "Debug [DICE]: Setting default configuration for ";
  switch (device.diceChipType) {
  case FWA::DICE::DiceChipType::DiceII:
    std::cerr << "DICE II";
    break;
  case FWA::DICE::DiceChipType::DiceMini:
    std::cerr << "DICE Mini";
    break;
  case FWA::DICE::DiceChipType::DiceJr:
    std::cerr << "DICE Jr";
    break;
  default:
    std::cerr << "Unknown";
    break;
  }
  std::cerr << " chip type." << std::endl;

  // Default values based on chip type
  switch (device.diceChipType) {
  case FWA::DICE::DiceChipType::DiceII:
    device.txStreamCount = 4;
    device.rxStreamCount = 4;
    break;
  case FWA::DICE::DiceChipType::DiceMini:
    device.txStreamCount = 2;
    device.rxStreamCount = 2;
    break;
  case FWA::DICE::DiceChipType::DiceJr:
    device.txStreamCount = 1;
    device.rxStreamCount = 1;
    break;
  default:
    // For unknown types, assume a conservative 2 streams each if not already
    // set
    if (device.txStreamCount == 0)
      device.txStreamCount = 2;
    if (device.rxStreamCount == 0)
      device.rxStreamCount = 2;
    break;
  }

  std::cerr << "Debug [DICE]: Using default TX streams: "
            << device.txStreamCount << std::endl;
  std::cerr << "Debug [DICE]: Using default RX streams: "
            << device.rxStreamCount << std::endl;
}

void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, FireWireDevice &device) {
  if (!deviceInterface || !*deviceInterface) {
    std::cerr
        << "Error [DICE]: Invalid device interface passed to readDiceRegisters."
        << std::endl;
    return;
  }

  // Get the current generation
  UInt32 generation = 0;
  std::cerr << "Debug [DICE]: Calling GetBusGeneration..." << std::endl;
  IOReturn status =
      (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
  std::cerr << "Debug [DICE]: GetBusGeneration result: " << status
            << ", generation: " << generation << std::endl;
  if (status != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to get bus generation (status: "
              << status << "). Aborting register read." << std::endl;
    return;
  }

  // --- Discover Base Addresses using the helper function ---
  uint64_t globalBase = DICE_INVALID_OFFSET;
  uint64_t txBase = DICE_INVALID_OFFSET;
  uint64_t rxBase = DICE_INVALID_OFFSET;
  std::string baseDeterminationMethod = "None";

  // Call the helper function (definition to be added later)
  // Note: This will cause a linker error until the function is defined.
  if (!discoverDiceBaseAddressesInternal(deviceInterface, service, generation,
                                         globalBase, txBase, rxBase,
                                         baseDeterminationMethod)) {
    logError("Failed to determine valid DICE base addresses. Aborting DICE "
             "register read.");
    // Optionally set default bases here if you want to proceed with guesses,
    // but it's risky. device.diceGlobalBase = DICE_REGISTER_BASE;
    // device.diceTxBase = DICE_REGISTER_TX_BASE;
    // device.diceRxBase = DICE_REGISTER_RX_BASE;
    // device.diceBaseDeterminationMethod = "Failed Discovery - Using Defaults";
    return; // Exit if base discovery failed critically
  }
  // --- End Base Address Discovery ---

  logInfo("Final Determined Bases:");
  logInfo("  - Method: %s", baseDeterminationMethod.c_str());
  logInfo("  - Global: 0x%llx", globalBase);
  logInfo("  - TX:     0x%llx", txBase);
  logInfo("  - RX:     0x%llx", rxBase);

  // Store determined bases in the device struct
  device.diceGlobalBase = globalBase;
  device.diceTxBase = txBase;
  device.diceRxBase = rxBase;
  device.diceBaseDeterminationMethod = baseDeterminationMethod;

  // --- Try EAP (Can run independently of base discovery) ---
  bool eapCapabilitiesRead =
      readDiceEAPCapabilities(deviceInterface, service, device, generation);
  bool eapConfigRead = false;
  if (eapCapabilitiesRead) {
    logInfo("Detected DICE chipset via EAP: %d",
            static_cast<int>(device.diceChipType));
    eapConfigRead =
        readDiceEAPCurrentConfig(deviceInterface, service, device, generation);
  }
  if (!eapCapabilitiesRead || !eapConfigRead) {
    logWarning("EAP read failed or incomplete. Setting default DICE config "
               "based on chip type (if known).");
    setDefaultDiceConfig(device); // Set defaults if EAP fails
  }
  // --- End EAP ---

  // --- Test Reads (Optional but recommended) ---
  // Note: Verification reads are now part of discoverDiceBaseAddressesInternal,
  // so explicit test reads here are redundant. Removed.
  // --- End Test Reads ---

  // --- Read Standard DICE Registers using Helpers ---
  // Only proceed if globalBase is valid
  if (globalBase != DICE_INVALID_OFFSET) {
    readDiceGlobalRegistersInternal(deviceInterface, service, device,
                                    globalBase, generation);
  } else {
    logError("Skipping Global register reads due to invalid Global Base.");
  }

  // --- Read Targeted Registers (These use fixed bases internally for now) ---
  // These could potentially also be skipped if globalBase is invalid, depending
  // on whether their fixed bases are relative or absolute. Assuming absolute
  // for now.
  readGpcsrRegisters(deviceInterface, service, device, globalBase,
                     generation); // Pass globalBase for context/future use
  readClockControllerRegisters(deviceInterface, service, device, globalBase,
                               generation);
  readAesReceiverRegisters(deviceInterface, service, device, globalBase,
                           generation);
  readAudioMixerRegisters(deviceInterface, service, device, globalBase,
                          generation);
  readAvsRegisters(deviceInterface, service, device, globalBase, generation);

  // --- Read Stream-Specific Registers ---
  uint32_t txStreamSizeQuadlets = 256; // Default
  uint32_t rxStreamSizeQuadlets = 256; // Default

  // Read stream sizes and counts only if TX/RX bases are valid
  if (txBase != DICE_INVALID_OFFSET && rxBase != DICE_INVALID_OFFSET) {
    // Determine stream sizes (needed for stream register helpers)
    uint64_t txSizeAddr = txBase + DICE_REGISTER_TX_SZ_TX;
    UInt32 rawTxSize = 0;
    status = safeReadQuadlet(deviceInterface, service, txSizeAddr, rawTxSize,
                             generation);
    if (status == kIOReturnSuccess) {
      device.diceRegisters[txSizeAddr] = rawTxSize;
      txStreamSizeQuadlets = CFSwapInt32LittleToHost(rawTxSize);
      logInfo("Read TX stream size from register (0x%llx): %u", txSizeAddr,
              txStreamSizeQuadlets);
      if (txStreamSizeQuadlets == 0 || txStreamSizeQuadlets > 1024) {
        logWarning("Invalid TX stream size read (%u). Assuming 256.",
                   txStreamSizeQuadlets);
        txStreamSizeQuadlets = 256;
      }
    } else {
      logWarning("Could not read TX stream size register (0x%llx) (status: "
                 "%d). Assuming 256.",
                 txSizeAddr, status);
      // txStreamSizeQuadlets remains 256 (default)
    }

    uint64_t rxSizeAddr = rxBase + DICE_REGISTER_RX_SZ_RX;
    UInt32 rawRxSize = 0;
    status = safeReadQuadlet(deviceInterface, service, rxSizeAddr, rawRxSize,
                             generation);
    if (status == kIOReturnSuccess) {
      device.diceRegisters[rxSizeAddr] = rawRxSize;
      rxStreamSizeQuadlets = CFSwapInt32LittleToHost(rawRxSize);
      logInfo("Read RX stream size from register (0x%llx): %u", rxSizeAddr,
              rxStreamSizeQuadlets);
      if (rxStreamSizeQuadlets == 0 || rxStreamSizeQuadlets > 1024) {
        logWarning("Invalid RX stream size read (%u). Assuming 256.",
                   rxStreamSizeQuadlets);
        rxStreamSizeQuadlets = 256;
      }
    } else {
      logWarning("Could not read RX stream size register (0x%llx) (status: "
                 "%d). Assuming 256.",
                 rxSizeAddr, status);
      // rxStreamSizeQuadlets remains 256 (default)
    }

    // Read stream counts from registers
    uint64_t txCountAddr = txBase + DICE_REGISTER_TX_NB_TX;
    uint32_t rawTxCount = 0;
    IOReturn txCountStatus = safeReadQuadlet(
        deviceInterface, service, txCountAddr, rawTxCount, generation);
    if (txCountStatus == kIOReturnSuccess) {
      device.diceRegisters[txCountAddr] = rawTxCount;
      uint32_t readTxCount = CFSwapInt32LittleToHost(rawTxCount);
      logInfo("Read TX stream count from register (0x%llx): %u", txCountAddr,
              readTxCount);
      if (readTxCount <= 64) // Validate
      {
        device.txStreamCount = readTxCount;
      } else {
        logWarning("Unreasonable TX stream count detected (%u). Using "
                   "default/EAP count: %u",
                   readTxCount, device.txStreamCount);
      }
    } else {
      logWarning("Failed to read TX stream count register (0x%llx) (status: "
                 "%d). Using default/EAP TX stream count: %u",
                 txCountAddr, txCountStatus, device.txStreamCount);
    }

    uint64_t rxCountAddr = rxBase + DICE_REGISTER_RX_NB_RX;
    uint32_t rawRxCount = 0;
    IOReturn rxCountStatus = safeReadQuadlet(
        deviceInterface, service, rxCountAddr, rawRxCount, generation);
    if (rxCountStatus == kIOReturnSuccess) {
      device.diceRegisters[rxCountAddr] = rawRxCount;
      uint32_t readRxCount = CFSwapInt32LittleToHost(rawRxCount);
      logInfo("Read RX stream count from register (0x%llx): %u", rxCountAddr,
              readRxCount);
      if (readRxCount <= 64) // Validate
      {
        device.rxStreamCount = readRxCount;
      } else {
        logWarning("Unreasonable RX stream count detected (%u). Using "
                   "default/EAP count: %u",
                   readRxCount, device.rxStreamCount);
      }
    } else {
      logWarning("Failed to read RX stream count register (0x%llx) (status: "
                 "%d). Using default/EAP RX stream count: %u",
                 rxCountAddr, rxCountStatus, device.rxStreamCount);
    }

    logInfo("Final stream counts: TX=%u, RX=%u", device.txStreamCount,
            device.rxStreamCount);

    // Read individual stream registers
    readDiceTxStreamRegisters(deviceInterface, service, device, txBase,
                              generation, txStreamSizeQuadlets);
    readDiceRxStreamRegisters(deviceInterface, service, device, rxBase,
                              generation, rxStreamSizeQuadlets);
  } else {
    logError("Skipping Stream register reads due to invalid TX/RX Base.");
  }
  // --- End Standard DICE Register Reads ---

  // --- Memory exploration (call helper from utils_explore_general.cpp) ---
  // Note: This is kept separate as it's purely for debugging/discovery
  logInfo("\n\n========== MEMORY EXPLORATION ==========\n");

  // Explore around the determined global DICE register area if valid
  if (globalBase != DICE_INVALID_OFFSET) {
    FWA::SCANNER::exploreDiceMemoryLayout(deviceInterface, service, device,
                                          generation, globalBase);
  } else {
    logWarning(
        "Skipping memory layout exploration due to invalid Global Base.");
  }

  // Perform a detailed exploration of the channel names area (independent of
  // DICE base)
  FWA::SCANNER::exploreChannelNamesArea(deviceInterface, service, device,
                                        generation);
}

} // namespace FWA::SCANNER
