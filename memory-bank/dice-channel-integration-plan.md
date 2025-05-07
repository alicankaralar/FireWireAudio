---
**STATUS UPDATE (2025-05-06):** The `firewire_scanner` tool has undergone a "Comprehensive Channel Name Extraction Overhaul" which successfully identifies channel names, types, and counts. This provides a strong foundation and validated data source for the integration steps outlined in this plan. However, the actual integration of this information into `DiceAudioDevice.cpp` as detailed below is likely still pending.
---
# DICE Channel Integration Plan

This document outlines the plan to integrate the channel configuration information discovered by the scanner tool into the DICE driver implementation.

## Current State

1. The scanner tool can now successfully detect:
   - 24 mono output channels (OUTPUT CH1-CH24)
   - 24 mono input channels (INPUT CH1-CH24)
   - 4 stereo output pairs (8 channels)
   - 4 stereo input pairs (8 channels)
   - Total of 64 I/O channels in the Venice device

2. The DiceAudioDevice implementation:
   - Has member variables for TX/RX stream counts and sizes (m_nb_tx, m_tx_size, m_nb_rx, m_rx_size)
   - Has methods to read TX/RX registers (readTxReg, writeTxReg, readRxReg, writeRxReg)
   - Has methods to read channel names (getTxNameString, getRxNameString)
   - But these methods read from the standard DICE register locations, not the alternate memory location we discovered

## Integration Plan

### 1. Create a new method in DiceAudioDevice to read channel names from the alternate memory location

```cpp
// Add to DiceAudioDevice.h
std::expected<std::vector<std::string>, IOKitError> getAlternateChannelNames();
```

```cpp
// Implementation in DiceAudioDevice.cpp
std::expected<std::vector<std::string>, IOKitError> DiceAudioDevice::getAlternateChannelNames()
{
    // Define the alternate memory address where channel names are stored
    const uint64_t ALTERNATE_CHANNEL_NAMES_BASE = 0xffffe00001a8ULL;
    
    // Read a large block of memory to capture all channel names
    // We know from our scanner that we need at least 1024 bytes
    const size_t READ_SIZE = 1024;
    
    std::vector<std::string> channelNames;
    std::expected<std::vector<uint32_t>, IOKitError> readResult = readRegBlock(ALTERNATE_CHANNEL_NAMES_BASE, READ_SIZE);
    if (!readResult)
    {
        spdlog::error("Failed to read alternate channel names: {}", static_cast<int>(readResult.error()));
        return std::unexpected(readResult.error());
    }
    
    // Convert the raw data to a string
    std::vector<uint32_t> rawData = readResult.value();
    std::string rawString;
    for (uint32_t value : rawData)
    {
        // DICE registers are little-endian, so we need to swap bytes
        uint32_t swappedValue = CFSwapInt32LittleToHost(value);
        char bytes[4];
        memcpy(bytes, &swappedValue, 4);
        for (int i = 0; i < 4; i++)
        {
            if (bytes[i] != '\0')
                rawString += bytes[i];
        }
    }
    
    // Parse the string to extract channel names
    // Split by backslash character
    std::string delimiter = "\\";
    size_t pos = 0;
    std::string token;
    while ((pos = rawString.find(delimiter)) != std::string::npos)
    {
        token = rawString.substr(0, pos);
        if (!token.empty())
            channelNames.push_back(token);
        rawString.erase(0, pos + delimiter.length());
    }
    if (!rawString.empty())
        channelNames.push_back(rawString);
    
    return channelNames;
}
```

### 2. Create a method to categorize channel names

```cpp
// Add to DiceAudioDevice.h
struct ChannelConfiguration {
    std::vector<std::string> monoOutputChannels;
    std::vector<std::string> monoInputChannels;
    std::vector<std::string> stereoOutputChannels;
    std::vector<std::string> stereoInputChannels;
};

std::expected<ChannelConfiguration, IOKitError> getChannelConfiguration();
```

```cpp
// Implementation in DiceAudioDevice.cpp
std::expected<ChannelConfiguration, IOKitError> DiceAudioDevice::getChannelConfiguration()
{
    ChannelConfiguration config;
    
    // Get all channel names
    auto channelNamesResult = getAlternateChannelNames();
    if (!channelNamesResult)
    {
        return std::unexpected(channelNamesResult.error());
    }
    
    std::vector<std::string> allChannels = channelNamesResult.value();
    
    // Regular expressions for different channel types
    std::regex monoOutputPattern(R"(OUTPUT CH(\d+))");
    std::regex monoInputPattern(R"(INPUT CH(\d+))");
    std::regex stereoOutputPattern(R"(OUTPUT ST CH(\d+)[LR])");
    std::regex stereoInputPattern(R"(INPUT ST CH(\d+)[LR])");
    
    // Categorize each channel
    for (const auto& channel : allChannels)
    {
        std::smatch match;
        if (std::regex_match(channel, match, monoOutputPattern))
        {
            config.monoOutputChannels.push_back(channel);
        }
        else if (std::regex_match(channel, match, monoInputPattern))
        {
            config.monoInputChannels.push_back(channel);
        }
        else if (std::regex_match(channel, match, stereoOutputPattern))
        {
            config.stereoOutputChannels.push_back(channel);
        }
        else if (std::regex_match(channel, match, stereoInputPattern))
        {
            config.stereoInputChannels.push_back(channel);
        }
    }
    
    return config;
}
```

### 3. Extend the AudioDevice class to expose channel information

```cpp
// Add to AudioDevice.h
struct ChannelInfo {
    std::string name;
    bool isInput;
    bool isStereo;
    int channelNumber;
    std::string stereoSide; // "L" or "R" for stereo channels
};

virtual std::expected<std::vector<ChannelInfo>, IOKitError> getChannelInfo();
```

```cpp
// Implementation in DiceAudioDevice.cpp
std::expected<std::vector<ChannelInfo>, IOKitError> DiceAudioDevice::getChannelInfo()
{
    std::vector<ChannelInfo> channelInfo;
    
    // Get channel configuration
    auto configResult = getChannelConfiguration();
    if (!configResult)
    {
        return std::unexpected(configResult.error());
    }
    
    ChannelConfiguration config = configResult.value();
    
    // Regular expressions for extracting channel numbers and stereo sides
    std::regex monoPattern(R"((?:OUTPUT|INPUT) CH(\d+))");
    std::regex stereoPattern(R"((?:OUTPUT|INPUT) ST CH(\d+)([LR]))");
    
    // Process mono output channels
    for (const auto& channel : config.monoOutputChannels)
    {
        std::smatch match;
        if (std::regex_match(channel, match, monoPattern))
        {
            ChannelInfo info;
            info.name = channel;
            info.isInput = false;
            info.isStereo = false;
            info.channelNumber = std::stoi(match[1]);
            info.stereoSide = "";
            channelInfo.push_back(info);
        }
    }
    
    // Process mono input channels
    for (const auto& channel : config.monoInputChannels)
    {
        std::smatch match;
        if (std::regex_match(channel, match, monoPattern))
        {
            ChannelInfo info;
            info.name = channel;
            info.isInput = true;
            info.isStereo = false;
            info.channelNumber = std::stoi(match[1]);
            info.stereoSide = "";
            channelInfo.push_back(info);
        }
    }
    
    // Process stereo output channels
    for (const auto& channel : config.stereoOutputChannels)
    {
        std::smatch match;
        if (std::regex_match(channel, match, stereoPattern))
        {
            ChannelInfo info;
            info.name = channel;
            info.isInput = false;
            info.isStereo = true;
            info.channelNumber = std::stoi(match[1]);
            info.stereoSide = match[2];
            channelInfo.push_back(info);
        }
    }
    
    // Process stereo input channels
    for (const auto& channel : config.stereoInputChannels)
    {
        std::smatch match;
        if (std::regex_match(channel, match, stereoPattern))
        {
            ChannelInfo info;
            info.name = channel;
            info.isInput = true;
            info.isStereo = true;
            info.channelNumber = std::stoi(match[1]);
            info.stereoSide = match[2];
            channelInfo.push_back(info);
        }
    }
    
    return channelInfo;
}
```

### 4. Update the DiceAudioDevice::init method to read channel configuration

```cpp
// Modify DiceAudioDevice::init in DiceAudioDevice.cpp
std::expected<void, IOKitError> DiceAudioDevice::init()
{
    // ... existing code ...
    
    // Read channel configuration
    auto channelConfigResult = getChannelConfiguration();
    if (channelConfigResult)
    {
        ChannelConfiguration config = channelConfigResult.value();
        spdlog::info("Channel configuration:");
        spdlog::info("  Mono Output Channels: {}", config.monoOutputChannels.size());
        spdlog::info("  Mono Input Channels: {}", config.monoInputChannels.size());
        spdlog::info("  Stereo Output Channels: {}", config.stereoOutputChannels.size());
        spdlog::info("  Stereo Input Channels: {}", config.stereoInputChannels.size());
    }
    else
    {
        spdlog::warn("Could not read channel configuration: {}", static_cast<int>(channelConfigResult.error()));
    }
    
    return {};
}
```

### 5. Add a method to get the total number of physical channels

```cpp
// Add to DiceAudioDevice.h
struct ChannelCounts {
    int totalOutputChannels;
    int totalInputChannels;
    int monoOutputChannels;
    int monoInputChannels;
    int stereoOutputPairs;
    int stereoInputPairs;
};

std::expected<ChannelCounts, IOKitError> getChannelCounts();
```

```cpp
// Implementation in DiceAudioDevice.cpp
std::expected<ChannelCounts, IOKitError> DiceAudioDevice::getChannelCounts()
{
    ChannelCounts counts = {0, 0, 0, 0, 0, 0};
    
    // Get channel configuration
    auto configResult = getChannelConfiguration();
    if (!configResult)
    {
        return std::unexpected(configResult.error());
    }
    
    ChannelConfiguration config = configResult.value();
    
    // Count channels
    counts.monoOutputChannels = config.monoOutputChannels.size();
    counts.monoInputChannels = config.monoInputChannels.size();
    
    // Count stereo pairs (divide by 2 because each pair has L and R)
    counts.stereoOutputPairs = config.stereoOutputChannels.size() / 2;
    counts.stereoInputPairs = config.stereoInputChannels.size() / 2;
    
    // Calculate total physical channels
    counts.totalOutputChannels = counts.monoOutputChannels + (counts.stereoOutputPairs * 2);
    counts.totalInputChannels = counts.monoInputChannels + (counts.stereoInputPairs * 2);
    
    return counts;
}
```

## Benefits

1. **Accurate Channel Information**: The driver will have accurate information about the number and types of channels available on the device.

2. **Better User Experience**: The application can display the correct channel names and types to the user.

3. **Improved Routing**: The driver can use the channel information to set up the correct routing matrix.

4. **Compatibility**: The driver will work correctly with devices that have different channel configurations.

## Implementation Steps

1. Create a new branch for the channel integration changes.
2. Add the new methods to DiceAudioDevice.h and DiceAudioDevice.cpp.
3. Update the DiceAudioDevice::init method to read the channel configuration.
4. Add unit tests for the new methods.
5. Test the changes with the Venice device.
6. Merge the changes into the main branch.

## Future Enhancements

1. **Channel Mapping**: Create a mapping between the channel names and the actual audio channels in the device.

2. **Channel Configuration UI**: Add a UI component to display and configure the channel routing.

3. **Persistence**: Save and restore channel configurations between sessions.

4. **Multiple Device Support**: Extend the implementation to support multiple DICE devices with different channel configurations.
