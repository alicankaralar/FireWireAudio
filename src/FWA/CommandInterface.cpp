#include "FWA/CommandInterface.h"

#include <IOKit/IOMessage.h>
#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>

#include "FWA/AudioDevice.h"
#include "FWA/Error.h"
#include "FWA/Helpers.h" // For formatHexBytes

namespace FWA {

// New enum to track what protocol we're using
enum class DeviceProtocol { UNKNOWN, AVC, DICE };

CommandInterface::CommandInterface(std::shared_ptr<AudioDevice> pAudioDevice)
    : pAudioDevice_(pAudioDevice), avcUnit_(pAudioDevice->getAVCDevice()),
      avcInterface_(nullptr), interestNotification_(0),
      notificationCallback_(nullptr), refCon_(nullptr),
      protocol_(DeviceProtocol::UNKNOWN), diceInterface_(nullptr) {

  // Determine protocol
  if (avcUnit_ != 0) {
    protocol_ = DeviceProtocol::AVC;
    spdlog::info("CommandInterface: Using AVC protocol");
  } else {
    // Check if device is a DICE device
    if (pAudioDevice->isDICEDevice()) {
      protocol_ = DeviceProtocol::DICE;
      spdlog::info("CommandInterface: Using DICE protocol");
    } else {
      spdlog::warn("CommandInterface: Unknown protocol - device is neither AVC "
                   "nor DICE");
    }
  }
}

CommandInterface::~CommandInterface() {
  auto result = deactivate();
  if (!result) {
    spdlog::error("~CommandInterface, deactivate failed");
  }
}

CommandInterface::CommandInterface(CommandInterface &&other) noexcept
    : pAudioDevice_(std::move(other.pAudioDevice_)), avcUnit_(other.avcUnit_),
      avcInterface_(other.avcInterface_),
      interestNotification_(other.interestNotification_),
      notificationCallback_(other.notificationCallback_),
      refCon_(other.refCon_), protocol_(other.protocol_),
      diceInterface_(other.diceInterface_) {
  other.pAudioDevice_ = nullptr;
  other.avcUnit_ = 0;
  other.avcInterface_ = nullptr;
  other.interestNotification_ = 0;
  other.notificationCallback_ = nullptr;
  other.refCon_ = nullptr;
  other.protocol_ = DeviceProtocol::UNKNOWN;
  other.diceInterface_ = nullptr;
}

CommandInterface &
CommandInterface::operator=(CommandInterface &&other) noexcept {
  if (this != &other) {
    deactivate();

    pAudioDevice_ = std::move(other.pAudioDevice_);
    avcUnit_ = other.avcUnit_;
    avcInterface_ = other.avcInterface_;
    interestNotification_ = other.interestNotification_;
    notificationCallback_ = other.notificationCallback_;
    refCon_ = other.refCon_;
    protocol_ = other.protocol_;
    diceInterface_ = other.diceInterface_;

    other.pAudioDevice_ = nullptr;
    other.avcUnit_ = 0;
    other.avcInterface_ = nullptr;
    other.interestNotification_ = 0;
    other.notificationCallback_ = nullptr;
    other.refCon_ = nullptr;
    other.protocol_ = DeviceProtocol::UNKNOWN;
    other.diceInterface_ = nullptr;
  }
  return *this;
}

std::expected<void, IOKitError> CommandInterface::activate() {
  if (avcInterface_ != nullptr || diceInterface_ != nullptr) {
    return std::unexpected(
        static_cast<IOKitError>(kIOReturnStillOpen)); // Already active.
  }

  // Activate the appropriate interface based on protocol
  if (protocol_ == DeviceProtocol::AVC) {
    auto result = createAVCUnitInterface();
    if (!result) {
      return std::unexpected(result.error());
    }
    // Register the notification callback.
    return setNotificationCallback(notificationCallback_, refCon_);
  } else if (protocol_ == DeviceProtocol::DICE) {
    auto result = createDICEInterface();
    if (!result) {
      return std::unexpected(result.error());
    }
    // Register the notification callback.
    return setNotificationCallback(notificationCallback_, refCon_);
  } else {
    spdlog::error("CommandInterface::activate: Unknown protocol");
    return std::unexpected(IOKitError(kIOReturnUnsupported));
  }
}

std::expected<void, IOKitError> CommandInterface::deactivate() {
  clearNotificationCallback();

  if (avcInterface_) {
    return releaseAVCUnitInterface();
  } else if (diceInterface_) {
    return releaseDICEInterface();
  }

  return {}; // Already deactivated.
}

std::expected<void, IOKitError> CommandInterface::createAVCUnitInterface() {
  // --- Add check for null avcUnit_ ---
  if (avcUnit_ == 0) {
    spdlog::warn("CommandInterface::createAVCUnitInterface: Cannot create "
                 "interface, avcUnit_ is null (Device might not be "
                 "an AVC Unit).");
    return std::unexpected(
        IOKitError(kIOReturnUnsupported)); // Or kIOReturnNotFound? Unsupported
                                           // seems better.
  }
  // --- End check ---

  IOCFPlugInInterface **plugInInterface = nullptr;
  SInt32 score = 0;

  IOReturn result = IOCreatePlugInInterfaceForService(
      avcUnit_, kIOFireWireAVCLibUnitTypeID, kIOCFPlugInInterfaceID,
      &plugInInterface, &score);

  if (result != kIOReturnSuccess) {
    spdlog::error("Failed to create the CFPlugin interface: 0x{:x}", result);
    return std::unexpected(static_cast<IOKitError>(result));
  }

  HRESULT comResult =
      (*plugInInterface)
          ->QueryInterface(
              plugInInterface,
              CFUUIDGetUUIDBytes(kIOFireWireAVCLibUnitInterfaceID_v2),
              (void **)&avcInterface_);

  (*plugInInterface)->Release(plugInInterface);

  if (comResult != S_OK || avcInterface_ == nullptr) {
    spdlog::error("Failed to get IOFireWireAVCLibUnitInterface: 0x{:x}",
                  static_cast<int>(comResult));
    return std::unexpected(static_cast<IOKitError>(comResult));
  }

  return {};
}

std::expected<void, IOKitError> CommandInterface::releaseAVCUnitInterface() {
  if (avcInterface_) {
    (*avcInterface_)->close(avcInterface_);
    (*avcInterface_)->Release(avcInterface_);
    avcInterface_ = nullptr;
  }
  return {};
}

std::expected<void, IOKitError> CommandInterface::createDICEInterface() {
  // This function would create the appropriate interface for DICE devices
  // The implementation depends on how DICE devices are accessed on macOS

  spdlog::debug(
      "CommandInterface::createDICEInterface: Creating DICE interface");

  // Get the FireWire device interface from the AudioDevice
  io_service_t fwDevice = pAudioDevice_->getFireWireDevice();
  if (fwDevice == 0) {
    spdlog::error(
        "CommandInterface::createDICEInterface: No FireWire device available");
    return std::unexpected(IOKitError(kIOReturnNotFound));
  }

  // Create your DICE interface here
  // This is a placeholder - you'll need to implement the actual interface
  // creation

  // For now, just set a dummy value to indicate success
  diceInterface_ = reinterpret_cast<void *>(1);

  return {};
}

std::expected<void, IOKitError> CommandInterface::releaseDICEInterface() {
  if (diceInterface_) {
    // Release your DICE interface here
    // This is a placeholder - you'll need to implement the actual interface
    // release

    diceInterface_ = nullptr;
  }
  return {};
}

std::expected<std::vector<uint8_t>, IOKitError>
CommandInterface::sendCommand(const std::vector<uint8_t> &command) {
  // Route command to the appropriate handler based on protocol
  if (protocol_ == DeviceProtocol::AVC) {
    return sendAVCCommand(command);
  } else if (protocol_ == DeviceProtocol::DICE) {
    return sendDICECommand(command);
  } else {
    spdlog::error(
        "CommandInterface::sendCommand: No supported protocol available");
    return std::unexpected(static_cast<IOKitError>(kIOReturnUnsupported));
  }
}

std::expected<std::vector<uint8_t>, IOKitError>
CommandInterface::sendAVCCommand(const std::vector<uint8_t> &command) {
  if (!avcInterface_) {
    spdlog::error("CommandInterface::sendCommand: Attempted to send command "
                  "but avcInterface_ is null.");
    return std::unexpected(static_cast<IOKitError>(kIOReturnNotOpen));
  }

  // --- Enhanced Logging ---
  spdlog::debug(
      "CommandInterface::sendCommand: Sending command with avcInterface_ = {}",
      (void *)avcInterface_);
  spdlog::debug(" --> CMD Bytes ({}): {}", command.size(),
                Helpers::formatHexBytes(command));
  // -----------------------

  UInt32 cmdLen = static_cast<UInt32>(command.size());
  UInt32 respCapacity = 512; // Start with a reasonable capacity
  std::vector<uint8_t> response(respCapacity, 0);
  UInt32 actualRespLen = respCapacity; // Store original capacity for logging

  IOReturn result =
      (*avcInterface_)
          ->AVCCommand(avcInterface_, command.data(), cmdLen, response.data(),
                       &actualRespLen); // Use actualRespLen here

  // --- Enhanced Logging ---
  spdlog::debug("CommandInterface::sendCommand: AVCCommand call returned "
                "IOReturn: 0x{:x} ({})",
                result, result);
  // Log response even if result is not kIOReturnSuccess, but check
  // actualRespLen validity
  if (actualRespLen > 0 && actualRespLen <= respCapacity) {
    response.resize(actualRespLen); // Resize to actual length *before* logging
    spdlog::debug(" --> RSP Bytes ({}): {}", response.size(),
                  Helpers::formatHexBytes(response));
  } else if (actualRespLen == 0) {
    response.clear(); // Clear if no bytes returned
    spdlog::debug(" --> RSP Bytes (0): (Empty response)");
  } else { // actualRespLen > respCapacity (Shouldn't happen if API works
           // correctly)
    response.clear(); // Clear response as it's invalid
    spdlog::error(" --> RSP Error: Invalid actualRespLen ({}) returned, "
                  "exceeds capacity ({}).",
                  actualRespLen, respCapacity);
    // Continue to handle the IOReturn error below, but response is invalid
  }
  // -----------------------

  if (result != kIOReturnSuccess) {
    spdlog::error("CommandInterface::sendCommand: Error sending command "
                  "(IOReturn: 0x{:x})",
                  result);
    // Return the specific IOReturn code mapped to our error type
    return std::unexpected(static_cast<IOKitError>(result));
  }

  return response; // Return the resized response vector
}

std::expected<std::vector<uint8_t>, IOKitError>
CommandInterface::sendDICECommand(const std::vector<uint8_t> &command) {
  if (!diceInterface_) {
    spdlog::error(
        "CommandInterface::sendDICECommand: Attempted to send command "
        "but diceInterface_ is null.");
    return std::unexpected(static_cast<IOKitError>(kIOReturnNotOpen));
  }

  spdlog::debug("CommandInterface::sendDICECommand: Sending DICE command");
  spdlog::debug(" --> CMD Bytes ({}): {}", command.size(),
                Helpers::formatHexBytes(command));

  // DICE uses a different protocol - typically register reads/writes
  // We'll need to interpret the command vector differently

  // This is a placeholder implementation - you'll need to implement the actual
  // DICE protocol Based on your specific DICE device's requirements

  // Example implementation:
  uint32_t address = 0;
  if (command.size() >= 4) {
    // Assuming first 4 bytes are the register address
    address = (command[0] << 24) | (command[1] << 16) | (command[2] << 8) |
              command[3];
  }

  // Create response buffer
  std::vector<uint8_t> response;

  // Example read operation (actual implementation will depend on your DICE
  // device)
  UInt32 readValue = 0;
  IOReturn result = kIOReturnSuccess;

  // Implement the actual DICE read/write logic here
  // result = readDICERegister(address, &readValue);

  if (result == kIOReturnSuccess) {
    // Format response - for example, return the 4-byte value read
    response.push_back((readValue >> 24) & 0xFF);
    response.push_back((readValue >> 16) & 0xFF);
    response.push_back((readValue >> 8) & 0xFF);
    response.push_back(readValue & 0xFF);

    spdlog::debug(" --> DICE RSP Bytes ({}): {}", response.size(),
                  Helpers::formatHexBytes(response));
  } else {
    spdlog::error("CommandInterface::sendDICECommand: Error sending command "
                  "(IOReturn: 0x{:x})",
                  result);
    return std::unexpected(static_cast<IOKitError>(result));
  }

  return response;
}

std::expected<void, IOKitError>
CommandInterface::setNotificationCallback(DeviceStatusCallback callback,
                                          void *refCon) {
  if (notificationCallback_) {
    return std::unexpected(static_cast<IOKitError>(kIOReturnExclusiveAccess));
  }
  notificationCallback_ = callback;
  refCon_ = refCon;

  io_object_t interestNotification = 0;
  auto result = IOServiceAddInterestNotification(
      pAudioDevice_->getNotificationPort(), avcUnit_, kIOGeneralInterest,
      deviceInterestCallback, this, &interestNotification);

  if (result != kIOReturnSuccess) {
    spdlog::error("Failed to add interest notification: {}", result);
    return std::unexpected(static_cast<IOKitError>(result));
  }
  interestNotification_ = interestNotification;
  return {};
}

void CommandInterface::clearNotificationCallback() {
  if (interestNotification_) {
    IOObjectRelease(interestNotification_);
    interestNotification_ = 0;
  }
  notificationCallback_ = nullptr;
  refCon_ = nullptr;
}

void CommandInterface::deviceInterestCallback(void *refCon,
                                              io_service_t service,
                                              natural_t messageType,
                                              void *messageArgument) {
  CommandInterface *self = static_cast<CommandInterface *>(refCon);
  if (!self)
    return;

  if (messageType == kIOMessageServiceIsTerminated) {
    spdlog::info("Device terminated!");
    if (self->notificationCallback_) {
      self->notificationCallback_(self->pAudioDevice_, messageType,
                                  messageArgument);
    }
  }
}

} // namespace FWA
