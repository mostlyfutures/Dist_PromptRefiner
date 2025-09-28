#include "orchestrator/resources/token_bucket_manager.h"
#include <thread>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <random>

namespace dist_prompt {
namespace orchestrator {
namespace resources {

// Private implementation class (PIMPL idiom)
class TokenBucketResourceManager::Impl {
public:
    // Token bucket for a specific resource type
    struct TokenBucket {
        std::string resourceType;
        int maxTokens;
        int currentTokens;
        int refillRate;
        int burstSize;
        std::chrono::milliseconds refillInterval;
        std::chrono::system_clock::time_point lastRefill;
        mutable std::mutex mutex;
        
        // Statistics
        std::atomic<long> totalRequests{0};
        std::atomic<long> successfulRequests{0};
        std::atomic<long> totalTokensDispensed{0};
    };
    
    // Active resource allocation
    struct Allocation {
        std::string allocationId;
        std::string agentId;
        std::string resourceType;
        int tokensAllocated;
        std::chrono::system_clock::time_point allocationTime;
        std::chrono::system_clock::time_point expirationTime;
    };

    Impl() : running_(false), stopRequested_(false) {}
    
    ~Impl() {
        stop();
    }
    
    bool initialize(const std::map<std::string, ResourceConfig>& configs) {
        std::lock_guard<std::mutex> lock(bucketsMutex_);
        
        buckets_.clear();
        
        for (const auto& [resourceType, config] : configs) {
            if (!createBucket(config)) {
                return false;
            }
        }
        
        return true;
    }
    
    bool registerResource(const ResourceConfig& config) {
        std::lock_guard<std::mutex> lock(bucketsMutex_);
        return createBucket(config);
    }
    
    AllocationResult requestResources(const ResourceRequest& request) {
        AllocationResult result;
        result.success = false;
        result.tokensAllocated = 0;
        
        // Find the bucket for this resource type
        auto bucketIt = buckets_.find(request.resourceType);
        if (bucketIt == buckets_.end()) {
            result.errorMessage = "Resource type not found: " + request.resourceType;
            return result;
        }
        
        TokenBucket& bucket = bucketIt->second;
        
        // Update statistics
        bucket.totalRequests++;
        
        // Check agent quota
        if (!checkAgentQuota(request.agentId, request.resourceType, request.tokensRequested)) {
            result.errorMessage = "Agent quota exceeded";
            return result;
        }
        
        // Try to allocate tokens
        std::unique_lock<std::mutex> lock(bucket.mutex);
        
        // Refill tokens if needed
        refillBucket(bucket);
        
        if (bucket.currentTokens >= request.tokensRequested) {
            // Allocation successful
            bucket.currentTokens -= request.tokensRequested;
            bucket.successfulRequests++;
            bucket.totalTokensDispensed += request.tokensRequested;
            
            // Create allocation record
            std::string allocationId = generateAllocationId();
            Allocation allocation;
            allocation.allocationId = allocationId;
            allocation.agentId = request.agentId;
            allocation.resourceType = request.resourceType;
            allocation.tokensAllocated = request.tokensRequested;
            allocation.allocationTime = std::chrono::system_clock::now();
            allocation.expirationTime = allocation.allocationTime + request.timeout;
            
            // Store allocation
            {
                std::lock_guard<std::mutex> allocLock(allocationsMutex_);
                allocations_[allocationId] = allocation;
                
                // Update agent allocation tracking
                agentAllocations_[request.agentId][request.resourceType] += request.tokensRequested;
            }
            
            result.success = true;
            result.tokensAllocated = request.tokensRequested;
            result.allocationId = allocationId;
            result.expirationTime = allocation.expirationTime;
        } else {
            result.errorMessage = "Insufficient tokens available. Requested: " + 
                                 std::to_string(request.tokensRequested) + 
                                 ", Available: " + std::to_string(bucket.currentTokens);
        }
        
        return result;
    }
    
    bool releaseResources(const std::string& allocationId) {
        std::lock_guard<std::mutex> lock(allocationsMutex_);
        
        auto allocIt = allocations_.find(allocationId);
        if (allocIt == allocations_.end()) {
            return false;
        }
        
        const Allocation& allocation = allocIt->second;
        
        // Return tokens to bucket
        auto bucketIt = buckets_.find(allocation.resourceType);
        if (bucketIt != buckets_.end()) {
            TokenBucket& bucket = bucketIt->second;
            std::lock_guard<std::mutex> bucketLock(bucket.mutex);
            
            bucket.currentTokens = std::min(bucket.currentTokens + allocation.tokensAllocated, 
                                           bucket.maxTokens);
        }
        
        // Update agent allocation tracking
        agentAllocations_[allocation.agentId][allocation.resourceType] -= allocation.tokensAllocated;
        if (agentAllocations_[allocation.agentId][allocation.resourceType] <= 0) {
            agentAllocations_[allocation.agentId].erase(allocation.resourceType);
            if (agentAllocations_[allocation.agentId].empty()) {
                agentAllocations_.erase(allocation.agentId);
            }
        }
        
        // Remove allocation
        allocations_.erase(allocIt);
        
        return true;
    }
    
    int getAvailableTokens(const std::string& resourceType) const {
        auto bucketIt = buckets_.find(resourceType);
        if (bucketIt == buckets_.end()) {
            return 0;
        }
        
        const TokenBucket& bucket = bucketIt->second;
        std::lock_guard<std::mutex> lock(bucket.mutex);
        return bucket.currentTokens;
    }
    
    std::map<std::string, double> getResourceStats(const std::string& resourceType) const {
        std::map<std::string, double> stats;
        
        auto bucketIt = buckets_.find(resourceType);
        if (bucketIt == buckets_.end()) {
            return stats;
        }
        
        const TokenBucket& bucket = bucketIt->second;
        std::lock_guard<std::mutex> lock(bucket.mutex);
        
        long totalRequests = bucket.totalRequests.load();
        long successfulRequests = bucket.successfulRequests.load();
        
        stats["total_requests"] = static_cast<double>(totalRequests);
        stats["successful_requests"] = static_cast<double>(successfulRequests);
        stats["success_rate"] = totalRequests > 0 ? 
            static_cast<double>(successfulRequests) / totalRequests : 0.0;
        stats["total_tokens_dispensed"] = static_cast<double>(bucket.totalTokensDispensed.load());
        stats["current_tokens"] = static_cast<double>(bucket.currentTokens);
        stats["max_tokens"] = static_cast<double>(bucket.maxTokens);
        stats["utilization"] = bucket.maxTokens > 0 ? 
            1.0 - (static_cast<double>(bucket.currentTokens) / bucket.maxTokens) : 0.0;
        
        return stats;
    }
    
    bool setAgentQuota(const std::string& agentId, 
                      const std::string& resourceType, 
                      int maxTokens) {
        std::lock_guard<std::mutex> lock(quotasMutex_);
        agentQuotas_[agentId][resourceType] = maxTokens;
        return true;
    }
    
    int getAgentAllocation(const std::string& agentId, 
                          const std::string& resourceType) const {
        std::lock_guard<std::mutex> lock(allocationsMutex_);
        
        auto agentIt = agentAllocations_.find(agentId);
        if (agentIt == agentAllocations_.end()) {
            return 0;
        }
        
        auto resourceIt = agentIt->second.find(resourceType);
        if (resourceIt == agentIt->second.end()) {
            return 0;
        }
        
        return resourceIt->second;
    }
    
    bool start() {
        if (running_.load()) {
            return true;
        }
        
        stopRequested_ = false;
        refillThread_ = std::thread(&Impl::refillLoop, this);
        cleanupThread_ = std::thread(&Impl::cleanupLoop, this);
        running_ = true;
        
        return true;
    }
    
    void stop() {
        if (!running_.load()) {
            return;
        }
        
        stopRequested_ = true;
        
        if (refillThread_.joinable()) {
            refillThread_.join();
        }
        
        if (cleanupThread_.joinable()) {
            cleanupThread_.join();
        }
        
        running_ = false;
    }
    
    bool isRunning() const {
        return running_.load();
    }

private:
    std::unordered_map<std::string, TokenBucket> buckets_;
    mutable std::mutex bucketsMutex_;
    
    std::unordered_map<std::string, Allocation> allocations_;
    mutable std::mutex allocationsMutex_;
    
    std::unordered_map<std::string, std::unordered_map<std::string, int>> agentAllocations_;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> agentQuotas_;
    mutable std::mutex quotasMutex_;
    
    std::atomic<bool> running_;
    std::atomic<bool> stopRequested_;
    std::thread refillThread_;
    std::thread cleanupThread_;
    
    bool createBucket(const ResourceConfig& config) {
        TokenBucket bucket;
        bucket.resourceType = config.resourceType;
        bucket.maxTokens = config.maxTokens;
        bucket.currentTokens = config.maxTokens;  // Start full
        bucket.refillRate = config.refillRate;
        bucket.burstSize = config.burstSize;
        bucket.refillInterval = config.refillInterval;
        bucket.lastRefill = std::chrono::system_clock::now();
        
        buckets_[config.resourceType] = bucket;
        return true;
    }
    
    void refillBucket(TokenBucket& bucket) {
        auto now = std::chrono::system_clock::now();
        auto timeSinceLastRefill = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - bucket.lastRefill);
        
        if (timeSinceLastRefill >= bucket.refillInterval) {
            // Calculate how many tokens to add
            int intervalsElapsed = static_cast<int>(timeSinceLastRefill / bucket.refillInterval);
            int tokensToAdd = intervalsElapsed * bucket.refillRate;
            
            // Apply burst limit
            tokensToAdd = std::min(tokensToAdd, bucket.burstSize);
            
            // Add tokens up to maximum
            bucket.currentTokens = std::min(bucket.currentTokens + tokensToAdd, bucket.maxTokens);
            
            // Update last refill time
            bucket.lastRefill = now;
        }
    }
    
    bool checkAgentQuota(const std::string& agentId, 
                        const std::string& resourceType, 
                        int requestedTokens) {
        std::lock_guard<std::mutex> lock(quotasMutex_);
        
        auto agentIt = agentQuotas_.find(agentId);
        if (agentIt == agentQuotas_.end()) {
            return true;  // No quota set
        }
        
        auto resourceIt = agentIt->second.find(resourceType);
        if (resourceIt == agentIt->second.end()) {
            return true;  // No quota for this resource type
        }
        
        int currentAllocation = getAgentAllocation(agentId, resourceType);
        return (currentAllocation + requestedTokens) <= resourceIt->second;
    }
    
    std::string generateAllocationId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        
        std::string id;
        for (int i = 0; i < 16; ++i) {
            int val = dis(gen);
            if (val < 10) {
                id += ('0' + val);
            } else {
                id += ('a' + val - 10);
            }
        }
        
        return id;
    }
    
    void refillLoop() {
        while (!stopRequested_) {
            {
                std::lock_guard<std::mutex> lock(bucketsMutex_);
                for (auto& [resourceType, bucket] : buckets_) {
                    std::lock_guard<std::mutex> bucketLock(bucket.mutex);
                    refillBucket(bucket);
                }
            }
            
            // Sleep for minimum refill interval
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void cleanupLoop() {
        while (!stopRequested_) {
            auto now = std::chrono::system_clock::now();
            
            {
                std::lock_guard<std::mutex> lock(allocationsMutex_);
                
                auto it = allocations_.begin();
                while (it != allocations_.end()) {
                    if (now > it->second.expirationTime) {
                        // Allocation has expired, release it
                        releaseResources(it->first);
                        it = allocations_.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            
            // Check every 5 seconds
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
};

// TokenBucketResourceManager implementation

TokenBucketResourceManager::TokenBucketResourceManager() 
    : pImpl_(std::make_unique<Impl>()) {}

TokenBucketResourceManager::~TokenBucketResourceManager() = default;

bool TokenBucketResourceManager::initialize(const std::map<std::string, ResourceConfig>& configs) {
    return pImpl_->initialize(configs);
}

bool TokenBucketResourceManager::registerResource(const ResourceConfig& config) {
    return pImpl_->registerResource(config);
}

TokenBucketResourceManager::AllocationResult TokenBucketResourceManager::requestResources(
    const ResourceRequest& request) {
    return pImpl_->requestResources(request);
}

bool TokenBucketResourceManager::releaseResources(const std::string& allocationId) {
    return pImpl_->releaseResources(allocationId);
}

int TokenBucketResourceManager::getAvailableTokens(const std::string& resourceType) const {
    return pImpl_->getAvailableTokens(resourceType);
}

std::map<std::string, double> TokenBucketResourceManager::getResourceStats(
    const std::string& resourceType) const {
    return pImpl_->getResourceStats(resourceType);
}

bool TokenBucketResourceManager::setAgentQuota(const std::string& agentId, 
                                              const std::string& resourceType, 
                                              int maxTokens) {
    return pImpl_->setAgentQuota(agentId, resourceType, maxTokens);
}

int TokenBucketResourceManager::getAgentAllocation(const std::string& agentId, 
                                                  const std::string& resourceType) const {
    return pImpl_->getAgentAllocation(agentId, resourceType);
}

bool TokenBucketResourceManager::start() {
    return pImpl_->start();
}

void TokenBucketResourceManager::stop() {
    pImpl_->stop();
}

bool TokenBucketResourceManager::isRunning() const {
    return pImpl_->isRunning();
}

} // namespace resources
} // namespace orchestrator
} // namespace dist_prompt