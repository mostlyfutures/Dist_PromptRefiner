#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace dist_prompt {
namespace integration {

/**
 * @brief Software pattern structure
 */
struct SoftwarePattern {
    std::string patternId;
    std::string name;
    std::string category;  // "creational", "structural", "behavioral"
    std::string description;
    std::vector<std::string> components;
    std::map<std::string, std::string> rules;
    double confidence;
    std::string templatePath;
};

/**
 * @brief Pattern matching result
 */
struct PatternMatchResult {
    bool found;
    SoftwarePattern pattern;
    double matchScore;
    std::vector<std::string> matchedElements;
    std::map<std::string, std::string> bindings;
    std::string explanation;
};

/**
 * @brief Pattern transformation parameters
 */
struct TransformationParams {
    std::string sourceCode;
    std::string targetLanguage;
    std::map<std::string, std::string> variables;
    bool preserveComments;
    std::string styleGuide;
};

/**
 * @brief Pattern application result
 */
struct ApplicationResult {
    bool success;
    std::string transformedCode;
    std::vector<std::string> appliedPatterns;
    std::map<std::string, std::string> modifications;
    std::string errorMessage;
};

/**
 * @brief PatternMatcher interface for PCAM → Patterns Module integration
 * 
 * This interface defines rule-based pattern matching and template-based
 * transformations as specified in the integration flow.
 */
class PatternMatcher {
public:
    virtual ~PatternMatcher() = default;
    
    /**
     * @brief Initialize the pattern matcher with rule database
     * 
     * @param rulesDatabasePath Path to pattern rules database
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::string& rulesDatabasePath) = 0;
    
    /**
     * @brief Load pattern definitions from file
     * 
     * @param patternsPath Path to patterns definition file
     * @return bool True if patterns were loaded successfully
     */
    virtual bool loadPatterns(const std::string& patternsPath) = 0;
    
    /**
     * @brief Identify patterns in source code
     * 
     * @param sourceCode Code to analyze for patterns
     * @param language Programming language
     * @return std::vector<PatternMatchResult> Found patterns with scores
     */
    virtual std::vector<PatternMatchResult> identifyPatterns(
        const std::string& sourceCode,
        const std::string& language) = 0;
    
    /**
     * @brief Apply pattern transformation to code
     * 
     * @param pattern Pattern to apply
     * @param params Transformation parameters
     * @return ApplicationResult Result of pattern application
     */
    virtual ApplicationResult applyPattern(
        const SoftwarePattern& pattern,
        const TransformationParams& params) = 0;
    
    /**
     * @brief Suggest patterns for given requirements
     * 
     * @param requirements System requirements or constraints
     * @param context Development context
     * @return std::vector<SoftwarePattern> Recommended patterns
     */
    virtual std::vector<SoftwarePattern> suggestPatterns(
        const std::vector<std::string>& requirements,
        const std::map<std::string, std::string>& context) = 0;
    
    /**
     * @brief Verify pattern implementation correctness
     * 
     * @param sourceCode Code to verify
     * @param expectedPattern Pattern that should be implemented
     * @return bool True if pattern is correctly implemented
     */
    virtual bool verifyPatternImplementation(
        const std::string& sourceCode,
        const SoftwarePattern& expectedPattern) = 0;
    
    /**
     * @brief Get available patterns by category
     * 
     * @param category Pattern category filter
     * @return std::vector<SoftwarePattern> Patterns in category
     */
    virtual std::vector<SoftwarePattern> getPatternsByCategory(
        const std::string& category) = 0;
    
    /**
     * @brief Create custom pattern from code analysis
     * 
     * @param codeExamples Example implementations
     * @param patternName Name for the new pattern
     * @return SoftwarePattern Generated pattern definition
     */
    virtual SoftwarePattern createCustomPattern(
        const std::vector<std::string>& codeExamples,
        const std::string& patternName) = 0;
    
    /**
     * @brief Export pattern as template
     * 
     * @param pattern Pattern to export
     * @param templateFormat Format ("mustache", "jinja2", "handlebars")
     * @return std::string Template representation
     */
    virtual std::string exportAsTemplate(
        const SoftwarePattern& pattern,
        const std::string& templateFormat) = 0;
};

/**
 * @brief Factory function to create PatternMatcher instance
 * 
 * @return std::unique_ptr<PatternMatcher> PatternMatcher instance
 */
std::unique_ptr<PatternMatcher> createPatternMatcher();

} // namespace integration
} // namespace dist_prompt