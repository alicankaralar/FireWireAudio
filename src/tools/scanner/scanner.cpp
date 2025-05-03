#include "scanner.hpp" // Include the header for the class definition

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip> // For std::hex, std::setw, std::setfill

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h> // For kIOFireWireLibTypeID etc. (FWStandardHeaders.h is deprecated/moved)

namespace FWA::SCANNER
{

	// --- FireWireScanner Class Implementation ---

	FireWireScanner::FireWireScanner() : masterPort_(kIOMainPortDefault)
	{
		// kIOMainPortDefault doesn't return an error code to check here
		std::cerr << "Debug [Scanner]: FireWireScanner object created." << std::endl;
	}

	FireWireScanner::~FireWireScanner()
	{
		// No deallocation needed for kIOMainPortDefault
		std::cerr << "Debug [Scanner]: FireWireScanner object destroyed." << std::endl;
	}

	std::vector<FireWireDevice> FireWireScanner::scanDevices()
	{
		std::cerr << "Debug [Scanner]: Starting scanDevices()..." << std::endl;
		std::vector<FireWireDevice> devices;

		CFMutableDictionaryRef matchingDict = IOServiceMatching("IOFireWireDevice");
		if (!matchingDict)
		{
			std::cerr << "Error [Scanner]: IOServiceMatching failed to create dictionary." << std::endl;
			return devices;
		}
		std::cerr << "Debug [Scanner]: Created matching dictionary: " << matchingDict << std::endl;

		io_iterator_t deviceIterator = 0;
		kern_return_t result = KERN_FAILURE;

		std::cerr << "Debug [Scanner]: Calling IOServiceGetMatchingServices..." << std::endl;
		result = IOServiceGetMatchingServices(masterPort_, matchingDict, &deviceIterator);
		// Note: matchingDict is consumed by IOServiceGetMatchingServices on success
		std::cerr << "Debug [Scanner]: IOServiceGetMatchingServices result: " << result << ", iterator: " << deviceIterator << std::endl;

		if (result != KERN_SUCCESS)
		{
			std::cerr << "Error [Scanner]: Failed to get matching services: " << result << std::endl;
			CFRelease(matchingDict); // Release dict on failure
			return devices;
		}
		if (deviceIterator == 0)
		{
			std::cerr << "Warning [Scanner]: IOServiceGetMatchingServices succeeded but returned a null iterator." << std::endl;
			// Assuming consumed even if iterator is null
			return devices;
		}
		// If successful, matchingDict is consumed.

		io_service_t device = 0;
		int deviceCount = 0;
		std::cerr << "Debug [Scanner]: Entering device iteration loop..." << std::endl;
		while ((device = IOIteratorNext(deviceIterator)) != 0)
		{
			deviceCount++;
			std::cerr << "Debug [Scanner]: IOIteratorNext returned device #" << deviceCount << " (service: " << device << ")" << std::endl;

			// Call helper function from io_helpers.cpp (via scanner.hpp)
			FireWireDevice fwDevice = FWA::SCANNER::getDeviceInfo(device);
			std::cerr << "Debug [Scanner]: Got device info for GUID: 0x" << std::hex << fwDevice.guid << std::dec << " Name: " << fwDevice.name << std::endl;

			// --- Interface Handling ---
			IOCFPlugInInterface **plugInInterface = nullptr;
			IOFireWireDeviceInterface **deviceInterface = nullptr;
			SInt32 score;
			IOReturn pluginResult = kIOReturnError;

			std::cerr << "Debug [Scanner]: Calling IOCreatePlugInInterfaceForService..." << std::endl;
			pluginResult = IOCreatePlugInInterfaceForService(
					device, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
			std::cerr << "Debug [Scanner]: IOCreatePlugInInterfaceForService result: " << pluginResult << ", plugInInterface: " << plugInInterface << std::endl;

			if (pluginResult == kIOReturnSuccess && plugInInterface)
			{
				std::cerr << "Debug [Scanner]: Calling plugInInterface->QueryInterface..." << std::endl;
				HRESULT hr = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), (void **)&deviceInterface);
				std::cerr << "Debug [Scanner]: QueryInterface result (HRESULT): " << hr << ", deviceInterface: " << deviceInterface << std::endl;

				std::cerr << "Debug [Scanner]: Releasing plugInInterface..." << std::endl;
				(*plugInInterface)->Release(plugInInterface);
				plugInInterface = nullptr;
				std::cerr << "Debug [Scanner]: Released plugInInterface." << std::endl;

				if (hr == S_OK && deviceInterface)
				{
					std::cerr << "Debug [Scanner]: QueryInterface succeeded. Checking *deviceInterface..." << std::endl;
					if (*deviceInterface)
					{
						std::cerr << "Debug [Scanner]: Opening device interface..." << std::endl;
						IOReturn openResult = (*deviceInterface)->Open(deviceInterface);
						std::cerr << "Debug [Scanner]: Open result: " << openResult << std::endl;

						if (openResult == kIOReturnSuccess)
						{
							std::cerr << "Debug [Scanner]: Attempting to read DICE registers..." << std::endl;
							// Call helper function from dice_helpers.cpp (via scanner.hpp)
							FWA::SCANNER::readDiceRegisters(deviceInterface, device, fwDevice);
							std::cerr << "Debug [Scanner]: readDiceRegisters returned." << std::endl;

							std::cerr << "Debug [Scanner]: Closing device interface..." << std::endl;
							(*deviceInterface)->Close(deviceInterface);
							std::cerr << "Debug [Scanner]: Closed device interface." << std::endl;
						}
						else
						{
							std::cerr << "Error [Scanner]: Failed to open device interface for GUID: 0x" << std::hex << fwDevice.guid << std::dec << " (IOReturn: " << openResult << ")" << std::endl;
						}
					}
					else
					{
						std::cerr << "Error [Scanner]: *deviceInterface pointer is null after successful QueryInterface for GUID: 0x" << std::hex << fwDevice.guid << std::dec << std::endl;
					}

					std::cerr << "Debug [Scanner]: Releasing deviceInterface..." << std::endl;
					(*deviceInterface)->Release(deviceInterface);
					deviceInterface = nullptr;
					std::cerr << "Debug [Scanner]: Released deviceInterface." << std::endl;
				}
				else
				{
					std::cerr << "Warning [Scanner]: Failed to query IOFireWireDeviceInterface for GUID: 0x" << std::hex << fwDevice.guid << std::dec << " (HRESULT: " << hr << ")" << std::endl;
					if (deviceInterface)
					{
						std::cerr << "Warning [Scanner]: Releasing potentially invalid deviceInterface after failed QueryInterface..." << std::endl;
						(*deviceInterface)->Release(deviceInterface);
						deviceInterface = nullptr;
					}
				}
			}
			else
			{
				std::cerr << "Warning [Scanner]: Failed to create plugin interface for GUID: 0x" << std::hex << fwDevice.guid << std::dec << " (IOReturn: " << pluginResult << ")" << std::endl;
			}

			devices.push_back(fwDevice);
			IOObjectRelease(device);
			std::cerr << "Debug [Scanner]: Released device service #" << deviceCount << std::endl;
		} // End while loop
		std::cerr << "Debug [Scanner]: Exited device loop." << std::endl;

		IOObjectRelease(deviceIterator);
		std::cerr << "Debug [Scanner]: Released device iterator." << std::endl;
		return devices;
	}

} // namespace FWA::SCANNER