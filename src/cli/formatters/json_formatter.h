#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace dist_prompt {
namespace cli {
namespace formatters {

/**
 * @brief JSON formatter using nlohmann/json v3.11.2
 * 
 * Handles formatting output results as JSON.
 */
class JsonFormatter {
public:
    JsonFormatter();
    ~JsonFormatter() = default;
    
    /**
     * @brief Format data as JSON
     * 
     * @param data JSON object to format
     * @return std::string Formatted JSON string
     */
    std::string format(const nlohmann::json& data);
    
    /**
     * @brief Set the indentation level for formatted output
     * 
     * @param indent Number of spaces for indentation
     */
    void setIndent(int indent);
    
    /**
     * @brief Validate if a string is valid JSON
     * 
     * @param jsonStr String to validate
     * @return bool True if the string is valid JSON
     */
    bool isValidJson(const std::string& jsonStr);
    
private:
    int indentLevel_;
};

} // namespace formatters
} // namespace cli
} // namespace dist_prompt