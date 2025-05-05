#pragma once

/*
 * DeviceDiscoverySolution.h
 *
 * This file provides a solution for DICE device detection issues
 * by modifying device discovery to look for generic FireWire devices
 * in addition to AVC units.
 *
 * The key insight is that while some audio devices present themselves
 * as FireWire AVC units, DICE devices (especially DICE Jr) might present
 * as generic FireWire devices without the AVC unit interface.
 */

#include "FWA/DiceDefines.hpp" // Include DICE register definitions
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h> // Added include for FireWire types
#include <string>                         // For std::string in error messages
#include <vector>

namespace FWA {
namespace DeviceDiscoverySolution {
// Use this device matching dictionary instead of specifically looking for AVC
// units
inline CFMutableDictionaryRef CreateDeviceMatchingDictionary() {
  // Create a dictionary that will match any FireWire device
  CFMutableDictionaryRef matchingDict = IOServiceMatching("IOFireWireDevice");

  if (!matchingDict) {
    // Error handled without direct spdlog dependency in header
    return nullptr;
  }

  return matchingDict;
}

/*
 * Using this approach will modify your device discovery to find all FireWire
 * devices, not just AVC units. You'll then need to filter these devices based
 * on additional criteria (like whether they respond to DICE register reads).
 *
 * To implement this solution, modify src/FWA/IOKitFireWireDeviceDiscovery.cpp:
 *
 * 1. Find where it creates the matching dictionary using:
 *    CFMutableDictionaryRef matchingDictAdded =
 * IOServiceMatching("IOFireWireAVCUnit");
 *
 * 2. Replace it with:
 *    CFMutableDictionaryRef matchingDictAdded =
 * DeviceDiscoverySolution::CreateDeviceMatchingDictionary();
 *
 * 3. You may need to add additional filtering in the deviceAdded callback to
 * ensure you're only processing audio devices (by testing DICE register
 * access).
 */

// Helper method to determine if a FireWire device is a DICE device by testing
// register access
inline bool IsDiceDevice(IOFireWireDeviceInterface **deviceInterface,
                         uint64_t guid) {
  if (!deviceInterface) {
    return false;
  }

  // Use DICE register definitions from DiceDefines.hpp directly
  std::vector<uint64_t> testRegisters = {
      DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_PAR_SPACE_OFF,
      DICE_REGISTER_BASE + DICE_REGISTER_GLOBAL_PAR_SPACE_SZ,
      DICE_REGISTER_BASE + DICE_REGISTER_TX_PAR_SPACE_OFF};

  // Get the current generation
  UInt32 generation;
  IOReturn status =
      (*deviceInterface)->GetBusGeneration(deviceInterface, &generation);
  if (status != kIOReturnSuccess) {
    // Debug message handled at implementation level
    return false;
  }

  // Try reading the registers
  for (const auto &offset : testRegisters) {
    FWAddress addr;
    addr.addressHi = static_cast<UInt16>((offset >> 32) & 0xFFFF);
    addr.addressLo = static_cast<UInt32>(offset & 0xFFFFFFFF);

    UInt32 value;
    status = ((IOFireWireDeviceInterface_t *)deviceInterface)
                 ->ReadQuadlet(deviceInterface, 0, &addr, &value,
                               kFWFailOnReset, generation);

    if (status == kIOReturnSuccess) {
      // If we can read even one register, consider it a DICE device
      // Success logging handled at implementation level
      return true;
    }
  }

  // Couldn't read any DICE registers
  return false;
}

} // namespace DeviceDiscoverySolution
} // namespace FWA
