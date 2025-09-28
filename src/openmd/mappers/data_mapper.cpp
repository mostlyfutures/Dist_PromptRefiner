#include "openmd/mappers/data_mapper.h"
#include "openmd/errors/error_codes.h"
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <fstream>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

namespace dist_prompt {
namespace openmd {
namespace mappers {

// Private implementation class (PIMPL idiom)
class DataMapper::Impl {
public:
    Impl() = default;
    ~Impl() = default;
    
    bool loadSchemas(const std::string& schemaPath) {
        if (!fs::exists(schemaPath) || !fs::is_directory(schemaPath)) {
            return false;
        }
        
        try {
            for (const auto& entry : fs::directory_iterator(schemaPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::string filename = entry.path().filename().string();
                    std::string schemaName = filename.substr(0, filename.find_first_of('.'));
                    
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        nlohmann::json schemaJson;
                        file >> schemaJson;
                        
                        // Store the schema
                        schemas_[schemaName] = schemaJson;
                    }
                }
            }
            
            // Also look for mapping rules
            std::string mappingRulesPath = fs::path(schemaPath) / "mapping_rules.json";
            if (fs::exists(mappingRulesPath)) {
                std::ifstream file(mappingRulesPath);
                if (file.is_open()) {
                    file >> mappingRules_;
                }
            }
            
            return !schemas_.empty();
        } catch (const std::exception& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::SCHEMA_VALIDATION_ERROR,
                "Failed to load schemas: " + std::string(e.what()),
                "Path: " + schemaPath
            );
        }
    }
    
    std::string convertToOpenMD(const nlohmann::json& appData) {
        try {
            // Validate against app schema if available
            if (schemas_.find("app") != schemas_.end()) {
                nlohmann::json_schema::json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
                validator.set_root_schema(schemas_["app"]);
                validator.validate(appData);
            }
            
            // Apply mapping rules
            nlohmann::json openMDJson;
            if (!mappingRules_.is_null() && mappingRules_.contains("toOpenMD")) {
                applyMappingRules(appData, openMDJson, mappingRules_["toOpenMD"]);
            } else {
                // Default direct mapping if no rules
                openMDJson = appData;
            }
            
            // Validate against OpenMD schema if available
            if (schemas_.find("openmd") != schemas_.end()) {
                nlohmann::json_schema::json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
                validator.set_root_schema(schemas_["openmd"]);
                validator.validate(openMDJson);
            }
            
            // Apply any custom type mappings
            for (const auto& [typeName, mapping] : customMappings_) {
                if (openMDJson.contains("type") && openMDJson["type"] == typeName) {
                    return mapping.toOpenMDFn(openMDJson);
                }
            }
            
            // Default serialization
            return openMDJson.dump();
        } catch (const nlohmann::json_schema::validation_error& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::SCHEMA_VALIDATION_ERROR,
                "JSON schema validation error: " + std::string(e.what())
            );
        } catch (const nlohmann::json::exception& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::MAPPING_ERROR,
                "JSON mapping error: " + std::string(e.what())
            );
        } catch (const std::exception& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::TRANSFORMATION_ERROR,
                "Transformation to OpenMD failed: " + std::string(e.what())
            );
        }
    }
    
    nlohmann::json convertFromOpenMD(const std::string& openMDData) {
        try {
            // Parse OpenMD data as JSON
            nlohmann::json openMDJson;
            try {
                openMDJson = nlohmann::json::parse(openMDData);
            } catch (...) {
                // Handle non-JSON OpenMD data
                // Check if any custom type handlers can parse it
                for (const auto& [typeName, mapping] : customMappings_) {
                    try {
                        return mapping.fromOpenMDFn(openMDData);
                    } catch (...) {
                        // Try next mapping
                    }
                }
                
                // If we get here, no custom mapping could handle it
                throw errors::OpenMDException(
                    errors::ErrorCode::MAPPING_ERROR,
                    "Cannot parse OpenMD data as JSON and no custom mapping can handle it"
                );
            }
            
            // Validate against OpenMD schema if available
            if (schemas_.find("openmd") != schemas_.end()) {
                nlohmann::json_schema::json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
                validator.set_root_schema(schemas_["openmd"]);
                validator.validate(openMDJson);
            }
            
            // Apply any custom type mappings
            for (const auto& [typeName, mapping] : customMappings_) {
                if (openMDJson.contains("type") && openMDJson["type"] == typeName) {
                    return mapping.fromOpenMDFn(openMDData);
                }
            }
            
            // Apply mapping rules
            nlohmann::json appData;
            if (!mappingRules_.is_null() && mappingRules_.contains("fromOpenMD")) {
                applyMappingRules(openMDJson, appData, mappingRules_["fromOpenMD"]);
            } else {
                // Default direct mapping if no rules
                appData = openMDJson;
            }
            
            // Validate against app schema if available
            if (schemas_.find("app") != schemas_.end()) {
                nlohmann::json_schema::json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
                validator.set_root_schema(schemas_["app"]);
                validator.validate(appData);
            }
            
            return appData;
        } catch (const nlohmann::json_schema::validation_error& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::SCHEMA_VALIDATION_ERROR,
                "JSON schema validation error: " + std::string(e.what())
            );
        } catch (const nlohmann::json::exception& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::MAPPING_ERROR,
                "JSON mapping error: " + std::string(e.what())
            );
        } catch (const errors::OpenMDException&) {
            // Re-throw OpenMD exceptions directly
            throw;
        } catch (const std::exception& e) {
            throw errors::OpenMDException(
                errors::ErrorCode::TRANSFORMATION_ERROR,
                "Transformation from OpenMD failed: " + std::string(e.what())
            );
        }
    }
    
    bool addCustomMapping(
        const std::string& typeName,
        std::function<std::string(const nlohmann::json&)> toOpenMDFn,
        std::function<nlohmann::json(const std::string&)> fromOpenMDFn) {
        
        if (typeName.empty() || !toOpenMDFn || !fromOpenMDFn) {
            return false;
        }
        
        CustomMapping mapping;
        mapping.toOpenMDFn = toOpenMDFn;
        mapping.fromOpenMDFn = fromOpenMDFn;
        
        customMappings_[typeName] = mapping;
        return true;
    }
    
    bool validate(const nlohmann::json& data, const std::string& schemaName) {
        auto it = schemas_.find(schemaName);
        if (it == schemas_.end()) {
            return false;
        }
        
        try {
            nlohmann::json_schema::json_validator validator(nullptr, nlohmann::json_schema::default_string_format_check);
            validator.set_root_schema(it->second);
            validator.validate(data);
            return true;
        } catch (...) {
            return false;
        }
    }
    
    std::vector<std::string> getSchemaNames() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : schemas_) {
            names.push_back(name);
        }
        return names;
    }
    
private:
    struct CustomMapping {
        std::function<std::string(const nlohmann::json&)> toOpenMDFn;
        std::function<nlohmann::json(const std::string&)> fromOpenMDFn;
    };
    
    std::unordered_map<std::string, nlohmann::json> schemas_;
    std::unordered_map<std::string, CustomMapping> customMappings_;
    nlohmann::json mappingRules_;
    
    void applyMappingRules(const nlohmann::json& input, 
                          nlohmann::json& output,
                          const nlohmann::json& rules) {
        // Apply field mappings
        if (rules.contains("fields") && rules["fields"].is_object()) {
            for (auto it = rules["fields"].begin(); it != rules["fields"].end(); ++it) {
                const std::string& targetField = it.key();
                const auto& sourceRule = it.value();
                
                if (sourceRule.is_string()) {
                    // Direct field mapping
                    const std::string& sourceField = sourceRule.get<std::string>();
                    if (input.contains(sourceField)) {
                        output[targetField] = input[sourceField];
                    }
                } else if (sourceRule.is_object() && sourceRule.contains("path")) {
                    // JSON path mapping
                    const std::string& path = sourceRule["path"].get<std::string>();
                    nlohmann::json value = input;
                    
                    // Navigate path segments
                    std::istringstream pathStream(path);
                    std::string segment;
                    
                    while (std::getline(pathStream, segment, '.')) {
                        if (value.contains(segment)) {
                            value = value[segment];
                        } else {
                            value = nullptr;
                            break;
                        }
                    }
                    
                    if (!value.is_null()) {
                        // Apply any transformation
                        if (sourceRule.contains("transform") && sourceRule["transform"].is_string()) {
                            const std::string& transform = sourceRule["transform"].get<std::string>();
                            
                            if (transform == "toString") {
                                output[targetField] = value.dump();
                            } else if (transform == "toNumber" && value.is_string()) {
                                output[targetField] = std::stod(value.get<std::string>());
                            } else if (transform == "toBoolean") {
                                if (value.is_string()) {
                                    std::string strValue = value.get<std::string>();
                                    output[targetField] = (strValue == "true" || strValue == "1");
                                } else {
                                    output[targetField] = value.get<bool>();
                                }
                            } else {
                                // No transform or unknown transform
                                output[targetField] = value;
                            }
                        } else {
                            // No transform
                            output[targetField] = value;
                        }
                    }
                } else if (sourceRule.is_object() && sourceRule.contains("value")) {
                    // Constant value
                    output[targetField] = sourceRule["value"];
                }
            }
        }
        
        // Apply template if specified
        if (rules.contains("template") && rules["template"].is_string()) {
            const std::string& templateName = rules["template"].get<std::string>();
            auto templateIt = schemas_.find("template." + templateName);
            
            if (templateIt != schemas_.end()) {
                // Start with template as base
                nlohmann::json templateJson = templateIt->second;
                
                // Override with mapped fields
                for (auto it = output.begin(); it != output.end(); ++it) {
                    templateJson[it.key()] = it.value();
                }
                
                output = templateJson;
            }
        }
        
        // Apply type if specified
        if (rules.contains("type") && rules["type"].is_string()) {
            output["type"] = rules["type"].get<std::string>();
        }
    }
};

// DataMapper implementation

DataMapper::DataMapper() : pImpl_(std::make_unique<Impl>()) {}

DataMapper::~DataMapper() = default;

bool DataMapper::initialize(const std::string& schemaPath) {
    return pImpl_->loadSchemas(schemaPath);
}

std::string DataMapper::mapToOpenMD(const nlohmann::json& appData) {
    return pImpl_->convertToOpenMD(appData);
}

nlohmann::json DataMapper::mapFromOpenMD(const std::string& openMDData) {
    return pImpl_->convertFromOpenMD(openMDData);
}

bool DataMapper::registerCustomMapping(
    const std::string& typeName,
    std::function<std::string(const nlohmann::json&)> toOpenMDFn,
    std::function<nlohmann::json(const std::string&)> fromOpenMDFn) {
    
    return pImpl_->addCustomMapping(typeName, toOpenMDFn, fromOpenMDFn);
}

bool DataMapper::validateData(const nlohmann::json& data, const std::string& schemaName) {
    return pImpl_->validate(data, schemaName);
}

std::vector<std::string> DataMapper::getAvailableSchemas() const {
    return pImpl_->getSchemaNames();
}

} // namespace mappers
} // namespace openmd
} // namespace dist_prompt