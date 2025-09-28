#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

namespace dist_prompt {

/**
 * @brief AgentCoordinator class - Interface for the Agentic Orchestrator
 * 
 * This class coordinates the workflow between all components and manages
 * the autonomous agent interactions.
 */
class AgentCoordinator {
public:
    /**
     * @brief Enum representing agent states in the FSM
     */
    enum class AgentState {
        IDLE,
        INITIALIZING,
        ANALYZING,
        PLANNING,
        EXECUTING,
        REFLECTING,
        TERMINATING
    };
    
    /**
     * @brief Structure representing agent configuration
     */
    struct AgentConfig {
        std::string agentName;
        std::map<std::string, std::string> capabilities;
        int priority;
        std::vector<std::string> dependencies;
    };

    /**
     * @brief Default constructor
     */
    AgentCoordinator() = default;
    
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~AgentCoordinator() = default;
    
    /**
     * @brief Initialize the agent coordinator with a workflow definition
     * 
     * @param workflowDefinition JSON string defining the agent workflow
     * @return bool True if initialization was successful, false otherwise
     */
    virtual bool initializeWorkflow(const std::string& workflowDefinition) = 0;
    
    /**
     * @brief Register a component with the coordinator
     * 
     * @param componentType Type of the component (e.g., "cli", "pcam", etc.)
     * @param componentInstance Shared pointer to the component instance
     * @return bool True if registration was successful, false otherwise
     */
    virtual bool registerComponent(const std::string& componentType, 
                                 void* componentInstance) = 0;
    
    /**
     * @brief Start the workflow execution
     * 
     * @param input Initial input data for the workflow
     * @return bool True if workflow started successfully, false otherwise
     */
    virtual bool startWorkflow(const std::string& input) = 0;
    
    /**
     * @brief Get the current state of an agent
     * 
     * @param agentName Name of the agent
     * @return AgentState Current state of the agent
     */
    virtual AgentState getAgentState(const std::string& agentName) const = 0;
    
    /**
     * @brief Set a callback for workflow events
     * 
     * @param callback Function to call when workflow events occur
     */
    virtual void setWorkflowEventCallback(
        std::function<void(const std::string&, const std::string&)> callback) = 0;
    
    /**
     * @brief Get the final result of the workflow execution
     * 
     * @return std::string JSON string containing the workflow results
     */
    virtual std::string getWorkflowResult() const = 0;
    
    /**
     * @brief Manage resources using token bucket algorithm
     * 
     * @param resourceType Type of resource (e.g., "cpu", "memory")
     * @param amount Amount of resource to request
     * @return bool True if resources were successfully allocated, false otherwise
     */
    virtual bool requestResources(const std::string& resourceType, int amount) = 0;
};

} // namespace dist_prompt