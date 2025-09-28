#include "cli/validators/idea_validator.h"
#include <algorithm>
#include <cctype>

namespace dist_prompt {
namespace cli {
namespace validators {

IdeaValidator::IdeaValidator() = default;

bool IdeaValidator::validate(const std::string& ideaText) {
    // Clear previous errors
    errors_.clear();
    
    // Run all validation checks
    bool lengthValid = validateLength(ideaText);
    bool contentValid = validateContent(ideaText);
    bool noDisallowedContent = validateDisallowedContent(ideaText);
    
    // Return true only if all validations pass
    return lengthValid && contentValid && noDisallowedContent;
}

std::vector<std::string> IdeaValidator::getErrors() const {
    return errors_;
}

bool IdeaValidator::validateLength(const std::string& ideaText) {
    if (ideaText.length() < minLength_) {
        errors_.push_back("Idea text is too short. Minimum length is " + 
                         std::to_string(minLength_) + " characters.");
        return false;
    }
    
    if (ideaText.length() > maxLength_) {
        errors_.push_back("Idea text is too long. Maximum length is " + 
                         std::to_string(maxLength_) + " characters.");
        return false;
    }
    
    return true;
}

bool IdeaValidator::validateContent(const std::string& ideaText) {
    bool valid = true;
    
    // Check if the idea contains at least some non-whitespace content
    if (std::all_of(ideaText.begin(), ideaText.end(), [](char c) { return std::isspace(c); })) {
        errors_.push_back("Idea text cannot be empty or contain only whitespace.");
        valid = false;
    }
    
    // Check if the idea contains some basic required elements
    std::regex functionPattern("\\b(function|feature|capability)\\b", std::regex_constants::icase);
    if (!std::regex_search(ideaText, functionPattern)) {
        errors_.push_back("Idea text should describe at least one function, feature, or capability.");
        valid = false;
    }
    
    return valid;
}

bool IdeaValidator::validateDisallowedContent(const std::string& ideaText) {
    bool valid = true;
    
    // Check for disallowed content (e.g., offensive language, code injection)
    std::vector<std::regex> disallowedPatterns = {
        std::regex("\\b(exec|system|popen|eval)\\s*\\(", std::regex_constants::icase),
        std::regex("\\b(rm|del|format)\\s+(-rf|/s|c:)", std::regex_constants::icase)
    };
    
    for (const auto& pattern : disallowedPatterns) {
        if (std::regex_search(ideaText, pattern)) {
            errors_.push_back("Idea text contains potentially harmful content.");
            valid = false;
            break;
        }
    }
    
    return valid;
}

} // namespace validators
} // namespace cli
} // namespace dist_prompt