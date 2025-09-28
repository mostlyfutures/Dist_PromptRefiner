#include "patterns/transformers/pattern_transformer.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <unordered_map>

namespace fs = std::filesystem;

namespace dist_prompt {
namespace patterns {
namespace transformers {

// Private implementation class (PIMPL idiom)
class PatternTransformer::Impl {
public:
    // Structure for pattern templates
    struct PatternTemplate {
        std::string patternId;
        std::string templateContent;
        std::vector<std::string> requiredParams;
    };

    Impl() = default;
    ~Impl() = default;
    
    bool loadTemplates(const std::string& templateDir) {
        if (!fs::exists(templateDir) || !fs::is_directory(templateDir)) {
            return false;
        }
        
        templates_.clear();
        
        try {
            for (const auto& entry : fs::directory_iterator(templateDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".tmpl") {
                    std::string filename = entry.path().filename().string();
                    std::string patternId = filename.substr(0, filename.find_first_of('.'));
                    
                    // Load template content
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        
                        PatternTemplate templ;
                        templ.patternId = patternId;
                        templ.templateContent = buffer.str();
                        
                        // Extract required parameters
                        extractRequiredParams(templ);
                        
                        templates_[patternId] = templ;
                    }
                }
            }
            
            return !templates_.empty();
        } catch (const std::exception&) {
            return false;
        }
    }
    
    TransformationResult transform(
        const std::string& ideaData,
        const PatternIdentifier::RecognizedPattern& pattern) {
        
        TransformationResult result;
        result.success = false;
        result.appliedPatternId = pattern.id;
        
        // Check if template exists
        auto it = templates_.find(pattern.id);
        if (it == templates_.end()) {
            result.transformationMetadata["error"] = "No template found for pattern: " + pattern.id;
            return result;
        }
        
        const PatternTemplate& templ = it->second;
        
        // Check if all required parameters are present
        for (const auto& param : templ.requiredParams) {
            if (pattern.parameters.find(param) == pattern.parameters.end()) {
                result.transformationMetadata["error"] = 
                    "Missing required parameter: " + param + " for pattern: " + pattern.id;
                return result;
            }
        }
        
        // Parse idea data if it's JSON
        nlohmann::json ideaJson;
        bool isJson = false;
        try {
            ideaJson = nlohmann::json::parse(ideaData);
            isJson = true;
        } catch (...) {
            // Not JSON, treat as plain text
        }
        
        // Apply the template
        std::string transformed = applyTemplate(templ.templateContent, pattern.parameters, 
                                               isJson ? ideaJson : nlohmann::json());
        
        result.success = true;
        result.transformedData = transformed;
        result.transformationMetadata["template"] = templ.patternId;
        result.transformationMetadata["timestamp"] = getCurrentTimestamp();
        
        return result;
    }
    
    TransformationResult transformMultiple(
        const std::string& ideaData,
        const std::vector<PatternIdentifier::RecognizedPattern>& patterns) {
        
        std::string currentData = ideaData;
        TransformationResult finalResult;
        finalResult.success = true;
        
        for (const auto& pattern : patterns) {
            TransformationResult result = transform(currentData, pattern);
            
            if (!result.success) {
                // Propagate error from first failed transformation
                return result;
            }
            
            // Use transformed data for next pattern
            currentData = result.transformedData;
            
            // Track which patterns were applied
            finalResult.transformationMetadata["applied_patterns"] += 
                (finalResult.transformationMetadata["applied_patterns"].empty() ? "" : ",") + 
                pattern.id;
        }
        
        finalResult.transformedData = currentData;
        finalResult.transformationMetadata["pattern_count"] = std::to_string(patterns.size());
        finalResult.transformationMetadata["timestamp"] = getCurrentTimestamp();
        
        if (!patterns.empty()) {
            finalResult.appliedPatternId = patterns.back().id; // Last applied pattern
        }
        
        return finalResult;
    }
    
    bool hasTemplate(const std::string& patternId) const {
        return templates_.find(patternId) != templates_.end();
    }
    
    std::vector<std::string> getTemplateIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, _] : templates_) {
            ids.push_back(id);
        }
        return ids;
    }
    
private:
    std::unordered_map<std::string, PatternTemplate> templates_;
    
    void extractRequiredParams(PatternTemplate& templ) {
        // Extract parameters of the form {{param}}
        std::regex paramRegex("\\{\\{([a-zA-Z0-9_]+)\\}\\}");
        std::string::const_iterator searchStart(templ.templateContent.cbegin());
        std::smatch match;
        
        std::set<std::string> uniqueParams;
        
        while (std::regex_search(searchStart, templ.templateContent.cend(), match, paramRegex)) {
            uniqueParams.insert(match[1]);
            searchStart = match.suffix().first;
        }
        
        templ.requiredParams.assign(uniqueParams.begin(), uniqueParams.end());
    }
    
    std::string applyTemplate(
        const std::string& templateContent,
        const std::map<std::string, std::string>& params,
        const nlohmann::json& contextData) {
        
        std::string result = templateContent;
        
        // Replace parameters
        for (const auto& [key, value] : params) {
            std::string placeholder = "{{" + key + "}}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        
        // Replace context data references if any
        if (!contextData.is_null()) {
            std::regex contextRegex("\\{\\{context\\.([a-zA-Z0-9_.]+)\\}\\}");
            std::string::const_iterator searchStart(result.cbegin());
            std::smatch match;
            
            // Find all context placeholders and collect them
            std::vector<std::pair<std::string, std::string>> replacements;
            
            while (std::regex_search(searchStart, result.cend(), match, contextRegex)) {
                std::string path = match[1];
                std::string placeholder = match[0];
                
                // Navigate the JSON path
                nlohmann::json value = contextData;
                std::istringstream pathStream(path);
                std::string segment;
                
                while (std::getline(pathStream, segment, '.')) {
                    if (value.contains(segment)) {
                        value = value[segment];
                    } else {
                        value = nullptr;
                        break;
                    }
                }
                
                // If we found a value, add it to replacements
                if (value.is_string()) {
                    replacements.push_back({placeholder, value.get<std::string>()});
                } else if (!value.is_null()) {
                    replacements.push_back({placeholder, value.dump()});
                }
                
                searchStart = match.suffix().first;
            }
            
            // Apply all replacements
            for (const auto& [placeholder, value] : replacements) {
                size_t pos = 0;
                while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                    result.replace(pos, placeholder.length(), value);
                    pos += value.length();
                }
            }
        }
        
        return result;
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// PatternTransformer implementation

PatternTransformer::PatternTransformer() : pImpl_(std::make_unique<Impl>()) {}

PatternTransformer::~PatternTransformer() = default;

bool PatternTransformer::initialize(const std::string& templateDir) {
    return pImpl_->loadTemplates(templateDir);
}

PatternTransformer::TransformationResult PatternTransformer::applyPattern(
    const std::string& ideaData,
    const PatternIdentifier::RecognizedPattern& pattern) {
    
    return pImpl_->transform(ideaData, pattern);
}

PatternTransformer::TransformationResult PatternTransformer::applyPatterns(
    const std::string& ideaData,
    const std::vector<PatternIdentifier::RecognizedPattern>& patterns) {
    
    return pImpl_->transformMultiple(ideaData, patterns);
}

bool PatternTransformer::hasTemplateForPattern(const std::string& patternId) const {
    return pImpl_->hasTemplate(patternId);
}

std::vector<std::string> PatternTransformer::getAvailableTemplates() const {
    return pImpl_->getTemplateIds();
}

} // namespace transformers
} // namespace patterns
} // namespace dist_prompt