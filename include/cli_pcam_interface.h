#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace dist_prompt {
namespace integration {

/**
 * @brief Command processing result structure
 */
struct CommandResult {
    bool success;
    std::string output;
    std::string errorMessage;
    int exitCode;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Command input structure
 */
struct CommandInput {
    std::string command;
    std::vector<std::string> arguments;
    std::map<std::string, std::string> options;
    std::string inputData;
    std::string workingDirectory;
};

/**
 * @brief Progress callback function type
 */
using ProgressCallback = std::function<void(int percentage, const std::string& message)>;

/**
 * @brief CommandProcessor interface for CLI → PCAM integration
 * 
 * This interface defines the contract between the CLI Interface region
 * and the PCAM Core Engine region as specified in the integration flow.
 */
class CommandProcessor {
public:
    virtual ~CommandProcessor() = default;
    
    /**
     * @brief Initialize the command processor
     * 
     * @param configPath Path to configuration file
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::string& configPath) = 0;
    
    /**
     * @brief Process a command from the CLI
     * 
     * @param input Command input structure
     * @return CommandResult Result of command processing
     */
    virtual CommandResult processCommand(const CommandInput& input) = 0;
    
    /**
     * @brief Process a software idea through PCAM pipeline
     * 
     * @param idea Software idea description
     * @param callback Progress callback function
     * @return CommandResult Result with generated artifacts
     */
    virtual CommandResult processIdea(const std::string& idea, 
                                    ProgressCallback callback = nullptr) = 0;
    
    /**
     * @brief Get available commands
     * 
     * @return std::vector<std::string> List of supported commands
     */
    virtual std::vector<std::string> getAvailableCommands() const = 0;
    
    /**
     * @brief Get command help information
     * 
     * @param command Command name
     * @return std::string Help text for the command
     */
    virtual std::string getCommandHelp(const std::string& command) const = 0;
    
    /**
     * @brief Validate input before processing
     * 
     * @param input Command input to validate
     * @return bool True if input is valid
     */
    virtual bool validateInput(const CommandInput& input) const = 0;
    
    /**
     * @brief Set output format
     * 
     * @param format Output format ("json", "yaml", "text")
     * @return bool True if format is supported
     */
    virtual bool setOutputFormat(const std::string& format) = 0;
    
    /**
     * @brief Get processor status
     * 
     * @return std::map<std::string, std::string> Status information
     */
    virtual std::map<std::string, std::string> getStatus() const = 0;
    
    /**
     * @brief Shutdown the processor gracefully
     */
    virtual void shutdown() = 0;
};

/**
 * @brief Factory function to create CommandProcessor instance
 * 
 * @return std::unique_ptr<CommandProcessor> CommandProcessor instance
 */
std::unique_ptr<CommandProcessor> createCommandProcessor();

} // namespace integration
} // namespace dist_prompt