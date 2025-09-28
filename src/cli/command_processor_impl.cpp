#include "cli/command_processor_impl.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace dist_prompt {
namespace cli {

CommandProcessorImpl::CommandProcessorImpl() 
    : validator_(std::make_unique<validators::IdeaValidator>()),
      jsonFormatter_(std::make_unique<formatters::JsonFormatter>()),
      processingSuccess_(false) {
}

bool CommandProcessorImpl::processIdea(
    const std::string& ideaText, 
    const std::vector<std::string>& options) {
    
    // Validate the idea text
    if (!validator_->validate(ideaText)) {
        // Create error response
        nlohmann::json errorResult;
        errorResult["success"] = false;
        errorResult["errors"] = validator_->getErrors();
        processingResult_ = jsonFormatter_->format(errorResult);
        processingSuccess_ = false;
        return false;
    }
    
    // Report progress (starting)
    if (progressCallback_) {
        progressCallback_(10);
    }
    
    // Create a placeholder for processed idea data
    // In a real implementation, this would call the PCAM Core Engine
    nlohmann::json result;
    result["success"] = true;
    result["idea"] = ideaText;
    result["options"] = options;
    result["processed"] = true;
    result["regions"] = nlohmann::json::array();
    
    // Add some placeholder data for demonstration
    result["regions"].push_back({
        {"name", "CLI Interface"},
        {"description", "Command-line interface for user interaction"},
        {"files", nlohmann::json::array({"cli_parser.h", "cli_parser.cpp"})}
    });
    
    result["regions"].push_back({
        {"name", "Core Logic"},
        {"description", "Core processing logic for the software idea"},
        {"files", nlohmann::json::array({"processor.h", "processor.cpp"})}
    });
    
    // Report progress (completed)
    if (progressCallback_) {
        progressCallback_(100);
    }
    
    processingResult_ = jsonFormatter_->format(result);
    processingSuccess_ = true;
    return true;
}

void CommandProcessorImpl::setProgressCallback(std::function<void(int)> callback) {
    progressCallback_ = callback;
}

std::string CommandProcessorImpl::getResult(const std::string& format) const {
    if (format == "json") {
        return processingResult_;
    } else if (format == "text") {
        // Convert JSON to plain text format
        try {
            nlohmann::json resultJson = nlohmann::json::parse(processingResult_);
            std::string textResult;
            
            if (resultJson.contains("success")) {
                textResult += "Success: " + std::string(resultJson["success"] ? "Yes" : "No") + "\n";
            }
            
            if (resultJson.contains("errors")) {
                textResult += "Errors:\n";
                for (const auto& error : resultJson["errors"]) {
                    textResult += "  - " + error.get<std::string>() + "\n";
                }
            }
            
            if (resultJson.contains("regions")) {
                textResult += "Regions:\n";
                for (const auto& region : resultJson["regions"]) {
                    textResult += "  - " + region["name"].get<std::string>() + ": ";
                    textResult += region["description"].get<std::string>() + "\n";
                    
                    if (region.contains("files")) {
                        textResult += "    Files:\n";
                        for (const auto& file : region["files"]) {
                            textResult += "      - " + file.get<std::string>() + "\n";
                        }
                    }
                }
            }
            
            return textResult;
        } catch (const std::exception&) {
            // Fallback to returning the JSON if parsing fails
            return processingResult_;
        }
    } else {
        return "Unsupported format: " + format;
    }
}

} // namespace cli
} // namespace dist_prompt