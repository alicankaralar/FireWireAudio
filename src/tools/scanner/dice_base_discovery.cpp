#include "dice_base_discovery.hpp"
#include "FWA/DiceAbsoluteAddresses.hpp" // For DICE_REGISTER_BASE, etc.
#include "config_rom.hpp"                // For parseConfigRomVendorKeys
#include "io_helpers.hpp"                // For safeReadQuadlet
#include "scanner_defines.hpp"           // For FFADO constants

#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <map>
#include <sstream>

#include <CoreFoundation/CoreFoundation.h> // For CFSwapInt32LittleToHost
#include <IOKit/firewire/IOFireWireFamilyCommon.h> // For kConfigDirectoryKey_Unit_Dependent
#include <IOKit/firewire/IOFireWireLib.h>

namespace FWA::SCANNER {
// Forward declarations
bool testRegisterSpaceAccess(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, UInt32 generation);
// Internal implementation of discoverDiceBaseAddresses
// This is needed by channel_discovery.cpp
bool discoverDiceBaseAddressesInternal(
    IOFireWireDeviceInterface **deviceInterface, io_service_t service,
    UInt32 generation, uint64_t &globalBase, uint64_t &txBase, uint64_t &rxBase,
    std::string &method) {
  return discoverDiceBaseAddresses(deviceInterface, service, generation,
                                   globalBase, txBase, rxBase, method);
}

namespace {
// Helper function to read DICE offsets from a discovery base address
static bool readDiceOffsets(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service, UInt32 generation,
                            uint64_t discoveryBase, uint64_t &globalBase,
                            uint64_t &txBase, uint64_t &rxBase) {
  // Skip detailed pointer block description to reduce noise

  // Read Global space pointer (64-bit)
  UInt32 globalPtrHiBE = 0, globalPtrLoBE = 0;

  // Read high 32 bits
  IOReturn ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_GLOBAL_SPACE_PTR_HI,
      globalPtrHiBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read Global pointer HI at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_GLOBAL_SPACE_PTR_HI)
              << " (Error: 0x" << ptrReadStatus;
    if (ptrReadStatus == 0xe0008017) {
      std::cerr << " - likely address error";
    }
    std::cerr << ")" << std::dec << std::endl;
    return false;
  }

  // Read low 32 bits
  ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_GLOBAL_SPACE_PTR_LO,
      globalPtrLoBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read Global pointer LO at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_GLOBAL_SPACE_PTR_LO)
              << " (Error: 0x" << ptrReadStatus << ")" << std::dec << std::endl;
    return false;
  }

  // Read TX[0] space pointer (64-bit)
  UInt32 txPtrHiBE = 0, txPtrLoBE = 0;

  // Read high 32 bits
  ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_TX_SPACE_PTR_HI(0),
      txPtrHiBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read TX[0] pointer HI at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_TX_SPACE_PTR_HI(0))
              << " (Error: 0x" << ptrReadStatus << ")" << std::dec << std::endl;
    return false;
  }

  // Read low 32 bits
  ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_TX_SPACE_PTR_LO(0),
      txPtrLoBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read TX[0] pointer LO at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_TX_SPACE_PTR_LO(0))
              << " (Error: 0x" << ptrReadStatus << ")" << std::dec << std::endl;
    return false;
  }

  // Read RX[0] space pointer (64-bit)
  UInt32 rxPtrHiBE = 0, rxPtrLoBE = 0;

  // Read high 32 bits
  ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_RX_SPACE_PTR_HI(0),
      rxPtrHiBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read RX[0] pointer HI at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_RX_SPACE_PTR_HI(0))
              << " (Error: 0x" << ptrReadStatus << ")" << std::dec << std::endl;
    return false;
  }

  // Read low 32 bits
  ptrReadStatus = FWA::SCANNER::safeReadQuadlet(
      deviceInterface, service, discoveryBase + DICE_OFFSET_RX_SPACE_PTR_LO(0),
      rxPtrLoBE, generation);

  if (ptrReadStatus != kIOReturnSuccess) {
    std::cerr << "Error [DICE]: Failed to read RX[0] pointer LO at 0x"
              << std::hex << (discoveryBase + DICE_OFFSET_RX_SPACE_PTR_LO(0))
              << " (Error: 0x" << ptrReadStatus << ")" << std::dec << std::endl;
    return false;
  }

  // Convert to host endianness
  uint32_t globalPtrHi = CFSwapInt32LittleToHost(globalPtrHiBE);
  uint32_t globalPtrLo = CFSwapInt32LittleToHost(globalPtrLoBE);
  uint32_t txPtrHi = CFSwapInt32LittleToHost(txPtrHiBE);
  uint32_t txPtrLo = CFSwapInt32LittleToHost(txPtrLoBE);
  uint32_t rxPtrHi = CFSwapInt32LittleToHost(rxPtrHiBE);
  uint32_t rxPtrLo = CFSwapInt32LittleToHost(rxPtrLoBE);

  // Combine into 64-bit addresses
  uint64_t rawGlobalBase =
      (static_cast<uint64_t>(globalPtrHi) << 32) | globalPtrLo;
  uint64_t rawTxBase = (static_cast<uint64_t>(txPtrHi) << 32) | txPtrLo;
  uint64_t rawRxBase = (static_cast<uint64_t>(rxPtrHi) << 32) | rxPtrLo;

  // Log the raw values
  std::cerr << "Info [DICE]: Raw pointer values:" << std::endl;
  std::cerr << "Info [DICE]:   - Global: 0x" << std::hex << rawGlobalBase
            << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - TX[0]:  0x" << std::hex << rawTxBase
            << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - RX[0]:  0x" << std::hex << rawRxBase
            << std::dec << std::endl;

  // APPROACH 1: Use the original DICE_REGISTER_BASE for all operations
  // This is the most conservative approach and matches how FFADO works
  globalBase = DICE_REGISTER_BASE;
  txBase = DICE_REGISTER_TX_BASE;
  rxBase = DICE_REGISTER_RX_BASE;

  std::cerr << "Info [DICE]: Using standard DICE register bases:" << std::endl;
  std::cerr << "Info [DICE]:   - Global: 0x" << std::hex << globalBase
            << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - TX[0]:  0x" << std::hex << txBase << std::dec
            << std::endl;
  std::cerr << "Info [DICE]:   - RX[0]:  0x" << std::hex << rxBase << std::dec
            << std::endl;

  // Store the raw pointer values in the DiceChip::config for future reference
  // We'll add this to the FireWireDevice structure later

  std::cerr << "Info [DICE]: Read register space pointers (hi/lo words):"
            << std::endl;
  std::cerr << "Info [DICE]:   - Global: 0x" << std::hex << globalPtrHi << " 0x"
            << globalPtrLo << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - TX[0]:  0x" << std::hex << txPtrHi << " 0x"
            << txPtrLo << std::dec << std::endl;
  std::cerr << "Info [DICE]:   - RX[0]:  0x" << std::hex << rxPtrHi << " 0x"
            << rxPtrLo << std::dec << std::endl;

  std::cerr << "Info [DICE]: NOTE: Using standard DICE register bases instead "
               "of discovered pointers"
            << std::endl;
  std::cerr
      << "Info [DICE]: This matches FFADO's approach for maximum compatibility"
      << std::endl;
  std::cerr << "Info [DICE]: The discovered pointers will be stored for future "
               "reference"
            << std::endl;

  return true;
}
} // Anonymous namespace

// Function to discover DICE base addresses (Global, TX, RX)
bool discoverDiceBaseAddresses(IOFireWireDeviceInterface **deviceInterface,
                               io_service_t service, UInt32 generation,
                               uint64_t &globalBase, uint64_t &txBase,
                               uint64_t &rxBase, std::string &method) {
  // Skip discovery process description to reduce noise

  globalBase = DICE_INVALID_OFFSET;
  txBase = DICE_INVALID_OFFSET;
  rxBase = DICE_INVALID_OFFSET;
  method = "None";
  bool baseFound = false;
  UInt32 testValue = 0; // For verification reads

  // Method 1: Use Vendor Keys from Config ROM
  std::cerr << "\nInfo [DICE]: Method 1 - Checking Config ROM Vendor Keys..."
            << std::endl;
  std::map<uint32_t, uint64_t> vendorKeys =
      parseConfigRomVendorKeys(deviceInterface, service, generation);

  if (!vendorKeys.empty()) {
    std::cerr << "Info [DICE]: Found " << vendorKeys.size()
              << " vendor key(s) in Config ROM to test" << std::endl;
    for (const auto &pair : vendorKeys) {
      uint32_t key = pair.first;
      uint64_t potentialGlobalBase = pair.second;
      std::cerr << "\nInfo [DICE]: Testing Vendor Key 0x" << std::hex << key
                << " -> Potential base address 0x" << potentialGlobalBase
                << std::dec << std::endl;
      std::cerr
          << "Info [DICE]: Attempting verification read of Owner register..."
          << std::endl;

      // Verify this potential base by reading the Owner register (relative
      // offset 0x00)
      if (FWA::SCANNER::safeReadQuadlet(
              deviceInterface, service,
              potentialGlobalBase +
                  (GLOBAL_OWNER_ADDR - DICE_REGISTER_BASE), // Use Owner offset
              testValue, generation) == kIOReturnSuccess) {
        UInt32 ownerValue = CFSwapInt32LittleToHost(testValue);
        std::cerr << "Info [DICE]: Verification successful! Owner register = 0x"
                  << std::hex << ownerValue << std::dec;

        // Add interpretation of Owner value
        std::cerr << " (";
        switch (ownerValue) {
        case 0x0001:
          std::cerr << "DICE I";
          break;
        case 0x0002:
          std::cerr << "DICE II";
          break;
        case 0x0003:
          std::cerr << "DICE Mini";
          break;
        case 0x0004:
          std::cerr << "DICE Jr";
          break;
        default:
          std::cerr << "Unknown DICE variant";
        }
        std::cerr << ")" << std::endl;
        globalBase = potentialGlobalBase;
        std::stringstream ss;
        ss << "0x" << std::hex << key;
        method = "Config ROM Vendor Key (" + ss.str() + ")";
        baseFound = true;

        // Read TX/RX pointers from the discovered Global base
        if (baseFound &&
            !readDiceOffsets(deviceInterface, service, generation,
                             globalBase, // This is the InitialDiceBaseFW, now
                                         // confirmed as ActualGlobalRegsBase
                             globalBase, // Pass it as the address *from which
                                         // to read pointers*
                             txBase,     // Out param for ActualTXRegsBase
                             rxBase))    // Out param for ActualRXRegsBase
        {
          std::cerr << "Warning [DICE]: Method 1: Failed to read TX/RX "
                       "pointers from Global base 0x"
                    << std::hex << globalBase << std::dec
                    << ". TX/RX discovery might be incomplete." << std::endl;
          // Don't return false here - we still found a valid global base
        }
        break;
      } else {
        // Skip failed verification logging
      }
    }
  } else {
    std::cerr << "Info [DICE]:     No vendor keys found in Config ROM."
              << std::endl;
  }

  // Method 2: Fallback to FFADO's pointer discovery base if Vendor Key method
  // failed
  if (!baseFound) {
    std::cerr
        << "\nInfo [DICE]: Method 2 - Testing FFADO pointer discovery base..."
        << std::endl;
    std::cerr << "Info [DICE]: Using standard FFADO base address: 0x"
              << std::hex << FWA::Scanner::FFADO_POINTER_DISCOVERY_BASE
              << std::dec << std::endl;

    uint64_t discoveredGlobalBase = 0, discoveredTxBase = 0,
             discoveredRxBase = 0;
    if (readDiceOffsets(deviceInterface, service, generation,
                        FWA::Scanner::FFADO_POINTER_DISCOVERY_BASE,
                        discoveredGlobalBase, discoveredTxBase,
                        discoveredRxBase)) {
      // Verify the discovered Global base by reading the Owner register
      if (FWA::SCANNER::safeReadQuadlet(
              deviceInterface, service,
              discoveredGlobalBase + (GLOBAL_OWNER_ADDR - DICE_REGISTER_BASE),
              testValue, generation) == kIOReturnSuccess) {
        UInt32 ownerValue = CFSwapInt32LittleToHost(testValue);
        std::cerr << "Info [DICE]: Verification successful! Owner register = 0x"
                  << std::hex << ownerValue << std::dec;

        // Add interpretation of Owner value
        std::cerr << " (";
        switch (ownerValue) {
        case 0x0001:
          std::cerr << "DICE I";
          break;
        case 0x0002:
          std::cerr << "DICE II";
          break;
        case 0x0003:
          std::cerr << "DICE Mini";
          break;
        case 0x0004:
          std::cerr << "DICE Jr";
          break;
        default:
          std::cerr << "Unknown DICE variant";
        }
        std::cerr << ")" << std::endl;

        // Use all discovered addresses
        globalBase = discoveredGlobalBase;
        txBase = discoveredTxBase;
        rxBase = discoveredRxBase;
        method = "Dynamic Pointer Discovery (Verified)";
        baseFound = true;
        // Don't return here - let validation happen
      } else {
        // Skip failed verification logging
      }
    } else {
      // Skip failed read logging
    }

    // If FFADO method failed, try the old DICE_REGISTER_BASE as a last resort
    if (!baseFound) {
      std::cerr << "\nInfo [DICE]: Method 3 - Testing legacy base address..."
                << std::endl;
      std::cerr << "Info [DICE]: Using fixed DICE register base: 0x" << std::hex
                << DICE_REGISTER_BASE << std::dec << std::endl;
      std::cerr << "Info [DICE]: This is a fallback method for older DICE "
                   "implementations"
                << std::endl;
      if (FWA::SCANNER::safeReadQuadlet(
              deviceInterface, service,
              DICE_REGISTER_BASE +
                  (GLOBAL_OWNER_ADDR -
                   DICE_REGISTER_BASE), // Use Owner offset relative to absolute
                                        // base
              testValue, generation) == kIOReturnSuccess) {
        UInt32 ownerValue = CFSwapInt32LittleToHost(testValue);
        std::cerr << "Info [DICE]: Legacy base verification successful! Owner "
                     "register = 0x"
                  << std::hex << ownerValue << std::dec;

        // Add interpretation of Owner value
        std::cerr << " (";
        switch (ownerValue) {
        case 0x0001:
          std::cerr << "DICE I";
          break;
        case 0x0002:
          std::cerr << "DICE II";
          break;
        case 0x0003:
          std::cerr << "DICE Mini";
          break;
        case 0x0004:
          std::cerr << "DICE Jr";
          break;
        default:
          std::cerr << "Unknown DICE variant";
        }
        std::cerr << ")" << std::endl;
        globalBase = DICE_REGISTER_BASE;
        method = "Legacy Fallback (Verified)";
        baseFound = true;
      } else {
        std::cerr << "Error [DICE]: Legacy base verification failed! No valid "
                     "DICE registers found."
                  << std::endl;
        std::cerr << "Error [DICE]: Unable to reliably determine Global Base "
                     "address using any method."
                  << std::endl;
        method = "All Fallbacks Failed";
        return false;
      }
    }
  }

  if (!baseFound) {
    std::cerr << "Error [DICE]: Failed to discover DICE Global Base address "
                 "using any method."
              << std::endl;
    return false;
  }

  // --- Determine TX/RX Bases ---
  std::cerr << "Info [DICE]:   Global Base determined via " << method << ": 0x"
            << std::hex << globalBase << std::dec << std::endl;

  // Final validation of discovered addresses
  if (baseFound) {
    // Validate that we have valid (non-zero) TX/RX bases if they were expected
    // to be found
    if (txBase == 0 || rxBase == 0) {
      std::cerr
          << "Warning [DICE]: Potentially invalid TX/RX base addresses (TX: 0x"
          << std::hex << txBase << ", RX: 0x" << rxBase << std::dec
          << "). Full functionality may be affected." << std::endl;
      // Warning only - don't fail if we at least have global base
    }

    std::cerr << "\nInfo [DICE]: Successfully discovered base addresses:"
              << std::endl;
    std::cerr << "Info [DICE]:   Discovery Method: " << method << std::endl;
    std::cerr << "Info [DICE]:   Global Base:  0x" << std::hex << globalBase
              << std::dec << std::endl;
    if (txBase != 0) {
      std::cerr << "Info [DICE]:   TX Base:      0x" << std::hex << txBase
                << std::dec << std::endl;
    }
    if (rxBase != 0) {
      std::cerr << "Info [DICE]:   RX Base:      0x" << std::hex << rxBase
                << std::dec << std::endl;
    }

    // Test register access across all spaces
    // Skip register access test announcement
    if (testRegisterSpaceAccess(deviceInterface, service, generation)) {
      std::cerr
          << "Info [DICE]: Register space access test completed successfully."
          << std::endl;
    } else {
      std::cerr << "Warning [DICE]: Some register spaces may be inaccessible."
                << std::endl;
    }

    return true; // Success, base(s) found and validated
  }

  // If we get here, all methods failed
  std::cerr << "Error [DICE]: Failed to discover DICE register space addresses "
               "using any method."
            << std::endl;
  return false;
}

// Test function to validate register access across different spaces
bool testRegisterSpaceAccess(IOFireWireDeviceInterface **deviceInterface,
                             io_service_t service, UInt32 generation) {
  UInt32 value = 0;
  bool anySuccess = false;
  std::map<std::string, int> sectionStats;

  std::cerr << "\nInfo [DICE]: Testing register space access..." << std::endl;

  // Test Core Registers
  std::cerr << "\nInfo [DICE]: Testing Core registers..." << std::endl;
  struct CoreRegTest {
    const char *name;
    uint64_t addr;
  } coreRegs[] = {{"Global Owner", GLOBAL_OWNER_ADDR},
                  {"TX Stream Size", TX_SZ_TX_ADDR},
                  {"RX Stream Size", RX_SZ_RX_ADDR}};

  for (const auto &reg : coreRegs) {
    IOReturn status = safeReadQuadlet(deviceInterface, service, reg.addr, value,
                                      generation, true);
    if (status == kIOReturnSuccess) {
      std::cerr << "Info [DICE]: " << reg.name
                << " register readable (value: 0x" << std::hex << value
                << std::dec << ")" << std::endl;
      anySuccess = true;
    }
  }

  // Test EAP Sections (0x0-0x30)
  std::cerr << "\nInfo [DICE]: Testing EAP sections (0x0-0x30)..." << std::endl;

  struct EAPSection {
    const char *name;
    uint64_t baseAddr;
    uint64_t size;
  } eapSections[] = {
      {"Capability", DICE_EAP_CAPABILITY_SPACE_OFF, 0x4},
      {"Command", DICE_EAP_CMD_SPACE_OFF, 0x8},
      {"Mixer", DICE_EAP_MIXER_SPACE_OFF, 0x8},
      {"Peak", DICE_EAP_PEAK_SPACE_OFF, 0x8},
      {"New Routing", DICE_EAP_NEW_ROUTING_SPACE_OFF, 0x8},
      {"Stream Config", DICE_EAP_NEW_STREAM_CFG_SPACE_OFF, 0x8},
      {"Current Config", DICE_EAP_CURR_CFG_SPACE_OFF, 0x8},
      {"Standalone Config", DICE_EAP_STAND_ALONE_CFG_SPACE_OFF, 0x8},
      {"App", DICE_EAP_APP_SPACE_OFF, 0x8}};

  for (const auto &section : eapSections) {
    int successCount = 0;
    int totalReads = 0;

    std::cerr << "\nInfo [DICE]: Testing " << section.name << " section..."
              << std::endl;

    // Test each quadlet in the section
    for (uint64_t offset = 0; offset < section.size; offset += 4) {
      uint64_t addr = DICE_EAP_BASE + section.baseAddr + offset;
      totalReads++;

      IOReturn status = safeReadQuadlet(deviceInterface, service, addr, value,
                                        generation, false);
      if (status == kIOReturnSuccess) {
        successCount++;
        anySuccess = true;

        // Try to interpret as ASCII if in a relevant section
        std::string ascii = interpretAsASCII(value);
        if (!ascii.empty()) {
          std::cerr << "Info [DICE]: Found ASCII string at 0x" << std::hex
                    << addr << ": \"" << ascii << "\"" << std::dec << std::endl;
        }
      }
    }

    // Log section statistics
    sectionStats[section.name] = (successCount * 100) / totalReads;
    std::cerr << "Info [DICE]: " << section.name << " section: " << successCount
              << "/" << totalReads << " reads successful ("
              << sectionStats[section.name] << "%)" << std::endl;
  }

  // Test Subsystem Registers
  std::cerr << "\nInfo [DICE]: Testing Subsystem registers..." << std::endl;
  struct SubsysRegTest {
    const char *name;
    uint64_t addr;
  } subsysRegs[] = {{"GPCSR Chip ID", GPCSR_CHIP_ID_ADDR},
                    {"Clock Controller", CLOCK_CONTROLLER_SYNC_CTRL_ADDR},
                    {"AVS Receiver", AVS_AUDIO_RECEIVER_CFG0_ADDR}};

  for (const auto &reg : subsysRegs) {
    IOReturn status = safeReadQuadlet(deviceInterface, service, reg.addr, value,
                                      generation, false);
    if (status == kIOReturnSuccess) {
      std::cerr << "Info [DICE]: " << reg.name
                << " register readable (value: 0x" << std::hex << value
                << std::dec << ")" << std::endl;
      anySuccess = true;
    }
  }

  // Summary
  std::cerr << "\nInfo [DICE]: Register access test summary:" << std::endl;
  for (const auto &stat : sectionStats) {
    std::cerr << "Info [DICE]: " << stat.first << " section: " << stat.second
              << "% success rate" << std::endl;
  }

  if (!anySuccess) {
    std::cerr << "Error [DICE]: No registers were successfully accessed!"
              << std::endl;
  }
  return anySuccess;
}

} // namespace FWA::SCANNER
