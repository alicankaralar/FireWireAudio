# Summary for src/FWA/AudioStreamFormat.cpp

This C++ file implements the `FWA::AudioStreamFormat` class. This class is a straightforward data structure used to represent the format of an audio stream, including its sample rate, bit depth, and number of channels.

**Key Functionalities:**

-   **Constructors:**
    -   Default Constructor: `AudioStreamFormat()`
        -   Initializes the format to a common default: 44100 Hz sample rate, 16-bit depth, and 2 channels (stereo).
    -   Parameterized Constructor: `AudioStreamFormat(double sampleRate, uint8_t bitDepth, uint8_t numChannels)`
        -   Allows creating an instance with specific format parameters.

-   **Data Storage (Member Variables):**
    -   `_sampleRate`: A `double` storing the sample rate in Hz (e.g., 44100.0, 48000.0).
    -   `_bitDepth`: A `uint8_t` storing the bit depth of the audio samples (e.g., 16, 24, 32).
    -   `_numChannels`: A `uint8_t` storing the number of audio channels (e.g., 1 for mono, 2 for stereo).

-   **Accessor Methods (Getters):**
    -   `getSampleRate() const`: Returns the sample rate.
    -   `getBitDepth() const`: Returns the bit depth.
    -   `getNumChannels() const`: Returns the number of channels.

-   **Mutator Methods (Setters):**
    -   `setSampleRate(double sampleRate)`
    -   `setBitDepth(uint8_t bitDepth)`
    -   `setNumChannels(uint8_t numChannels)`

-   **Comparison Operators:**
    -   `operator==(const AudioStreamFormat& other) const`: Compares two `AudioStreamFormat` objects for equality based on all three properties (sample rate, bit depth, and number of channels).
    -   `operator!=(const AudioStreamFormat& other) const`: The logical negation of `operator==`.

-   **JSON Serialization (`toJson() const`):**
    -   Converts the `AudioStreamFormat` object's properties into a `nlohmann::json` object.
    -   The JSON object will have keys like "sample_rate", "bit_depth", and "num_channels".
    -   This is used by other classes (like `AudioPlug`) when serializing their own information to JSON.

**Overall Role:**
The `AudioStreamFormat` class provides a standardized way to represent and manage audio format information within the FWA library. It's used when:
-   Parsing device descriptors to determine supported formats.
-   Configuring audio streams for input or output.
-   Comparing formats.
-   Providing device capability information (e.g., via JSON).
Its simplicity and clear interface make it easy to work with audio format details throughout the codebase.
