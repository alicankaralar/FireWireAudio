#include "utils_explore_general.hpp"
#include "io_helpers.hpp"   // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"      // For FireWireDevice, DiceDefines.hpp constants
#include "utils_string.hpp" // For StringMatch, extractStringsFromMemory

#include <algorithm> // For std::sort
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>
#include <map>
#include <regex> // For std::regex, std::smatch
#include <set>   // For std::set
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER {

// --- Utility and Debugging Functions Implementation ---

void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation, uint64_t baseAddr) {
  std::cerr << "Debug [Utils]: Exploring memory around address 0x" << std::hex
            << baseAddr << std::dec << "..." << std::endl;

  // Define regions to explore (offsets from the provided base address)
  int EXPLORE_RANGE = 128;    // Quadlets (128 * 4 = 512 bytes) each side
  const int EXPLORE_STEP = 1; // Explore every quadlet

  // Special case for exploring around the output channel names
  if (baseAddr == 0xffffe0000000ULL) {
    // Check if we're specifically looking at the output channel names area
    uint64_t channelNamesAddr = 0xffffe00001a8;
    if (channelNamesAddr >= baseAddr - EXPLORE_RANGE * 4 &&
        channelNamesAddr <= baseAddr + EXPLORE_RANGE * 4) {
      // Extend the range to capture more channel names
      EXPLORE_RANGE = 256; // Quadlets (256 * 4 = 1024 bytes) each side
      std::cerr << "Debug [Utils]: Extended scan range to "
                << EXPLORE_RANGE * 16 << " bytes to capture more channel names"
                << std::endl;
    }
  }

  std::cerr << "Debug [Utils]: Scanning " << EXPLORE_RANGE * 16
            << " bytes below and above 0x" << std::hex << baseAddr << std::dec
            << " for possible DICE structures" << std::endl;

  // First look for a valid version register
  std::cerr << "\n--- Potential Version Registers ---" << std::endl;
  for (int offset = -EXPLORE_RANGE; offset <= EXPLORE_RANGE;
       offset += EXPLORE_STEP) {
    uint64_t testAddr = baseAddr + offset * 4; // 4 bytes per quadlet
    UInt32 value = 0;

    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, testAddr, value, generation);
    if (status == kIOReturnSuccess) {
      // Check if this could be a version register (typically in format A.B.C.D
      // where each is a byte) A valid version is usually something like 1.0.7.0
      // or similar
      uint32_t swappedValue =
          CFSwapInt32LittleToHost(value); // DICE uses Little Endian
      uint8_t versionA = DICE_DRIVER_SPEC_VERSION_NUMBER_GET_A(swappedValue);
      uint8_t versionB = DICE_DRIVER_SPEC_VERSION_NUMBER_GET_B(swappedValue);
      uint8_t versionC = DICE_DRIVER_SPEC_VERSION_NUMBER_GET_C(swappedValue);
      uint8_t versionD = DICE_DRIVER_SPEC_VERSION_NUMBER_GET_D(swappedValue);

      std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);
      // Heuristic: Version numbers usually small, D often 0
      bool isLikelyVersion =
          versionA < 10 && versionB < 20 && versionC < 50 && versionD < 10;

      if (isLikelyVersion) {
        std::cerr << "Addr: 0x" << std::hex << testAddr << " Value: 0x"
                  << swappedValue << std::dec;
        std::cerr << " **POSSIBLE VERSION: " << (int)versionA << "."
                  << (int)versionB << "." << (int)versionC << "."
                  << (int)versionD << "**";
        if (!ascii.empty())
          std::cerr << " (ASCII: '" << ascii << "')";
        std::cerr << std::endl;
      } else if (!ascii.empty() &&
                 ascii.length() >=
                     2) { // Log potentially interesting ASCII too
                          // std::cerr << "Addr: 0x" << std::hex << testAddr <<
                          // " Value: 0x" << swappedValue << std::dec; std::cerr
                          // << " (ASCII: '" << ascii << "')" << std::endl;
      }
    }
  }

  // Look for an area with coherent string data (channel names)
  std::cerr << "\n--- Potential String Data ---" << std::endl;
  std::string currentString;
  uint64_t stringStartAddr = 0;
  bool inString = false;

  for (int offset = -EXPLORE_RANGE; offset <= EXPLORE_RANGE;
       offset += EXPLORE_STEP) {
    uint64_t testAddr = baseAddr + offset * 4;
    UInt32 value = 0;
    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, testAddr, value, generation);

    if (status == kIOReturnSuccess) {
      uint32_t swappedValue = CFSwapInt32LittleToHost(value);
      std::string ascii = FWA::SCANNER::interpretAsASCII(swappedValue);
      bool hasPrintable = false;
      for (char c : ascii) {
        if (c >= 32 && c <= 126) {
          hasPrintable = true;
          break;
        }
      }

      if (hasPrintable) {
        if (!inString) {
          inString = true;
          stringStartAddr = testAddr;
        }
        currentString += ascii;
      } else {
        if (inString) {
          if (currentString.length() > 4) { // Arbitrary length threshold
            std::cerr << "Found string at 0x" << std::hex << stringStartAddr
                      << ": '" << currentString << "'" << std::dec << std::endl;
          }
          currentString.clear();
          inString = false;
        }
      }
    } else {
      if (inString) { // End of string due to read error
        if (currentString.length() > 4) {
          std::cerr << "Found string at 0x" << std::hex << stringStartAddr
                    << ": '" << currentString << "'" << std::dec << std::endl;
        }
        currentString.clear();
        inString = false;
      }
    }
  }
  if (inString &&
      currentString.length() > 4) { // Check if we were in a string at the end
    std::cerr << "Found string at 0x" << std::hex << stringStartAddr << ": '"
              << currentString << "'" << std::dec << std::endl;
  }

  // Look for structured numeric data (e.g., consecutive zeros, patterns)
  std::cerr << "\n--- Potential Structured Data (Zero Blocks) ---" << std::endl;
  int zeroCount = 0;
  uint64_t zeroStartAddr = 0;
  for (int offset = -EXPLORE_RANGE; offset <= EXPLORE_RANGE;
       offset += EXPLORE_STEP) {
    uint64_t testAddr = baseAddr + offset * 4;
    UInt32 value = 0;
    IOReturn status = FWA::SCANNER::safeReadQuadlet(
        deviceInterface, service, testAddr, value, generation);

    if (status == kIOReturnSuccess) {
      if (value == 0) { // Check raw Big Endian value for zero
        if (zeroCount == 0) {
          zeroStartAddr = testAddr;
        }
        zeroCount++;
      } else {
        if (zeroCount > 4) { // Report if more than 4 consecutive zeros
          std::cerr << "Found " << zeroCount
                    << " consecutive zeros starting at 0x" << std::hex
                    << zeroStartAddr << std::dec << std::endl;
        }
        zeroCount = 0;
      }
    } else {
      if (zeroCount > 4) {
        std::cerr << "Found " << zeroCount
                  << " consecutive zeros starting at 0x" << std::hex
                  << zeroStartAddr << std::dec << std::endl;
      }
      zeroCount = 0;
    }
  }
  if (zeroCount > 4) { // Check at the end
    std::cerr << "Found " << zeroCount << " consecutive zeros starting at 0x"
              << std::hex << zeroStartAddr << std::dec << std::endl;
  }

  std::cerr << "\nDebug [Utils]: Memory exploration complete" << std::endl;
}

void exploreTargetedMemoryAreas(IOFireWireDeviceInterface **deviceInterface,
                                io_service_t service, FireWireDevice &device,
                                UInt32 generation,
                                uint64_t discoveredDiceBase) {
  std::cerr << "Debug [Utils]: Exploring targeted memory areas..." << std::endl;

  // Explore key functional blocks based on datasheet memory map
  // Using discoveredDiceBase if available, otherwise falling back to hardcoded
  // DICE_REGISTER_BASE
  uint64_t baseToUse = (discoveredDiceBase != DICE_INVALID_OFFSET)
                           ? discoveredDiceBase
                           : DICE_REGISTER_BASE;

  std::cerr << "\n--- Exploring GPCSR Area (0xC700_0000) ---" << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          DICE_REGISTER_GPCSR_BASE);

  std::cerr << "\n--- Exploring Clock Controller Area (0xCE01_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          DICE_REGISTER_CLOCK_CONTROLLER_BASE);

  std::cerr << "\n--- Exploring AES Receiver Area (0xCE02_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          DICE_REGISTER_AES_RECEIVER_BASE);

  std::cerr << "\n--- Exploring Audio Mixer Area (0xCE06_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          DICE_REGISTER_AUDIO_MIXER_BASE);

  std::cerr << "\n--- Exploring AVS Sub System Area (0xCF00_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          DICE_REGISTER_AVS_SUB_SYSTEM_BASE);

  // Add other targeted areas as needed based on further analysis (e.g., InS,
  // ADAT, ARM Audio Transceiver)

  std::cerr << "\nDebug [Utils]: Targeted memory exploration complete."
            << std::endl;
}

void extractCoherentRegisterStrings(const FireWireDevice &device) {
  std::cout << "\n=== COHERENT ASCII STRINGS FROM REGISTERS ===\n" << std::endl;

  // Skip if no registers were read
  if (device.diceRegisters.empty()) {
    std::cout << "No DICE registers available for analysis." << std::endl;
    return;
  }

  // Add known addresses where strings have been found in previous scans
  std::cout << "=== KNOWN STRING LOCATIONS ===\n" << std::endl;

  // Define known string locations from previous memory explorations
  std::vector<std::pair<uint64_t, std::string>> knownStringAddresses = {
      {0xffffe0000034, "Device Name"},
      {0xffffe0000090, "Channel Configuration"},
      {0xffffe00001a8, "Output Channels"}};

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

  // Sort registers by address for sequential processing
  std::vector<std::pair<uint64_t, uint32_t>> sortedRegisters;
  for (const auto &regPair : device.diceRegisters) {
    sortedRegisters.push_back(regPair);
  }
  std::sort(sortedRegisters.begin(), sortedRegisters.end());

  // Group registers by address proximity
  std::map<uint64_t, std::vector<std::pair<uint64_t, uint32_t>>> addressRanges;
  uint64_t currentBase = 0;
  std::vector<std::pair<uint64_t, uint32_t>> currentGroup;

  for (size_t i = 0; i < sortedRegisters.size(); i++) {
    uint64_t addr = sortedRegisters[i].first;
    uint32_t value = sortedRegisters[i].second;

    if (currentGroup.empty() || addr - currentGroup.back().first <= 16) {
      // Within 16 bytes, consider part of the same group
      if (currentGroup.empty()) {
        currentBase = addr;
      }
      currentGroup.push_back({addr, value});
    } else {
      // Start a new group
      if (!currentGroup.empty()) {
        addressRanges[currentBase] = currentGroup;
        currentGroup.clear();
      }
      currentBase = addr;
      currentGroup.push_back({addr, value});
    }
  }

  // Add the last group if not empty
  if (!currentGroup.empty()) {
    addressRanges[currentBase] = currentGroup;
  }

  // Process each address range for coherent strings
  for (const auto &range : addressRanges) {
    uint64_t baseAddr = range.first;
    const auto &registers = range.second;

    // Skip very small groups
    if (registers.size() < 2)
      continue;

    std::cout << "Address Range: 0x" << std::hex << baseAddr << " - 0x"
              << registers.back().first << std::dec << " (" << registers.size()
              << " registers)" << std::endl;

    // Extract strings from this range
    std::string currentString;
    uint64_t stringStartAddr = 0;
    bool inString = false;

    for (const auto &reg : registers) {
      uint64_t addr = reg.first;
      uint32_t rawValue = reg.second;
      uint32_t hostValue =
          CFSwapInt32LittleToHost(rawValue); // Convert to host endianness
      std::string ascii = interpretAsASCII(hostValue);

      // Also try byte-swapped interpretation for little-endian strings
      std::string swappedAscii =
          interpretAsASCII(CFSwapInt32BigToHost(rawValue));
      if (swappedAscii.length() > ascii.length() ||
          (swappedAscii.length() == ascii.length() &&
           swappedAscii.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJ"
                                          "KLMNOPQRSTUVWXYZ0123456789") >
               ascii.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM"
                                       "NOPQRSTUVWXYZ0123456789"))) {
        // Use the swapped version if it's longer or has more alphanumeric
        // characters
        ascii = swappedAscii;
      }

      if (!ascii.empty()) {
        if (!inString) {
          inString = true;
          stringStartAddr = addr;
        }
        currentString += ascii;
      } else {
        if (inString) {
          // End of string
          if (currentString.length() >= 3) { // Minimum meaningful length
            std::cout << "  String at 0x" << std::hex << stringStartAddr
                      << std::dec << ": \"" << currentString << "\""
                      << std::endl;
          }
          currentString.clear();
          inString = false;
        }
      }
    }

    // Check if we were in a string at the end
    if (inString && currentString.length() >= 3) {
      std::cout << "  String at 0x" << std::hex << stringStartAddr << std::dec
                << ": \"" << currentString << "\"" << std::endl;
    }

    // Also look for patterns where ASCII is split across registers
    std::string combinedAscii;
    uint64_t combinedStartAddr = 0;
    bool inCombinedString = false;

    for (size_t i = 0; i < registers.size(); i++) {
      uint64_t addr = registers[i].first;
      uint32_t rawValue = registers[i].second;
      uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);

      // Try both byte orders for better string detection
      uint32_t swappedValue = CFSwapInt32BigToHost(rawValue);

      // Extract individual bytes and check if they form ASCII when combined
      // Try both byte orders for better string detection
      std::string normalAscii;
      std::string swappedAscii;
      bool foundNormalAscii = false;
      bool foundSwappedAscii = false;

      // Check normal byte order
      for (int bytePos = 0; bytePos < 4; bytePos++) {
        char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);
        if (c >= 32 && c <= 126) {
          normalAscii += c;
          foundNormalAscii = true;
        }
      }

      // Check swapped byte order
      for (int bytePos = 0; bytePos < 4; bytePos++) {
        char c = static_cast<char>((swappedValue >> (8 * bytePos)) & 0xFF);
        if (c >= 32 && c <= 126) {
          swappedAscii += c;
          foundSwappedAscii = true;
        }
      }

      // Use the better string (more printable characters)
      std::string betterAscii;
      if (foundSwappedAscii && (swappedAscii.length() > normalAscii.length())) {
        betterAscii = swappedAscii;
      } else if (foundNormalAscii) {
        betterAscii = normalAscii;
      }

      // Process the better string
      if (!betterAscii.empty()) {
        if (!inCombinedString) {
          inCombinedString = true;
          combinedStartAddr = addr;
        }
        combinedAscii += betterAscii;
      } else {
        // No ASCII found in either byte order
        if (inCombinedString) {
          // End of string (no more ASCII in this register)
          if (combinedAscii.length() >= 3) {
            std::cout << "  Byte-level string at 0x" << std::hex
                      << combinedStartAddr << std::dec << ": \""
                      << combinedAscii << "\"" << std::endl;
          }
          combinedAscii.clear();
          inCombinedString = false;
        }
      }
    }

    // Check for any remaining combined string
    if (inCombinedString && combinedAscii.length() >= 3) {
      std::cout << "  Byte-level string at 0x" << std::hex << combinedStartAddr
                << std::dec << ": \"" << combinedAscii << "\"" << std::endl;
    }

    std::cout << std::endl;
  }

  // Special analysis for known string regions
  std::cout << "=== CHANNEL AND DEVICE NAMES ===\n" << std::endl;

  // Look for TX/RX name base registers
  for (const auto &regPair : device.diceRegisters) {
    uint64_t addr = regPair.first;
    uint32_t rawValue = regPair.second;
    uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);

    // Check if this is a name base register
    bool isTxNameBase = false;
    bool isRxNameBase = false;
    bool isNickNameBase = false;

    // Extract the register offset from the absolute address
    uint64_t baseAddr = DICE_REGISTER_BASE; // Use discovered base if available
    int64_t offset = addr - baseAddr;

    // Check against known offsets
    if (offset == DICE_REGISTER_GLOBAL_NICK_NAME) {
      isNickNameBase = true;
    } else if ((offset - DICE_REGISTER_TX_NAMES_BASE) % (256 * 4) == 0) {
      isTxNameBase = true;
    } else if ((offset - DICE_REGISTER_RX_NAMES_BASE) % (256 * 4) == 0) {
      isRxNameBase = true;
    }

    if (isTxNameBase || isRxNameBase || isNickNameBase) {
      std::string nameType =
          isNickNameBase
              ? "Device Nick Name"
              : (isTxNameBase ? "TX Channel Name" : "RX Channel Name");

      // The value is likely a pointer to the actual name data
      uint64_t nameDataAddr =
          baseAddr + (hostValue * 4); // Convert quadlet offset to byte address

      std::cout << nameType << " Base at 0x" << std::hex << addr
                << " points to 0x" << nameDataAddr << std::dec << std::endl;

      // Try to read the name data if it's in our register map
      std::string nameData;
      bool foundData = false;

      // Look for registers in the vicinity of the name data address
      for (const auto &reg : device.diceRegisters) {
        if (reg.first >= nameDataAddr && reg.first < nameDataAddr + 64) {
          uint32_t nameValue = CFSwapInt32LittleToHost(reg.second);
          std::string ascii = interpretAsASCII(nameValue);
          if (!ascii.empty()) {
            nameData += ascii;
            foundData = true;
          }
        }
      }

      if (foundData) {
        std::cout << "  Name data: \"" << nameData << "\"" << std::endl;
      } else {
        std::cout << "  Name data not found in register map" << std::endl;
      }
    }
  }

  // Look for clock source names
  std::cout << "\n=== CLOCK SOURCE NAMES ===\n" << std::endl;
  uint64_t clockSourceNamesAddr = 0;

  // Find the clock source names base register
  for (const auto &regPair : device.diceRegisters) {
    uint64_t addr = regPair.first;
    uint64_t baseAddr = DICE_REGISTER_BASE;
    int64_t offset = addr - baseAddr;

    if (offset == DICE_REGISTER_GLOBAL_CLOCKSOURCENAMES) {
      uint32_t hostValue = CFSwapInt32LittleToHost(regPair.second);
      clockSourceNamesAddr = baseAddr + (hostValue * 4);
      std::cout << "Clock Source Names Base at 0x" << std::hex << addr
                << " points to 0x" << clockSourceNamesAddr << std::dec
                << std::endl;
      break;
    }
  }

  if (clockSourceNamesAddr != 0) {
    // Try to extract clock source names
    for (int i = 0; i < 16; i++) { // Assume up to 16 clock sources
      std::string clockName;
      bool foundClockName = false;
      uint64_t clockNameAddr =
          clockSourceNamesAddr + (i * 16); // Typically 16 bytes per name

      for (int j = 0; j < 4; j++) { // Up to 4 quadlets per name
        uint64_t addrToCheck = clockNameAddr + (j * 4);
        for (const auto &reg : device.diceRegisters) {
          if (reg.first == addrToCheck) {
            uint32_t nameValue = CFSwapInt32LittleToHost(reg.second);
            std::string ascii = interpretAsASCII(nameValue);
            if (!ascii.empty()) {
              clockName += ascii;
              foundClockName = true;
            }
            break;
          }
        }
      }

      if (foundClockName) {
        std::cout << "  Clock Source " << i << " at 0x" << std::hex
                  << clockNameAddr << std::dec << ": \"" << clockName << "\""
                  << std::endl;
      }
    }
  } else {
    std::cout << "  Clock Source Names Base register not found" << std::endl;
  }
}

} // namespace FWA::SCANNER
