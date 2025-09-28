#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace dist_prompt {
namespace integration {

/**
 * @brief Geometric region structure
 */
struct GeometricRegion {
    std::string regionId;
    std::string name;
    std::vector<double> coordinates;  // Multi-dimensional coordinates
    std::vector<double> boundaries;   // Region boundaries
    std::map<std::string, std::string> properties;
    int depth;
    std::vector<std::string> subRegions;
};

/**
 * @brief Decomposition parameters
 */
struct DecompositionParams {
    int maxDepth;
    int minRegionSize;
    std::string partitioningStrategy;  // "kd-tree", "grid", "adaptive"
    std::vector<double> problemSpace;
    std::map<std::string, double> weights;
};

/**
 * @brief Decomposition result
 */
struct DecompositionResult {
    bool success;
    std::vector<GeometricRegion> regions;
    std::string decompositionTree;
    std::map<std::string, std::string> statistics;
    std::string errorMessage;
};

/**
 * @brief RegionDecomposer interface for PCAM → Geometric Module integration
 * 
 * This interface defines spatial problem decomposition algorithms using
 * k-d tree with configurable depth as specified in the integration flow.
 */
class RegionDecomposer {
public:
    virtual ~RegionDecomposer() = default;
    
    /**
     * @brief Initialize the decomposer with configuration
     * 
     * @param config Configuration parameters
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::map<std::string, std::string>& config) = 0;
    
    /**
     * @brief Decompose problem space into geometric regions
     * 
     * @param problemDescription Description of the problem to decompose
     * @param params Decomposition parameters
     * @return DecompositionResult Result containing regions and metadata
     */
    virtual DecompositionResult decompose(const std::string& problemDescription,
                                        const DecompositionParams& params) = 0;
    
    /**
     * @brief Assign tasks to regions using graph coloring
     * 
     * @param regions List of regions to assign
     * @param tasks List of tasks to distribute
     * @return std::map<std::string, std::vector<std::string>> Region to tasks mapping
     */
    virtual std::map<std::string, std::vector<std::string>> assignTasks(
        const std::vector<GeometricRegion>& regions,
        const std::vector<std::string>& tasks) = 0;
    
    /**
     * @brief Validate region assignments using 4-color theorem
     * 
     * @param assignments Region assignments to validate
     * @return bool True if assignments are valid
     */
    virtual bool validateAssignments(
        const std::map<std::string, std::vector<std::string>>& assignments) = 0;
    
    /**
     * @brief Optimize region boundaries
     * 
     * @param regions Regions to optimize
     * @return std::vector<GeometricRegion> Optimized regions
     */
    virtual std::vector<GeometricRegion> optimizeRegions(
        const std::vector<GeometricRegion>& regions) = 0;
    
    /**
     * @brief Get decomposition statistics
     * 
     * @return std::map<std::string, double> Performance and quality metrics
     */
    virtual std::map<std::string, double> getStatistics() const = 0;
    
    /**
     * @brief Merge regions based on criteria
     * 
     * @param regions Regions to potentially merge
     * @param criteria Merging criteria
     * @return std::vector<GeometricRegion> Merged regions
     */
    virtual std::vector<GeometricRegion> mergeRegions(
        const std::vector<GeometricRegion>& regions,
        const std::map<std::string, double>& criteria) = 0;
    
    /**
     * @brief Export decomposition to visualization format
     * 
     * @param regions Regions to export
     * @param format Export format ("json", "svg", "graphviz")
     * @return std::string Exported representation
     */
    virtual std::string exportVisualization(
        const std::vector<GeometricRegion>& regions,
        const std::string& format) const = 0;
};

/**
 * @brief Factory function to create RegionDecomposer instance
 * 
 * @return std::unique_ptr<RegionDecomposer> RegionDecomposer instance
 */
std::unique_ptr<RegionDecomposer> createRegionDecomposer();

} // namespace integration
} // namespace dist_prompt