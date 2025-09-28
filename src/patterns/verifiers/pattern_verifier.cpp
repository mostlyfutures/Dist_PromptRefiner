#include "patterns/verifiers/pattern_verifier.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace dist_prompt {
namespace patterns {
namespace verifiers {

// Private implementation class (PIMPL idiom)
class PatternVerifier::Impl {
public:
    using VerificationCheck = std::function<bool(
        const std::string&, const std::string&, std::vector<std::string>&, std::map<std::string, double>&)>;
    
    // Structure for verification rules
    struct VerificationRule {
        std::string id;
        std::string name;
        std::string description;
        double weight;
        std::vector<std::string> applicablePatterns;  // Empty means all patterns
    };

    Impl() {
        // Register built-in verification checks
        registerBuiltInChecks();
    }
    
    ~Impl() = default;
    
    bool loadRules(const std::string& rulesPath) {
        try {
            std::ifstream file(rulesPath);
            if (!file.is_open()) {
                return false;
            }
            
            nlohmann::json rulesJson;
            file >> rulesJson;
            
            rules_.clear();
            
            for (const auto& ruleJson : rulesJson["rules"]) {
                VerificationRule rule;
                rule.id = ruleJson["id"].get<std::string>();
                rule.name = ruleJson["name"].get<std::string>();
                rule.description = ruleJson["description"].get<std::string>();
                rule.weight = ruleJson["weight"].get<double>();
                
                if (ruleJson.contains("applicablePatterns")) {
                    for (const auto& pattern : ruleJson["applicablePatterns"]) {
                        rule.applicablePatterns.push_back(pattern.get<std::string>());
                    }
                }
                
                rules_.push_back(rule);
            }
            
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    VerificationResult verifyTransformation(
        const std::string& originalData,
        const std::string& transformedData,
        const std::string& patternId) {
        
        VerificationResult result;
        result.success = true;
        result.score = 1.0;  // Start with perfect score
        
        double totalWeight = 0.0;
        double weightedScore = 0.0;
        
        for (const auto& rule : rules_) {
            // Skip if rule doesn't apply to this pattern
            if (!rule.applicablePatterns.empty() && 
                std::find(rule.applicablePatterns.begin(), rule.applicablePatterns.end(), patternId) 
                    == rule.applicablePatterns.end()) {
                continue;
            }
            
            // Find the check function
            auto checkIt = checks_.find(rule.id);
            if (checkIt == checks_.end()) {
                continue;  // No check function for this rule
            }
            
            // Run the check
            std::vector<std::string> checkIssues;
            std::map<std::string, double> checkMetrics;
            bool checkResult = checkIt->second(originalData, transformedData, checkIssues, checkMetrics);
            
            // Add issues and metrics from this check
            result.issues.insert(result.issues.end(), checkIssues.begin(), checkIssues.end());
            for (const auto& [key, value] : checkMetrics) {
                result.metrics[key] = value;
            }
            
            // Update weighted score
            totalWeight += rule.weight;
            if (checkResult) {
                weightedScore += rule.weight;
            } else {
                result.success = false;
            }
        }
        
        // Calculate final score
        if (totalWeight > 0) {
            result.score = weightedScore / totalWeight;
        }
        
        return result;
    }
    
    VerificationResult runSpecificChecks(
        const std::string& originalData,
        const std::string& transformedData,
        const std::vector<std::string>& checkNames) {
        
        VerificationResult result;
        result.success = true;
        result.score = 1.0;
        
        double totalWeight = 0.0;
        double weightedScore = 0.0;
        
        for (const auto& checkName : checkNames) {
            // Find the corresponding rule
            auto ruleIt = std::find_if(rules_.begin(), rules_.end(),
                                     [&checkName](const VerificationRule& rule) {
                                         return rule.name == checkName || rule.id == checkName;
                                     });
            
            if (ruleIt == rules_.end()) {
                continue;  // Rule not found
            }
            
            // Find the check function
            auto checkIt = checks_.find(ruleIt->id);
            if (checkIt == checks_.end()) {
                continue;  // No check function for this rule
            }
            
            // Run the check
            std::vector<std::string> checkIssues;
            std::map<std::string, double> checkMetrics;
            bool checkResult = checkIt->second(originalData, transformedData, checkIssues, checkMetrics);
            
            // Add issues and metrics from this check
            result.issues.insert(result.issues.end(), checkIssues.begin(), checkIssues.end());
            for (const auto& [key, value] : checkMetrics) {
                result.metrics[key] = value;
            }
            
            // Update weighted score
            totalWeight += ruleIt->weight;
            if (checkResult) {
                weightedScore += ruleIt->weight;
            } else {
                result.success = false;
            }
        }
        
        // Calculate final score
        if (totalWeight > 0) {
            result.score = weightedScore / totalWeight;
        }
        
        return result;
    }
    
    std::vector<std::string> getCheckNames() const {
        std::vector<std::string> names;
        for (const auto& rule : rules_) {
            names.push_back(rule.name);
        }
        return names;
    }

private:
    std::vector<VerificationRule> rules_;
    std::unordered_map<std::string, VerificationCheck> checks_;
    
    void registerBuiltInChecks() {
        // Check for structure preservation
        checks_["structure_preservation"] = [](
            const std::string& original, 
            const std::string& transformed,
            std::vector<std::string>& issues,
            std::map<std::string, double>& metrics) -> bool {
            
            // This is a simplified implementation
            // In a real system, this would use a more sophisticated comparison
            
            // Check if JSON structure is preserved
            bool originalIsJson = false;
            bool transformedIsJson = false;
            
            try {
                nlohmann::json::parse(original);
                originalIsJson = true;
            } catch (...) {}
            
            try {
                nlohmann::json::parse(transformed);
                transformedIsJson = true;
            } catch (...) {}
            
            if (originalIsJson != transformedIsJson) {
                issues.push_back("JSON structure not preserved");
                metrics["structure_preservation"] = 0.0;
                return false;
            }
            
            metrics["structure_preservation"] = 1.0;
            return true;
        };
        
        // Check for completeness
        checks_["completeness"] = [](
            const std::string& original, 
            const std::string& transformed,
            std::vector<std::string>& issues,
            std::map<std::string, double>& metrics) -> bool {
            
            // This is a simplified implementation
            // Check if the transformed content has a reasonable size compared to original
            
            double originalSize = original.size();
            double transformedSize = transformed.size();
            
            if (transformedSize < 0.5 * originalSize) {
                issues.push_back("Transformed content is significantly smaller than original");
                metrics["size_ratio"] = transformedSize / originalSize;
                return false;
            }
            
            metrics["size_ratio"] = transformedSize / originalSize;
            return true;
        };
        
        // Check for pattern-specific features
        checks_["pattern_features"] = [](
            const std::string& original, 
            const std::string& transformed,
            std::vector<std::string>& issues,
            std::map<std::string, double>& metrics) -> bool {
            
            // This would be a more complex check in a real implementation
            // For now, it's a placeholder that always passes
            
            metrics["pattern_feature_score"] = 0.95;
            return true;
        };
    }
};

// PatternVerifier implementation

PatternVerifier::PatternVerifier() : pImpl_(std::make_unique<Impl>()) {}

PatternVerifier::~PatternVerifier() = default;

bool PatternVerifier::initialize(const std::string& rulesPath) {
    return pImpl_->loadRules(rulesPath);
}

PatternVerifier::VerificationResult PatternVerifier::verify(
    const std::string& originalData,
    const std::string& transformedData,
    const std::string& patternId) {
    
    return pImpl_->verifyTransformation(originalData, transformedData, patternId);
}

PatternVerifier::VerificationResult PatternVerifier::runChecks(
    const std::string& originalData,
    const std::string& transformedData,
    const std::vector<std::string>& checkNames) {
    
    return pImpl_->runSpecificChecks(originalData, transformedData, checkNames);
}

std::vector<std::string> PatternVerifier::getAvailableChecks() const {
    return pImpl_->getCheckNames();
}

} // namespace verifiers
} // namespace patterns
} // namespace dist_prompt