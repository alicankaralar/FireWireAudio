#include "FWADriverDevice.hpp"
#include <aspl/Device.hpp> // Include necessary ASPL headers
#include <aspl/Tracer.hpp>
#include <aspl/Context.hpp>
#include <string>
#include <algorithm>       // For std::min, std::copy
#include <cstring>
#include <libkern/OSByteOrder.h>
#include <limits>
#include <cassert>
#include "FWADriverHandler.hpp"
#include <CoreAudio/AudioServerPlugIn.h>
constexpr const char* LogPrefix = "FWADriverASPL: ";

// --- Local Helper Function ---
static inline std::string FormatFourCharCode(UInt32 code) {
    char chars[5];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    UInt32 beCode = OSSwapHostToBigInt32(code);
#else
    UInt32 beCode = code;
#endif
    memcpy(chars, &beCode, 4);
    for (int i = 0; i < 4; ++i) {
        if (chars[i] < 32 || chars[i] > 126) {
            chars[i] = '?';
        }
    }
    chars[4] = '\0';
    return std::string("'") + chars + "'";
}

FWADriverDevice::FWADriverDevice(std::shared_ptr<const aspl::Context> context,
                                 const aspl::DeviceParameters& params)
    : aspl::Device(context, params) // Forward to base class
{
    GetContext()->Tracer->Message("%sFWADriverDevice[%u]: Constructed with name '%s'", LogPrefix, GetID(), params.Name.c_str());
    // Custom initialization if needed
}

// --- Property Dispatch Implementations ---

Boolean FWADriverDevice::HasProperty(AudioObjectID objectID,
                                    pid_t clientPID,
                                    const AudioObjectPropertyAddress* address) const
{
    std::string selectorStr = address ? FormatFourCharCode(address->mSelector) : "NULL";
    // Check if it's a property we handle specifically
    if (address && address->mSelector == kAudioDevicePropertyAvailableNominalSampleRates &&
        address->mScope == kAudioObjectPropertyScopeGlobal &&
        address->mElement == kAudioObjectPropertyElementMain) // Use Main
    {
        GetContext()->Tracer->Message("%sFWADriverDevice[%u]::HasProperty(Selector: %s): YES", LogPrefix, GetID(), selectorStr.c_str());
        return true;
    }

    // Otherwise, let the base class handle it
    bool baseHas = aspl::Device::HasProperty(objectID, clientPID, address);
    return baseHas;
}

OSStatus FWADriverDevice::GetPropertyDataSize(AudioObjectID objectID,
                                            pid_t clientPID,
                                            const AudioObjectPropertyAddress* address,
                                            UInt32 qualifierDataSize,
                                            const void* qualifierData,
                                            UInt32* outDataSize) const
{
    std::string selectorStr = address ? FormatFourCharCode(address->mSelector) : "NULL";
    if (!address || !outDataSize) {
        GetContext()->Tracer->Message("%sERROR: FWADriverDevice[%u]::GetPropertyDataSize - Invalid address or outDataSize pointer.", LogPrefix, GetID());
        return kAudioHardwareBadObjectError;
    }

    // Handle our specific property
    if (address->mSelector == kAudioDevicePropertyAvailableNominalSampleRates &&
        address->mScope == kAudioObjectPropertyScopeGlobal &&
        address->mElement == kAudioObjectPropertyElementMain) // Use Main
    {
        // Simulate getting rates (replace with XPC call later)
        auto rates = GetSimulatedAvailableSampleRates();
        size_t requiredSize_t = rates.size() * sizeof(AudioValueRange);
        if (requiredSize_t > std::numeric_limits<UInt32>::max()) {
            GetContext()->Tracer->Message("%sERROR: FWADriverDevice[%u]::GetPropertyDataSize(Selector: %s) - Required size (%zu) exceeds UINT32_MAX.", LogPrefix, GetID(), selectorStr.c_str(), requiredSize_t);
            *outDataSize = 0;
            return kAudioHardwareUnspecifiedError;
        }
        *outDataSize = static_cast<UInt32>(requiredSize_t);
        GetContext()->Tracer->Message("%sFWADriverDevice[%u]::GetPropertyDataSize(Selector: %s): %u bytes", LogPrefix, GetID(), selectorStr.c_str(), *outDataSize);
        return kAudioHardwareNoError;
    }

    // Let the base class handle others
    return aspl::Device::GetPropertyDataSize(objectID, clientPID, address, qualifierDataSize, qualifierData, outDataSize);
}

OSStatus FWADriverDevice::GetPropertyData(AudioObjectID objectID,
                                        pid_t clientPID,
                                        const AudioObjectPropertyAddress* address,
                                        UInt32 qualifierDataSize,
                                        const void* qualifierData,
                                        UInt32 inDataSize,
                                        UInt32* outDataSize,
                                        void* outData) const
{
    std::string selectorStr = address ? FormatFourCharCode(address->mSelector) : "NULL";
     if (!address || !outDataSize || !outData) {
        GetContext()->Tracer->Message("%sERROR: FWADriverDevice[%u]::GetPropertyData - Invalid address, outDataSize, or outData pointer.", LogPrefix, GetID());
        return kAudioHardwareBadObjectError;
    }

    // Handle our specific property
    if (address->mSelector == kAudioDevicePropertyAvailableNominalSampleRates &&
        address->mScope == kAudioObjectPropertyScopeGlobal &&
        address->mElement == kAudioObjectPropertyElementMain) // Use Main
    {
        // Simulate getting rates (replace with XPC call later)
        auto rates = GetSimulatedAvailableSampleRates();
        size_t requiredSize_t = rates.size() * sizeof(AudioValueRange);
        if (requiredSize_t > std::numeric_limits<UInt32>::max()) {
            GetContext()->Tracer->Message("%sERROR: FWADriverDevice[%u]::GetPropertyData(Selector: %s) - Required size (%zu) exceeds UINT32_MAX.", LogPrefix, GetID(), selectorStr.c_str(), requiredSize_t);
            *outDataSize = 0;
            return kAudioHardwareUnspecifiedError;
        }
        UInt32 calculatedSize = static_cast<UInt32>(requiredSize_t);

        UInt32 bytesToWrite = std::min(inDataSize, calculatedSize);
        *outDataSize = bytesToWrite; // Return how much we *actually* wrote

        if (bytesToWrite > 0) {
             GetContext()->Tracer->Message("%sFWADriverDevice[%u]::GetPropertyData(Selector: %s): Writing %u bytes (of %u needed)", LogPrefix, GetID(), selectorStr.c_str(), bytesToWrite, calculatedSize);
             memcpy(outData, rates.data(), bytesToWrite);
        } else if (inDataSize == 0) {
             GetContext()->Tracer->Message("%sWARNING: FWADriverDevice[%u]::GetPropertyData(Selector: %s): Zero-size buffer provided.", LogPrefix, GetID(), selectorStr.c_str());
             *outDataSize = 0;
        } else {
             GetContext()->Tracer->Message("%sWARNING: FWADriverDevice[%u]::GetPropertyData(Selector: %s): Buffer too small (needed %u, got %u), wrote 0 bytes.", LogPrefix, GetID(), selectorStr.c_str(), calculatedSize, inDataSize);
             *outDataSize = 0;
        }
         return kAudioHardwareNoError;
    }

    // Let the base class handle others
    return aspl::Device::GetPropertyData(objectID, clientPID, address, qualifierDataSize, qualifierData, inDataSize, outDataSize, outData);
}

// --- Helper Implementation ---

std::vector<AudioValueRange> FWADriverDevice::GetSimulatedAvailableSampleRates() const
{
    // !! Placeholder !! Replace this with an XPC call to the daemon later
    return {
        {44100.0, 44100.0},
        {48000.0, 48000.0},
        {88200.0, 88200.0},
        {96000.0, 96000.0}
    };
}

OSStatus FWADriverDevice::DoIOOperation(AudioObjectID objectID,
                                        AudioObjectID streamID,
                                        UInt32 clientID,
                                        UInt32 operationID,
                                        UInt32 ioBufferFrameSize,
                                        const AudioServerPlugInIOCycleInfo* ioCycleInfo,
                                        void* ioMainBuffer,
                                        void* ioSecondaryBuffer)
{
    // Only handle WriteMix and ReadInput
    if (operationID == kAudioServerPlugInIOOperationWriteMix) {
        auto stream = GetStreamByID(streamID);
        if (!stream) {
            GetContext()->Tracer->Message("%sERROR: DoIOOperation: Unknown stream ID %u", LogPrefix, streamID);
            return kAudioHardwareBadStreamError;
        }
        AudioStreamBasicDescription format = stream->GetVirtualFormat();
        uint32_t bytesPerFrame = format.mBytesPerFrame;
        if (bytesPerFrame == 0) {
            GetContext()->Tracer->Message("%sERROR: DoIOOperation: Invalid bytesPerFrame (0) for stream %u", LogPrefix, streamID);
            return kAudioHardwareUnspecifiedError;
        }
        FWADriverHandler* ioHandler = static_cast<FWADriverHandler*>(GetIOHandler());
        if (!ioHandler || !ioHandler->IsSharedMemoryReady()) {
            GetContext()->Tracer->Message("%sERROR: DoIOOperation: Shared memory not ready", LogPrefix);
            return kAudioHardwareUnspecifiedError;
        }
        bool success = ioHandler->PushToSharedMemory(static_cast<const AudioBufferList*>(ioMainBuffer),
                                                     ioCycleInfo->mOutputTime,
                                                     ioBufferFrameSize,
                                                     bytesPerFrame);
        // Optionally log on overrun (already handled in handler)
        (void)success;
        return kAudioHardwareNoError;
    } else if (operationID == kAudioServerPlugInIOOperationReadInput) {
        // Provide silence for input
        AudioBufferList* inputBufferList = static_cast<AudioBufferList*>(ioMainBuffer);
        for (UInt32 i = 0; i < inputBufferList->mNumberBuffers; ++i) {
            if (inputBufferList->mBuffers[i].mData) {
                memset(inputBufferList->mBuffers[i].mData, 0, inputBufferList->mBuffers[i].mDataByteSize);
            }
        }
        return kAudioHardwareNoError;
    }
    // Ignore other operations
    return kAudioHardwareNoError;
}
