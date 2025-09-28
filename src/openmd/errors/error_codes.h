#pragma once

#include <string>
#include <map>
#include <stdexcept>

namespace dist_prompt {
namespace openmd {
namespace errors {

/**
 * @brief Error codes for the OpenMD Integration Layer
 */
enum class ErrorCode {
    // General errors (0-99)
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    NOT_IMPLEMENTED = 2,
    INVALID_ARGUMENT = 3,
    TIMEOUT = 4,
    
    // Initialization errors (100-199)
    INITIALIZATION_FAILED = 100,
    LIBRARY_NOT_FOUND = 101,
    INCOMPATIBLE_VERSION = 102,
    CONFIGURATION_ERROR = 103,
    
    // API binding errors (200-299)
    BINDING_ERROR = 200,
    FUNCTION_NOT_FOUND = 201,
    TYPE_MISMATCH = 202,
    MARSHALLING_ERROR = 203,
    
    // Simulation errors (300-399)
    SIMULATION_FAILED = 300,
    CONVERGENCE_ERROR = 301,
    NUMERICAL_INSTABILITY = 302,
    BOUNDARY_CONDITION_ERROR = 303,
    
    // Data transformation errors (400-499)
    TRANSFORMATION_ERROR = 400,
    SCHEMA_VALIDATION_ERROR = 401,
    MAPPING_ERROR = 402,
    DATA_CORRUPTION = 403,
    
    // Resource errors (500-599)
    RESOURCE_ERROR = 500,
    OUT_OF_MEMORY = 501,
    FILE_IO_ERROR = 502,
    NETWORK_ERROR = 503
};

/**
 * @brief OpenMD exception class
 * 
 * Exception thrown for errors in the OpenMD Integration Layer.
 */
class OpenMDException : public std::runtime_error {
public:
    /**
     * @brief Constructor
     * 
     * @param code Error code
     * @param message Error message
     * @param details Additional error details
     */
    OpenMDException(ErrorCode code, const std::string& message, const std::string& details = "")
        : std::runtime_error(message), code_(code), details_(details) {}
    
    /**
     * @brief Get the error code
     * 
     * @return ErrorCode The error code
     */
    ErrorCode getCode() const { return code_; }
    
    /**
     * @brief Get the error details
     * 
     * @return std::string The error details
     */
    const std::string& getDetails() const { return details_; }

private:
    ErrorCode code_;
    std::string details_;
};

/**
 * @brief Get a string representation of an error code
 * 
 * @param code Error code
 * @return std::string String representation
 */
std::string errorCodeToString(ErrorCode code);

/**
 * @brief Get a description for an error code
 * 
 * @param code Error code
 * @return std::string Error description
 */
std::string errorCodeDescription(ErrorCode code);

/**
 * @brief Check if an error code represents a recoverable error
 * 
 * @param code Error code
 * @return bool True if the error is recoverable
 */
bool isErrorRecoverable(ErrorCode code);

/**
 * @brief Get the error category for an error code
 * 
 * @param code Error code
 * @return std::string Error category
 */
std::string getErrorCategory(ErrorCode code);

} // namespace errors
} // namespace openmd
} // namespace dist_prompt