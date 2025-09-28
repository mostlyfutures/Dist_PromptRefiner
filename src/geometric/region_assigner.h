#pragma once

#include "geometric/spatial_partitioner.h"
#include <vector>
#include <map>
#include <set>
#include <string>

namespace dist_prompt {
namespace geometric {

/**
 * @brief Region assignment using graph coloring algorithm with 4-color theorem
 * 
 * Assigns colors to regions in a way that no adjacent regions have the same color,
 * using the 4-color theorem as a basis.
 */
class RegionAssigner {
public:
    /**
     * @brief Color enumeration for region assignment
     */
    enum class Color {
        RED,
        GREEN,
        BLUE,
        YELLOW
    };
    
    /**
     * @brief Structure representing a colored region
     */
    struct ColoredRegion {
        std::string id;
        std::string name;
        Color color;
        std::vector<std::string> adjacentRegions;
    };
    
    /**
     * @brief Constructor
     */
    RegionAssigner();
    
    /**
     * @brief Destructor
     */
    ~RegionAssigner() = default;
    
    /**
     * @brief Set the regions to assign colors to
     * 
     * @param regions Regions from spatial partitioner
     */
    void setRegions(const std::vector<SpatialPartitioner::Region>& regions);
    
    /**
     * @brief Determine adjacency between regions
     * 
     * @return bool True if adjacency was determined successfully
     */
    bool determineAdjacency();
    
    /**
     * @brief Assign colors to regions using graph coloring algorithm
     * 
     * @return bool True if coloring was successful
     */
    bool assignColors();
    
    /**
     * @brief Get the colored regions
     * 
     * @return std::vector<ColoredRegion> List of colored regions
     */
    std::vector<ColoredRegion> getColoredRegions() const;
    
    /**
     * @brief Verify that no adjacent regions have the same color
     * 
     * @return bool True if coloring is valid
     */
    bool verifyColoring() const;
    
    /**
     * @brief Get color as a string
     * 
     * @param color Color enum value
     * @return std::string String representation of the color
     */
    static std::string colorToString(Color color);

private:
    std::vector<SpatialPartitioner::Region> regions_;
    std::map<std::string, std::set<std::string>> adjacencyGraph_;
    std::vector<ColoredRegion> coloredRegions_;
    
    /**
     * @brief Check if two regions are adjacent
     * 
     * @param region1 First region
     * @param region2 Second region
     * @return bool True if the regions are adjacent
     */
    bool areRegionsAdjacent(const SpatialPartitioner::Region& region1, 
                           const SpatialPartitioner::Region& region2) const;
    
    /**
     * @brief Try to color a region
     * 
     * @param regionIdx Index of the region to color
     * @param colors Current color assignments
     * @return bool True if coloring was successful
     */
    bool tryColor(int regionIdx, std::vector<Color>& colors);
    
    /**
     * @brief Check if a color is valid for a region
     * 
     * @param regionIdx Index of the region to check
     * @param color Color to check
     * @param colors Current color assignments
     * @return bool True if the color is valid for the region
     */
    bool isColorValid(int regionIdx, Color color, const std::vector<Color>& colors) const;
};

} // namespace geometric
} // namespace dist_prompt