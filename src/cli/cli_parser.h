#pragma once

#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

namespace dist_prompt {
namespace cli {

/**
 * @brief Command-line parser implementation using Boost.Program_options v1.78.0
 */
class CliParser {
public:
    CliParser();
    ~CliParser() = default;
    
    /**
     * @brief Parse command line arguments
     * 
     * @param argc Argument count
     * @param argv Argument values
     * @return bool True if parsing was successful
     */
    bool parse(int argc, char** argv);
    
    /**
     * @brief Get the parsed idea text
     * 
     * @return std::string The idea text
     */
    std::string getIdeaText() const;
    
    /**
     * @brief Get additional options
     * 
     * @return std::vector<std::string> List of additional options
     */
    std::vector<std::string> getOptions() const;
    
    /**
     * @brief Get the output format
     * 
     * @return std::string Output format (json, text)
     */
    std::string getOutputFormat() const;
    
private:
    boost::program_options::options_description desc_;
    boost::program_options::variables_map vm_;
    std::string ideaText_;
    std::vector<std::string> options_;
    std::string outputFormat_;
};

} // namespace cli
} // namespace dist_prompt