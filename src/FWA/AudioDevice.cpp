#include "FWA/AudioDevice.h"
#include "FWA/CommandInterface.h"
#include "FWA/DeviceController.h"
#include "FWA/DeviceParser.hpp"
#include "FWA/Enums.hpp"
#include "FWA/Helpers.h"             // For cfStringToString
#include <CoreFoundation/CFNumber.h> // For CFNumber*
#include <IOKit/IOMessage.h>
#include <format>
#include <memory> // Required for std::make_shared
#include <spdlog/spdlog.h>

namespace FWA {

AudioDevice::AudioDevice(std::uint64_t guid, const std::string &deviceName,
                         const std::string &vendorName,
                         io_service_t fwDeviceService, // Changed parameter name
                         DeviceController *deviceController)
    : guid_(guid), deviceName_(deviceName), vendorName_(vendorName),
      deviceController_(deviceController),
      fwDevice_(fwDeviceService), // Initialize fwDevice_ directly
      busController_(0), notificationPort_(nullptr), interestNotification_(0),
      deviceInterface(nullptr), avcInterface_(nullptr),
      avcUnit_(0),              // Initialize re-added member
      hasAvcCapability_(false), // Initialize AVC capability flag
      hasDICESupport_(false)    // Initialize DICE support flag
{
  spdlog::info("AudioDevice::AudioDevice - Creating device with GUID: 0x{:x}",
               guid);

  if (fwDevice_) { // Retain the passed fwDeviceService
    IOObjectRetain(fwDevice_);
  }
  // Do NOT call shared_from_this() here!
}

AudioDevice::~AudioDevice() {
  if (interestNotification_) {
    IOObjectRelease(interestNotification_);
    interestNotification_ = 0;
  }
  if (notificationPort_) {
    IONotificationPortDestroy(notificationPort_);
    notificationPort_ = nullptr;
  }
  if (deviceInterface) {
    (*deviceInterface)->Close(deviceInterface);
    (*deviceInterface)->Release(deviceInterface);
    deviceInterface = nullptr;
  }
  if (avcInterface_) {
    avcInterface_ = nullptr;
  }
  if (busController_) {
    IOObjectRelease(busController_);
    busController_ = 0;
  }
  if (fwDevice_) {
    IOObjectRelease(fwDevice_);
    fwDevice_ = 0;
  }

  spdlog::debug("AudioDevice::~AudioDevice - Destroyed device GUID: 0x{:x}",
                guid_);
}

std::expected<void, IOKitError> AudioDevice::init() {
  spdlog::debug("AudioDevice::init - Initializing device GUID: 0x{:x}", guid_);
  if (!fwDevice_) { // Check fwDevice_ passed from constructor
    spdlog::error("AudioDevice::init: Cannot initialize without a valid "
                  "fwDevice service.");
    return std::unexpected(IOKitError(kIOReturnNotAttached));
  }

  // Get bus controller directly from fwDevice_
  IOReturn result = IORegistryEntryGetParentEntry(fwDevice_, kIOServicePlane,
                                                  &busController_);
  if (result != kIOReturnSuccess || !busController_) {
    spdlog::error(
        "AudioDevice::init: Failed to get busController from fwDevice_: 0x{:x}",
        result);
    return std::unexpected(static_cast<IOKitError>(
        result != kIOReturnSuccess ? result : kIOReturnNotFound));
  }
  spdlog::debug("AudioDevice::init: Got busController_ service: {:#x}",
                busController_);

  // ---- Call the new method to read Vendor/Model Info ----
  IOReturn infoResult = readVendorAndModelInfo();
  if (infoResult != kIOReturnSuccess) {
    spdlog::warn("AudioDevice::init: Failed to read vendor/model info: 0x{:x}. "
                 "Continuing initialization.",
                 infoResult);
  } else {
    spdlog::info("AudioDevice::init: Read VendorID=0x{:08X}, ModelID=0x{:08X}, "
                 "VendorName='{}'",
                 vendorID_, modelID_, vendorName_);
  }
  // ---------------------------------------------------------

  // Detect DICE support
  detectDICESupport();

  if (hasDICESupport_) {
    spdlog::info("AudioDevice::init: DICE device detected");
  }

  // --- Attempt to find the associated AVC Unit service ---
  avcUnit_ = 0; // Ensure it's null initially
  io_iterator_t childIterator = 0;
  result = IORegistryEntryGetChildIterator(fwDevice_, kIOServicePlane,
                                           &childIterator);
  if (result == kIOReturnSuccess && childIterator != 0) {
    io_service_t childService;
    while ((childService = IOIteratorNext(childIterator)) != 0) {
      // Check if the child conforms to IOFireWireAVCUnit
      if (IOObjectConformsTo(childService, "IOFireWireAVCUnit")) {
        spdlog::debug(
            "AudioDevice::init: Found IOFireWireAVCUnit child service: {:#x}",
            childService);
        avcUnit_ = childService;  // Found it!
        IOObjectRetain(avcUnit_); // Retain the found service
        break;                    // Stop searching once found
      }
      IOObjectRelease(childService); // Release non-matching child
    }
    IOObjectRelease(childIterator); // Release iterator
  }
  if (avcUnit_ == 0) {
    // If no child was found, check if fwDevice_ itself is the AVC Unit
    if (IOObjectConformsTo(fwDevice_, "IOFireWireAVCUnit")) {
      spdlog::debug("AudioDevice::init: fwDevice_ ({:#x}) itself conforms to "
                    "IOFireWireAVCUnit.",
                    fwDevice_);
      avcUnit_ = fwDevice_;     // Use fwDevice_ itself as the AVC Unit
      IOObjectRetain(avcUnit_); // Retain it since we are assigning it
    } else {
      spdlog::info("AudioDevice::init: No AVC unit found for device. "
                   "AVC functionality will be disabled, but basic FireWire "
                   "functionality will still be available.");
      // Continue without AVC - device may have basic FireWire capabilities
      hasAvcCapability_ = false;
    }
  } else {
    // We found an AVC unit
    hasAvcCapability_ = true;
  }
  // --- End AVC Unit search ---

  // 2. Create the notification port.
  notificationPort_ = IONotificationPortCreate(kIOMainPortDefault);
  if (!notificationPort_) {
    spdlog::error("AudioDevice::init: Failed to create notification port");
    return std::unexpected(IOKitError::Error);
  }

  // create interfaces
  result = createFWDeviceInterface(); // Uses fwDevice_ internally now

  if (result != kIOReturnSuccess) {
    spdlog::error(
        "AudioDevice::init: Failed to create FW device interface: 0x{:x}",
        result);
    // Consider returning error here? Or allow proceeding without
    // deviceInterface? For now, log and continue, CommandInterface creation
    // might still work if avcUnit_ was found.
  }

  /* Keep commented out
  CFRunLoopAddSource(CFRunLoopGetCurrent(),
                     IONotificationPortGetRunLoopSource(notificationPort_),
                     kCFRunLoopDefaultMode);
  */

  // Do not discover capabilities for now // Keep commented out

  // 3. Now safely create the CommandInterface using shared_from_this().
  //    This will use the avcUnit_ found (or not found) above.
  try {
    if (hasAvcCapability_) {
      commandInterface_ =
          std::make_shared<CommandInterface>(shared_from_this());
      spdlog::debug("AudioDevice::init: CommandInterface created.");
      auto activationResult = commandInterface_->activate();
      if (!activationResult &&
          activationResult.error() != IOKitError::StillOpen) {
        spdlog::error("AudioDevice::init: Failed to activate CommandInterface "
                      "before parsing: 0x{:x}",
                      static_cast<int>(activationResult.error()));
        commandInterface_.reset();
        // Non-fatal error - continue with limited functionality
        spdlog::warn(
            "AudioDevice::init: Continuing with limited AVC functionality.");
      } else {
        spdlog::debug("AudioDevice::init: CommandInterface activated (or was "
                      "already active).");
      }
    } else {
      spdlog::info("AudioDevice::init: Skipping CommandInterface creation for "
                   "non-AVC device.");
    }
  } catch (const std::bad_weak_ptr &e) {
    spdlog::critical("AudioDevice::init: Failed to create CommandInterface. "
                     "Was AudioDevice created using "
                     "std::make_shared? Exception: "
                     "{}",
                     e.what());
    // Non-fatal error - we'll continue without AVC functionality
    hasAvcCapability_ = false;
    spdlog::warn("AudioDevice::init: Continuing without AVC functionality due "
                 "to error.");
  }

  // 4. Discover device capabilities using DeviceParser.
  spdlog::info("AudioDevice::init: Starting device capability discovery...");
  if (hasAvcCapability_ && commandInterface_) {
    auto parser = std::make_shared<DeviceParser>(this);
    auto parseResult = parser->parse();
    if (!parseResult) {
      spdlog::error(
          "AudioDevice::init: Device capability parsing failed: 0x{:x}",
          static_cast<int>(parseResult.error()));
      // Non-fatal error - device might still be usable with limited
      // functionality
    } else {
      spdlog::info("AudioDevice::init: Device capability parsing completed "
                   "successfully.");
    }
  } else {
    spdlog::info(
        "AudioDevice::init: Skipping capability discovery for non-AVC device.");
    // Initialize with basic capabilities for non-AVC devices
    initializeBasicCapabilities();
  }

  spdlog::info("AudioDevice::init completed successfully for GUID: 0x{:x}",
               guid_);
  return {};
}

IOReturn AudioDevice::createFWDeviceInterface() {
  // Local Vars
  IOCFPlugInInterface **theCFPlugInInterface;
  SInt32 theScore;
  IOReturn result = kIOReturnSuccess;

  result = IOCreatePlugInInterfaceForService(
      fwDevice_, kIOFireWireLibTypeID,
      kIOCFPlugInInterfaceID, // interfaceType,
      &theCFPlugInInterface, &theScore);
  if (!result) {
    HRESULT comErr;
    comErr = (*theCFPlugInInterface)
                 ->QueryInterface(theCFPlugInInterface,
                                  CFUUIDGetUUIDBytes(kIOFireWireNubInterfaceID),
                                  (void **)&deviceInterface);
    if (comErr == S_OK) {
      result = (*deviceInterface)
                   ->AddCallbackDispatcherToRunLoop(
                       deviceInterface, deviceController_->getRunLoopRef());
    } else
      result = comErr;

    (*theCFPlugInInterface)
        ->Release(theCFPlugInInterface); // Leave just one reference.

    // Open the interface
    if (!result) {
      // If the avc interface is already open, use it's session ref to open the
      // device interface
      if (avcInterface_)
        result = (*deviceInterface)
                     ->OpenWithSessionRef(
                         deviceInterface,
                         (*avcInterface_)->getSessionRef(avcInterface_));
      else
        result = (*deviceInterface)->Open(deviceInterface);
      if (result) {
        (*deviceInterface)->Release(deviceInterface);
        deviceInterface = 0;
      }
    }
  }

  return result;
}

IOReturn AudioDevice::readVendorAndModelInfo() {
  if (!fwDevice_) { // Use fwDevice_ instead of fwUnit_
    spdlog::error("AudioDevice::readVendorAndModelInfo: fwDevice_ is null.");
    return kIOReturnNotReady;
  }
  CFMutableDictionaryRef properties = nullptr;
  IOReturn result = IORegistryEntryCreateCFProperties(
      fwDevice_, &properties, kCFAllocatorDefault,
      kNilOptions); // Use fwDevice_
  if (result != kIOReturnSuccess) {
    spdlog::error("AudioDevice::readVendorAndModelInfo: "
                  "IORegistryEntryCreateCFProperties failed: 0x{:x}",
                  result);
    return result;
  }
  if (!properties) {
    spdlog::error("AudioDevice::readVendorAndModelInfo: "
                  "IORegistryEntryCreateCFProperties succeeded but returned "
                  "null properties.");
    return kIOReturnNotFound;
  }
  // Get Vendor ID
  CFNumberRef unitVendorIDRef =
      (CFNumberRef)CFDictionaryGetValue(properties, CFSTR("Vendor_ID"));
  if (unitVendorIDRef) {
    if (CFGetTypeID(unitVendorIDRef) == CFNumberGetTypeID()) {
      if (!CFNumberGetValue(unitVendorIDRef, kCFNumberLongType, &vendorID_)) {
        spdlog::warn("AudioDevice::readVendorAndModelInfo: CFNumberGetValue "
                     "failed for Vendor_ID.");
      }
      spdlog::debug(
          "AudioDevice::readVendorAndModelInfo: Found Vendor ID: 0x{:08X}",
          vendorID_);
    } else {
      spdlog::warn("AudioDevice::readVendorAndModelInfo: Vendor_ID property is "
                   "not a CFNumber.");
    }
  } else {
    spdlog::warn(
        "AudioDevice::readVendorAndModelInfo: Vendor_ID property not found.");
  }
  // Get Model ID
  CFNumberRef unitModelIDRef =
      (CFNumberRef)CFDictionaryGetValue(properties, CFSTR("Model_ID"));
  if (unitModelIDRef) {
    if (CFGetTypeID(unitModelIDRef) == CFNumberGetTypeID()) {
      if (!CFNumberGetValue(unitModelIDRef, kCFNumberLongType, &modelID_)) {
        spdlog::warn("AudioDevice::readVendorAndModelInfo: CFNumberGetValue "
                     "failed for Model_ID.");
      }
      spdlog::debug(
          "AudioDevice::readVendorAndModelInfo: Found Model ID: 0x{:08X}",
          modelID_);
    } else {
      spdlog::warn("AudioDevice::readVendorAndModelInfo: Model_ID property is "
                   "not a CFNumber.");
    }
  } else {
    spdlog::warn(
        "AudioDevice::readVendorAndModelInfo: Model_ID property not found.");
  }
  // Get Vendor Name
  CFStringRef unitVendorStrDesc = (CFStringRef)CFDictionaryGetValue(
      properties, CFSTR("FireWire Vendor Name"));
  if (unitVendorStrDesc) {
    if (CFGetTypeID(unitVendorStrDesc) == CFStringGetTypeID()) {
      std::string name = Helpers::cfStringToString(unitVendorStrDesc);
      if (!name.empty()) {
        vendorName_ = name;
        spdlog::debug(
            "AudioDevice::readVendorAndModelInfo: Found Vendor Name: '{}'",
            vendorName_);
      } else {
        spdlog::warn("AudioDevice::readVendorAndModelInfo: cfStringToString "
                     "failed for FireWire Vendor Name.");
      }
    } else {
      spdlog::warn("AudioDevice::readVendorAndModelInfo: FireWire Vendor Name "
                   "property is not a CFString.");
    }
  } else {
    spdlog::warn("AudioDevice::readVendorAndModelInfo: FireWire Vendor Name "
                 "property not found.");
  }
  CFRelease(properties);
  return kIOReturnSuccess;
}

bool AudioDevice::isDICEDevice() const { return hasDICESupport_; }

void AudioDevice::detectDICESupport() {
  // DICE devices typically have specific vendor IDs and specific registers
  // This is a placeholder implementation - you'll need to adapt it for your
  // specific DICE device

  // Check vendor ID - TC Applied Technologies Ltd. (TCAT) is 0x000166 for DICE
  if (vendorID_ == 0x000166) {
    spdlog::info("AudioDevice::detectDICESupport: Found TCAT vendor ID "
                 "(0x000166) - likely DICE device");
    hasDICESupport_ = true;
    return;
  }

  // For other DICE vendors or more specific detection:
  // Example: Check if there are specific DICE properties
  if (fwDevice_) {
    CFMutableDictionaryRef properties = nullptr;
    IOReturn result = IORegistryEntryCreateCFProperties(
        fwDevice_, &properties, kCFAllocatorDefault, kNilOptions);

    if (result == kIOReturnSuccess && properties) {
      // Look for properties that might indicate DICE
      // For example, some devices expose a "DICE" property or specific model
      // strings

      CFStringRef modelName =
          (CFStringRef)CFDictionaryGetValue(properties, CFSTR("Model"));

      if (modelName && CFGetTypeID(modelName) == CFStringGetTypeID()) {
        std::string model = Helpers::cfStringToString(modelName);
        if (model.find("DICE") != std::string::npos ||
            model.find("dice") != std::string::npos) {
          spdlog::info(
              "AudioDevice::detectDICESupport: Found DICE in model name: {}",
              model);
          hasDICESupport_ = true;
        }
      }

      CFRelease(properties);
    }
  }

  // You could also try to read a known DICE register as a test
}

// --- PRIVATE HELPERS ---
std::expected<void, IOKitError> AudioDevice::checkControlResponse(
    const std::expected<std::vector<uint8_t>, IOKitError> &result,
    const char *commandName) {
  if (!result) {
    spdlog::error("{} command failed: 0x{:x}", commandName,
                  static_cast<int>(result.error()));
    return std::unexpected(result.error());
  }
  const auto &response = result.value();
  if (response.empty()) {
    spdlog::error("{} command returned empty response.", commandName);
    return std::unexpected(IOKitError::BadArgument);
  }
  uint8_t avcStatus = response[0];
  spdlog::debug("{} response status: 0x{:02x}", commandName, avcStatus);
  if (avcStatus == kAVCAcceptedStatus) {
    return {};
  } else if (avcStatus == kAVCRejectedStatus) {
    spdlog::warn("{} command REJECTED.", commandName);
    return std::unexpected(IOKitError::NotPermitted);
  } else if (avcStatus == kAVCNotImplementedStatus) {
    spdlog::error("{} command NOT IMPLEMENTED.", commandName);
    return std::unexpected(IOKitError::Unsupported);
  } else if (avcStatus == kAVCInterimStatus) {
    spdlog::info("{} command returned INTERIM. Further NOTIFY expected.",
                 commandName);
    return {};
  } else {
    spdlog::error("{} command failed with unexpected status 0x{:02x}",
                  commandName, avcStatus);
    return std::unexpected(IOKitError::BadArgument);
  }
}

std::expected<void, IOKitError>
AudioDevice::checkDestPlugConfigureControlSubcommandResponse(
    const std::vector<uint8_t> &response, const char *commandName) {
  if (response.size() < 13) {
    spdlog::error("{} response too short ({}) for subcommand status.",
                  commandName, response.size());
    return std::unexpected(IOKitError::BadArgument);
  }
  uint8_t subcmdResultStatus = response[6];
  spdlog::debug("{} subcommand result status: 0x{:02x}", commandName,
                subcmdResultStatus);
  if (subcmdResultStatus == kAVCDestPlugResultStatusOK) {
    return {};
  } else if (subcmdResultStatus == kAVCDestPlugResultMusicPlugNotExist) {
    spdlog::error("{} failed: Music Plug does not exist.", commandName);
    return std::unexpected(IOKitError::NotFound);
  } else if (subcmdResultStatus == kAVCDestPlugResultSubunitPlugNotExist) {
    spdlog::error("{} failed: Destination Subunit Plug does not exist.",
                  commandName);
    return std::unexpected(IOKitError::NotFound);
  } else if (subcmdResultStatus == kAVCDestPlugResultMusicPlugConnected) {
    spdlog::error("{} failed: Music Plug already connected.", commandName);
    return std::unexpected(IOKitError::StillOpen);
  } else if (subcmdResultStatus == kAVCDestPlugResultNoConnection) {
    spdlog::error("{} failed: No connection (Unknown subfunction) reported in "
                  "subcommand status.",
                  commandName);
    return std::unexpected(IOKitError::BadArgument);
  } else if (subcmdResultStatus == kAVCDestPlugResultUnknownMusicPlugType) {
    spdlog::error("{} failed: Unknown music plug type reported.", commandName);
    return std::unexpected(IOKitError::BadArgument);
  } else {
    spdlog::error("{} failed with subcommand status 0x{:02x}", commandName,
                  subcmdResultStatus);
    return std::unexpected(IOKitError::Error);
  }
}

std::vector<uint8_t> AudioDevice::buildDestPlugConfigureControlCmd(
    uint8_t subfunction, uint8_t musicPlugType, uint16_t musicPlugID,
    uint8_t destSubunitPlugID, uint8_t streamPosition0,
    uint8_t streamPosition1) {
  std::vector<uint8_t> cmd;
  uint8_t subunitAddr = Helpers::getSubunitAddress(
      SubunitType::Music, info_.getMusicSubunit().getId());
  cmd.push_back(kAVCControlCommand);
  cmd.push_back(subunitAddr);
  cmd.push_back(kAVCDestinationPlugConfigureOpcode);
  cmd.push_back(1);
  cmd.push_back(0xFF);
  cmd.push_back(0xFF);
  cmd.push_back(subfunction);
  cmd.push_back(musicPlugType);
  cmd.push_back(static_cast<uint8_t>(musicPlugID >> 8));
  cmd.push_back(static_cast<uint8_t>(musicPlugID & 0xFF));
  cmd.push_back(destSubunitPlugID);
  cmd.push_back(streamPosition0);
  cmd.push_back(streamPosition1);
  return cmd;
}

std::vector<uint8_t> AudioDevice::buildSetStreamFormatControlCmd(
    PlugDirection direction, uint8_t plugNum,
    const std::vector<uint8_t> &formatBytes) {
  std::vector<uint8_t> cmd;
  uint8_t subunitAddr = kAVCUnitAddress;
  uint8_t streamFormatOpcode = kAVCStreamFormatOpcodePrimary;
  cmd.push_back(kAVCControlCommand);
  cmd.push_back(subunitAddr);
  cmd.push_back(streamFormatOpcode);
  cmd.push_back(kAVCStreamFormatSetSubfunction);
  cmd.push_back((direction == PlugDirection::Input) ? 0x00 : 0x01);
  uint8_t plugType = (plugNum < 0x80) ? 0x00 : 0x01;
  if (plugType != 0x00) {
    spdlog::error("buildSetStreamFormatControlCmd: Can only set format for Iso "
                  "plugs (num < 128).");
    return {};
  }
  cmd.push_back(plugType);
  cmd.push_back(plugType);
  cmd.push_back(plugNum);
  cmd.push_back(0xFF);
  cmd.insert(cmd.end(), formatBytes.begin(), formatBytes.end());
  return cmd;
}

std::expected<void, IOKitError> AudioDevice::connectMusicPlug(
    uint8_t musicPlugType, uint16_t musicPlugID, uint8_t destSubunitPlugID,
    uint8_t streamPosition0, uint8_t streamPosition1) {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (!info_.hasMusicSubunit())
    return std::unexpected(IOKitError::NotFound);
  spdlog::info("AudioDevice::connectMusicPlug: Type=0x{:02x}, ID={}, "
               "DestPlug={}, StreamPos=[{}, {}]",
               musicPlugType, musicPlugID, destSubunitPlugID, streamPosition0,
               streamPosition1);
  std::vector<uint8_t> cmd = buildDestPlugConfigureControlCmd(
      kAVCDestPlugSubfuncConnect, musicPlugType, musicPlugID, destSubunitPlugID,
      streamPosition0, streamPosition1);
  spdlog::trace(" -> Sending Connect Music Plug command (0x40/00): {}",
                Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);
  auto check = checkControlResponse(result, "ConnectMusicPlug(0x40/00)");
  if (!check)
    return check;
  return checkDestPlugConfigureControlSubcommandResponse(
      result.value(), "ConnectMusicPlug(0x40/00)");
}

std::expected<void, IOKitError>
AudioDevice::disconnectMusicPlug(uint8_t musicPlugType, uint16_t musicPlugID) {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (!info_.hasMusicSubunit())
    return std::unexpected(IOKitError::NotFound);
  spdlog::info("AudioDevice::disconnectMusicPlug: Type=0x{:02x}, ID={}",
               musicPlugType, musicPlugID);
  std::vector<uint8_t> cmd = buildDestPlugConfigureControlCmd(
      kAVCDestPlugSubfuncDisconnect, musicPlugType, musicPlugID, 0xFF, 0xFF,
      0xFF);
  spdlog::trace(" -> Sending Disconnect Music Plug command (0x40/02): {}",
                Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);
  auto check = checkControlResponse(result, "DisconnectMusicPlug(0x40/02)");
  if (!check)
    return check;
  return checkDestPlugConfigureControlSubcommandResponse(
      result.value(), "DisconnectMusicPlug(0x40/02)");
}

std::expected<void, IOKitError> AudioDevice::setUnitIsochPlugStreamFormat(
    PlugDirection direction, uint8_t plugNum, const AudioStreamFormat &format) {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (plugNum >= 0x80) {
    spdlog::error(
        "setUnitIsochPlugStreamFormat: Invalid plug number {} for Iso plug.",
        plugNum);
    return std::unexpected(IOKitError::BadArgument);
  }
  spdlog::info("AudioDevice::setUnitIsochPlugStreamFormat: Dir={}, PlugNum={}, "
               "Format=[{}]",
               (direction == PlugDirection::Input ? "Input" : "Output"),
               plugNum, format.toString());
  std::vector<uint8_t> formatBytes = format.serializeToBytes();
  if (formatBytes.empty()) {
    spdlog::error("setUnitIsochPlugStreamFormat: Failed to serialize the "
                  "provided AudioStreamFormat.");
    return std::unexpected(IOKitError::BadArgument);
  }
  spdlog::trace(" -> Serialized format bytes: {}",
                Helpers::formatHexBytes(formatBytes));
  std::vector<uint8_t> cmd =
      buildSetStreamFormatControlCmd(direction, plugNum, formatBytes);
  if (cmd.empty()) {
    return std::unexpected(IOKitError::BadArgument);
  }
  spdlog::trace(" -> Sending Set Stream Format command (0xBF/C2): {}",
                Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);
  return checkControlResponse(result, "SetUnitIsochPlugStreamFormat(0xBF/C2)");
}

std::expected<void, IOKitError> AudioDevice::changeMusicPlugConnection(
    uint8_t musicPlugType, uint16_t musicPlugID, uint8_t newDestSubunitPlugID,
    uint8_t newStreamPosition0, uint8_t newStreamPosition1) {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (!info_.hasMusicSubunit())
    return std::unexpected(IOKitError::NotFound);

  spdlog::info("AudioDevice::changeMusicPlugConnection: Type=0x{:02x}, ID={}, "
               "NewDestPlug={}, NewStreamPos=[{}, {}]",
               musicPlugType, musicPlugID, newDestSubunitPlugID,
               newStreamPosition0, newStreamPosition1);

  std::vector<uint8_t> cmd = buildDestPlugConfigureControlCmd(
      kAVCDestPlugSubfuncChangeConnection, // Subfunction 0x01
      musicPlugType, musicPlugID,
      newDestSubunitPlugID, // New destination
      newStreamPosition0,   // New stream pos
      newStreamPosition1);  // New stream pos

  spdlog::trace(
      " -> Sending Change Music Plug Connection command (0x40/01): {}",
      Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);

  auto check =
      checkControlResponse(result, "ChangeMusicPlugConnection(0x40/01)");
  if (!check)
    return check;

  return checkDestPlugConfigureControlSubcommandResponse(
      result.value(), "ChangeMusicPlugConnection(0x40/01)");
}

std::expected<void, IOKitError>
AudioDevice::disconnectAllMusicPlugs(uint8_t fromDestSubunitPlugID) {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (!info_.hasMusicSubunit())
    return std::unexpected(IOKitError::NotFound);

  spdlog::info("AudioDevice::disconnectAllMusicPlugs: FromDestPlugID={}",
               fromDestSubunitPlugID);

  std::vector<uint8_t> cmd = buildDestPlugConfigureControlCmd(
      kAVCDestPlugSubfuncDisconnectAll, // Subfunction 0x03
      0xFF,                             // musicPlugType = FF
      0xFFFF,                           // musicPlugID = FFFF
      fromDestSubunitPlugID, // Specify which destination plug to clear
      0xFF,                  // streamPosition[0] = FF
      0xFF);                 // streamPosition[1] = FF

  spdlog::trace(" -> Sending Disconnect All Music Plugs command (0x40/03): {}",
                Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);

  auto check = checkControlResponse(result, "DisconnectAllMusicPlugs(0x40/03)");
  if (!check)
    return check;

  return checkDestPlugConfigureControlSubcommandResponse(
      result.value(), "DisconnectAllMusicPlugs(0x40/03)");
}

std::expected<void, IOKitError> AudioDevice::defaultConfigureMusicPlugs() {
  if (!hasAvcCapability_)
    return std::unexpected(IOKitError::Unsupported);
  if (!commandInterface_)
    return std::unexpected(IOKitError::NotReady);
  if (!info_.hasMusicSubunit())
    return std::unexpected(IOKitError::NotFound);

  spdlog::info("AudioDevice::defaultConfigureMusicPlugs: Resetting connections "
               "to default.");

  std::vector<uint8_t> cmd = buildDestPlugConfigureControlCmd(
      kAVCDestPlugSubfuncDefaultConfigure, // Subfunction 0x04
      0xFF,                                // musicPlugType = FF
      0xFFFF,                              // musicPlugID = FFFF
      0xFF,                                // destSubunitPlugID = FF
      0xFF,                                // streamPosition[0] = FF
      0xFF);                               // streamPosition[1] = FF

  spdlog::trace(
      " -> Sending Default Configure Music Plugs command (0x40/04): {}",
      Helpers::formatHexBytes(cmd));
  auto result = commandInterface_->sendCommand(cmd);

  auto check =
      checkControlResponse(result, "DefaultConfigureMusicPlugs(0x40/04)");
  if (!check)
    return check;

  return checkDestPlugConfigureControlSubcommandResponse(
      result.value(), "DefaultConfigureMusicPlugs(0x40/04)");
}

void AudioDevice::initializeBasicCapabilities() {
  spdlog::info("AudioDevice::initializeBasicCapabilities: Setting up basic "
               "capabilities for non-AVC device");

  // Check if device supports isochronous transfers
  bool hasIsochronousCapability = false;

  if (deviceInterface) {
    // Try to determine if device has isochronous capabilities
    CFMutableDictionaryRef properties = nullptr;
    IOReturn result = IORegistryEntryCreateCFProperties(
        fwDevice_, &properties, kCFAllocatorDefault, kNilOptions);

    if (result == kIOReturnSuccess && properties) {
      // Look for properties that might indicate isochronous capability
      CFBooleanRef isoCapable = (CFBooleanRef)CFDictionaryGetValue(
          properties, CFSTR("IOFireWireIsochronousCapabilities"));

      if (isoCapable && CFGetTypeID(isoCapable) == CFBooleanGetTypeID()) {
        hasIsochronousCapability = CFBooleanGetValue(isoCapable);
        spdlog::info("AudioDevice::initializeBasicCapabilities: Device has "
                     "isochronous capability: {}",
                     hasIsochronousCapability ? "YES" : "NO");
      }

      // Release properties when done
      CFRelease(properties);
    }
  }

  // Initialize device info with basic FireWire capabilities
  // Set up the device info with basic information we've gathered

  spdlog::info("AudioDevice::initializeBasicCapabilities: Device will have "
               "limited functionality");
  spdlog::info("AudioDevice::initializeBasicCapabilities: GUID: 0x{:x}, "
               "Vendor: {}, Model: {}",
               guid_, vendorName_, deviceName_);
}
} // namespace FWA
