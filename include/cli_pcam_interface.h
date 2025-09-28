#pragma once

#include <string>
#include <vector>
#include <functional>

namespace dist_prompt {

/**
 * @brief CommandProcessor class - Interface between CLI and PCAM Core Engine
 * 
 * This class processes commands from the CLI and passes them to the PCAM Core Engine.
 * It serves as the primary integration point between user input and the processing logic.
 */
class CommandProcessor {
public:
    /**
     * @brief Default constructor
     */
    CommandProcessor() = default;
    
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~CommandProcessor() = default;
    
    /**
     * @brief Process a software idea from command line input
     * 
     * @param ideaText The raw text describing the software idea
     * @param options Additional processing options
     * @return bool True if processing was successful, false otherwise
     */
    virtual bool processIdea(const std::string& ideaText, 
                            const std::vector<std::string>& options = {}) = 0;
    
    /**
     * @brief Set a callback for progress updates
     * 
     * @param callback Function to call with progress updates (0-100%)
     */
    virtual void setProgressCallback(std::function<void(int)> callback) = 0;
    
    /**
     * @brief Get the processing result as a string
     * 
     * @param format Output format (e.g., "json", "text")
     * @return std::string The formatted result
     */
    virtual std::string getResult(const std::string& format = "text") const = 0;
};

} // namespace dist_prompt