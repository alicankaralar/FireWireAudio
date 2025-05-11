# Summary for include/FWA/Subunit.hpp

This C++ header file defines the `FWA::Subunit` class. This class serves as an abstract base class for different types of AV/C (Audio Video Control) subunits that can exist on a FireWire device, such as Audio subunits, Music (MIDI) subunits, or Vendor-Unique subunits.

**Key Declarations and Components:**

-   **Namespace `FWA`:** The class is defined within the `FWA` namespace.

-   **Includes:**
    -   `<string>`: For `std::string`.
    -   `<memory>`: For `std::shared_ptr`.
    -   `"Enums.hpp"`: For the `SubunitType` enumeration.
    -   `"CommandInterface.h"`: For the `CommandInterface` class, used to send commands to this subunit.
    -   `"DescriptorReader.hpp"`: For the `DescriptorReader` class, used by derived classes to parse their specific descriptors.
    -   `<spdlog/spdlog.h>`: For logging.
    -   `<nlohmann/json.hpp>`: For JSON serialization.

-   **Struct `SubunitIdentifier` (Often defined here or in `Enums.hpp`):**
    -   A simple struct to hold `SubunitType type;` and `uint8_t id;`.
    -   Includes an `operator==` for comparison.

-   **Class `Subunit` (Abstract Base Class):**
    -   **Protected Constructor:**
        -   `Subunit(SubunitType type, uint8_t id, std::shared_ptr<CommandInterface> commandInterface, std::shared_ptr<spdlog::logger> logger);`
        -   This constructor is intended to be called by derived classes (e.g., `AudioSubunit`, `MusicSubunit`). It initializes the common properties of a subunit: its type, ID, a command interface to communicate with it, and a logger.
    -   **Virtual Destructor:**
        -   `virtual ~Subunit() = default;`
        -   Ensures proper cleanup for derived classes when deleted via a base class pointer.
    -   **Pure Virtual Method (`parseDescriptors`):**
        -   `virtual void parseDescriptors(DescriptorReader& reader) = 0;`
        -   This is a pure virtual function, making `Subunit` an abstract class.
        -   Derived classes (like `AudioSubunit`, `MusicSubunit`) *must* implement this method to parse their own specific set of descriptors using the provided `DescriptorReader`. This is where the unique capabilities of each subunit type are discovered.
    -   **Public Accessor Methods:**
        -   `SubunitType getType() const;`: Returns the type of the subunit.
        -   `uint8_t getID() const;`: Returns the ID of the subunit.
        -   `const std::string& getName() const;`: Returns the name of the subunit (if parsed).
    -   **JSON Serialization (Virtual Method):**
        -   `virtual nlohmann::json toJson() const;`
        -   Provides a base JSON representation for a subunit, typically including its ID, type (as a string), and name.
        -   Derived classes will override this method to add their specific information (e.g., lists of plugs for `AudioSubunit`, capabilities for `MusicSubunit`) to the JSON output, often calling the base class's `toJson()` method first.
    -   **Protected Members:**
        -   `SubunitType _type;`
        -   `uint8_t _id;`
        -   `std::string _name;`
        -   `std::shared_ptr<CommandInterface> _commandInterface;`
        -   `std::shared_ptr<spdlog::logger> _logger;`
    -   **Protected Helper Method:**
        -   `void parseName(DescriptorReader& reader);`
        -   A helper function that derived classes can call within their `parseDescriptors` method. It uses the `DescriptorReader` to attempt to find and parse an INFO block descriptor associated with this subunit to retrieve its human-readable name and store it in `_name`.

**Overall Role:**
The `Subunit` class provides a common foundation and interface for all types of AV/C subunits.
-   **Abstraction:** It defines the essential properties (type, ID, name) and behaviors (descriptor parsing, JSON serialization) common to all subunits.
-   **Polymorphism:** Allows `AudioDevice` to manage a collection of different subunit types through base class pointers (`std::shared_ptr<Subunit>`).
-   **Extensibility:** New subunit types can be supported by deriving from this base class and implementing the `parseDescriptors` and `toJson` methods.
It ensures that each subunit type can discover its own specific features while sharing common identification and communication mechanisms.
