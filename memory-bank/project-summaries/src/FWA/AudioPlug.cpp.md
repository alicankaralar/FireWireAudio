# Summary for src/FWA/AudioPlug.cpp

This C++ file implements the `FWA::AudioPlug` class. This class is a data structure representing a single audio plug (either an input or an output) on a FireWire audio device. It encapsulates information about the plug's characteristics and capabilities.

**Key Functionalities:**

-   **Constructor:**
    -   `AudioPlug(uint8_t id, PlugDirection direction, uint8_t channelCount, std::string name, std::vector<AudioStreamFormat> supportedFormats)`:
        -   Initializes an `AudioPlug` object with its ID, direction (input or output, likely an enum `PlugDirection`), number of audio channels, a user-friendly name, and a vector of `AudioStreamFormat` objects that this plug supports.

-   **Data Storage (Member Variables):**
    -   `_id`: The numerical ID of the plug.
    -   `_direction`: Indicates if it's an input or output plug.
    -   `_channelCount`: The number of audio channels associated with this plug.
    -   `_name`: A string name for the plug (e.g., "Mic In 1", "Main Out L/R").
    -   `_supportedFormats`: A `std::vector` storing `AudioStreamFormat` objects that this plug can handle.

-   **Accessor Methods (Getters):**
    -   `getID() const`: Returns the plug ID.
    -   `getDirection() const`: Returns the plug direction.
    -   `getChannelCount() const`: Returns the channel count.
    -   `getName() const`: Returns the plug name.
    -   `getSupportedFormats() const`: Returns a const reference to the vector of supported audio formats.

-   **Format Management:**
    -   `addSupportedFormat(const AudioStreamFormat& format)`: Adds a new `AudioStreamFormat` to the list of supported formats for this plug.
    -   `isFormatSupported(const AudioStreamFormat& format) const`: Checks if a given `AudioStreamFormat` is present in the list of supported formats for this plug. It likely compares sample rate, bit depth, and number of channels.

-   **JSON Serialization (`toJson() const`):**
    -   Converts the `AudioPlug`'s information into a `nlohmann::json` object.
    -   This includes fields for "id", "direction" (as a string "input" or "output"), "name", "channelCount", and an array of "supportedFormats" (where each format is also converted to JSON, likely by calling a `toJson()` method on `AudioStreamFormat`).
    -   This method is used by `AudioDevice::getDeviceInfoAsJson()` to provide a comprehensive JSON representation of the device.

**Overall Role:**
The `AudioPlug` class is primarily a data container. Instances of this class are created and populated by the `DeviceParser` (or similar descriptor parsing logic) within the `AudioDevice` class. It holds the discovered properties of each physical audio connection point on the FireWire device, making this information easily accessible to other parts of the FWA library or to client applications. This information is crucial for setting up audio streams correctly.
