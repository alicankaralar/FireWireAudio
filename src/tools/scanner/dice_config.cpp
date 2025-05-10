#include "dice_config.hpp"
#include "FWA/dice/DiceAbsoluteAddresses.hpp" // For DICE_REGISTER_BASE, etc.
#include "FWA/dice/DiceDefines.hpp"
#include "dice_base_discovery.hpp"   // For discoverDiceBaseAddresses
#include "dice_register_readers.hpp" // For readDiceGlobalRegisters, etc.
#include "dice_stream_helpers.hpp"   // For readStreamSize, readStreamCount
#include "dice_stream_registers.hpp" // For readDiceTxStreamRegisters, readDiceRxStreamRegisters
#include "eap_helpers.hpp" // For readDiceEAPCapabilities, readDiceEAPCurrentConfig
#include "endianness_helpers.hpp"     // For deviceToHostInt32
#include "io_helpers.hpp"             // For safeReadQuadlet
#include "utils_explore_channels.hpp" // For exploreChannelNamesArea
#include "utils_explore_general.hpp"  // For exploreDiceMemoryLayout

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
namespace {
// Internal implementation of setDefaultDiceConfig
} // Anonymous namespace

// Public implementation of setDefaultDiceConfig

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

  UInt64 globalBase = DICE_REGISTER_BASE;
  UInt64 txBase = DICE_REGISTER_TX_BASE;
  UInt64 rxBase = DICE_REGISTER_RX_BASE;

  device.diceGlobalBase = globalBase;
  device.diceTxBase = txBase;
  device.diceRxBase = rxBase;
  readAllDiceRegisters(deviceInterface, service, device, globalBase,
                       generation);

  initializeStreamingParams(deviceInterface, service, globalBase, generation);
  showDevice(deviceInterface, service, generation);
  // Read TX stream size using dynamic TX base

  /*   // Read TX stream size
    readStreamSize(deviceInterface, service, device, txBase,
    DICE_REGISTER_TX_SIZE_TX_OFFSET, "TX", generation, txStreamSizeQuadlets);

    // Read RX stream size
    readStreamSize(deviceInterface, service, device, rxBase,
    DICE_REGISTER_RX_SIZE_RX_OFFSET, "RX", generation, rxStreamSizeQuadlets);

    // Read TX stream count
    readStreamCount(deviceInterface, service, device, txBase,
    DICE_REGISTER_TX_NUMBER_TX_OFFSET, "TX", generation, device.txStreamCount);

    // Read RX stream count
    readStreamCount(deviceInterface, service, device, rxBase,
    DICE_REGISTER_TX_NUMBER_TX_OFFSET, "RX", generation, device.rxStreamCount);

    std::cerr << "Info [DICE]: Final stream counts: TX=" << device.txStreamCount
              << ", RX=" << device.rxStreamCount << std::endl;

    // Use the dynamically discovered TX/RX bases for stream register reads
    readDiceTxStreamRegisters(deviceInterface, service, device, txBase,
                              generation, txStreamSizeQuadlets);
    readDiceRxStreamRegisters(deviceInterface, service, device, rxBase,
                              generation, rxStreamSizeQuadlets);
   */
  /*
    std::cerr << "\n\nInfo [DICE]: ========== MEMORY EXPLORATION ==========\n"
              << std::endl;
    exploreDiceMemoryLayout(deviceInterface, service, device, generation,
    globalBase);
    exploreChannelNamesArea(deviceInterface, service, device, generation);
  */
}
} // namespace FWA::SCANNER
