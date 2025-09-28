#include "pcam/plan_decomposer.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <algorithm>
#include <random>
#include <sstream>

namespace dist_prompt {
namespace pcam {

PlanDecomposer::PlanDecomposer() = default;

bool PlanDecomposer::decompose(const std::string& ideaText) {
    // Clear previous state
    components_.clear();
    relationships_.clear();
    
    // Apply the PCAM-DX algorithm stages
    if (!extractFeatures(ideaText)) {
        return false;
    }
    
    if (!identifyDependencies()) {
        return false;
    }
    
    if (!optimizeModules()) {
        return false;
    }
    
    return true;
}

std::vector<PlanDecomposer::PlanComponent> PlanDecomposer::getPlanComponents() const {
    return components_;
}

std::map<std::string, std::vector<std::string>> PlanDecomposer::getRelationships() const {
    return relationships_;
}

std::string PlanDecomposer::getSerializedPlan() const {
    nlohmann::json plan;
    
    // Add components
    nlohmann::json componentsJson = nlohmann::json::array();
    for (const auto& component : components_) {
        nlohmann::json comp;
        comp["id"] = component.id;
        comp["type"] = component.type;
        comp["name"] = component.name;
        comp["description"] = component.description;
        comp["dependencies"] = component.dependencies;
        comp["metadata"] = component.metadata;
        componentsJson.push_back(comp);
    }
    plan["components"] = componentsJson;
    
    // Add relationships
    nlohmann::json relationshipsJson = nlohmann::json::object();
    for (const auto& [id, deps] : relationships_) {
        relationshipsJson[id] = deps;
    }
    plan["relationships"] = relationshipsJson;
    
    return plan.dump(2);
}

bool PlanDecomposer::extractFeatures(const std::string& ideaText) {
    // This is a simplified implementation of feature extraction
    // In a real implementation, this would use NLP or ML techniques
    
    // Split idea text into sentences
    std::regex sentenceDelim("[.!?]\\s+");
    std::sregex_token_iterator it(ideaText.begin(), ideaText.end(), sentenceDelim, -1);
    std::vector<std::string> sentences(it, std::sregex_token_iterator());
    
    // Look for feature indicators in each sentence
    std::regex featurePatterns[] = {
        std::regex("\\b(feature|functionality|capability)\\b.*?\\b(for|to|that)\\b", std::regex_constants::icase),
        std::regex("\\b(should|must|will|can)\\b.*?\\b(have|include|support|provide)\\b", std::regex_constants::icase),
        std::regex("\\b(implement|create|develop|build)\\b.*?\\b(a|an|the)\\b", std::regex_constants::icase)
    };
    
    // Generate a few components based on pattern matching
    for (const auto& sentence : sentences) {
        for (const auto& pattern : featurePatterns) {
            std::smatch match;
            if (std::regex_search(sentence, match, pattern)) {
                // Create a new component
                PlanComponent component;
                component.id = "COMP_" + std::to_string(components_.size() + 1);
                component.type = "feature";
                
                // Extract name (simplified)
                size_t nameEnd = sentence.find_first_of(".!?");
                component.name = sentence.substr(0, nameEnd);
                if (component.name.length() > 50) {
                    component.name = component.name.substr(0, 47) + "...";
                }
                
                component.description = sentence;
                
                // Add to components list
                components_.push_back(component);
                break;
            }
        }
    }
    
    // If we couldn't extract any components, create some default ones
    if (components_.empty()) {
        // Create default components
        PlanComponent core;
        core.id = "COMP_1";
        core.type = "module";
        core.name = "Core System";
        core.description = "The core functionality of the system";
        components_.push_back(core);
        
        PlanComponent ui;
        ui.id = "COMP_2";
        ui.type = "module";
        ui.name = "User Interface";
        ui.description = "The user interface components";
        components_.push_back(ui);
        
        PlanComponent data;
        data.id = "COMP_3";
        data.type = "module";
        data.name = "Data Management";
        data.description = "Data storage and retrieval functionality";
        components_.push_back(data);
    }
    
    return true;
}

bool PlanDecomposer::identifyDependencies() {
    // This is a simplified implementation of dependency identification
    // In a real implementation, this would use semantic analysis
    
    // For demonstration, create some random dependencies
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    
    for (size_t i = 0; i < components_.size(); ++i) {
        for (size_t j = 0; j < components_.size(); ++j) {
            // Skip self-dependencies
            if (i == j) {
                continue;
            }
            
            // 30% chance of dependency
            if (dis(gen) < 30) {
                // Add dependency
                components_[i].dependencies.push_back(components_[j].id);
                
                // Update relationships map
                relationships_[components_[i].id].push_back(components_[j].id);
            }
        }
    }
    
    return true;
}

bool PlanDecomposer::optimizeModules() {
    // This is a simplified implementation of module optimization
    // In a real implementation, this would use graph algorithms
    
    // Check for circular dependencies (simplified)
    for (const auto& [id, deps] : relationships_) {
        for (const auto& dep : deps) {
            // Check if there's a reverse dependency
            if (relationships_.count(dep) > 0 && 
                std::find(relationships_[dep].begin(), relationships_[dep].end(), id) != relationships_[dep].end()) {
                
                // Found circular dependency, resolve it by removing one direction
                auto& depList = relationships_[dep];
                depList.erase(std::remove(depList.begin(), depList.end(), id), depList.end());
                
                // Also update component dependencies
                for (auto& comp : components_) {
                    if (comp.id == dep) {
                        comp.dependencies.erase(
                            std::remove(comp.dependencies.begin(), comp.dependencies.end(), id), 
                            comp.dependencies.end()
                        );
                        break;
                    }
                }
            }
        }
    }
    
    // Add metadata to components for optimization info
    for (auto& component : components_) {
        component.metadata["optimized"] = "true";
        component.metadata["modularity_score"] = std::to_string(relationships_[component.id].size());
    }
    
    return true;
}

} // namespace pcam
} // namespace dist_prompt