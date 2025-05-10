#include "register_string_analyzer.hpp"
#include "register_address_ranges.hpp"
#include "register_known_locations.hpp"
#include "register_name_analyzer.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

namespace FWA::SCANNER {

void extractCoherentRegisterStrings(const FireWireDevice &device) {
  std::cout << "\n=== COHERENT ASCII STRINGS FROM REGISTERS ===\n" << std::endl;

  // Skip if no registers were read
  if (device.diceRegisters.empty()) {
    std::cout << "No DICE registers available for analysis." << std::endl;
    return;
  }

  // Step 1: Check known string locations
  std::map<UInt64, UInt32> tempRegisters = checkKnownStringLocations(device);

  // Step 2: Group registers by address range
  std::vector<RegisterAddressRange> ranges =
      groupRegistersByAddressRange(tempRegisters);

  // Step 3: Process each address range for coherent strings
  processAddressRangesForStrings(ranges);

  // Step 4: Analyze channel and device names
  analyzeChannelAndDeviceNames(device);

  // Step 5: Analyze clock source names
  analyzeClockSourceNames(device);
}

} // namespace FWA::SCANNER
