#pragma once

#include <string>
#include <vector>
#include <regex>

namespace dist_prompt {
namespace cli {
namespace validators {

/**
 * @brief Validates software idea text input
 * 
 * Checks for minimum length, maximum length, and content requirements.
 */
class IdeaValidator {
public:
    IdeaValidator();
    ~IdeaValidator() = default;
    
    /**
     * @brief Validate the idea text
     * 
     * @param ideaText The idea text to validate
     * @return bool True if the idea text is valid
     */
    bool validate(const std::string& ideaText);
    
    /**
     * @brief Get validation error messages
     * 
     * @return std::vector<std::string> List of error messages
     */
    std::vector<std::string> getErrors() const;
    
private:
    std::vector<std::string> errors_;
    const size_t minLength_ = 10;
    const size_t maxLength_ = 5000;
    
    // Check for minimum length
    bool validateLength(const std::string& ideaText);
    
    // Check for required content elements
    bool validateContent(const std::string& ideaText);
    
    // Check for disallowed content
    bool validateDisallowedContent(const std::string& ideaText);
};

} // namespace validators
} // namespace cli
} // namespace dist_prompt