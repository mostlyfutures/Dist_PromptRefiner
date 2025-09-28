#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>

namespace dist_prompt {
namespace geometric {

/**
 * @brief Spatial partitioning using k-d tree with configurable depth
 * 
 * Implements spatial partitioning for geometric decomposition of software ideas.
 */
class SpatialPartitioner {
public:
    /**
     * @brief Structure representing a point in conceptual space
     */
    struct Point {
        std::vector<double> coordinates;
        std::string id;
        std::map<std::string, std::string> metadata;
    };
    
    /**
     * @brief Structure representing a region in conceptual space
     */
    struct Region {
        std::string id;
        std::string name;
        std::vector<Point> points;
        std::vector<double> min;  // Min bounds for each dimension
        std::vector<double> max;  // Max bounds for each dimension
    };
    
    /**
     * @brief k-d tree node structure
     */
    struct KdNode {
        std::unique_ptr<KdNode> left;
        std::unique_ptr<KdNode> right;
        Region region;
        int splitDimension;
        double splitValue;
        bool isLeaf;
    };
    
    /**
     * @brief Constructor
     * 
     * @param dimensions Number of dimensions in the conceptual space
     * @param maxDepth Maximum depth of the k-d tree
     */
    SpatialPartitioner(int dimensions = 3, int maxDepth = 5);
    
    /**
     * @brief Destructor
     */
    ~SpatialPartitioner() = default;
    
    /**
     * @brief Add a point to the conceptual space
     * 
     * @param point Point to add
     */
    void addPoint(const Point& point);
    
    /**
     * @brief Build the k-d tree
     * 
     * @return bool True if the tree was built successfully
     */
    bool buildKdTree();
    
    /**
     * @brief Get all regions from the k-d tree
     * 
     * @return std::vector<Region> List of regions
     */
    std::vector<Region> getRegions() const;
    
    /**
     * @brief Set the maximum depth of the k-d tree
     * 
     * @param depth Maximum depth (must be >= 1)
     */
    void setMaxDepth(int depth);
    
    /**
     * @brief Get the current maximum depth of the k-d tree
     * 
     * @return int Current maximum depth
     */
    int getMaxDepth() const;
    
    /**
     * @brief Find the region containing a specific point
     * 
     * @param point Point to locate
     * @return Region Region containing the point
     */
    Region findRegion(const Point& point) const;

private:
    int dimensions_;
    int maxDepth_;
    std::vector<Point> points_;
    std::unique_ptr<KdNode> root_;
    std::vector<Region> regions_;
    
    /**
     * @brief Recursive function to build the k-d tree
     * 
     * @param points Points to include in this subtree
     * @param depth Current depth
     * @param min Minimum bounds
     * @param max Maximum bounds
     * @return std::unique_ptr<KdNode> Root of the subtree
     */
    std::unique_ptr<KdNode> buildKdTreeRecursive(
        const std::vector<Point>& points, 
        int depth, 
        const std::vector<double>& min, 
        const std::vector<double>& max
    );
    
    /**
     * @brief Collect all regions from the k-d tree
     * 
     * @param node Current node
     * @param regions Output vector of regions
     */
    void collectRegions(const KdNode* node, std::vector<Region>& regions) const;
    
    /**
     * @brief Find the region containing a point (recursive)
     * 
     * @param node Current node
     * @param point Point to locate
     * @return const Region* Pointer to region containing the point, or nullptr if not found
     */
    const Region* findRegionRecursive(const KdNode* node, const Point& point) const;
};

} // namespace geometric
} // namespace dist_prompt