# Summary for include/FWA/AudioSubunit.hpp

This C++ header file defines the `FWA::AudioSubunit` class. This class represents an AV/C (Audio Video Control) Audio Subunit found on a FireWire device. Audio Subunits are responsible for audio-related functionalities, such as managing audio input/output plugs and their associated stream formats. It inherits from the base `FWA::Subunit` class.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<vector>`: For `std::vector`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `<optional>`: For `std::optional`.
    -   `"Subunit.hpp"`: For the base `Subunit` class.
    -   `"AudioPlug.hpp"`: For the `AudioPlug` class, as an Audio Subunit contains audio plugs.
    -   `"CommandInterface.h"`: For the `CommandInterface` to send AV/C commands.
    -   `"DescriptorReader.hpp"`: For the `DescriptorReader` used in parsing.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<nlohmann/json.hpp>`: For JSON serialization.

-   **Class `AudioSubunit` (inherits from `Subunit`):**
    -   **Public Interface:**
        -   **Constructor:**
            -   `AudioSubunit(uint8_t id, std::shared_ptr<CommandInterface> commandInterface, std::shared_ptr<spdlog::logger> logger);`
            -   Initializes the subunit with its `id`, a shared pointer to a `CommandInterface` (for sending AV/C commands to this subunit), and a logger.
            -   It calls the base `Subunit` constructor, passing `SubunitType::Audio` and the `id`.
        -   **Descriptor Parsing (Virtual Override):**
            -   `void parseDescriptors(DescriptorReader& reader) override;`
            -   This method is responsible for using the provided `DescriptorReader` to:
                -   Read "Audio Subunit Information Descriptor" (if defined by the device for this subunit, though often capabilities are primarily defined by its plugs).
                -   Discover and parse all "Audio Plug Descriptors" associated with this subunit.
                -   For each discovered audio plug, it would create an `AudioPlug` object.
                -   It would then likely use a `PlugDetailParser` (or similar logic within `DescriptorReader` or this class) to populate each `AudioPlug` with its supported `AudioStreamFormat`s and its name (from an INFO block).
                -   The parsed `AudioPlug` objects are stored in `_inputPlugs` and `_outputPlugs` vectors.
        -   **Plug Accessors:**
            -   `const std::vector<AudioPlug>& getInputPlugs() const;`: Returns a const reference to the vector of input audio plugs.
            -   `const std::vector<AudioPlug>& getOutputPlugs() const;`: Returns a const reference to the vector of output audio plugs.
            -   `std::optional<AudioPlug> getPlugByID(uint8_t plugID) const;`: Attempts to find and return an `AudioPlug` (input or output) by its ID.
        -   **JSON Serialization (Virtual Override):**
            -   `nlohmann::json toJson() const override;`:
                -   Serializes the Audio Subunit's information to a `nlohmann::json` object.
                -   This would include the subunit's ID and type (from the base class `Subunit::toJson()`).
                -   It would then add arrays for "inputPlugs" and "outputPlugs", where each element is the JSON representation of an `AudioPlug` (obtained by calling `AudioPlug::toJson()`).
        -   **Audio Control Methods (Conceptual - specific methods would be added as needed):**
            -   The class might declare methods for controlling audio-specific features of this subunit, such as:
                -   Volume or mute controls for specific plugs or internal audio paths within the subunit.
                -   Controls for any signal processing capabilities (e.g., EQ, effects) if the subunit has DSP.
                -   These methods would use the `_commandInterface` to send targeted AV/C commands to this subunit.

    -   **Protected/Private Members (Conceptual - implementation details in .cpp):**
        -   `_inputPlugs`: `std::vector<AudioPlug>` - Stores discovered input audio plugs.
        -   `_outputPlugs`: `std::vector<AudioPlug>` - Stores discovered output audio plugs.
        -   (The `_commandInterface` and `_logger` are inherited or stored from the constructor).

**Overall Role:**
The `AudioSubunit` class provides an object-oriented representation of an AV/C Audio Subunit within a FireWire device. Its primary responsibilities are:
1.  To discover and model the audio input and output plugs it contains, along with their supported audio formats.
2.  To provide an interface for querying these plugs and their capabilities.
3.  To potentially offer methods for controlling audio-specific parameters of the subunit via AV/C commands.
It works in conjunction with `DescriptorReader` and `PlugDetailParser` to understand its own structure and capabilities from the device's descriptors. An instance of `AudioSubunit` is typically created and managed by the `AudioDevice` class.
