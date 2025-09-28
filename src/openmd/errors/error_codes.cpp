#include "openmd/errors/error_codes.h"
#include <unordered_map>
#include <string>

namespace dist_prompt {
namespace openmd {
namespace errors {

// String representation of error codes
std::string errorCodeToString(ErrorCode code) {
    static const std::unordered_map<ErrorCode, std::string> codeToString = {
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
    
    auto it = codeToString.find(code);
    return it != codeToString.end() ? it->second : "UNKNOWN_ERROR_CODE_" + std::to_string(static_cast<int>(code));
}

std::string errorCodeDescription(ErrorCode code) {
    static const std::unordered_map<ErrorCode, std::string> codeToDescription = {
        // General errors
        {ErrorCode::SUCCESS, "Operation completed successfully"},
        {ErrorCode::UNKNOWN_ERROR, "An unknown error occurred"},
        {ErrorCode::NOT_IMPLEMENTED, "The requested functionality is not implemented"},
        {ErrorCode::INVALID_ARGUMENT, "An invalid argument was provided"},
        {ErrorCode::TIMEOUT, "The operation timed out"},
        
        // Initialization errors
        {ErrorCode::INITIALIZATION_FAILED, "Failed to initialize OpenMD"},
        {ErrorCode::LIBRARY_NOT_FOUND, "The OpenMD library could not be found"},
        {ErrorCode::INCOMPATIBLE_VERSION, "Incompatible OpenMD library version"},
        {ErrorCode::CONFIGURATION_ERROR, "Invalid OpenMD configuration"},
        
        // API binding errors
        {ErrorCode::BINDING_ERROR, "Failed to bind to OpenMD API"},
        {ErrorCode::FUNCTION_NOT_FOUND, "Required OpenMD function not found"},
        {ErrorCode::TYPE_MISMATCH, "Type mismatch in OpenMD function call"},
        {ErrorCode::MARSHALLING_ERROR, "Failed to marshall data for OpenMD call"},
        
        // Simulation errors
        {ErrorCode::SIMULATION_FAILED, "OpenMD simulation failed"},
        {ErrorCode::CONVERGENCE_ERROR, "Simulation failed to converge"},
        {ErrorCode::NUMERICAL_INSTABILITY, "Numerical instability detected in simulation"},
        {ErrorCode::BOUNDARY_CONDITION_ERROR, "Invalid boundary conditions"},
        
        // Data transformation errors
        {ErrorCode::TRANSFORMATION_ERROR, "Failed to transform data"},
        {ErrorCode::SCHEMA_VALIDATION_ERROR, "Data failed schema validation"},
        {ErrorCode::MAPPING_ERROR, "Failed to map data between formats"},
        {ErrorCode::DATA_CORRUPTION, "Data corruption detected"},
        
        // Resource errors
        {ErrorCode::RESOURCE_ERROR, "Resource allocation or management error"},
        {ErrorCode::OUT_OF_MEMORY, "Out of memory"},
        {ErrorCode::FILE_IO_ERROR, "File I/O error"},
        {ErrorCode::NETWORK_ERROR, "Network communication error"}
    };
    
    auto it = codeToDescription.find(code);
    return it != codeToDescription.end() ? it->second : "Unknown error code: " + std::to_string(static_cast<int>(code));
}

bool isErrorRecoverable(ErrorCode code) {
    static const std::unordered_map<ErrorCode, bool> recoverable = {
        {ErrorCode::SUCCESS, true}, {ErrorCode::INVALID_ARGUMENT, true}, {ErrorCode::TIMEOUT, true},
        {ErrorCode::CONFIGURATION_ERROR, true}, {ErrorCode::MARSHALLING_ERROR, true},
        {ErrorCode::SIMULATION_FAILED, true}, {ErrorCode::CONVERGENCE_ERROR, true},
        {ErrorCode::NUMERICAL_INSTABILITY, true}, {ErrorCode::BOUNDARY_CONDITION_ERROR, true},
        {ErrorCode::TRANSFORMATION_ERROR, true}, {ErrorCode::SCHEMA_VALIDATION_ERROR, true},
        {ErrorCode::MAPPING_ERROR, true}, {ErrorCode::RESOURCE_ERROR, true},
        {ErrorCode::OUT_OF_MEMORY, true}, {ErrorCode::FILE_IO_ERROR, true}, {ErrorCode::NETWORK_ERROR, true}
    };
    
    auto it = recoverable.find(code);
    return it != recoverable.end() ? it->second : false;
}

std::string getErrorCategory(ErrorCode code) {
    int codeValue = static_cast<int>(code);
    
    if (codeValue >= 0 && codeValue < 100) return "General";
    if (codeValue >= 100 && codeValue < 200) return "Initialization";
    if (codeValue >= 200 && codeValue < 300) return "API Binding";
    if (codeValue >= 300 && codeValue < 400) return "Simulation";
    if (codeValue >= 400 && codeValue < 500) return "Data Transformation";
    if (codeValue >= 500 && codeValue < 600) return "Resource";
    
    return "Unknown";
}

} // namespace errors
} // namespace openmd
} // namespace dist_prompt