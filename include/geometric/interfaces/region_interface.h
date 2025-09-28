#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace dist_prompt {
namespace geometric {
namespace interfaces {

/**
 * @brief Abstract interface for region management
 * 
 * Defines the interface for working with conceptual regions in the geometric decomposition.
 */
class RegionInterface {
public:
    /**
     * @brief Structure representing a point in conceptual space
     */
    struct Point {
        std::string id;
        std::vector<double> coordinates;
        std::map<std::string, std::string> metadata;
    };
    
    /**
     * @brief Structure representing a conceptual region
     */
    struct Region {
        std::string id;
        std::string name;
        std::string description;
        std::vector<Point> points;
        std::map<std::string, std::string> properties;
    };

    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~RegionInterface() = default;
    
    /**
     * @brief Create a new region
     * 
     * @param name Region name
     * @param description Region description
     * @return std::string ID of the created region
     */
    virtual std::string createRegion(const std::string& name, const std::string& description) = 0;
    
    /**
     * @brief Add a point to a region
     * 
     * @param regionId ID of the region
     * @param point Point to add
     * @return bool True if the point was added successfully
     */
    virtual bool addPointToRegion(const std::string& regionId, const Point& point) = 0;
    
    /**
     * @brief Get a region by ID
     * 
     * @param regionId ID of the region
     * @return Region The region
     */
    virtual Region getRegion(const std::string& regionId) const = 0;
    
    /**
     * @brief Get all regions
     * 
     * @return std::vector<Region> List of all regions
     */
    virtual std::vector<Region> getAllRegions() const = 0;
    
    /**
     * @brief Set a property for a region
     * 
     * @param regionId ID of the region
     * @param key Property key
     * @param value Property value
     * @return bool True if the property was set successfully
     */
    virtual bool setRegionProperty(const std::string& regionId, 
                                  const std::string& key, 
                                  const std::string& value) = 0;
    
    /**
     * @brief Get a property for a region
     * 
     * @param regionId ID of the region
     * @param key Property key
     * @return std::string Property value, or empty string if not found
     */
    virtual std::string getRegionProperty(const std::string& regionId, 
                                         const std::string& key) const = 0;
};

} // namespace interfaces
} // namespace geometric
} // namespace dist_prompt