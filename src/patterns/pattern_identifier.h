#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace dist_prompt {
namespace patterns {

/**
 * @brief Identifies program structure patterns in software ideas
 * 
 * Implements a rule-based pattern matcher to identify applicable program structure patterns.
 */
class PatternIdentifier {
public:
    /**
     * @brief Structure representing a recognized pattern
     */
    struct RecognizedPattern {
        std::string id;
        std::string name;
        std::string category;
        std::string description;
        double confidence;
        std::map<std::string, std::string> parameters;
    };
    
    /**
     * @brief Constructor
     */
    PatternIdentifier();
    
    /**
     * @brief Destructor
     */
    ~PatternIdentifier();
    
    /**
     * @brief Initialize the pattern identifier with a rule set
     * 
     * @param rulesetPath Path to the pattern ruleset file (JSON format)
     * @return bool True if initialization was successful
     */
    bool initialize(const std::string& rulesetPath);
    
    /**
     * @brief Identify patterns in a software idea
     * 
     * @param ideaData Structured data representing the software idea
     * @param minConfidence Minimum confidence threshold (0.0-1.0)
     * @return std::vector<RecognizedPattern> List of recognized patterns
     */
    std::vector<RecognizedPattern> identifyPatterns(
        const std::string& ideaData, 
        double minConfidence = 0.7);
    
    /**
     * @brief Get pattern details by ID
     * 
     * @param patternId Pattern ID
     * @return RecognizedPattern Pattern details, or empty pattern if not found
     */
    RecognizedPattern getPatternDetails(const std::string& patternId) const;
    
    /**
     * @brief Get all available patterns in the ruleset
     * 
     * @return std::vector<std::string> List of pattern IDs
     */
    std::vector<std::string> getAvailablePatterns() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace patterns
} // namespace dist_prompt