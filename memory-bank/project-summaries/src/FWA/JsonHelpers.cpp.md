# Summary for src/FWA/JsonHelpers.cpp

This C++ file implements a set of helper functions within the `FWA::JsonHelpers` namespace. These functions are designed to safely extract values of various types from `nlohmann::json` objects. They provide a convenient way to handle JSON parsing where keys might be missing or values might not be of the expected type, returning `std::optional` to indicate success or failure of the extraction.

**Key Functionalities (Helper Functions):**

-   **`std::optional<std::string> getOptionalString(const nlohmann::json& j, const std::string& key)`:**
    -   Attempts to find the specified `key` within the JSON object `j`.
    -   If the key exists and its corresponding value is a string, it returns an `std::optional` containing the string value.
    -   If the key does not exist, or if the value associated with the key is not a string, it returns `std::nullopt`.

-   **`std::optional<int> getOptionalInt(const nlohmann::json& j, const std::string& key)`:**
    -   Similar to `getOptionalString`, but attempts to extract an integer value.
    -   Returns `std::optional<int>`.

-   **`std::optional<double> getOptionalDouble(const nlohmann::json& j, const std::string& key)`:**
    -   Attempts to extract a double-precision floating-point value.
    -   Returns `std::optional<double>`.

-   **`std::optional<bool> getOptionalBool(const nlohmann::json& j, const std::string& key)`:**
    -   Attempts to extract a boolean value.
    -   Returns `std::optional<bool>`.

-   **`std::optional<nlohmann::json> getOptionalObject(const nlohmann::json& j, const std::string& key)`:**
    -   Attempts to find the specified `key` and checks if its value is a JSON object (i.e., `j.is_object()`).
    -   If successful, returns an `std::optional` containing the nested JSON object.
    -   Otherwise, returns `std::nullopt`.

-   **`std::optional<nlohmann::json> getOptionalArray(const nlohmann::json& j, const std::string& key)`:**
    -   Attempts to find the specified `key` and checks if its value is a JSON array (i.e., `j.is_array()`).
    -   If successful, returns an `std::optional` containing the JSON array.
    -   Otherwise, returns `std::nullopt`.

**Overall Role:**
The `JsonHelpers` provide robust and safe utility functions for parsing `nlohmann::json` data. By returning `std::optional`, they allow calling code to gracefully handle situations where expected JSON fields are missing or have an incorrect type, preventing crashes or exceptions that might occur with direct access (e.g., `j.at(key)` or `j[key].get<type>()`). These helpers are likely used by any classes in the FWA library that need to deserialize data from JSON, such as when loading configurations or interpreting JSON-formatted device information. They promote safer and more resilient JSON parsing.
