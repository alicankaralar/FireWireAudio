#include "dice_config.hpp"
#include "FWA/DiceAbsoluteAddresses.hpp" // For DICE_REGISTER_BASE, etc.
#include "dice_base_discovery.hpp"       // For discoverDiceBaseAddresses
#include "dice_register_readers.hpp"     // For readDiceGlobalRegisters, etc.
#include "dice_stream_registers.hpp" // For readDiceTxStreamRegisters, readDiceRxStreamRegisters
#include "eap_helpers.hpp" // For readDiceEAPCapabilities, readDiceEAPCurrentConfig
#include "endianness_helpers.hpp"     // For deviceToHostInt32
#include "io_helpers.hpp"             // For safeReadQuadlet
#include "scanner_defines.hpp"        // For DICE_REL_OFFSET_* constants
#include "utils_explore_channels.hpp" // For exploreChannelNamesArea
#include "utils_explore_general.hpp"  // For exploreDiceMemoryLayout

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
namespace {
// Internal implementation of setDefaultDiceConfig
static void setDefaultDiceConfigInternal(FireWireDevice &device) {
  std::cerr << "Debug [DICE]: Setting default configuration for ";
  switch (device.diceChipType) {
  case DiceChipType::DiceII:
    std::cerr << "DICE II";
    break;
  case DiceChipType::DiceMini:
    std::cerr << "DICE Mini";
    break;
  case DiceChipType::DiceJr:
    std::cerr << "DICE Jr";
    break;
  default:
    std::cerr << "Unknown";
    break;
  }
  std::cerr << " chip type." << std::endl;

  switch (device.diceChipType) {
  case DiceChipType::DiceII:
    device.txStreamCount = 4;
    device.rxStreamCount = 4;
    break;
  case DiceChipType::DiceMini:
    device.txStreamCount = 2;
    device.rxStreamCount = 2;
    break;
  case DiceChipType::DiceJr:
    device.txStreamCount = 1;
    device.rxStreamCount = 1;
    break;
  default:
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
} // Anonymous namespace

// Public implementation of setDefaultDiceConfig
void setDefaultDiceConfig(FireWireDevice &device) {
  // Call the internal implementation
  setDefaultDiceConfigInternal(device);
}

// Public implementation of readDiceRegisters
void readDiceRegisters(IOFireWireDeviceInterface **deviceInterface,
                       io_service_t service, FireWireDevice &device) {
  if (!deviceInterface || !*deviceInterface) {
    std::cerr
        << "Error [DICE]: Invalid device interface passed to readDiceRegisters."
        << std::endl;
    return;
  }

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

  uint64_t globalBase = DICE_INVALID_OFFSET; // Use constexpr
  uint64_t txBase = DICE_INVALID_OFFSET;     // Use constexpr
  uint64_t rxBase = DICE_INVALID_OFFSET;     // Use constexpr
  std::string baseDeterminationMethod = "None";

  if (!discoverDiceBaseAddresses(deviceInterface, service, generation,
                                 globalBase, txBase, rxBase,
                                 baseDeterminationMethod)) {
    std::cerr << "Error [DICE]: Failed to determine valid DICE base addresses. "
                 "Aborting DICE register read."
              << std::endl;
    return;
  }

  std::cerr << "Info [DICE]: Final Determined Bases:" << std::endl;
  std::cerr << "Info [DICE]:   - Method: " << baseDeterminationMethod
            << std::endl;
  std::cerr << "Info [DICE]:   - Global: 0x" << std::hex << globalBase
            << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - TX:     0x" << std::hex << txBase << std::dec
            << std::endl;
  std::cerr << "Info [DICE]:   - RX:     0x" << std::hex << rxBase << std::dec
            << std::endl;

  // Store the discovered base addresses in the device structure
  device.diceGlobalBase = globalBase;
  device.diceTxBase = txBase;
  device.diceRxBase = rxBase;
  device.diceBaseDeterminationMethod = baseDeterminationMethod;

  // Read EAP capabilities and configuration using the discovered base address
  bool eapCapabilitiesRead =
      readDiceEAPCapabilities(deviceInterface, service, device, generation);
  bool eapConfigRead = false;
  if (eapCapabilitiesRead) {
    std::cerr << "Info [DICE]: Detected DICE chipset via EAP: "
              << static_cast<int>(device.diceChipType) << std::endl;
    eapConfigRead =
        readDiceEAPCurrentConfig(deviceInterface, service, device, generation);
  }
  if (!eapCapabilitiesRead || !eapConfigRead) {
    std::cerr << "Warning [DICE]: EAP read failed or incomplete. Setting "
                 "default DICE config based on chip type (if known)."
              << std::endl;
    setDefaultDiceConfigInternal(device);
  }

  if (globalBase != DICE_INVALID_OFFSET) // Use constexpr
  {
    readDiceGlobalRegisters(deviceInterface, service, device, globalBase,
                            generation);
  } else {
    std::cerr << "Error [DICE]: Skipping Global register reads due to invalid "
                 "Global Base."
              << std::endl;
  }

  readGpcsrRegisters(deviceInterface, service, device, globalBase, generation);
  readClockControllerRegisters(deviceInterface, service, device, globalBase,
                               generation);
  readAesReceiverRegisters(deviceInterface, service, device, globalBase,
                           generation);
  readAudioMixerRegisters(deviceInterface, service, device, globalBase,
                          generation);
  readAvsRegisters(deviceInterface, service, device, globalBase, generation);

  uint32_t txStreamSizeQuadlets = 256;
  uint32_t rxStreamSizeQuadlets = 256;

  if (txBase != DICE_INVALID_OFFSET &&
      rxBase != DICE_INVALID_OFFSET) // Use constexpr
  {
    // Read TX stream size using dynamic TX base
    uint64_t txSizeAddr = txBase + FWA::Scanner::DICE_REL_OFFSET_TX_SZ_TX;
    UInt32 rawTxSize = 0;
    status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, txSizeAddr,
                                           rawTxSize, generation);
    if (status == kIOReturnSuccess) {
      device.diceRegisters[txSizeAddr] = rawTxSize;
      txStreamSizeQuadlets =
          deviceToHostInt32(rawTxSize, device.deviceEndianness);
      std::cerr << "Info [DICE]: Read TX stream size from register (0x"
                << std::hex << txSizeAddr << std::dec
                << "): " << txStreamSizeQuadlets << std::endl;
      if (txStreamSizeQuadlets == 0 || txStreamSizeQuadlets > 1024) {
        std::cerr << "Warning [DICE]: Invalid TX stream size read ("
                  << txStreamSizeQuadlets << "). Assuming 256." << std::endl;
        txStreamSizeQuadlets = 256;
      }
    } else {
      std::cerr << "Warning [DICE]: Could not read TX stream size register (0x"
                << std::hex << txSizeAddr << std::dec << ") (status: " << status
                << "). Assuming 256." << std::endl;
    }

    // Read RX stream size using dynamic RX base
    uint64_t rxSizeAddr = rxBase + FWA::Scanner::DICE_REL_OFFSET_RX_SZ_RX;
    UInt32 rawRxSize = 0;
    status = FWA::SCANNER::safeReadQuadlet(deviceInterface, service, rxSizeAddr,
                                           rawRxSize, generation);
    if (status == kIOReturnSuccess) {
      device.diceRegisters[rxSizeAddr] = rawRxSize;
      rxStreamSizeQuadlets =
          deviceToHostInt32(rawRxSize, device.deviceEndianness);
      std::cerr << "Info [DICE]: Read RX stream size from register (0x"
                << std::hex << rxSizeAddr << std::dec
                << "): " << rxStreamSizeQuadlets << std::endl;
      if (rxStreamSizeQuadlets == 0 || rxStreamSizeQuadlets > 1024) {
        std::cerr << "Warning [DICE]: Invalid RX stream size read ("
                  << rxStreamSizeQuadlets << "). Assuming 256." << std::endl;
        rxStreamSizeQuadlets = 256;
      }
    } else {
      std::cerr << "Warning [DICE]: Could not read RX stream size register (0x"
                << std::hex << rxSizeAddr << std::dec << ") (status: " << status
                << "). Assuming 256." << std::endl;
    }

    // Read TX stream count using dynamic TX base
    uint64_t txCountAddr = txBase + FWA::Scanner::DICE_REL_OFFSET_TX_NB_TX;
    uint32_t rawTxCount = 0;
    IOReturn txCountStatus = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, txCountAddr, rawTxCount, generation);
    if (txCountStatus == kIOReturnSuccess) {
      device.diceRegisters[txCountAddr] = rawTxCount;
      uint32_t readTxCount =
          deviceToHostInt32(rawTxCount, device.deviceEndianness);
      std::cerr << "Info [DICE]: Read TX stream count from register (0x"
                << std::hex << txCountAddr << std::dec << "): " << readTxCount
                << std::endl;
      if (readTxCount <= 64) {
        device.txStreamCount = readTxCount;
      } else {
        std::cerr << "Warning [DICE]: Unreasonable TX stream count detected ("
                  << readTxCount
                  << "). Using default/EAP count: " << device.txStreamCount
                  << std::endl;
      }
    } else {
      std::cerr << "Warning [DICE]: Failed to read TX stream count register (0x"
                << std::hex << txCountAddr << std::dec
                << ") (status: " << txCountStatus
                << "). Using default/EAP TX stream count: "
                << device.txStreamCount << std::endl;
    }

    // Read RX stream count using dynamic RX base
    uint64_t rxCountAddr = rxBase + FWA::Scanner::DICE_REL_OFFSET_RX_NB_RX;
    uint32_t rawRxCount = 0;
    IOReturn rxCountStatus = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, rxCountAddr, rawRxCount, generation);
    if (rxCountStatus == kIOReturnSuccess) {
      device.diceRegisters[rxCountAddr] = rawRxCount;
      uint32_t readRxCount =
          deviceToHostInt32(rawRxCount, device.deviceEndianness);
      std::cerr << "Info [DICE]: Read RX stream count from register (0x"
                << std::hex << rxCountAddr << std::dec << "): " << readRxCount
                << std::endl;
      if (readRxCount <= 64) {
        device.rxStreamCount = readRxCount;
      } else {
        std::cerr << "Warning [DICE]: Unreasonable RX stream count detected ("
                  << readRxCount
                  << "). Using default/EAP count: " << device.rxStreamCount
                  << std::endl;
      }
    } else {
      std::cerr << "Warning [DICE]: Failed to read RX stream count register (0x"
                << std::hex << rxCountAddr << std::dec
                << ") (status: " << rxCountStatus
                << "). Using default/EAP RX stream count: "
                << device.rxStreamCount << std::endl;
    }

    std::cerr << "Info [DICE]: Final stream counts: TX=" << device.txStreamCount
              << ", RX=" << device.rxStreamCount << std::endl;

    // Use the dynamically discovered TX/RX bases for stream register reads
    readDiceTxStreamRegisters(deviceInterface, service, device, txBase,
                              generation, txStreamSizeQuadlets);
    readDiceRxStreamRegisters(deviceInterface, service, device, rxBase,
                              generation, rxStreamSizeQuadlets);
  } else {
    std::cerr << "Error [DICE]: Skipping Stream register reads due to invalid "
                 "TX/RX Base."
              << std::endl;
  }

  std::cerr << "\n\nInfo [DICE]: ========== MEMORY EXPLORATION ==========\n"
            << std::endl;
  if (globalBase != DICE_INVALID_OFFSET) // Use constexpr
  {
    exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                            globalBase);
  } else {
    std::cerr << "Warning [DICE]: Skipping memory layout exploration due to "
                 "invalid Global Base."
              << std::endl;
  }
  exploreChannelNamesArea(deviceInterface, service, device, generation);
}

} // namespace FWA::SCANNER
