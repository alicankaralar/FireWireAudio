# Channel Name Deduplication Plan

## Issue

The current implementation of the channel name detection in the scanner tool has a duplication issue. When running the scanner, we're seeing duplicate channel names in the output, but not all channels are duplicated. This suggests there might be an issue with how we're collecting and processing the channel names.

After analyzing the code, I've identified the root cause:

1. We're finding channel names in two different ways:
   - Quadlet-level strings (whole registers)
   - Byte-level strings (individual bytes within registers)

2. We're using a set called `uniqueStrings` to avoid duplicates when combining all string texts for pattern matching, but we're still adding all matches to the `outputChannelNames` and `inputChannelNames` vectors without checking for duplicates.

3. The regex search is finding the same channel names multiple times in the combined text.

## Solution

To fix this issue, we need to modify the code to properly deduplicate the channel names before reporting them. Here's the plan:

1. Use a set to store unique channel names instead of a vector.
2. After finding all matches with regex, convert the set to a vector for reporting.
3. Alternatively, we could use a map to store channel names with their numbers as keys, which would automatically deduplicate them.

## Implementation Details

The issue is in the `exploreChannelNamesArea` function in `src/tools/scanner/utils.cpp`, specifically in the section where we search for channel names using regex (around lines 764-813).

Here's how we should modify the code:

1. Replace the vectors with sets:
   ```cpp
   std::set<std::string> outputChannelNames;
   std::set<std::string> inputChannelNames;
   ```

2. When adding matches to the sets, they will automatically be deduplicated:
   ```cpp
   while (std::regex_search(searchStart, allText.cend(), match, outputChannelPattern))
   {
       outputChannelNames.insert(match[0]);
       // Extract the channel number
       int channelNum = std::stoi(match[1]);
       outputChannelNumbers.insert(channelNum);
       searchStart = match.suffix().first;
   }
   ```

3. When reporting the channel names, convert the sets to vectors if needed:
   ```cpp
   if (!outputChannelNames.empty())
   {
       std::cout << "Found " << outputChannelNames.size() << " output channel names matching pattern 'OUTPUT CH#':" << std::endl;
       for (const auto &name : outputChannelNames)
       {
           std::cout << "  " << name << std::endl;
       }
   }
   ```

4. Update the calculation of total channels to use the size of the sets:
   ```cpp
   int totalOutputChannels = outputChannelNames.size();
   int totalInputChannels = inputChannelNames.size();
   ```

This approach will ensure that each channel name is only counted and reported once, regardless of whether it was found in quadlet-level or byte-level strings.

## Next Steps

1. Switch to Code mode to implement these changes.
2. Test the changes by running the scanner tool again.
3. Verify that the channel names are no longer duplicated in the output.
4. Continue with the next steps in the scanner improvement plan.