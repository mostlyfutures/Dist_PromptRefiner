#pragma once

#include <string>
#include <vector>
#include <memory>

namespace dist_prompt {

/**
 * @brief RegionDecomposer class - Interface between PCAM and Geometric Module
 * 
 * This class handles the geometric decomposition of software ideas into
 * conceptual regions that can be processed in parallel.
 */
class RegionDecomposer {
public:
    /**
     * @brief Structure representing a decomposed region
     */
    struct Region {
        std::string name;
        std::string description;
        std::vector<std::string> dependencies;
        double complexity;
        // Additional region metadata
    };
    
    /**
     * @brief Default constructor
     */
    RegionDecomposer() = default;
    
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~RegionDecomposer() = default;
    
    /**
     * @brief Decompose a software idea into geometric regions
     * 
     * @param ideaData The structured data representing the software idea
     * @param maxRegions Maximum number of regions to create (default: 6)
     * @return bool True if decomposition was successful, false otherwise
     */
    virtual bool decomposeIntoRegions(const std::string& ideaData, 
                                     int maxRegions = 6) = 0;
    
    /**
     * @brief Get the decomposed regions
     * 
     * @return std::vector<Region> The decomposed regions
     */
    virtual std::vector<Region> getRegions() const = 0;
    
    /**
     * @brief Configure spatial partitioning parameters
     * 
     * @param kdTreeDepth Depth of the k-d tree for spatial partitioning
     * @param balanceFactor Balance factor for region distribution (0.0-1.0)
     */
    virtual void configureSpatialPartitioning(int kdTreeDepth, double balanceFactor) = 0;
};

} // namespace dist_prompt