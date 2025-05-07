---
**STATUS UPDATE (2025-05-06):** Significant portions of this plan have been implemented as part of the `firewire_scanner` tool enhancements.
Key completed areas include:
*   Dynamic DICE Base Address Discovery (Section 1.1)
*   Robust RX/TX Stream Register Reading (Section 3)
*   Comprehensive Channel Name Extraction Overhaul (Sections 1.2, 1.3, 2.1, 2.2, 4.1)
*   Channel Count Validation Systems (Sections 1.3, 3.2, 3.3, 4.2, 4.3)
The scanner now compiles successfully with these improvements.
---
# FireWire Scanner Improvement Plan

This document outlines a plan to improve the firewire_scanner tool to make it more dynamic, remove duplicated logic, and improve how it reads RX/TX registers and discovers channels.

## Current Issues

1. **Hardcoded Channel Discovery**:
   - The tool uses hardcoded addresses for channel names (e.g., 0xffffe00001a8)
   - It uses regex patterns to find channel names in the combined text from registers
   - The channel counts are calculated based on the number of regex matches found

2. **Duplicated Logic**:
   - There's duplicated logic between dice_helpers.cpp and utils.cpp for reading registers
   - The exploreDiceMemoryLayout and exploreChannelNamesArea functions have similar code for finding strings
   - The extractCoherentRegisterStrings function also has similar string extraction logic

3. **RX/TX Register Reading Issues**:
   - The readDiceTxStreamRegistersInternal and readDiceRxStreamRegistersInternal functions use hardcoded offsets and sizes
   - They have limits on the number of streams to read, but these limits may not reflect reality
   - The tool doesn't validate the stream counts against the actual hardware capabilities

4. **Unreasonable Channel Counts**:
   - The tool may be reading too many channels because it's not properly detecting the end of the channel list
   - The regex patterns might be matching more than they should
   - The tool doesn't validate the channel numbers against the actual hardware capabilities

## Improvement Plan

### 1. Make Channel Configuration Discovery More Dynamic

#### 1.1 Implement Dynamic Address Discovery

Instead of hardcoding the channel names address, we should try to discover it dynamically:

```cpp
// Add to utils.hpp
uint64_t discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface, 
                                    io_service_t service, 
                                    UInt32 generation);
```

```cpp
// Implementation in utils.cpp
uint64_t discoverChannelNamesAddress(IOFireWireDeviceInterface **deviceInterface, 
                                    io_service_t service, 
                                    UInt32 generation)
{
    // Known potential addresses for channel names
    std::vector<uint64_t> potentialAddresses = {
        0xffffe00001a8, // Known address from previous scans
        0xffffe0000090, // Channel Configuration area
        0xffffe0000100  // Another potential area
    };
    
    // Try each address and look for channel name patterns
    for (uint64_t addr : potentialAddresses)
    {
        // Read a block of memory at this address
        const int BLOCK_SIZE = 256; // 256 quadlets = 1024 bytes
        std::vector<uint32_t> memoryBlock;
        bool validBlock = true;
        
        for (int i = 0; i < BLOCK_SIZE; i++)
        {
            UInt32 value = 0;
            IOReturn status = safeReadQuadlet(deviceInterface, service, addr + (i * 4), value, generation);
            if (status != kIOReturnSuccess)
            {
                validBlock = false;
                break;
            }
            memoryBlock.push_back(value);
        }
        
        if (!validBlock)
            continue;
        
        // Convert the memory block to a string
        std::string blockText;
        for (uint32_t value : memoryBlock)
        {
            uint32_t hostValue = CFSwapInt32LittleToHost(value);
            // Extract individual bytes
            for (int bytePos = 3; bytePos >= 0; bytePos--)
            {
                char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);
                if (c >= 32 && c <= 126)
                    blockText += c;
            }
        }
        
        // Check if this block contains channel name patterns
        std::regex channelPattern(R"((?:OUTPUT|INPUT)(?:\s+ST)?\s+CH\d+)");
        std::smatch match;
        if (std::regex_search(blockText, match, channelPattern))
        {
            // Found a channel name pattern, this is likely the channel names address
            return addr;
        }
    }
    
    // If no address was found, return the known address as a fallback
    return 0xffffe00001a8;
}
```

#### 1.2 Improve Channel Name Extraction

Modify the exploreChannelNamesArea function to use a more robust approach for extracting channel names:

```cpp
void exploreChannelNamesArea(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service,
                            FireWireDevice &device,
                            UInt32 generation)
{
    std::cout << "\n=== DETAILED CHANNEL NAMES EXPLORATION ===\n" << std::endl;
    
    // Dynamically discover the channel names address
    uint64_t channelNamesBaseAddr = discoverChannelNamesAddress(deviceInterface, service, generation);
    std::cout << "Using channel names base address: 0x" << std::hex << channelNamesBaseAddr << std::dec << std::endl;
    
    // Rest of the function remains similar, but with improved string extraction...
    
    // Improved regex patterns for channel detection
    std::regex outputChannelPattern(R"(OUTPUT\s+CH(\d+)(?!\S))"); // Ensure it's not part of a larger word
    std::regex inputChannelPattern(R"(INPUT\s+CH(\d+)(?!\S))");
    std::regex stereoOutputPattern(R"(OUTPUT\s+ST\s+CH(\d+)([LR])(?!\S))");
    std::regex stereoInputPattern(R"(INPUT\s+ST\s+CH(\d+)([LR])(?!\S))");
    
    // ... (rest of the function)
    
    // Validate channel numbers
    std::set<int> outputChannelNumbers;
    std::set<int> inputChannelNumbers;
    std::set<int> stereoOutputChannelNumbers;
    std::set<int> stereoInputChannelNumbers;
    
    // Extract channel numbers from names and validate them
    for (const auto& name : outputChannelNames)
    {
        std::smatch match;
        if (std::regex_match(name, match, outputChannelPattern))
        {
            int channelNum = std::stoi(match[1]);
            outputChannelNumbers.insert(channelNum);
        }
    }
    
    // Do the same for other channel types...
    
    // Validate that channel numbers are sequential and within reasonable limits
    validateChannelNumbers(outputChannelNumbers, "Output");
    validateChannelNumbers(inputChannelNumbers, "Input");
    validateChannelNumbers(stereoOutputChannelNumbers, "Stereo Output");
    validateChannelNumbers(stereoInputChannelNumbers, "Stereo Input");
}
```

#### 1.3 Add Channel Number Validation

Add a function to validate channel numbers:

```cpp
void validateChannelNumbers(const std::set<int>& channelNumbers, const std::string& channelType)
{
    if (channelNumbers.empty())
        return;
    
    // Check if the numbers are sequential
    int expectedNext = *channelNumbers.begin();
    bool sequential = true;
    std::vector<int> gaps;
    
    for (int num : channelNumbers)
    {
        if (num != expectedNext)
        {
            sequential = false;
            gaps.push_back(expectedNext);
            while (++expectedNext < num)
                gaps.push_back(expectedNext);
        }
        expectedNext = num + 1;
    }
    
    // Report findings
    std::cout << channelType << " channel numbers: ";
    if (sequential)
    {
        std::cout << "Sequential from " << *channelNumbers.begin() << " to " << *channelNumbers.rbegin() << std::endl;
    }
    else
    {
        std::cout << "Non-sequential with gaps: ";
        for (int gap : gaps)
            std::cout << gap << " ";
        std::cout << std::endl;
    }
    
    // Check if the numbers are within reasonable limits
    const int MAX_REASONABLE_CHANNELS = 64; // Adjust based on device capabilities
    if (*channelNumbers.rbegin() > MAX_REASONABLE_CHANNELS)
    {
        std::cout << "Warning: " << channelType << " channel numbers exceed reasonable limit of " 
                  << MAX_REASONABLE_CHANNELS << std::endl;
    }
}
```

### 2. Remove Duplicated Logic

#### 2.1 Create a Common String Extraction Utility

Create a common utility for string extraction to be used by all functions:

```cpp
// Add to utils.hpp
struct StringMatch {
    std::string text;
    uint64_t address;
    bool isByteLevel; // Whether the string was found at the byte level or quadlet level
};

std::vector<StringMatch> extractStringsFromMemory(const std::map<uint64_t, uint32_t>& registers);
```

```cpp
// Implementation in utils.cpp
std::vector<StringMatch> extractStringsFromMemory(const std::map<uint64_t, uint32_t>& registers)
{
    std::vector<StringMatch> results;
    
    // Sort registers by address
    std::vector<std::pair<uint64_t, uint32_t>> sortedRegisters;
    for (const auto& regPair : registers)
    {
        sortedRegisters.push_back(regPair);
    }
    std::sort(sortedRegisters.begin(), sortedRegisters.end());
    
    // Extract quadlet-level strings
    std::string currentString;
    uint64_t stringStartAddr = 0;
    bool inString = false;
    
    for (const auto& regPair : sortedRegisters)
    {
        uint64_t addr = regPair.first;
        uint32_t rawValue = regPair.second;
        uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);
        std::string ascii = interpretAsASCII(hostValue);
        
        if (!ascii.empty())
        {
            if (!inString)
            {
                inString = true;
                stringStartAddr = addr;
            }
            currentString += ascii;
        }
        else
        {
            if (inString)
            {
                // End of string
                if (currentString.length() >= 3) // Minimum meaningful length
                {
                    StringMatch match;
                    match.text = currentString;
                    match.address = stringStartAddr;
                    match.isByteLevel = false;
                    results.push_back(match);
                }
                currentString.clear();
                inString = false;
            }
        }
    }
    
    // Check if we were in a string at the end
    if (inString && currentString.length() >= 3)
    {
        StringMatch match;
        match.text = currentString;
        match.address = stringStartAddr;
        match.isByteLevel = false;
        results.push_back(match);
    }
    
    // Extract byte-level strings
    std::string byteString;
    uint64_t byteStringStartAddr = 0;
    bool inByteString = false;
    
    for (const auto& regPair : sortedRegisters)
    {
        uint64_t addr = regPair.first;
        uint32_t rawValue = regPair.second;
        uint32_t hostValue = CFSwapInt32LittleToHost(rawValue);
        
        // Extract individual bytes
        for (int bytePos = 3; bytePos >= 0; bytePos--)
        {
            char c = static_cast<char>((hostValue >> (8 * bytePos)) & 0xFF);
            
            if (c >= 32 && c <= 126) // Printable ASCII
            {
                if (!inByteString)
                {
                    inByteString = true;
                    byteStringStartAddr = addr;
                }
                byteString += c;
            }
            else if (c == 0 && inByteString)
            {
                // Null terminator - potential end of string
                if (byteString.length() >= 3)
                {
                    StringMatch match;
                    match.text = byteString;
                    match.address = byteStringStartAddr;
                    match.isByteLevel = true;
                    results.push_back(match);
                }
                byteString.clear();
                inByteString = false;
            }
            else if (inByteString)
            {
                // Non-printable, non-null character breaks the string
                if (byteString.length() >= 3)
                {
                    StringMatch match;
                    match.text = byteString;
                    match.address = byteStringStartAddr;
                    match.isByteLevel = true;
                    results.push_back(match);
                }
                byteString.clear();
                inByteString = false;
            }
        }
    }
    
    // Check for any remaining byte string
    if (inByteString && byteString.length() >= 3)
    {
        StringMatch match;
        match.text = byteString;
        match.address = byteStringStartAddr;
        match.isByteLevel = true;
        results.push_back(match);
    }
    
    return results;
}
```

#### 2.2 Refactor exploreDiceMemoryLayout and exploreChannelNamesArea

Refactor these functions to use the common string extraction utility:

```cpp
void exploreDiceMemoryLayout(IOFireWireDeviceInterface **deviceInterface,
                            io_service_t service,
                            FireWireDevice &device,
                            UInt32 generation,
                            uint64_t baseAddr)
{
    // ... (existing code for reading registers)
    
    // Use the common string extraction utility
    std::vector<StringMatch> strings = extractStringsFromMemory(channelRegisters);
    
    // Report the results
    std::cout << "\n--- String Data ---" << std::endl;
    for (const auto& match : strings)
    {
        std::cout << (match.isByteLevel ? "Byte-level" : "Quadlet-level") << " string at 0x" 
                  << std::hex << match.address << std::dec << ": \"" << match.text << "\"" << std::endl;
    }
    
    // ... (rest of the function)
}
```

### 3. Improve RX/TX Register Reading

#### 3.1 Make Stream Count Detection More Dynamic

Modify the readDiceTxStreamRegistersInternal and readDiceRxStreamRegistersInternal functions to be more dynamic:

```cpp
static void readDiceTxStreamRegistersInternal(IOFireWireDeviceInterface **deviceInterface, io_service_t service,
                                            FireWireDevice &device, uint64_t discoveredDiceBase, UInt32 generation,
                                            uint32_t txStreamSizeQuadlets)
{
    // Validate stream count against hardware capabilities
    uint32_t maxStreams = 0;
    switch (device.diceChipType)
    {
    case DiceChipType::DiceII:
        maxStreams = 4;
        break;
    case DiceChipType::DiceMini:
        maxStreams = 2;
        break;
    case DiceChipType::DiceJr:
        maxStreams = 1;
        break;
    default:
        maxStreams = 8; // Conservative default
        break;
    }
    
    // Limit the stream count to the maximum for this chip type
    uint32_t streamCount = std::min(device.txStreamCount, maxStreams);
    
    if (streamCount == 0 || txStreamSizeQuadlets == 0)
    {
        std::cerr << "Debug [DICE]: Skipping TX stream register read (count=" << streamCount 
                  << ", size=" << txStreamSizeQuadlets << ")." << std::endl;
        return;
    }
    
    std::cerr << "Debug [DICE]: Reading TX stream registers for " << streamCount 
              << " streams (size=" << txStreamSizeQuadlets << " quadlets)..." << std::endl;
    
    // ... (rest of the function, using streamCount instead of device.txStreamCount)
}
```

#### 3.2 Add Validation for Stream Counts

Add a function to validate stream counts:

```cpp
void validateStreamCounts(FireWireDevice &device)
{
    // Determine maximum streams based on chip type
    uint32_t maxTxStreams = 0;
    uint32_t maxRxStreams = 0;
    
    switch (device.diceChipType)
    {
    case DiceChipType::DiceII:
        maxTxStreams = 4;
        maxRxStreams = 4;
        break;
    case DiceChipType::DiceMini:
        maxTxStreams = 2;
        maxRxStreams = 2;
        break;
    case DiceChipType::DiceJr:
        maxTxStreams = 1;
        maxRxStreams = 1;
        break;
    default:
        maxTxStreams = 8; // Conservative default
        maxRxStreams = 8;
        break;
    }
    
    // Check if the stream counts are reasonable
    if (device.txStreamCount > maxTxStreams)
    {
        std::cerr << "Warning [DICE]: TX stream count (" << device.txStreamCount 
                  << ") exceeds maximum for this chip type (" << maxTxStreams 
                  << "). Limiting to " << maxTxStreams << "." << std::endl;
        device.txStreamCount = maxTxStreams;
    }
    
    if (device.rxStreamCount > maxRxStreams)
    {
        std::cerr << "Warning [DICE]: RX stream count (" << device.rxStreamCount 
                  << ") exceeds maximum for this chip type (" << maxRxStreams 
                  << "). Limiting to " << maxRxStreams << "." << std::endl;
        device.rxStreamCount = maxRxStreams;
    }
}
```

#### 3.3 Add Stream Channel Count Validation

Add a function to validate the number of channels per stream:

```cpp
void validateStreamChannels(FireWireDevice &device, uint64_t discoveredDiceBase, 
                           IOFireWireDeviceInterface **deviceInterface, io_service_t service, UInt32 generation)
{
    // Find TX Parameter Space Offset
    uint64_t txParamSpaceOffsetAddr = discoveredDiceBase + DICE_REGISTER_TX_PAR_SPACE_OFF;
    uint32_t txParamSpaceOffsetQuadlets = 0;
    if (device.diceRegisters.count(txParamSpaceOffsetAddr))
    {
        txParamSpaceOffsetQuadlets = CFSwapInt32LittleToHost(device.diceRegisters[txParamSpaceOffsetAddr]);
    }
    else
    {
        std::cerr << "Warning [DICE]: TX Parameter Space Offset not previously read." << std::endl;
        return;
    }
    uint64_t txParamSpaceBase = discoveredDiceBase + (txParamSpaceOffsetQuadlets * 4);
    
    // Check each TX stream for reasonable channel counts
    for (unsigned int i = 0; i < device.txStreamCount; ++i)
    {
        uint64_t streamInstanceOffsetBytes = i * device.txStreamSizeQuadlets * 4;
        uint64_t nbAudioAddr = txParamSpaceBase + streamInstanceOffsetBytes + DICE_REGISTER_TX_NB_AUDIO_BASE;
        
        UInt32 nbAudio = 0;
        IOReturn status = safeReadQuadlet(deviceInterface, service, nbAudioAddr, nbAudio, generation);
        if (status == kIOReturnSuccess)
        {
            nbAudio = CFSwapInt32LittleToHost(nbAudio);
            
            // Check if the channel count is reasonable
            const uint32_t MAX_CHANNELS_PER_STREAM = 64; // Adjust based on device capabilities
            if (nbAudio > MAX_CHANNELS_PER_STREAM)
            {
                std::cerr << "Warning [DICE]: TX stream " << i << " has " << nbAudio 
                          << " channels, which exceeds the reasonable limit of " 
                          << MAX_CHANNELS_PER_STREAM << "." << std::endl;
            }
        }
    }
    
    // Do the same for RX streams...
}
```

### 4. Address Unreasonable Channel Counts

#### 4.1 Improve Channel Name Matching

Modify the regex patterns to be more specific:

```cpp
// More specific regex patterns
std::regex outputChannelPattern(R"(OUTPUT\s+CH(\d+)(?!\S))"); // Ensure it's not part of a larger word
std::regex inputChannelPattern(R"(INPUT\s+CH(\d+)(?!\S))");
std::regex stereoOutputPattern(R"(OUTPUT\s+ST\s+CH(\d+)([LR])(?!\S))");
std::regex stereoInputPattern(R"(INPUT\s+ST\s+CH(\d+)([LR])(?!\S))");
```

#### 4.2 Add Channel Count Validation

Add a function to validate the total channel count:

```cpp
void validateTotalChannelCount(int totalChannels)
{
    // Check if the total channel count is reasonable
    const int MAX_REASONABLE_TOTAL_CHANNELS = 128; // Adjust based on device capabilities
    if (totalChannels > MAX_REASONABLE_TOTAL_CHANNELS)
    {
        std::cout << "Warning: Total channel count (" << totalChannels 
                  << ") exceeds reasonable limit of " << MAX_REASONABLE_TOTAL_CHANNELS << std::endl;
    }
}
```

#### 4.3 Cross-Validate Channel Counts with Stream Information

Add a function to cross-validate channel counts with stream information:

```cpp
void crossValidateChannelCounts(FireWireDevice &device, int totalMonoOutputs, int totalMonoInputs,
                               int totalStereoOutputs, int totalStereoInputs)
{
    // Calculate total channels from stream information
    int totalStreamOutputChannels = 0;
    int totalStreamInputChannels = 0;
    
    // Iterate through TX streams
    for (const auto& regPair : device.diceRegisters)
    {
        uint64_t addr = regPair.first;
        uint64_t baseAddr = DICE_REGISTER_BASE;
        int64_t offset = addr - baseAddr;
        
        // Check if this is a TX_NB_AUDIO register
        if ((offset - DICE_REGISTER_TX_NB_AUDIO_BASE) % (device.txStreamSizeQuadlets * 4) == 0)
        {
            uint32_t channelCount = CFSwapInt32LittleToHost(regPair.second);
            totalStreamOutputChannels += channelCount;
        }
        
        // Check if this is a RX_NB_AUDIO register
        if ((offset - DICE_REGISTER_RX_NB_AUDIO_BASE) % (device.rxStreamSizeQuadlets * 4) == 0)
        {
            uint32_t channelCount = CFSwapInt32LittleToHost(regPair.second);
            totalStreamInputChannels += channelCount;
        }
    }
    
    // Calculate total channels from channel names
    int totalNamedOutputChannels = totalMonoOutputs + (totalStereoOutputs * 2);
    int totalNamedInputChannels = totalMonoInputs + (totalStereoInputs * 2);
    
    // Compare the two counts
    std::cout << "Channel count comparison:" << std::endl;
    std::cout << "  Output channels from stream info: " << totalStreamOutputChannels << std::endl;
    std::cout << "  Output channels from channel names: " << totalNamedOutputChannels << std::endl;
    std::cout << "  Input channels from stream info: " << totalStreamInputChannels << std::endl;
    std::cout << "  Input channels from channel names: " << totalNamedInputChannels << std::endl;
    
    // Check for discrepancies
    if (totalStreamOutputChannels != totalNamedOutputChannels)
    {
        std::cout << "Warning: Discrepancy in output channel counts!" << std::endl;
    }
    
    if (totalStreamInputChannels != totalNamedInputChannels)
    {
        std::cout << "Warning: Discrepancy in input channel counts!" << std::endl;
    }
}
```

## Implementation Steps

1. Create a new branch for the scanner improvements.
2. Implement the common string extraction utility in utils.cpp.
3. Modify the exploreDiceMemoryLayout and exploreChannelNamesArea functions to use the common utility.
4. Implement the dynamic channel names address discovery.
5. Improve the regex patterns for channel name matching.
6. Add validation functions for stream counts and channel counts.
7. Modify the readDiceTxStreamRegistersInternal and readDiceRxStreamRegistersInternal functions to be more dynamic.
8. Add cross-validation between channel names and stream information.
9. Test the changes with the Venice device.
10. Merge the changes into the main branch.

## Expected Benefits

1. **More Dynamic Channel Discovery**: The tool will be able to discover channel names addresses dynamically, making it more robust across different devices.

2. **Reduced Code Duplication**: By using common utilities for string extraction, the code will be more maintainable and less prone to bugs.

3. **More Accurate Stream Reading**: By validating stream counts against hardware capabilities, the tool will avoid reading too many streams.

4. **More Accurate Channel Counts**: By improving regex patterns and adding validation, the tool will report more accurate channel counts.

5. **Better Cross-Validation**: By cross-validating channel counts with stream information, the tool will provide more reliable information about the device's capabilities.
