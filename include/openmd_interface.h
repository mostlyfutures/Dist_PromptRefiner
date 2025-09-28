#pragma once

#include <string>
#include <vector>
#include <memory>

namespace dist_prompt {

/**
 * @brief SimulationConnector class - Interface between PCAM and OpenMD
 * 
 * This class provides integration with OpenMD for simulation-based refinement
 * of decomposed software ideas.
 */
class SimulationConnector {
public:
    /**
     * @brief Structure representing simulation parameters
     */
    struct SimulationParams {
        int iterations;
        double timeStep;
        double temperature;
        bool useParallelization;
        int numCores;
        // Additional simulation parameters
    };
    
    /**
     * @brief Structure representing simulation results
     */
    struct SimulationResult {
        bool success;
        std::string optimizedStructure;
        double stabilityScore;
        double convergenceMetric;
        std::vector<std::string> warnings;
    };

    /**
     * @brief Default constructor
     */
    SimulationConnector() = default;
    
    /**
     * @brief Virtual destructor to ensure proper cleanup in derived classes
     */
    virtual ~SimulationConnector() = default;
    
    /**
     * @brief Initialize the OpenMD simulation environment
     * 
     * @param configPath Path to OpenMD configuration file
     * @return bool True if initialization was successful, false otherwise
     */
    virtual bool initializeSimulation(const std::string& configPath) = 0;
    
    /**
     * @brief Run an OpenMD simulation on the decomposed structure
     * 
     * @param structure Structured data representing the decomposed software idea
     * @param params Simulation parameters
     * @return SimulationResult Results of the simulation
     */
    virtual SimulationResult runSimulation(const std::string& structure, 
                                         const SimulationParams& params) = 0;
    
    /**
     * @brief Check if OpenMD is available and correctly configured
     * 
     * @return bool True if OpenMD is available, false otherwise
     */
    virtual bool isOpenMDAvailable() const = 0;
    
    /**
     * @brief Get the version of the integrated OpenMD library
     * 
     * @return std::string Version string of OpenMD
     */
    virtual std::string getOpenMDVersion() const = 0;
};

} // namespace dist_prompt