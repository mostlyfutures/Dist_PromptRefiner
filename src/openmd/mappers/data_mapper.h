#pragma once

#include "openmd/errors/error_codes.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace dist_prompt {
namespace openmd {
namespace mappers {

/**
 * @brief Bidirectional data mapper for OpenMD integration
 * 
 * Maps between application data models and OpenMD data structures.
 */
class DataMapper {
public:
    /**
     * @brief Constructor
     */
    DataMapper();
    
    /**
     * @brief Destructor
     */
    ~DataMapper();
    
    /**
     * @brief Initialize the mapper with schema definitions
     * 
     * @param schemaPath Path to schema definition files
     * @return bool True if initialization was successful
     */
    bool initialize(const std::string& schemaPath);
    
    /**
     * @brief Map from application JSON to OpenMD format
     * 
     * @param appData Application data in JSON format
     * @return std::string OpenMD data representation
     * @throws OpenMDException if mapping fails
     */
    std::string mapToOpenMD(const nlohmann::json& appData);
    
    /**
     * @brief Map from OpenMD format to application JSON
     * 
     * @param openMDData OpenMD data representation
     * @return nlohmann::json Application data in JSON format
     * @throws OpenMDException if mapping fails
     */
    nlohmann::json mapFromOpenMD(const std::string& openMDData);
    
    /**
     * @brief Register a custom mapping function
     * 
     * @param typeName Type name to register mapping for
     * @param toOpenMDFn Function to convert from app data to OpenMD
     * @param fromOpenMDFn Function to convert from OpenMD to app data
     * @return bool True if registration was successful
     */
    bool registerCustomMapping(
        const std::string& typeName,
        std::function<std::string(const nlohmann::json&)> toOpenMDFn,
        std::function<nlohmann::json(const std::string&)> fromOpenMDFn);
    
    /**
     * @brief Validate data against schema
     * 
     * @param data Data to validate
     * @param schemaName Name of the schema to validate against
     * @return bool True if validation passes
     */
    bool validateData(const nlohmann::json& data, const std::string& schemaName);
    
    /**
     * @brief Get available schema names
     * 
     * @return std::vector<std::string> List of available schema names
     */
    std::vector<std::string> getAvailableSchemas() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace mappers
} // namespace openmd
} // namespace dist_prompt