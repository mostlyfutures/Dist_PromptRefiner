#include "patterns/pattern_identifier.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>
#include <algorithm>
#include <stdexcept>

namespace dist_prompt {
namespace patterns {

// Private implementation class (PIMPL idiom)
class PatternIdentifier::Impl {
public:
    // Structure for pattern rules
    struct PatternRule {
        std::string id;
        std::string name;
        std::string category;
        std::string description;
        std::vector<std::regex> patterns;
        std::vector<std::string> keywords;
        std::map<std::string, std::string> defaultParameters;
    };
    
    Impl() = default;
    ~Impl() = default;
    
    bool loadRuleset(const std::string& rulesetPath) {
        try {
            std::ifstream file(rulesetPath);
            if (!file.is_open()) {
                return false;
            }
            
            nlohmann::json rulesJson;
            file >> rulesJson;
            
            rules_.clear();
            
            for (const auto& ruleJson : rulesJson["patterns"]) {
                PatternRule rule;
                rule.id = ruleJson["id"].get<std::string>();
                rule.name = ruleJson["name"].get<std::string>();
                rule.category = ruleJson["category"].get<std::string>();
                rule.description = ruleJson["description"].get<std::string>();
                
                // Load pattern regexes
                for (const auto& pattern : ruleJson["patterns"]) {
                    rule.patterns.push_back(std::regex(
                        pattern.get<std::string>(), 
                        std::regex_constants::icase
                    ));
                }
                
                // Load keywords
                for (const auto& keyword : ruleJson["keywords"]) {
                    rule.keywords.push_back(keyword.get<std::string>());
                }
                
                // Load default parameters
                if (ruleJson.contains("defaultParameters")) {
                    for (auto it = ruleJson["defaultParameters"].begin(); 
                         it != ruleJson["defaultParameters"].end(); ++it) {
                        rule.defaultParameters[it.key()] = it.value().get<std::string>();
                    }
                }
                
                rules_.push_back(rule);
            }
            
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    std::vector<PatternIdentifier::RecognizedPattern> matchPatterns(
        const std::string& ideaData, double minConfidence) {
        
        std::vector<PatternIdentifier::RecognizedPattern> results;
        
        // Parse ideaData as JSON if it's in JSON format
        nlohmann::json ideaJson;
        bool isJson = false;
        try {
            ideaJson = nlohmann::json::parse(ideaData);
            isJson = true;
        } catch (...) {
            // Not JSON, treat as plain text
        }
        
        // Extract text for pattern matching
        std::string textToMatch;
        if (isJson) {
            if (ideaJson.contains("description")) {
                textToMatch = ideaJson["description"].get<std::string>();
            } else {
                textToMatch = ideaJson.dump();
            }
        } else {
            textToMatch = ideaData;
        }
        
        // Match each rule against the text
        for (const auto& rule : rules_) {
            double confidence = calculateConfidence(rule, textToMatch);
            
            if (confidence >= minConfidence) {
                PatternIdentifier::RecognizedPattern pattern;
                pattern.id = rule.id;
                pattern.name = rule.name;
                pattern.category = rule.category;
                pattern.description = rule.description;
                pattern.confidence = confidence;
                
                // Add default parameters
                pattern.parameters = rule.defaultParameters;
                
                // If JSON, extract additional parameters
                if (isJson && ideaJson.contains("parameters")) {
                    for (auto it = ideaJson["parameters"].begin(); 
                         it != ideaJson["parameters"].end(); ++it) {
                        pattern.parameters[it.key()] = it.value().get<std::string>();
                    }
                }
                
                results.push_back(pattern);
            }
        }
        
        // Sort by confidence (descending)
        std::sort(results.begin(), results.end(), 
                 [](const PatternIdentifier::RecognizedPattern& a, 
                    const PatternIdentifier::RecognizedPattern& b) {
                     return a.confidence > b.confidence;
                 });
        
        return results;
    }
    
    PatternIdentifier::RecognizedPattern getPattern(const std::string& patternId) const {
        for (const auto& rule : rules_) {
            if (rule.id == patternId) {
                PatternIdentifier::RecognizedPattern pattern;
                pattern.id = rule.id;
                pattern.name = rule.name;
                pattern.category = rule.category;
                pattern.description = rule.description;
                pattern.confidence = 1.0;  // Default confidence for direct lookup
                pattern.parameters = rule.defaultParameters;
                return pattern;
            }
        }
        
        return PatternIdentifier::RecognizedPattern();
    }
    
    std::vector<std::string> getAllPatternIds() const {
        std::vector<std::string> ids;
        for (const auto& rule : rules_) {
            ids.push_back(rule.id);
        }
        return ids;
    }

private:
    std::vector<PatternRule> rules_;
    
    double calculateConfidence(const PatternRule& rule, const std::string& text) {
        double confidence = 0.0;
        
        // Check regex patterns
        for (const auto& pattern : rule.patterns) {
            if (std::regex_search(text, pattern)) {
                confidence += 0.4;  // 40% boost for each matching pattern
            }
        }
        
        // Check keywords
        for (const auto& keyword : rule.keywords) {
            // Case insensitive search
            std::string lowerText = text;
            std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
            
            std::string lowerKeyword = keyword;
            std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
            
            if (lowerText.find(lowerKeyword) != std::string::npos) {
                confidence += 0.1;  // 10% boost for each keyword
            }
        }
        
        // Cap confidence at 1.0
        return std::min(confidence, 1.0);
    }
};

// PatternIdentifier implementation

PatternIdentifier::PatternIdentifier() : pImpl_(std::make_unique<Impl>()) {}

PatternIdentifier::~PatternIdentifier() = default;

bool PatternIdentifier::initialize(const std::string& rulesetPath) {
    return pImpl_->loadRuleset(rulesetPath);
}

std::vector<PatternIdentifier::RecognizedPattern> PatternIdentifier::identifyPatterns(
    const std::string& ideaData, double minConfidence) {
    
    return pImpl_->matchPatterns(ideaData, minConfidence);
}

PatternIdentifier::RecognizedPattern PatternIdentifier::getPatternDetails(
    const std::string& patternId) const {
    
    return pImpl_->getPattern(patternId);
}

std::vector<std::string> PatternIdentifier::getAvailablePatterns() const {
    return pImpl_->getAllPatternIds();
}

} // namespace patterns
} // namespace dist_prompt