#include "openmd/errors/error_codes.h"
#include <unordered_map>

namespace dist_prompt {
namespace openmd {
namespace errors {

// String representation of error codes
std::string errorCodeToString(ErrorCode code) {
    static const std::unordered_map<ErrorCode, std::string> errorStrings = {
        // General errors
        {ErrorCode::SUCCESS, "SUCCESS"},
        {ErrorCode::UNKNOWN_ERROR, "UNKNOWN_ERROR"},
        {ErrorCode::NOT_IMPLEMENTED, "NOT_IMPLEMENTED"},
        {ErrorCode::INVALID_ARGUMENT, "INVALID_ARGUMENT"},
        {ErrorCode::TIMEOUT, "TIMEOUT"},
        
        // Initialization errors
        {ErrorCode::INITIALIZATION_FAILED, "INITIALIZATION_FAILED"},
        {ErrorCode::LIBRARY_NOT_FOUND, "LIBRARY_NOT_FOUND"},
        {ErrorCode::INCOMPATIBLE_VERSION, "INCOMPATIBLE_VERSION"},
        {ErrorCode::CONFIGURATION_ERROR, "CONFIGURATION_ERROR"},
        
        // API binding errors
        {ErrorCode::BINDING_ERROR, "BINDING_ERROR"},
        {ErrorCode::FUNCTION_NOT_FOUND, "FUNCTION_NOT_FOUND"},
        {ErrorCode::TYPE_MISMATCH, "TYPE_MISMATCH"},
        {ErrorCode::MARSHALLING_ERROR, "MARSHALLING_ERROR"},
        
        // Simulation errors
        {ErrorCode::SIMULATION_FAILED, "SIMULATION_FAILED"},
        {ErrorCode::CONVERGENCE_ERROR, "CONVERGENCE_ERROR"},
        {ErrorCode::NUMERICAL_INSTABILITY, "NUMERICAL_INSTABILITY"},
        {ErrorCode::BOUNDARY_CONDITION_ERROR, "BOUNDARY_CONDITION_ERROR"},
        
        // Data transformation errors
        {ErrorCode::TRANSFORMATION_ERROR, "TRANSFORMATION_ERROR"},
        {ErrorCode::SCHEMA_VALIDATION_ERROR, "SCHEMA_VALIDATION_ERROR"},
        {ErrorCode::MAPPING_ERROR, "MAPPING_ERROR"},
        {ErrorCode::DATA_CORRUPTION, "DATA_CORRUPTION"},
        
        // Resource errors
        {ErrorCode::RESOURCE_ERROR, "RESOURCE_ERROR"},
        {ErrorCode::OUT_OF_MEMORY, "OUT_OF_MEMORY"},
        {ErrorCode::FILE_IO_ERROR, "FILE_IO_ERROR"},
        {ErrorCode::NETWORK_ERROR, "NETWORK_ERROR"}
    };
    
    auto it = errorStrings.find(code);
    if (it != errorStrings.end()) {
        return it->second;
    }
    
    return "UNDEFINED_ERROR_CODE";
}

// Error code descriptions
std::string errorCodeDescription(ErrorCode code) {
    static const std::unordered_map<ErrorCode, std::string> errorDescriptions = {
        // General errors
        {ErrorCode::SUCCESS, "Operation completed successfully"},
        {ErrorCode::UNKNOWN_ERROR, "An unknown error occurred"},
        {ErrorCode::NOT_IMPLEMENTED, "The requested functionality is not implemented"},
        {ErrorCode::INVALID_ARGUMENT, "Invalid argument provided to function"},
        {ErrorCode::TIMEOUT, "Operation timed out"},
        
        // Initialization errors
        {ErrorCode::INITIALIZATION_FAILED, "Failed to initialize OpenMD"},
        {ErrorCode::LIBRARY_NOT_FOUND, "OpenMD library could not be found or loaded"},
        {ErrorCode::INCOMPATIBLE_VERSION, "Incompatible version of OpenMD"},
        {ErrorCode::CONFIGURATION_ERROR, "Error in OpenMD configuration"},
        
        // API binding errors
        {ErrorCode::BINDING_ERROR, "Error in OpenMD API binding"},
        {ErrorCode::FUNCTION_NOT_FOUND, "OpenMD function not found"},
        {ErrorCode::TYPE_MISMATCH, "Type mismatch in OpenMD API call"},
        {ErrorCode::MARSHALLING_ERROR, "Error marshalling data for OpenMD API"},
        
        // Simulation errors
        {ErrorCode::SIMULATION_FAILED, "OpenMD simulation failed"},
        {ErrorCode::CONVERGENCE_ERROR, "Simulation failed to converge"},
        {ErrorCode::NUMERICAL_INSTABILITY, "Numerical instability in simulation"},
        {ErrorCode::BOUNDARY_CONDITION_ERROR, "Error in boundary conditions"},
        
        // Data transformation errors
        {ErrorCode::TRANSFORMATION_ERROR, "Error transforming data"},
        {ErrorCode::SCHEMA_VALIDATION_ERROR, "Data failed schema validation"},
        {ErrorCode::MAPPING_ERROR, "Error mapping between data formats"},
        {ErrorCode::DATA_CORRUPTION, "Data corruption detected"},
        
        // Resource errors
        {ErrorCode::RESOURCE_ERROR, "Resource allocation error"},
        {ErrorCode::OUT_OF_MEMORY, "Out of memory"},
        {ErrorCode::FILE_IO_ERROR, "File I/O error"},
        {ErrorCode::NETWORK_ERROR, "Network communication error"}
    };
    
    auto it = errorDescriptions.find(code);
    if (it != errorDescriptions.end()) {
        return it->second;
    }
    
    return "No description available for error code";
}

// Check if an error is recoverable
bool isErrorRecoverable(ErrorCode code) {
    // Define which error codes are considered recoverable
    static const std::unordered_map<ErrorCode, bool> recoverableErrors = {
        // Some errors are recoverable
        {ErrorCode::TIMEOUT, true},
        {ErrorCode::CONVERGENCE_ERROR, true},
        {ErrorCode::NUMERICAL_INSTABILITY, true}
        // All other errors are non-recoverable by default
    };
    
    auto it = recoverableErrors.find(code);
    if (it != recoverableErrors.end()) {
        return it->second;
    }
    
    // By default, errors are not recoverable
    return false;
}

// Get the category for an error code
std::string getErrorCategory(ErrorCode code) {
    int codeValue = static_cast<int>(code);
    
    if (codeValue >= 0 && codeValue < 100) {
        return "General";
    } else if (codeValue >= 100 && codeValue < 200) {
        return "Initialization";
    } else if (codeValue >= 200 && codeValue < 300) {
        return "API Binding";
    } else if (codeValue >= 300 && codeValue < 400) {
        return "Simulation";
    } else if (codeValue >= 400 && codeValue < 500) {
        return "Data Transformation";
    } else if (codeValue >= 500 && codeValue < 600) {
        return "Resource";
    }
    
    return "Unknown";
}

} // namespace errors
} // namespace openmd
} // namespace dist_prompt