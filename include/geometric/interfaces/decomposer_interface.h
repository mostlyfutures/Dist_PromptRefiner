#pragma once

#include "geometric/interfaces/region_interface.h"
#include <string>
#include <memory>

namespace dist_prompt {
namespace geometric {
namespace interfaces {

/**
 * @brief Abstract interface for geometric decomposition
 * 
 * Defines the interface for decomposing software ideas into geometric regions.
 */
class DecomposerInterface {
public:
    /**
     * @brief Structure representing decomposition parameters
     */
    struct DecompositionParams {
        int maxRegions;
        int dimensions;
        double minSimilarity;
        bool useHierarchical;
        std::map<std::string, std::string> additionalParams;
    };
    
    /**
     * @brief Structure representing decomposition results
     */
    struct DecompositionResult {
        bool success;
        std::vector<std::string> regionIds;
        double qualityScore;
        std::string message;
    };

    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~DecomposerInterface() = default;
    
    /**
     * @brief Set the region interface implementation
     * 
     * @param regionInterface Shared pointer to region interface implementation
     */
    virtual void setRegionInterface(std::shared_ptr<RegionInterface> regionInterface) = 0;
    
    /**
     * @brief Initialize the decomposer with configuration
     * 
     * @param config JSON configuration string
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::string& config) = 0;
    
    /**
     * @brief Decompose a software idea into geometric regions
     * 
     * @param ideaData Structured data representing the software idea
     * @param params Decomposition parameters
     * @return DecompositionResult Results of the decomposition
     */
    virtual DecompositionResult decompose(const std::string& ideaData, 
                                        const DecompositionParams& params) = 0;
    
    /**
     * @brief Get adjacency information between regions
     * 
     * @return std::map<std::string, std::vector<std::string>> Map of region IDs to adjacent region IDs
     */
    virtual std::map<std::string, std::vector<std::string>> getAdjacencyInfo() const = 0;
    
    /**
     * @brief Get a report of the decomposition process
     * 
     * @param format Report format (e.g., "json", "text")
     * @return std::string Formatted report
     */
    virtual std::string getDecompositionReport(const std::string& format = "json") const = 0;
};

} // namespace interfaces
} // namespace geometric
} // namespace dist_prompt