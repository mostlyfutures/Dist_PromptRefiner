#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace dist_prompt {
namespace orchestrator {
namespace communication {

/**
 * @brief gRPC communication protocol with TLS for agent coordination
 * 
 * Implements secure communication between agents using gRPC v1.46.3 with TLS encryption.
 */
class GrpcCommunicationProtocol {
public:
    /**
     * @brief Message structure for agent communication
     */
    struct AgentMessage {
        std::string senderId;
        std::string receiverId;
        std::string messageType;
        std::string payload;
        int64_t timestamp;
        std::string correlationId;
    };
    
    /**
     * @brief Response structure for agent communication
     */
    struct AgentResponse {
        bool success;
        std::string responseData;
        std::string errorMessage;
        int64_t timestamp;
        std::string correlationId;
    };
    
    /**
     * @brief Message handler function type
     */
    using MessageHandler = std::function<AgentResponse(const AgentMessage&)>;

    /**
     * @brief Constructor
     */
    GrpcCommunicationProtocol();
    
    /**
     * @brief Destructor
     */
    ~GrpcCommunicationProtocol();
    
    /**
     * @brief Initialize the gRPC server with TLS
     * 
     * @param serverAddress Address to bind the server (e.g., "0.0.0.0:50051")
     * @param tlsCertPath Path to TLS certificate file
     * @param tlsKeyPath Path to TLS private key file
     * @param caCertPath Path to CA certificate file (optional)
     * @return bool True if initialization was successful
     */
    bool initializeServer(const std::string& serverAddress,
                         const std::string& tlsCertPath,
                         const std::string& tlsKeyPath,
                         const std::string& caCertPath = "");
    
    /**
     * @brief Initialize a gRPC client connection
     * 
     * @param serverAddress Address of the server to connect to
     * @param tlsCertPath Path to client TLS certificate file
     * @param tlsKeyPath Path to client TLS private key file
     * @param caCertPath Path to CA certificate file
     * @return bool True if initialization was successful
     */
    bool initializeClient(const std::string& serverAddress,
                         const std::string& tlsCertPath,
                         const std::string& tlsKeyPath,
                         const std::string& caCertPath);
    
    /**
     * @brief Start the gRPC server
     * 
     * @return bool True if server started successfully
     */
    bool startServer();
    
    /**
     * @brief Stop the gRPC server
     */
    void stopServer();
    
    /**
     * @brief Send a message to another agent
     * 
     * @param message Message to send
     * @return AgentResponse Response from the target agent
     */
    AgentResponse sendMessage(const AgentMessage& message);
    
    /**
     * @brief Send an asynchronous message to another agent
     * 
     * @param message Message to send
     * @param callback Callback function to handle the response
     * @return bool True if message was queued successfully
     */
    bool sendMessageAsync(const AgentMessage& message, 
                         std::function<void(const AgentResponse&)> callback);
    
    /**
     * @brief Register a message handler for a specific message type
     * 
     * @param messageType Type of message to handle
     * @param handler Handler function
     * @return bool True if registration was successful
     */
    bool registerMessageHandler(const std::string& messageType, MessageHandler handler);
    
    /**
     * @brief Broadcast a message to all connected agents
     * 
     * @param message Message to broadcast
     * @return std::vector<AgentResponse> Responses from all agents
     */
    std::vector<AgentResponse> broadcastMessage(const AgentMessage& message);
    
    /**
     * @brief Get connection status
     * 
     * @return bool True if connected
     */
    bool isConnected() const;
    
    /**
     * @brief Get server status
     * 
     * @return bool True if server is running
     */
    bool isServerRunning() const;
    
    /**
     * @brief Set connection timeout in milliseconds
     * 
     * @param timeoutMs Timeout in milliseconds
     */
    void setConnectionTimeout(int timeoutMs);
    
    /**
     * @brief Get number of active connections
     * 
     * @return int Number of active connections
     */
    int getActiveConnectionCount() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace communication
} // namespace orchestrator
} // namespace dist_prompt