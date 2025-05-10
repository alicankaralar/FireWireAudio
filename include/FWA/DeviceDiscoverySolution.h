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

#include "FWA/dice/DiceDefines.hpp" // Include DICE enums, bitmasks, etc.
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h> // Added include for FireWire types
#include <string>                         // For std::string in error messages
#include <vector>

// Define FireWire Configuration ROM CSR keys if not already defined
#ifndef kFWVendorID
#define kFWVendorID 0x03
#endif
#ifndef kFWModelID
#define kFWModelID 0x17
#endif

namespace FWA
{
    namespace DeviceDiscoverySolution
    {
        // Use this device matching dictionary instead of specifically looking for AVC
        // units
        inline CFMutableDictionaryRef CreateDeviceMatchingDictionary()
        {
            // Create a dictionary that will match any FireWire device
            CFMutableDictionaryRef matchingDict = IOServiceMatching("IOFireWireDevice");

            if (!matchingDict)
            {
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

        // Helper method to determine if a FireWire device is a DICE device by checking its Configuration ROM
        inline bool IsDiceDevice(IOFireWireDeviceInterface **deviceInterface)
        {
            if (!deviceInterface || !*deviceInterface)
            {
                return false;
            }

            // Get the config directory interface instead of using CopyConfigDirectory
            IOFireWireLibConfigDirectoryRef configDir = nullptr;
            configDir = (*deviceInterface)->GetConfigDirectory(deviceInterface, CFUUIDGetUUIDBytes(kIOFireWireConfigDirectoryInterfaceID));

            if (!configDir)
            {
                return false;
            }

            bool isDice = false;

            // Static list of known DICE (VendorID, ModelID) pairs from libffado configuration
            static const std::vector<std::pair<SInt32, SInt32>> knownDiceDevices = {
                {0x000ff2, 0x000006},     // Loud Technologies Inc. / Onyx 1640i (DICE)
                {0x000ff2, 0x000007},     // Loud Technologies Inc. / Onyx Blackbird
                {0x000a92, 0x00000008},   // Presonus / Firestudio 26x26
                {0x000a92, 0x0000000b},   // Presonus / Firestudio Project
                {0x000a92, 0x0000000c},   // Presonus / Firestudio Tube
                {0x000a92, 0x00000011},   // PreSonus / Firestudio Mobile
                {0x000a92, 0x00000012},   // PreSonus / STUDIOLIVE_2442
                {0x000a92, 0x00000013},   // PreSonus / STUDIOLIVE_1602
                {0x000a92, 0x00000014},   // PreSonus / STUDIOLIVE_3242_mk2
                {0x000d6c, 0x00000010},   // M-Audio / ProFire 2626
                {0x000d6c, 0x00000011},   // M-Audio / ProFire 610
                {0x000166, 0x0001},       // TCAT / DiceII EVM (1)
                {0x000166, 0x0002},       // TCAT / DiceII EVM (2)
                {0x000166, 0x0004},       // TCAT / DiceII EVM (4)
                {0x000166, 0x00000020},   // TC Electronic / Konnekt 24D
                {0x000166, 0x00000021},   // TC Electronic / Konnekt 8
                {0x000166, 0x00000022},   // TC Electronic / Studio Konnekt 48
                {0x000166, 0x00000023},   // TC Electronic / Konnekt Live
                {0x000166, 0x00000024},   // TC Electronic / Desktop Konnekt 6
                {0x000166, 0x00000027},   // TC Electronic / ImpactTwin
                {0x000595, 0x00000001},   // Alesis / io|14
                {0x000595, 0x00000000},   // Alesis / MultiMix-12 / MultiMix-16 FireWire
                {0x000a92, 0x00000010},   // PreSonus / STUDIOLIVE_1642
                {0x00130e, 0x00000005},   // Focusrite / Saffire PRO 40
                {0x00130e, 0x00000007},   // Focusrite / Saffire PRO 24
                {0x00130e, 0x00000008},   // Focusrite / Saffire PRO 24 DSP
                {0x00130e, 0x00000009},   // Focusrite / Saffire PRO 14
                {0x00130e, 0x00000012},   // Focusrite / Saffire PRO 26
                {0x00130e, 0x000000DE},   // Focusrite / Saffire PRO 40-1
                {0x00130e, 0x00000013},   // Focusrite / Saffire PRO 40-1 (alt ID)
                {0x001C6A, 0x00000001},   // Weiss Engineering Ltd. / ADC 2
                {0x001C6A, 0x00000002},   // Weiss Engineering Ltd. / Vesta
                {0x001C6A, 0x00000003},   // Weiss Engineering Ltd. / Minerva
                {0x001C6A, 0x00000004},   // Weiss Engineering Ltd. / AFI 1
                {0x001C6A, 0x00000005},   // Weiss Engineering Ltd. / TAG DAC1
                {0x001C6A, 0x00000006},   // Weiss Engineering Ltd. / INT 202
                {0x001C6A, 0x00000007},   // Weiss Engineering Ltd. / DAC 202
                {0x001c2d, 0x00000001},   // FlexRadio_Systems / Flex-5000
                {0x00000F64, 0x00000003}, // DnR / Axum_FireWire_IO_card_16x16
                {0x00000FD7, 0x00000001}, // Lexicon / I-ONIX_FW810S
                {0x000004C4, 0x00000000}, // Allen and Heath / Zed R16
                {0x0010C73F, 0x00000001}  // Midas / Venice F32
            };

            // Read the vendor ID and model ID using IOFireWireConfigDirectoryInterface methods
            UInt32 vendorId = 0;
            UInt32 modelId = 0;

            IOReturn status = (*configDir)->GetKeyValue_UInt32(configDir, kFWVendorID, &vendorId, nullptr);
            if (status == kIOReturnSuccess)
            {
                status = (*configDir)->GetKeyValue_UInt32(configDir, kFWModelID, &modelId, nullptr);

                if (status == kIOReturnSuccess)
                {
                    // Check against our list of known DICE devices
                    for (const auto &knownDevice : knownDiceDevices)
                    {
                        if (vendorId == knownDevice.first && modelId == knownDevice.second)
                        {
                            isDice = true;
                            break;
                        }
                    }
                }
            }

            // Release the interface
            (*configDir)->Release(configDir);
            return isDice;
        }

    } // namespace DeviceDiscoverySolution
} // namespace FWA
