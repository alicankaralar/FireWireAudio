# Summary for include/FWA/JsonHelpers.hpp

This C++ header file defines the `FWA::JsonHelpers` namespace. This namespace contains a collection of utility functions designed to safely extract values of various types from `nlohmann::json` objects. These helpers are crucial for robust JSON parsing, especially when dealing with JSON data where certain fields might be optional or their types might not always be guaranteed.

**Key Declarations and Components:**

-   **Namespace `FWA::JsonHelpers`:** All utility functions are declared within this namespace.

-   **Includes:**
    -   `<string>`: For `std::string`.
    -   `<optional>`: For `std::optional`, which is used as the return type to indicate success or failure of value extraction.
    -   `<nlohmann/json.hpp>`: For the `nlohmann::json` class itself.

-   **Utility Functions Declared:**
    -   Each function attempts to find a specified `key` within a given `nlohmann::json` object `j`. If the key exists and its value is of the expected type, the function returns an `std::optional` containing that value. Otherwise (key not found, or value is of a different type), it returns `std::nullopt`.

    -   `std::optional<std::string> getOptionalString(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts a string value.

    -   `std::optional<int> getOptionalInt(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts an integer value.

    -   `std::optional<double> getOptionalDouble(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts a double-precision floating-point value.

    -   `std::optional<bool> getOptionalBool(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts a boolean value.

    -   `std::optional<nlohmann::json> getOptionalObject(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts a nested JSON object. Checks if `j.at(key).is_object()`.

    -   `std::optional<nlohmann::json> getOptionalArray(const nlohmann::json& j, const std::string& key);`
        -   Safely extracts a JSON array. Checks if `j.at(key).is_array()`.

**Overall Role:**
The `JsonHelpers` provide a layer of safety and convenience when working with `nlohmann::json` objects, particularly when deserializing data. Instead of directly accessing JSON fields (which might throw exceptions if a key is missing or if a type conversion fails), these helper functions allow for a more graceful way to handle such situations.
-   **Robust Parsing:** They prevent crashes due to unexpected JSON structures.
-   **Clearer Code:** Code using these helpers can explicitly check if an optional value is present before using it, leading to more readable and maintainable parsing logic.
These utilities are likely used by any class within the FWA library that needs to read data from JSON configurations or interpret JSON-formatted information received from other components (e.g., device information from the `FWADaemon`).
