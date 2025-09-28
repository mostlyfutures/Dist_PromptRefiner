#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace dist_prompt {

/**
 * @brief PatternMatcher class - Interface between PCAM and Pattern Module
 * 
 * This class identifies and applies program structure patterns to software ideas.
 */
class PatternMatcher {
public:
    /**
     * @brief Structure representing a program structure pattern
     */
    struct Pattern {
        std::string name;
        std::string category;
        std::string description;
        double confidence;
        std::map<std::string, std::string> parameters;
    };

    /**
     * @brief Default constructor
     */
    PatternMatcher() = default;
    
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~PatternMatcher() = default;
    
    /**
     * @brief Identify applicable patterns for a software idea
     * 
     * @param ideaData The structured data representing the software idea
     * @param minConfidence Minimum confidence threshold for pattern matching (0.0-1.0)
     * @return bool True if pattern identification was successful, false otherwise
     */
    virtual bool identifyPatterns(const std::string& ideaData, double minConfidence = 0.7) = 0;
    
    /**
     * @brief Apply identified patterns to transform the software idea
     * 
     * @param ideaData The structured data representing the software idea
     * @return std::string Transformed idea with applied patterns
     */
    virtual std::string applyPatterns(const std::string& ideaData) = 0;
    
    /**
     * @brief Get the identified patterns
     * 
     * @return std::vector<Pattern> The identified patterns with confidence scores
     */
    virtual std::vector<Pattern> getIdentifiedPatterns() const = 0;
    
    /**
     * @brief Verify pattern application correctness
     * 
     * @param originalIdea Original idea data
     * @param transformedIdea Transformed idea data
     * @return bool True if verification passes, false otherwise
     */
    virtual bool verifyPatternApplication(const std::string& originalIdea, 
                                         const std::string& transformedIdea) = 0;
};

} // namespace dist_prompt