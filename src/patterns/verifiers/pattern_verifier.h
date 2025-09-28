#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace dist_prompt {
namespace patterns {
namespace verifiers {

/**
 * @brief Verifies pattern application correctness
 * 
 * Provides verification for pattern-applied transformations.
 */
class PatternVerifier {
public:
    /**
     * @brief Verification result structure
     */
    struct VerificationResult {
        bool success;
        double score;
        std::vector<std::string> issues;
        std::map<std::string, double> metrics;
    };

    /**
     * @brief Constructor
     */
    PatternVerifier();
    
    /**
     * @brief Destructor
     */
    ~PatternVerifier();
    
    /**
     * @brief Initialize the verifier with verification rules
     * 
     * @param rulesPath Path to the verification rules file
     * @return bool True if initialization was successful
     */
    bool initialize(const std::string& rulesPath);
    
    /**
     * @brief Verify pattern application
     * 
     * @param originalData Original data before pattern application
     * @param transformedData Transformed data after pattern application
     * @param patternId ID of the applied pattern
     * @return VerificationResult Results of the verification
     */
    VerificationResult verify(
        const std::string& originalData,
        const std::string& transformedData,
        const std::string& patternId);
    
    /**
     * @brief Run specific verification checks
     * 
     * @param originalData Original data before pattern application
     * @param transformedData Transformed data after pattern application
     * @param checkNames Names of specific checks to run
     * @return VerificationResult Results of the verification
     */
    VerificationResult runChecks(
        const std::string& originalData,
        const std::string& transformedData,
        const std::vector<std::string>& checkNames);
    
    /**
     * @brief Get available verification check names
     * 
     * @return std::vector<std::string> List of available check names
     */
    std::vector<std::string> getAvailableChecks() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace verifiers
} // namespace patterns
} // namespace dist_prompt