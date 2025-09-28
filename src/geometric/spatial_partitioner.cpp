#include "geometric/spatial_partitioner.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <queue>

namespace dist_prompt {
namespace geometric {

SpatialPartitioner::SpatialPartitioner(int dimensions, int maxDepth)
    : dimensions_(dimensions), maxDepth_(maxDepth), root_(nullptr) {
}

void SpatialPartitioner::addPoint(const Point& point) {
    // Ensure the point has the correct number of dimensions
    if (point.coordinates.size() != static_cast<size_t>(dimensions_)) {
        throw std::invalid_argument("Point has incorrect number of dimensions");
    }
    
    points_.push_back(point);
}

bool SpatialPartitioner::buildKdTree() {
    if (points_.empty()) {
        return false;
    }
    
    // Initialize min and max bounds for all dimensions
    std::vector<double> min(dimensions_, std::numeric_limits<double>::max());
    std::vector<double> max(dimensions_, std::numeric_limits<double>::lowest());
    
    // Determine bounds for all points
    for (const auto& point : points_) {
        for (int d = 0; d < dimensions_; ++d) {
            min[d] = std::min(min[d], point.coordinates[d]);
            max[d] = std::max(max[d], point.coordinates[d]);
        }
    }
    
    // Build the k-d tree recursively
    root_ = buildKdTreeRecursive(points_, 0, min, max);
    
    // Collect regions from the tree
    regions_.clear();
    collectRegions(root_.get(), regions_);
    
    return true;
}

std::vector<SpatialPartitioner::Region> SpatialPartitioner::getRegions() const {
    return regions_;
}

void SpatialPartitioner::setMaxDepth(int depth) {
    if (depth >= 1) {
        maxDepth_ = depth;
    }
}

int SpatialPartitioner::getMaxDepth() const {
    return maxDepth_;
}

SpatialPartitioner::Region SpatialPartitioner::findRegion(const Point& point) const {
    const Region* region = findRegionRecursive(root_.get(), point);
    if (region) {
        return *region;
    }
    
    // If not found (shouldn't happen with a properly built tree), return an empty region
    return Region{};
}

std::unique_ptr<SpatialPartitioner::KdNode> SpatialPartitioner::buildKdTreeRecursive(
    const std::vector<Point>& points, 
    int depth, 
    const std::vector<double>& min, 
    const std::vector<double>& max) {
    
    // Create a new node
    auto node = std::make_unique<KdNode>();
    
    // If we've reached maximum depth or have too few points, create a leaf node
    if (depth >= maxDepth_ || points.size() <= 5) {
        node->isLeaf = true;
        node->region.id = "R" + std::to_string(regions_.size() + 1);
        node->region.name = "Region " + std::to_string(regions_.size() + 1);
        node->region.points = points;
        node->region.min = min;
        node->region.max = max;
        return node;
    }
    
    // Determine which dimension to split on (cycle through dimensions)
    int splitDim = depth % dimensions_;
    node->splitDimension = splitDim;
    
    // Find median value for the split dimension
    std::vector<double> values;
    values.reserve(points.size());
    for (const auto& point : points) {
        values.push_back(point.coordinates[splitDim]);
    }
    
    // Sort values to find median
    std::sort(values.begin(), values.end());
    double median = values[values.size() / 2];
    node->splitValue = median;
    
    // Partition points
    std::vector<Point> leftPoints, rightPoints;
    for (const auto& point : points) {
        if (point.coordinates[splitDim] <= median) {
            leftPoints.push_back(point);
        } else {
            rightPoints.push_back(point);
        }
    }
    
    // If we couldn't partition effectively, make this a leaf node
    if (leftPoints.empty() || rightPoints.empty()) {
        node->isLeaf = true;
        node->region.id = "R" + std::to_string(regions_.size() + 1);
        node->region.name = "Region " + std::to_string(regions_.size() + 1);
        node->region.points = points;
        node->region.min = min;
        node->region.max = max;
        return node;
    }
    
    // This is an internal node
    node->isLeaf = false;
    
    // Create bounds for left and right children
    std::vector<double> leftMax = max;
    leftMax[splitDim] = median;
    
    std::vector<double> rightMin = min;
    rightMin[splitDim] = median;
    
    // Recursively build left and right subtrees
    node->left = buildKdTreeRecursive(leftPoints, depth + 1, min, leftMax);
    node->right = buildKdTreeRecursive(rightPoints, depth + 1, rightMin, max);
    
    return node;
}

void SpatialPartitioner::collectRegions(const KdNode* node, std::vector<Region>& regions) const {
    if (!node) {
        return;
    }
    
    if (node->isLeaf) {
        regions.push_back(node->region);
    } else {
        collectRegions(node->left.get(), regions);
        collectRegions(node->right.get(), regions);
    }
}

const SpatialPartitioner::Region* SpatialPartitioner::findRegionRecursive(
    const KdNode* node, const Point& point) const {
    
    if (!node) {
        return nullptr;
    }
    
    if (node->isLeaf) {
        return &node->region;
    }
    
    // Check which side of the splitting plane the point lies on
    if (point.coordinates[node->splitDimension] <= node->splitValue) {
        return findRegionRecursive(node->left.get(), point);
    } else {
        return findRegionRecursive(node->right.get(), point);
    }
}

} // namespace geometric
} // namespace dist_prompt