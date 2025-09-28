#include "geometric/region_assigner.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace dist_prompt {
namespace geometric {

RegionAssigner::RegionAssigner() = default;

void RegionAssigner::setRegions(const std::vector<SpatialPartitioner::Region>& regions) {
    regions_ = regions;
    adjacencyGraph_.clear();
    coloredRegions_.clear();
}

bool RegionAssigner::determineAdjacency() {
    if (regions_.empty()) {
        return false;
    }
    
    // Build adjacency graph
    for (size_t i = 0; i < regions_.size(); ++i) {
        for (size_t j = i + 1; j < regions_.size(); ++j) {
            if (areRegionsAdjacent(regions_[i], regions_[j])) {
                // Add bidirectional adjacency
                adjacencyGraph_[regions_[i].id].insert(regions_[j].id);
                adjacencyGraph_[regions_[j].id].insert(regions_[i].id);
            }
        }
    }
    
    return true;
}

bool RegionAssigner::assignColors() {
    if (regions_.empty() || adjacencyGraph_.empty()) {
        return false;
    }
    
    // Initialize coloredRegions_
    coloredRegions_.clear();
    for (const auto& region : regions_) {
        ColoredRegion coloredRegion;
        coloredRegion.id = region.id;
        coloredRegion.name = region.name;
        
        // Get adjacent regions from the adjacency graph
        auto it = adjacencyGraph_.find(region.id);
        if (it != adjacencyGraph_.end()) {
            coloredRegion.adjacentRegions.insert(
                coloredRegion.adjacentRegions.end(), 
                it->second.begin(), 
                it->second.end()
            );
        }
        
        coloredRegions_.push_back(coloredRegion);
    }
    
    // Initialize color assignments to invalid color
    std::vector<Color> colors(regions_.size(), static_cast<Color>(-1));
    
    // Try to color the graph
    bool success = tryColor(0, colors);
    
    if (success) {
        // Apply colors to coloredRegions_
        for (size_t i = 0; i < regions_.size(); ++i) {
            coloredRegions_[i].color = colors[i];
        }
        return true;
    }
    
    return false;
}

std::vector<RegionAssigner::ColoredRegion> RegionAssigner::getColoredRegions() const {
    return coloredRegions_;
}

bool RegionAssigner::verifyColoring() const {
    // Check that every region has a valid color
    for (const auto& region : coloredRegions_) {
        if (static_cast<int>(region.color) < 0 || static_cast<int>(region.color) > 3) {
            return false;
        }
        
        // Check that no adjacent regions have the same color
        for (const auto& adjRegionId : region.adjacentRegions) {
            auto adjRegion = std::find_if(
                coloredRegions_.begin(), 
                coloredRegions_.end(), 
                [&adjRegionId](const ColoredRegion& r) { return r.id == adjRegionId; }
            );
            
            if (adjRegion != coloredRegions_.end() && adjRegion->color == region.color) {
                return false;
            }
        }
    }
    
    return true;
}

std::string RegionAssigner::colorToString(Color color) {
    switch (color) {
        case Color::RED:    return "RED";
        case Color::GREEN:  return "GREEN";
        case Color::BLUE:   return "BLUE";
        case Color::YELLOW: return "YELLOW";
        default:            return "UNKNOWN";
    }
}

bool RegionAssigner::areRegionsAdjacent(
    const SpatialPartitioner::Region& region1, 
    const SpatialPartitioner::Region& region2) const {
    
    // Two regions are adjacent if they share a boundary (simplified check)
    int dimensions = region1.min.size();
    
    // Check if regions are adjacent along each dimension
    for (int dim = 0; dim < dimensions; ++dim) {
        // Check if regions touch along dimension dim
        bool touching = false;
        
        // Region 1 min touches Region 2 max
        if (std::abs(region1.min[dim] - region2.max[dim]) < 1e-6) {
            touching = true;
        }
        
        // Region 1 max touches Region 2 min
        if (std::abs(region1.max[dim] - region2.min[dim]) < 1e-6) {
            touching = true;
        }
        
        if (touching) {
            // Check if regions overlap in all other dimensions
            bool overlap = true;
            for (int d = 0; d < dimensions; ++d) {
                if (d == dim) continue;
                
                // Check for overlap in dimension d
                if (region1.max[d] < region2.min[d] || region1.min[d] > region2.max[d]) {
                    overlap = false;
                    break;
                }
            }
            
            if (overlap) {
                return true;
            }
        }
    }
    
    return false;
}

bool RegionAssigner::tryColor(int regionIdx, std::vector<Color>& colors) {
    // If we've colored all regions, we're done
    if (regionIdx >= static_cast<int>(regions_.size())) {
        return true;
    }
    
    // Try each color for this region
    for (int c = 0; c < 4; ++c) {
        Color color = static_cast<Color>(c);
        
        // Check if this color is valid
        if (isColorValid(regionIdx, color, colors)) {
            // Assign the color
            colors[regionIdx] = color;
            
            // Recursively try to color the next region
            if (tryColor(regionIdx + 1, colors)) {
                return true;
            }
            
            // If we couldn't color the remaining regions, backtrack
            colors[regionIdx] = static_cast<Color>(-1);
        }
    }
    
    return false;
}

bool RegionAssigner::isColorValid(
    int regionIdx, Color color, const std::vector<Color>& colors) const {
    
    const auto& regionId = regions_[regionIdx].id;
    
    // Check all adjacent regions
    auto it = adjacencyGraph_.find(regionId);
    if (it != adjacencyGraph_.end()) {
        for (const auto& adjId : it->second) {
            // Find the index of the adjacent region
            auto adjRegionIt = std::find_if(
                regions_.begin(), 
                regions_.end(), 
                [&adjId](const SpatialPartitioner::Region& r) { return r.id == adjId; }
            );
            
            if (adjRegionIt != regions_.end()) {
                int adjIdx = std::distance(regions_.begin(), adjRegionIt);
                
                // If the adjacent region has been colored with the same color, this color is invalid
                if (static_cast<int>(colors[adjIdx]) != -1 && colors[adjIdx] == color) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

} // namespace geometric
} // namespace dist_prompt