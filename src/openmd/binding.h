#pragma once

#include "openmd/errors/error_codes.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace dist_prompt {
namespace openmd {

/**
 * @brief API binding for OpenMD using SWIG-generated FFI
 * 
 * Provides a C++ interface to the OpenMD library using SWIG-generated
 * foreign function interface bindings.
 */
class OpenMDBinding {
public:
    /**
     * @brief Version information structure
     */
    struct Version {
        int major;
        int minor;
        int patch;
        std::string suffix;
        
        std::string toString() const;
        bool isCompatibleWith(const Version& other) const;
    };
    
    /**
     * @brief Simulation parameters structure
     */
    struct SimulationParams {
        int iterations;
        double timeStep;
        double temperature;
        std::string forceField;
        std::map<std::string, std::string> additionalParams;
    };
    
    /**
     * @brief Simulation results structure
     */
    struct SimulationResult {
        bool success;
        std::string resultData;
        double energy;
        double runtime;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    /**
     * @brief Constructor
     */
    OpenMDBinding();
    
    /**
     * @brief Destructor
     */
    ~OpenMDBinding();
    
    /**
     * @brief Initialize the OpenMD binding
     * 
     * @param libraryPath Path to the OpenMD shared library
     * @param configPath Path to the OpenMD configuration file
     * @return bool True if initialization was successful
     * @throws OpenMDException if initialization fails
     */
    bool initialize(const std::string& libraryPath, const std::string& configPath);
    
    /**
     * @brief Check if OpenMD is available
     * 
     * @return bool True if OpenMD is available
     */
    bool isAvailable() const;
    
    /**
     * @brief Get the version of OpenMD
     * 
     * @return Version The OpenMD version
     * @throws OpenMDException if version check fails
     */
    Version getVersion() const;
    
    /**
     * @brief Check if a specific feature is supported
     * 
     * @param featureName Name of the feature
     * @return bool True if the feature is supported
     */
    bool isFeatureSupported(const std::string& featureName) const;
    
    /**
     * @brief Run an OpenMD simulation
     * 
     * @param inputData Input data for the simulation
     * @param params Simulation parameters
     * @return SimulationResult Results of the simulation
     * @throws OpenMDException if simulation fails
     */
    SimulationResult runSimulation(const std::string& inputData, 
                                 const SimulationParams& params);
    
    /**
     * @brief Set a callback for simulation progress
     * 
     * @param callback Function to call with progress updates (0-100%)
     */
    void setProgressCallback(std::function<void(int)> callback);
    
    /**
     * @brief Register a custom function with OpenMD
     * 
     * @param functionName Name of the function to register
     * @param function Function to register
     * @return bool True if registration was successful
     */
    bool registerCustomFunction(const std::string& functionName, 
                              void* function);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace openmd
} // namespace dist_prompt