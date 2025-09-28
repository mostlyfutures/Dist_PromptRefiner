#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace dist_prompt {
namespace pcam {

/**
 * @brief PCAM-DX Plan Decomposer
 * 
 * Implements the Plan-Code-Architect-Model Decomposition algorithm (PCAM-DX).
 * This class is responsible for decomposing a software idea into a structured plan.
 */
class PlanDecomposer {
public:
    /**
     * @brief Structure representing a plan component
     */
    struct PlanComponent {
        std::string id;
        std::string type;        // "feature", "module", "service", etc.
        std::string name;
        std::string description;
        std::vector<std::string> dependencies;
        std::map<std::string, std::string> metadata;
    };

    PlanDecomposer();
    ~PlanDecomposer() = default;
    
    /**
     * @brief Decompose a software idea into a plan
     * 
     * @param ideaText The raw text describing the software idea
     * @return bool True if decomposition was successful
     */
    bool decompose(const std::string& ideaText);
    
    /**
     * @brief Get the decomposed plan components
     * 
     * @return std::vector<PlanComponent> List of plan components
     */
    std::vector<PlanComponent> getPlanComponents() const;
    
    /**
     * @brief Get relationships between plan components
     * 
     * @return std::map<std::string, std::vector<std::string>> Map of component IDs to their dependencies
     */
    std::map<std::string, std::vector<std::string>> getRelationships() const;
    
    /**
     * @brief Get the serialized plan as JSON
     * 
     * @return std::string JSON representation of the plan
     */
    std::string getSerializedPlan() const;

private:
    std::vector<PlanComponent> components_;
    std::map<std::string, std::vector<std::string>> relationships_;
    
    // PCAM-DX Algorithm stages
    bool extractFeatures(const std::string& ideaText);
    bool identifyDependencies();
    bool optimizeModules();
};

} // namespace pcam
} // namespace dist_prompt