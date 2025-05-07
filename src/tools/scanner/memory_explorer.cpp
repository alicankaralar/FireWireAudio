#include "memory_explorer.hpp"
#include "io_helpers.hpp" // For safeReadQuadlet, interpretAsASCII
#include "scanner.hpp"    // For FireWireDevice, DiceDefines.hpp constants

#include <algorithm> // For std::sort
#include <iomanip> // For std::setw, std::setfill, std::left, std::hex, std::dec
#include <iostream>
#include <map>
#include <regex> // For std::regex, std::smatch
#include <set>   // For std::set
#include <string>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost, CFSwapInt32BigToHost

namespace FWA::SCANNER {

void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, FireWireDevice &device,
                             UInt32 generation, uint64_t baseAddr) {
  std::cerr << "Debug [Utils]: Exploring memory around address 0x" << std::hex
            << baseAddr << std::dec << "..." << std::endl;

  // Define regions to explore (offsets from the provided base address)
  int EXPLORE_RANGE = 128;    // Quadlets (128 * 4 = 512 bytes) each side
  const int EXPLORE_STEP = 1; // Explore every quadlet

  // Special case for exploring around the output channel names
  if (baseAddr == DICE_REGISTER_BASE) {
    // Check if we're specifically looking at the output channel names area
    uint64_t channelNamesAddr = DICE_CHANNEL_NAMES_ADDR_1;
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
                          GPCSR_BASE);

  std::cerr << "\n--- Exploring Clock Controller Area (0xCE01_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          REGISTER_CLOCK_CONTROLLER_BASE);

  std::cerr << "\n--- Exploring AES Receiver Area (0xCE02_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          REGISTER_AES_RECEIVER_BASE);

  std::cerr << "\n--- Exploring Audio Mixer Area (0xCE06_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          REGISTER_AUDIO_MIXER_BASE);

  std::cerr << "\n--- Exploring AVS Sub System Area (0xCF00_0000) ---"
            << std::endl;
  exploreDiceMemoryLayout(deviceInterface, service, device, generation,
                          REGISTER_AVS_SUB_SYSTEM_BASE);

  // Add other targeted areas as needed based on further analysis (e.g., InS,
  // ADAT, ARM Audio Transceiver)

  std::cerr << "\nDebug [Utils]: Targeted memory exploration complete."
            << std::endl;
}

} // namespace FWA::SCANNER
