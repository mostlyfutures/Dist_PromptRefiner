#include "orchestrator/agent_lifecycle.h"
#include <unordered_map>
#include <uuid/uuid.h>
#include <stdexcept>
#include <chrono>
#include <iostream>

namespace dist_prompt {
namespace orchestrator {

// State and Event string conversion
std::string toString(AgentLifecycle::State state) {
    static const std::unordered_map<AgentLifecycle::State, std::string> stateStrings = {
        {AgentLifecycle::State::UNINITIALIZED, "UNINITIALIZED"},
        {AgentLifecycle::State::INITIALIZING, "INITIALIZING"},
        {AgentLifecycle::State::READY, "READY"},
        {AgentLifecycle::State::RUNNING, "RUNNING"},
        {AgentLifecycle::State::PAUSED, "PAUSED"},
        {AgentLifecycle::State::ERROR, "ERROR"},
        {AgentLifecycle::State::TERMINATED, "TERMINATED"}
    };
    
    auto it = stateStrings.find(state);
    return (it != stateStrings.end()) ? it->second : "UNKNOWN";
}

std::string toString(AgentLifecycle::Event event) {
    static const std::unordered_map<AgentLifecycle::Event, std::string> eventStrings = {
        {AgentLifecycle::Event::INITIALIZE, "INITIALIZE"},
        {AgentLifecycle::Event::INITIALIZATION_COMPLETE, "INITIALIZATION_COMPLETE"},
        {AgentLifecycle::Event::INITIALIZATION_FAILED, "INITIALIZATION_FAILED"},
        {AgentLifecycle::Event::START, "START"},
        {AgentLifecycle::Event::STOP, "STOP"},
        {AgentLifecycle::Event::PAUSE, "PAUSE"},
        {AgentLifecycle::Event::RESUME, "RESUME"},
        {AgentLifecycle::Event::ERROR_OCCURRED, "ERROR_OCCURRED"},
        {AgentLifecycle::Event::RECOVERY_COMPLETE, "RECOVERY_COMPLETE"},
        {AgentLifecycle::Event::TERMINATE, "TERMINATE"}
    };
    
    auto it = eventStrings.find(event);
    return (it != eventStrings.end()) ? it->second : "UNKNOWN";
}

// Generate a UUID string
static std::string generateUuid() {
    uuid_t uuid;
    char uuid_str[37];
    
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    
    return std::string(uuid_str);
}

// Private implementation
class AgentLifecycle::Impl {
public:
    Impl() {
        setupDefaultTransitions();
    }
    
    ~Impl() = default;
    
    std::string createAgent(
        const std::string& name,
        const std::string& type,
        const std::map<std::string, std::string>& parameters) {
        
        std::string agentId = generateUuid();
        
        AgentContext context;
        context.id = agentId;
        context.name = name;
        context.type = type;
        context.parameters = parameters;
        context.currentState = State::UNINITIALIZED;
        
        agents_[agentId] = context;
        
        return agentId;
    }
    
    bool triggerEvent(
        const std::string& agentId,
        Event event,
        const std::string& eventData) {
        
        auto agentIt = agents_.find(agentId);
        if (agentIt == agents_.end()) {
            return false;
        }
        
        AgentContext& context = agentIt->second;
        State currentState = context.currentState;
        
        // Find valid transition
        TransitionKey key{currentState, event};
        auto transitionIt = transitions_.find(key);
        
        if (transitionIt == transitions_.end()) {
            // No valid transition found
            return false;
        }
        
        // Get target state and transition handler
        State targetState = transitionIt->second.targetState;
        auto transitionHandler = transitionIt->second.handler;
        
        // Execute state exit handler if present
        auto exitHandlerIt = stateExitHandlers_.find(currentState);
        if (exitHandlerIt != stateExitHandlers_.end() && exitHandlerIt->second) {
            exitHandlerIt->second(context);
        }
        
        // Execute transition handler if present
        if (transitionHandler) {
            transitionHandler(context, eventData);
        }
        
        // Update agent state
        context.previousState = currentState;
        context.currentState = targetState;
        
        // Execute state entry handler if present
        auto entryHandlerIt = stateEntryHandlers_.find(targetState);
        if (entryHandlerIt != stateEntryHandlers_.end() && entryHandlerIt->second) {
            entryHandlerIt->second(context);
        }
        
        return true;
    }
    
    State getAgentState(const std::string& agentId) const {
        auto it = agents_.find(agentId);
        if (it == agents_.end()) {
            throw std::invalid_argument("Agent not found: " + agentId);
        }
        
        return it->second.currentState;
    }
    
    const AgentContext& getAgentContext(const std::string& agentId) const {
        auto it = agents_.find(agentId);
        if (it == agents_.end()) {
            throw std::invalid_argument("Agent not found: " + agentId);
        }
        
        return it->second;
    }
    
    bool setStateTransitionHandler(
        State fromState,
        Event event,
        State toState,
        std::function<void(AgentContext&, const std::string&)> handler) {
        
        TransitionKey key{fromState, event};
        transitions_[key] = Transition{toState, handler};
        return true;
    }
    
    bool setStateEntryHandler(
        State state,
        std::function<void(AgentContext&)> handler) {
        
        stateEntryHandlers_[state] = handler;
        return true;
    }
    
    bool setStateExitHandler(
        State state,
        std::function<void(AgentContext&)> handler) {
        
        stateExitHandlers_[state] = handler;
        return true;
    }
    
    std::vector<std::string> getAllAgentIds() const {
        std::vector<std::string> agentIds;
        for (const auto& [id, _] : agents_) {
            agentIds.push_back(id);
        }
        return agentIds;
    }
    
    std::vector<std::string> getAgentsInState(State state) const {
        std::vector<std::string> agentIds;
        for (const auto& [id, context] : agents_) {
            if (context.currentState == state) {
                agentIds.push_back(id);
            }
        }
        return agentIds;
    }

private:
    // Structure for transition key (from state + event)
    struct TransitionKey {
        State fromState;
        Event event;
        
        bool operator==(const TransitionKey& other) const {
            return fromState == other.fromState && event == other.event;
        }
    };
    
    // Hash function for TransitionKey
    struct TransitionKeyHash {
        std::size_t operator()(const TransitionKey& key) const {
            return std::hash<int>{}(static_cast<int>(key.fromState)) ^ 
                   std::hash<int>{}(static_cast<int>(key.event));
        }
    };
    
    // Structure for state transition
    struct Transition {
        State targetState;
        std::function<void(AgentContext&, const std::string&)> handler;
    };
    
    // Setup default state transitions
    void setupDefaultTransitions() {
        // Initialize transitions
        transitions_[{State::UNINITIALIZED, Event::INITIALIZE}] = 
            {State::INITIALIZING, nullptr};
            
        transitions_[{State::INITIALIZING, Event::INITIALIZATION_COMPLETE}] = 
            {State::READY, nullptr};
            
        transitions_[{State::INITIALIZING, Event::INITIALIZATION_FAILED}] = 
            {State::ERROR, nullptr};
            
        transitions_[{State::READY, Event::START}] = 
            {State::RUNNING, nullptr};
            
        transitions_[{State::RUNNING, Event::STOP}] = 
            {State::READY, nullptr};
            
        transitions_[{State::RUNNING, Event::PAUSE}] = 
            {State::PAUSED, nullptr};
            
        transitions_[{State::PAUSED, Event::RESUME}] = 
            {State::RUNNING, nullptr};
            
        transitions_[{State::PAUSED, Event::STOP}] = 
            {State::READY, nullptr};
            
        // Error transitions
        transitions_[{State::RUNNING, Event::ERROR_OCCURRED}] = 
            {State::ERROR, nullptr};
            
        transitions_[{State::ERROR, Event::RECOVERY_COMPLETE}] = 
            {State::READY, nullptr};
            
        // Termination transitions from any state
        transitions_[{State::UNINITIALIZED, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
        transitions_[{State::INITIALIZING, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
        transitions_[{State::READY, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
        transitions_[{State::RUNNING, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
        transitions_[{State::PAUSED, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
        transitions_[{State::ERROR, Event::TERMINATE}] = 
            {State::TERMINATED, nullptr};
    }

    // Map of agent contexts by ID
    std::map<std::string, AgentContext> agents_;
    
    // Map of state transitions
    std::unordered_map<TransitionKey, Transition, TransitionKeyHash> transitions_;
    
    // State entry handlers
    std::map<State, std::function<void(AgentContext&)>> stateEntryHandlers_;
    
    // State exit handlers
    std::map<State, std::function<void(AgentContext&)>> stateExitHandlers_;
};

// AgentLifecycle implementation
AgentLifecycle::AgentLifecycle() : pImpl_(std::make_unique<Impl>()) {}

AgentLifecycle::~AgentLifecycle() = default;

std::string AgentLifecycle::createAgent(
    const std::string& name,
    const std::string& type,
    const std::map<std::string, std::string>& parameters) {
    
    return pImpl_->createAgent(name, type, parameters);
}

bool AgentLifecycle::triggerEvent(
    const std::string& agentId,
    Event event,
    const std::string& eventData) {
    
    return pImpl_->triggerEvent(agentId, event, eventData);
}

AgentLifecycle::State AgentLifecycle::getAgentState(const std::string& agentId) const {
    return pImpl_->getAgentState(agentId);
}

const AgentLifecycle::AgentContext& AgentLifecycle::getAgentContext(
    const std::string& agentId) const {
    
    return pImpl_->getAgentContext(agentId);
}

bool AgentLifecycle::setStateTransitionHandler(
    State fromState,
    Event event,
    State toState,
    std::function<void(AgentContext&, const std::string&)> handler) {
    
    return pImpl_->setStateTransitionHandler(fromState, event, toState, handler);
}

bool AgentLifecycle::setStateEntryHandler(
    State state,
    std::function<void(AgentContext&)> handler) {
    
    return pImpl_->setStateEntryHandler(state, handler);
}

bool AgentLifecycle::setStateExitHandler(
    State state,
    std::function<void(AgentContext&)> handler) {
    
    return pImpl_->setStateExitHandler(state, handler);
}

std::vector<std::string> AgentLifecycle::getAllAgentIds() const {
    return pImpl_->getAllAgentIds();
}

std::vector<std::string> AgentLifecycle::getAgentsInState(State state) const {
    return pImpl_->getAgentsInState(state);
}

AgentLifecycle::AgentLifecycle(const std::string& agentId) 
    : agentId_(agentId), currentState_(AgentState::IDLE) {
    
    // Initialize valid transitions for FSM
    setupValidTransitions();
}

void AgentLifecycle::setupValidTransitions() {
    // Define valid state transitions according to FSM
    validTransitions_[AgentState::IDLE] = {AgentState::INITIALIZING, AgentState::TERMINATING};
    validTransitions_[AgentState::INITIALIZING] = {AgentState::ANALYZING, AgentState::TERMINATING};
    validTransitions_[AgentState::ANALYZING] = {AgentState::PLANNING, AgentState::TERMINATING};
    validTransitions_[AgentState::PLANNING] = {AgentState::EXECUTING, AgentState::ANALYZING, AgentState::TERMINATING};
    validTransitions_[AgentState::EXECUTING] = {AgentState::REFLECTING, AgentState::TERMINATING};
    validTransitions_[AgentState::REFLECTING] = {AgentState::ANALYZING, AgentState::IDLE, AgentState::TERMINATING};
    validTransitions_[AgentState::TERMINATING] = {}; // Terminal state
}

bool AgentLifecycle::transitionTo(AgentState newState) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if transition is valid
    if (!isTransitionValid(currentState_, newState)) {
        return false;
    }
    
    AgentState previousState = currentState_;
    currentState_ = newState;
    
    // Record state change
    StateTransition transition;
    transition.fromState = previousState;
    transition.toState = newState;
    transition.timestamp = std::chrono::system_clock::now();
    stateHistory_.push_back(transition);
    
    // Execute state entry actions
    executeStateEntryActions(newState);
    
    // Notify callbacks
    if (stateChangeCallback_) {
        stateChangeCallback_(agentId_, previousState, newState);
    }
    
    return true;
}

AgentLifecycle::AgentState AgentLifecycle::getCurrentState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentState_;
}

std::string AgentLifecycle::getStateString() const {
    return stateToString(getCurrentState());
}

bool AgentLifecycle::isInTerminalState() const {
    return getCurrentState() == AgentState::TERMINATING;
}

void AgentLifecycle::setStateChangeCallback(StateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    stateChangeCallback_ = callback;
}

std::vector<AgentLifecycle::StateTransition> AgentLifecycle::getStateHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stateHistory_;
}

std::string AgentLifecycle::stateToString(AgentState state) {
    switch (state) {
        case AgentState::IDLE: return "IDLE";
        case AgentState::INITIALIZING: return "INITIALIZING";
        case AgentState::ANALYZING: return "ANALYZING";
        case AgentState::PLANNING: return "PLANNING";
        case AgentState::EXECUTING: return "EXECUTING";
        case AgentState::REFLECTING: return "REFLECTING";
        case AgentState::TERMINATING: return "TERMINATING";
        default: return "UNKNOWN";
    }
}

AgentLifecycle::AgentState AgentLifecycle::stringToState(const std::string& stateStr) {
    if (stateStr == "IDLE") return AgentState::IDLE;
    if (stateStr == "INITIALIZING") return AgentState::INITIALIZING;
    if (stateStr == "ANALYZING") return AgentState::ANALYZING;
    if (stateStr == "PLANNING") return AgentState::PLANNING;
    if (stateStr == "EXECUTING") return AgentState::EXECUTING;
    if (stateStr == "REFLECTING") return AgentState::REFLECTING;
    if (stateStr == "TERMINATING") return AgentState::TERMINATING;
    
    throw std::invalid_argument("Invalid state string: " + stateStr);
}

bool AgentLifecycle::isTransitionValid(AgentState from, AgentState to) const {
    auto it = validTransitions_.find(from);
    if (it == validTransitions_.end()) {
        return false;
    }
    
    const auto& validStates = it->second;
    return std::find(validStates.begin(), validStates.end(), to) != validStates.end();
}

void AgentLifecycle::executeStateEntryActions(AgentState state) {
    switch (state) {
        case AgentState::IDLE:
            // Reset any temporary state, prepare for new task
            break;
            
        case AgentState::INITIALIZING:
            // Initialize agent resources, load configuration
            break;
            
        case AgentState::ANALYZING:
            // Begin analysis phase, gather information
            break;
            
        case AgentState::PLANNING:
            // Create execution plans, determine strategies
            break;
            
        case AgentState::EXECUTING:
            // Execute planned actions, monitor progress
            break;
            
        case AgentState::REFLECTING:
            // Evaluate results, learn from execution
            break;
            
        case AgentState::TERMINATING:
            // Clean up resources, finalize state
            break;
    }
}

} // namespace orchestrator
} // namespace dist_prompt