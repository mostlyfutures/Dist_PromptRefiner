#pragma once

#include "patterns/pattern_identifier.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace dist_prompt {
namespace patterns {
namespace transformers {

/**
 * @brief Applies program structure patterns to transform software ideas
 * 
 * Template-based transformer for applying program structure patterns.
 */
class PatternTransformer {
public:
    /**
     * @brief Structure representing transformation results
     */
    struct TransformationResult {
        bool success;
        std::string transformedData;
        std::string appliedPatternId;
        std::map<std::string, std::string> transformationMetadata;
    };

    /**
     * @brief Constructor
     */
    PatternTransformer();
    
    /**
     * @brief Destructor
     */
    ~PatternTransformer();
    
    /**
     * @brief Initialize the transformer with template files
     * 
     * @param templateDir Directory containing pattern templates
     * @return bool True if initialization was successful
     */
    bool initialize(const std::string& templateDir);
    
    /**
     * @brief Apply a pattern to transform a software idea
     * 
     * @param ideaData Structured data representing the software idea
     * @param pattern The pattern to apply
     * @return TransformationResult Results of the transformation
     */
    TransformationResult applyPattern(
        const std::string& ideaData,
        const PatternIdentifier::RecognizedPattern& pattern);
    
    /**
     * @brief Apply multiple patterns sequentially
     * 
     * @param ideaData Structured data representing the software idea
     * @param patterns Patterns to apply, in order
     * @return TransformationResult Results of the combined transformation
     */
    TransformationResult applyPatterns(
        const std::string& ideaData,
        const std::vector<PatternIdentifier::RecognizedPattern>& patterns);
    
    /**
     * @brief Check if a template exists for a pattern
     * 
     * @param patternId ID of the pattern
     * @return bool True if a template exists
     */
    bool hasTemplateForPattern(const std::string& patternId) const;
    
    /**
     * @brief Get a list of all available templates
     * 
     * @return std::vector<std::string> List of pattern IDs with templates
     */
    std::vector<std::string> getAvailableTemplates() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace transformers
} // namespace patterns
} // namespace dist_prompt