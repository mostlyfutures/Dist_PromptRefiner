#include "openmd/mappers/data_mapper.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace dist_prompt {
namespace openmd {
namespace mappers {

// Private implementation class (PIMPL idiom)
class DataMapper::Impl {
public:
    // Mapping function types
    using ToOpenMDFn = std::function<std::string(const nlohmann::json&)>;
    using FromOpenMDFn = std::function<nlohmann::json(const std::string&)>;
    
    // Structure for custom type mapping
    struct TypeMapping {
        std::string typeName;
        ToOpenMDFn toOpenMDFn;
        FromOpenMDFn fromOpenMDFn;
    };

    Impl() = default;
    ~Impl() = default;
    
    bool loadSchemas(const std::string& schemaPath) {
        if (!fs::exists(schemaPath) || !fs::is_directory(schemaPath)) {
            return false;
        }
        
        schemas_.clear();
        
        try {
            for (const auto& entry : fs::directory_iterator(schemaPath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                    std::string schemaName = entry.path().stem().string();
                    
                    // Load schema content
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        nlohmann::json schema;
                        file >> schema;
                        schemas_[schemaName] = schema;
                    }
                }
            }
            
            return !schemas_.empty();
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    std::string convertToOpenMD(const nlohmann::json& appData) {
        // Check if we have a registered type mapping for this data
        if (appData.contains("type")) {
            std::string typeName = appData["type"].get<std::string>();
            auto it = typeMappings_.find(typeName);
            if (it != typeMappings_.end()) {
                return it->second.toOpenMDFn(appData);
            }
        }
        
        // Default conversion (direct JSON serialization)
        return appData.dump();
    }
    
    nlohmann::json convertFromOpenMD(const std::string& openMDData) {
        try {
            // First try to parse as JSON
            nlohmann::json parsed = nlohmann::json::parse(openMDData);
            
            // Check if we have a registered type mapping for this data
            if (parsed.contains("type")) {
                std::string typeName = parsed["type"].get<std::string>();
                auto it = typeMappings_.find(typeName);
                if (it != typeMappings_.end()) {
                    return it->second.fromOpenMDFn(openMDData);
                }
            }
            
            return parsed;
        } catch (const nlohmann::json::parse_error&) {
            // Not valid JSON, use type-specific mapping based on content patterns
            for (const auto& mapping : typeMappings_) {
                try {
                    nlohmann::json result = mapping.second.fromOpenMDFn(openMDData);
                    if (!result.is_null()) {
                        return result;
                    }
                } catch (...) {
                    // Try the next mapping
                }
            }
            
            // If no mapping worked, wrap the raw data
            nlohmann::json wrapper;
            wrapper["rawData"] = openMDData;
            return wrapper;
        }
    }
    
    bool addCustomMapping(
        const std::string& typeName,
        ToOpenMDFn toOpenMDFn,
        FromOpenMDFn fromOpenMDFn) {
        
        if (typeName.empty() || !toOpenMDFn || !fromOpenMDFn) {
            return false;
        }
        
        TypeMapping mapping{typeName, toOpenMDFn, fromOpenMDFn};
        typeMappings_[typeName] = mapping;
        return true;
    }
    
    bool validateAgainstSchema(const nlohmann::json& data, const std::string& schemaName) {
        auto it = schemas_.find(schemaName);
        if (it == schemas_.end()) {
            throw errors::OpenMDException(
                errors::ErrorCode::SCHEMA_VALIDATION_ERROR,
                "Schema not found: " + schemaName,
                "Available schemas: " + getAvailableSchemasString()
            );
        }
        
        // In a real implementation, this would use a JSON Schema validator library
        // For this example, we'll do some basic validation
        const nlohmann::json& schema = it->second;
        
        if (schema.contains("required") && schema["required"].is_array()) {
            for (const auto& required : schema["required"]) {
                if (required.is_string() && !data.contains(required.get<std::string>())) {
                    throw errors::OpenMDException(
                        errors::ErrorCode::SCHEMA_VALIDATION_ERROR,
                        "Missing required property: " + required.get<std::string>(),
                        "Validation failed for schema: " + schemaName
                    );
                }
            }
        }
        
        // In a real implementation, would validate types, constraints, etc.
        
        return true;
    }
    
    std::vector<std::string> getAvailableSchemaNames() const {
        std::vector<std::string> names;
        for (const auto& [name, _] : schemas_) {
            names.push_back(name);
        }
        return names;
    }
    
    std::string getAvailableSchemasString() const {
        std::string result;
        for (const auto& [name, _] : schemas_) {
            if (!result.empty()) {
                result += ", ";
            }
            result += name;
        }
        return result;
    }

private:
    std::map<std::string, nlohmann::json> schemas_;
    std::map<std::string, TypeMapping> typeMappings_;
};

// DataMapper implementation

DataMapper::DataMapper() : pImpl_(std::make_unique<Impl>()) {}

DataMapper::~DataMapper() = default;

bool DataMapper::initialize(const std::string& schemaPath) {
    return pImpl_->loadSchemas(schemaPath);
}

std::string DataMapper::mapToOpenMD(const nlohmann::json& appData) {
    try {
        return pImpl_->convertToOpenMD(appData);
    } catch (const std::exception& e) {
        throw errors::OpenMDException(
            errors::ErrorCode::MAPPING_ERROR,
            "Error mapping to OpenMD format",
            e.what()
        );
    }
}

nlohmann::json DataMapper::mapFromOpenMD(const std::string& openMDData) {
    try {
        return pImpl_->convertFromOpenMD(openMDData);
    } catch (const std::exception& e) {
        throw errors::OpenMDException(
            errors::ErrorCode::MAPPING_ERROR,
            "Error mapping from OpenMD format",
            e.what()
        );
    }
}

bool DataMapper::registerCustomMapping(
    const std::string& typeName,
    std::function<std::string(const nlohmann::json&)> toOpenMDFn,
    std::function<nlohmann::json(const std::string&)> fromOpenMDFn) {
    
    return pImpl_->addCustomMapping(typeName, toOpenMDFn, fromOpenMDFn);
}

bool DataMapper::validateData(const nlohmann::json& data, const std::string& schemaName) {
    return pImpl_->validateAgainstSchema(data, schemaName);
}

std::vector<std::string> DataMapper::getAvailableSchemas() const {
    return pImpl_->getAvailableSchemaNames();
}

} // namespace mappers
} // namespace openmd
} // namespace dist_prompt