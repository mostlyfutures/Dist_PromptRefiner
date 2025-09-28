#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <chrono>

namespace dist_prompt {
namespace integration {

/**
 * @brief Agent configuration structure
 */
struct AgentConfig {
    std::string agentId;
    std::string agentType;  // "cli", "pcam", "geometric", "pattern", "openmd"
    std::string endpoint;
    std::map<std::string, std::string> capabilities;
    int priority;
    std::chrono::milliseconds heartbeatInterval;
};

/**
 * @brief Workflow step structure
 */
struct WorkflowStep {
    std::string stepId;
    std::string agentType;
    std::string action;
    std::map<std::string, std::string> parameters;
    std::vector<std::string> dependencies;
    std::chrono::milliseconds timeout;
};

/**
 * @brief Workflow definition
 */
struct Workflow {
    std::string workflowId;
    std::string name;
    std::vector<WorkflowStep> steps;
    std::map<std::string, std::string> globalParams;
    std::string trigger;
};

/**
 * @brief Execution context
 */
struct ExecutionContext {
    std::string contextId;
    std::string workflowId;
    std::map<std::string, std::string> variables;
    std::string workingDirectory;
    std::chrono::system_clock::time_point startTime;
};

/**
 * @brief Agent status enumeration
 */
enum class AgentStatus {
    OFFLINE,
    IDLE,
    BUSY,
    ERROR,
    MAINTENANCE
};

/**
 * @brief Execution result
 */
struct ExecutionResult {
    bool success;
    std::string resultData;
    std::map<std::string, std::string> outputs;
    std::string errorMessage;
    std::chrono::milliseconds executionTime;
};

/**
 * @brief Workflow event callback
 */
using WorkflowCallback = std::function<void(const std::string& workflowId,
                                          const std::string& stepId,
                                          const std::string& event,
                                          const std::string& data)>;

/**
 * @brief AgentCoordinator interface for All → Orchestrator integration
 * 
 * This interface defines agent coordination and workflow management with
 * FSM (7 states), gRPC communication, and token bucket resource management
 * as specified in the integration flow.
 */
class AgentCoordinator {
public:
    virtual ~AgentCoordinator() = default;
    
    /**
     * @brief Initialize the orchestrator
     * 
     * @param configPath Path to orchestrator configuration
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::string& configPath) = 0;
    
    /**
     * @brief Register an agent with the orchestrator
     * 
     * @param config Agent configuration
     * @return bool True if registration was successful
     */
    virtual bool registerAgent(const AgentConfig& config) = 0;
    
    /**
     * @brief Unregister an agent
     * 
     * @param agentId Agent identifier
     * @return bool True if unregistration was successful
     */
    virtual bool unregisterAgent(const std::string& agentId) = 0;
    
    /**
     * @brief Get status of all registered agents
     * 
     * @return std::map<std::string, AgentStatus> Agent ID to status mapping
     */
    virtual std::map<std::string, AgentStatus> getAgentStatuses() = 0;
    
    /**
     * @brief Define a new workflow
     * 
     * @param workflow Workflow definition
     * @return bool True if workflow was defined successfully
     */
    virtual bool defineWorkflow(const Workflow& workflow) = 0;
    
    /**
     * @brief Execute a workflow
     * 
     * @param workflowId Workflow identifier
     * @param context Execution context
     * @param callback Progress callback
     * @return std::string Execution ID for tracking
     */
    virtual std::string executeWorkflow(const std::string& workflowId,
                                       const ExecutionContext& context,
                                       WorkflowCallback callback = nullptr) = 0;
    
    /**
     * @brief Get workflow execution status
     * 
     * @param executionId Execution identifier
     * @return std::string Current status
     */
    virtual std::string getExecutionStatus(const std::string& executionId) = 0;
    
    /**
     * @brief Get workflow execution results
     * 
     * @param executionId Execution identifier
     * @return ExecutionResult Results and outputs
     */
    virtual ExecutionResult getExecutionResults(const std::string& executionId) = 0;
    
    /**
     * @brief Cancel workflow execution
     * 
     * @param executionId Execution identifier
     * @return bool True if cancellation was successful
     */
    virtual bool cancelExecution(const std::string& executionId) = 0;
    
    /**
     * @brief Send message to specific agent
     * 
     * @param agentId Target agent identifier
     * @param message Message content
     * @param messageType Message type
     * @return bool True if message was sent successfully
     */
    virtual bool sendMessage(const std::string& agentId,
                           const std::string& message,
                           const std::string& messageType) = 0;
    
    /**
     * @brief Broadcast message to all agents of a type
     * 
     * @param agentType Target agent type
     * @param message Message content
     * @param messageType Message type
     * @return int Number of agents that received the message
     */
    virtual int broadcastMessage(const std::string& agentType,
                               const std::string& message,
                               const std::string& messageType) = 0;
    
    /**
     * @brief Allocate resources to an agent
     * 
     * @param agentId Agent identifier
     * @param resourceType Type of resource
     * @param amount Amount to allocate
     * @return std::string Allocation ID
     */
    virtual std::string allocateResources(const std::string& agentId,
                                        const std::string& resourceType,
                                        int amount) = 0;
    
    /**
     * @brief Release allocated resources
     * 
     * @param allocationId Allocation identifier
     * @return bool True if resources were released
     */
    virtual bool releaseResources(const std::string& allocationId) = 0;
    
    /**
     * @brief Get orchestrator statistics
     * 
     * @return std::map<std::string, double> Performance metrics
     */
    virtual std::map<std::string, double> getStatistics() = 0;
    
    /**
     * @brief Start the orchestrator services
     * 
     * @return bool True if started successfully
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop the orchestrator services
     */
    virtual void stop() = 0;
    
    /**
     * @brief Check if orchestrator is running
     * 
     * @return bool True if orchestrator is running
     */
    virtual bool isRunning() const = 0;
};

/**
 * @brief Factory function to create AgentCoordinator instance
 * 
 * @return std::unique_ptr<AgentCoordinator> AgentCoordinator instance
 */
std::unique_ptr<AgentCoordinator> createAgentCoordinator();

} // namespace integration
} // namespace dist_prompt