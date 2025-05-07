#include "register_known_locations.hpp"
#include "io_helpers.hpp" // For interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>

namespace FWA::SCANNER {

std::map<uint64_t, uint32_t>
checkKnownStringLocations(const FireWireDevice &device) {
  std::cout << "\n=== KNOWN STRING LOCATIONS ===\n" << std::endl;

  // Skip if no registers were read
  if (device.diceRegisters.empty()) {
    std::cout << "No DICE registers available for analysis." << std::endl;
    return {};
  }

  // Define known string locations from previous memory explorations
  std::vector<std::pair<uint64_t, std::string>> knownStringAddresses = {
      {DICE_DEVICE_NAME_ADDR, "Device Name"},
      {DICE_CHANNEL_NAMES_ADDR_2, "Channel Configuration"},
      {DICE_CHANNEL_NAMES_ADDR_1, "Output Channels"}};

  // Add these addresses to the device's register map for analysis
  // We'll use a temporary map to avoid modifying the original
  std::map<uint64_t, uint32_t> tempRegisters = device.diceRegisters;

  // Try to read from these known addresses
  for (const auto &addrPair : knownStringAddresses) {
    uint64_t addr = addrPair.first;
    std::string description = addrPair.second;

    // Check if we already have this address in our register map
    if (tempRegisters.find(addr) == tempRegisters.end()) {
      // We don't have it, so let's try to read it directly
      std::cout << "  Checking " << description << " at 0x" << std::hex << addr
                << std::dec << "..." << std::endl;

      // We can't read it here since we don't have the device interface
      // But we can check if any of our existing registers are in the vicinity
      bool foundNearby = false;
      for (const auto &reg : device.diceRegisters) {
        if (std::abs(static_cast<int64_t>(reg.first) -
                     static_cast<int64_t>(addr)) < 64) {
          std::cout << "    Found nearby register at 0x" << std::hex
                    << reg.first << std::dec << std::endl;
          foundNearby = true;
        }
      }

      if (!foundNearby) {
        std::cout << "    No nearby registers found in current scan"
                  << std::endl;
      }
    }
  }
  std::cout << std::endl;

  return tempRegisters;
}

} // namespace FWA::SCANNER
