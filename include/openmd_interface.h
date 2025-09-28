#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

namespace dist_prompt {
namespace integration {

/**
 * @brief Simulation configuration structure
 */
struct SimulationConfig {
    std::string configFile;
    std::map<std::string, std::string> parameters;
    std::string workingDirectory;
    int numProcessors;
    std::chrono::milliseconds timeout;
    std::string logLevel;
};

/**
 * @brief Simulation result structure
 */
struct SimulationResult {
    bool success;
    std::string simulationId;
    std::map<std::string, double> metrics;
    std::vector<std::string> outputFiles;
    std::string logOutput;
    std::string errorMessage;
    std::chrono::milliseconds executionTime;
};

/**
 * @brief Parallel job structure
 */
struct ParallelJob {
    std::string jobId;
    std::string jobType;
    SimulationConfig config;
    int priority;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Job status enumeration
 */
enum class JobStatus {
    QUEUED,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED,
    TIMEOUT
};

/**
 * @brief Progress callback for simulation monitoring
 */
using SimulationCallback = std::function<void(const std::string& jobId, 
                                            JobStatus status, 
                                            double progress,
                                            const std::string& message)>;

/**
 * @brief SimulationConnector interface for PCAM → OpenMD integration
 * 
 * This interface defines SWIG-generated FFI with version checking and
 * bidirectional data mappers as specified in the integration flow.
 */
class SimulationConnector {
public:
    virtual ~SimulationConnector() = default;
    
    /**
     * @brief Initialize connection to OpenMD
     * 
     * @param openmdPath Path to OpenMD installation
     * @param version Expected OpenMD version
     * @return bool True if initialization was successful
     */
    virtual bool initialize(const std::string& openmdPath, 
                           const std::string& version) = 0;
    
    /**
     * @brief Verify OpenMD version compatibility
     * 
     * @param requiredVersion Minimum required version
     * @return bool True if version is compatible
     */
    virtual bool verifyVersion(const std::string& requiredVersion) = 0;
    
    /**
     * @brief Submit simulation job to OpenMD
     * 
     * @param job Job configuration and parameters
     * @param callback Progress monitoring callback
     * @return std::string Job ID for tracking
     */
    virtual std::string submitJob(const ParallelJob& job,
                                 SimulationCallback callback = nullptr) = 0;
    
    /**
     * @brief Get job status
     * 
     * @param jobId Job identifier
     * @return JobStatus Current status of the job
     */
    virtual JobStatus getJobStatus(const std::string& jobId) = 0;
    
    /**
     * @brief Get simulation results
     * 
     * @param jobId Job identifier
     * @return SimulationResult Results and output data
     */
    virtual SimulationResult getResults(const std::string& jobId) = 0;
    
    /**
     * @brief Cancel running job
     * 
     * @param jobId Job identifier
     * @return bool True if job was cancelled successfully
     */
    virtual bool cancelJob(const std::string& jobId) = 0;
    
    /**
     * @brief Transform data to OpenMD format
     * 
     * @param inputData Data in source format
     * @param sourceFormat Source data format
     * @param targetFormat OpenMD format specification
     * @return std::string Transformed data
     */
    virtual std::string transformToOpenMD(const std::string& inputData,
                                         const std::string& sourceFormat,
                                         const std::string& targetFormat) = 0;
    
    /**
     * @brief Transform data from OpenMD format
     * 
     * @param openmdData Data in OpenMD format
     * @param targetFormat Desired output format
     * @return std::string Transformed data
     */
    virtual std::string transformFromOpenMD(const std::string& openmdData,
                                           const std::string& targetFormat) = 0;
    
    /**
     * @brief Get available simulation templates
     * 
     * @return std::vector<std::string> List of available templates
     */
    virtual std::vector<std::string> getAvailableTemplates() = 0;
    
    /**
     * @brief Load simulation template
     * 
     * @param templateName Name of the template
     * @return SimulationConfig Template configuration
     */
    virtual SimulationConfig loadTemplate(const std::string& templateName) = 0;
    
    /**
     * @brief Validate simulation configuration
     * 
     * @param config Configuration to validate
     * @return bool True if configuration is valid
     */
    virtual bool validateConfig(const SimulationConfig& config) = 0;
    
    /**
     * @brief Get system resource information
     * 
     * @return std::map<std::string, std::string> Available resources
     */
    virtual std::map<std::string, std::string> getSystemResources() = 0;
    
    /**
     * @brief Clean up completed jobs and temporary files
     * 
     * @param olderThan Clean jobs older than this duration
     * @return bool True if cleanup was successful
     */
    virtual bool cleanup(std::chrono::hours olderThan) = 0;
    
    /**
     * @brief Get connection status and statistics
     * 
     * @return std::map<std::string, std::string> Connection information
     */
    virtual std::map<std::string, std::string> getConnectionInfo() = 0;
};

/**
 * @brief Factory function to create SimulationConnector instance
 * 
 * @return std::unique_ptr<SimulationConnector> SimulationConnector instance
 */
std::unique_ptr<SimulationConnector> createSimulationConnector();

} // namespace integration
} // namespace dist_prompt