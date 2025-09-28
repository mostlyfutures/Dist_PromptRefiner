#pragma once

#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>

namespace dist_prompt {
namespace orchestrator {
namespace resources {

/**
 * @brief Token bucket resource manager for agent resource allocation
 * 
 * Implements token bucket algorithm for rate limiting and resource management.
 */
class TokenBucketResourceManager {
public:
    /**
     * @brief Resource configuration structure
     */
    struct ResourceConfig {
        std::string resourceType;
        int maxTokens;           // Maximum tokens in bucket
        int refillRate;          // Tokens added per second
        int burstSize;           // Maximum burst tokens allowed
        std::chrono::milliseconds refillInterval;
    };
    
    /**
     * @brief Resource request structure
     */
    struct ResourceRequest {
        std::string agentId;
        std::string resourceType;
        int tokensRequested;
        int priority;            // 0 = highest, higher numbers = lower priority
        std::chrono::milliseconds timeout;
    };
    
    /**
     * @brief Resource allocation result
     */
    struct AllocationResult {
        bool success;
        int tokensAllocated;
        std::string allocationId;
        std::chrono::system_clock::time_point expirationTime;
        std::string errorMessage;
    };

    /**
     * @brief Constructor
     */
    TokenBucketResourceManager();
    
    /**
     * @brief Destructor
     */
    ~TokenBucketResourceManager();
    
    /**
     * @brief Initialize resource manager with configurations
     * 
     * @param configs Map of resource type to configuration
     * @return bool True if initialization was successful
     */
    bool initialize(const std::map<std::string, ResourceConfig>& configs);
    
    /**
     * @brief Register a new resource type
     * 
     * @param config Resource configuration
     * @return bool True if registration was successful
     */
    bool registerResource(const ResourceConfig& config);
    
    /**
     * @brief Request resource allocation
     * 
     * @param request Resource request
     * @return AllocationResult Result of the allocation attempt
     */
    AllocationResult requestResources(const ResourceRequest& request);
    
    /**
     * @brief Release allocated resources
     * 
     * @param allocationId ID of the allocation to release
     * @return bool True if resources were released successfully
     */
    bool releaseResources(const std::string& allocationId);
    
    /**
     * @brief Get current token count for a resource type
     * 
     * @param resourceType Type of resource
     * @return int Current number of available tokens
     */
    int getAvailableTokens(const std::string& resourceType) const;
    
    /**
     * @brief Get resource usage statistics
     * 
     * @param resourceType Type of resource
     * @return std::map<std::string, double> Usage statistics
     */
    std::map<std::string, double> getResourceStats(const std::string& resourceType) const;
    
    /**
     * @brief Set resource quota for an agent
     * 
     * @param agentId Agent identifier
     * @param resourceType Type of resource
     * @param maxTokens Maximum tokens the agent can hold
     * @return bool True if quota was set successfully
     */
    bool setAgentQuota(const std::string& agentId, 
                      const std::string& resourceType, 
                      int maxTokens);
    
    /**
     * @brief Get agent's current resource allocation
     * 
     * @param agentId Agent identifier
     * @param resourceType Type of resource
     * @return int Number of tokens currently allocated to the agent
     */
    int getAgentAllocation(const std::string& agentId, 
                          const std::string& resourceType) const;
    
    /**
     * @brief Start the resource manager (begins token refill process)
     * 
     * @return bool True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the resource manager
     */
    void stop();
    
    /**
     * @brief Check if resource manager is running
     * 
     * @return bool True if running
     */
    bool isRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace resources
} // namespace orchestrator
} // namespace dist_prompt