#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

namespace dist_prompt {
namespace orchestrator {

/**
 * @brief Agent lifecycle manager using Finite State Machine (FSM)
 * 
 * Manages the lifecycle of agents in the system, tracking state transitions
 * and executing appropriate actions on state changes.
 */
class AgentLifecycle {
public:
    /**
     * @brief Agent state enumeration
     */
    enum class State {
        UNINITIALIZED,
        INITIALIZING,
        READY,
        RUNNING,
        PAUSED,
        ERROR,
        TERMINATED
    };
    
    /**
     * @brief Agent event enumeration
     */
    enum class Event {
        INITIALIZE,
        INITIALIZATION_COMPLETE,
        INITIALIZATION_FAILED,
        START,
        STOP,
        PAUSE,
        RESUME,
        ERROR_OCCURRED,
        RECOVERY_COMPLETE,
        TERMINATE
    };
    
    /**
     * @brief Agent context structure
     */
    struct AgentContext {
        std::string id;
        std::string name;
        std::string type;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> metadata;
        
        // State tracking
        State currentState = State::UNINITIALIZED;
        State previousState = State::UNINITIALIZED;
        
        // Performance metrics
        double cpuUsage = 0.0;
        double memoryUsage = 0.0;
        int operationsCompleted = 0;
        int operationsFailed = 0;
        
        // Runtime data
        std::map<std::string, std::string> runtimeData;
    };

    /**
     * @brief Constructor
     */
    AgentLifecycle();
    
    /**
     * @brief Destructor
     */
    ~AgentLifecycle();
    
    /**
     * @brief Create a new agent
     * 
     * @param name Agent name
     * @param type Agent type
     * @param parameters Agent parameters
     * @return std::string Agent ID
     */
    std::string createAgent(
        const std::string& name,
        const std::string& type,
        const std::map<std::string, std::string>& parameters = {});
    
    /**
     * @brief Trigger an event for an agent
     * 
     * @param agentId Agent ID
     * @param event Event to trigger
     * @param eventData Optional event data
     * @return bool True if event was processed successfully
     */
    bool triggerEvent(
        const std::string& agentId,
        Event event,
        const std::string& eventData = "");
    
    /**
     * @brief Get the current state of an agent
     * 
     * @param agentId Agent ID
     * @return State Current agent state
     */
    State getAgentState(const std::string& agentId) const;
    
    /**
     * @brief Get the context for an agent
     * 
     * @param agentId Agent ID
     * @return const AgentContext& Agent context
     */
    const AgentContext& getAgentContext(const std::string& agentId) const;
    
    /**
     * @brief Set a state transition handler
     * 
     * @param fromState State to transition from
     * @param event Event triggering the transition
     * @param toState State to transition to
     * @param handler Function to call on transition
     * @return bool True if handler was set successfully
     */
    bool setStateTransitionHandler(
        State fromState,
        Event event,
        State toState,
        std::function<void(AgentContext&, const std::string&)> handler);
    
    /**
     * @brief Set a state entry handler
     * 
     * @param state State to handle entry for
     * @param handler Function to call on state entry
     * @return bool True if handler was set successfully
     */
    bool setStateEntryHandler(
        State state,
        std::function<void(AgentContext&)> handler);
    
    /**
     * @brief Set a state exit handler
     * 
     * @param state State to handle exit for
     * @param handler Function to call on state exit
     * @return bool True if handler was set successfully
     */
    bool setStateExitHandler(
        State state,
        std::function<void(AgentContext&)> handler);
    
    /**
     * @brief Get all agent IDs
     * 
     * @return std::vector<std::string> List of agent IDs
     */
    std::vector<std::string> getAllAgentIds() const;
    
    /**
     * @brief Get agents in a specific state
     * 
     * @param state State to filter by
     * @return std::vector<std::string> List of agent IDs in that state
     */
    std::vector<std::string> getAgentsInState(State state) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

/**
 * @brief Convert agent state to string
 * 
 * @param state Agent state
 * @return std::string String representation
 */
std::string toString(AgentLifecycle::State state);

/**
 * @brief Convert agent event to string
 * 
 * @param event Agent event
 * @return std::string String representation
 */
std::string toString(AgentLifecycle::Event event);

} // namespace orchestrator
} // namespace dist_prompt