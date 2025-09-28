#pragma once

#include "cli_pcam_interface.h"
#include "cli/cli_parser.h"
#include "cli/validators/idea_validator.h"
#include "cli/formatters/json_formatter.h"
#include <memory>
#include <string>
#include <vector>

namespace dist_prompt {
namespace cli {

/**
 * @brief Implementation of CommandProcessor interface
 * 
 * Connects the CLI Interface with the PCAM Core Engine.
 */
class CommandProcessorImpl : public CommandProcessor {
public:
    CommandProcessorImpl();
    ~CommandProcessorImpl() override = default;
    
    /**
     * @brief Process a software idea from command line input
     * 
     * @param ideaText The raw text describing the software idea
     * @param options Additional processing options
     * @return bool True if processing was successful, false otherwise
     */
    bool processIdea(const std::string& ideaText, 
                    const std::vector<std::string>& options = {}) override;
    
    /**
     * @brief Set a callback for progress updates
     * 
     * @param callback Function to call with progress updates (0-100%)
     */
    void setProgressCallback(std::function<void(int)> callback) override;
    
    /**
     * @brief Get the processing result as a string
     * 
     * @param format Output format (e.g., "json", "text")
     * @return std::string The formatted result
     */
    std::string getResult(const std::string& format = "text") const override;
    
private:
    std::unique_ptr<validators::IdeaValidator> validator_;
    std::unique_ptr<formatters::JsonFormatter> jsonFormatter_;
    std::function<void(int)> progressCallback_;
    std::string processingResult_;
    bool processingSuccess_;
};

} // namespace cli
} // namespace dist_prompt