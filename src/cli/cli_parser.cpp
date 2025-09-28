#include "cli/cli_parser.h"
#include <iostream>
#include <fstream>

namespace po = boost::program_options;

namespace dist_prompt {
namespace cli {

CliParser::CliParser() : desc_("Allowed options") {
    // Setup command line options
    desc_.add_options()
        ("help,h", "Display this help message")
        ("idea,i", po::value<std::string>(&ideaText_), "Software idea text to process")
        ("idea-file,f", po::value<std::string>(), "File containing software idea text")
        ("output-format,o", po::value<std::string>(&outputFormat_)->default_value("text"), 
         "Output format (json, text)")
        ("verbose,v", "Enable verbose output")
        ("debug,d", "Enable debug mode")
        ("no-color", "Disable colored output")
        ("config", po::value<std::string>(), "Configuration file path");
}

bool CliParser::parse(int argc, char** argv) {
    try {
        po::store(po::parse_command_line(argc, argv, desc_), vm_);
        po::notify(vm_);
        
        // Handle --help option
        if (vm_.count("help")) {
            std::cout << desc_ << std::endl;
            return false;
        }
        
        // Handle --idea-file option
        if (vm_.count("idea-file")) {
            std::string filename = vm_["idea-file"].as<std::string>();
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return false;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            ideaText_ = buffer.str();
        }
        
        // Collect additional options
        if (vm_.count("verbose")) options_.push_back("verbose");
        if (vm_.count("debug")) options_.push_back("debug");
        if (vm_.count("no-color")) options_.push_back("no-color");
        
        // Validate that we have idea text from some source
        if (ideaText_.empty() && !vm_.count("idea-file")) {
            std::cerr << "Error: No idea text provided. Use --idea or --idea-file option." << std::endl;
            std::cout << desc_ << std::endl;
            return false;
        }
        
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

std::string CliParser::getIdeaText() const {
    return ideaText_;
}

std::vector<std::string> CliParser::getOptions() const {
    return options_;
}

std::string CliParser::getOutputFormat() const {
    return outputFormat_;
}

} // namespace cli
} // namespace dist_prompt