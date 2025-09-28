#include "openmd/binding.h"
#include "openmd/errors/error_codes.h"
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <mutex>

namespace dist_prompt {
namespace openmd {

// String representation of version
std::string OpenMDBinding::Version::toString() const {
    std::stringstream ss;
    ss << major << "." << minor << "." << patch;
    if (!suffix.empty()) {
        ss << "-" << suffix;
    }
    return ss.str();
}

// Check version compatibility
bool OpenMDBinding::Version::isCompatibleWith(const Version& other) const {
    // Major version must match exactly
    if (major != other.major) {
        return false;
    }
    
    // Minor version must be greater or equal
    if (minor < other.minor) {
        return false;
    }
    
    // If minor versions match, patch must be greater or equal
    if (minor == other.minor && patch < other.patch) {
        return false;
    }
    
    return true;
}

// Private implementation class (PIMPL idiom)
class OpenMDBinding::Impl {
public:
    Impl() : libraryHandle_(nullptr), initialized_(false) {}
    
    ~Impl() {
        if (libraryHandle_) {
            dlclose(libraryHandle_);
            libraryHandle_ = nullptr;
        }
    }
    
    bool initialize(const std::string& libraryPath, const std::string& configPath) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Load the OpenMD shared library
        libraryHandle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!libraryHandle_) {
            throw errors::OpenMDException(
                errors::ErrorCode::LIBRARY_NOT_FOUND,
                "Failed to load OpenMD library: " + std::string(dlerror()),
                "Path: " + libraryPath
            );
        }
        
        // Load required functions
        if (!loadFunctions()) {
            dlclose(libraryHandle_);
            libraryHandle_ = nullptr;
            throw errors::OpenMDException(
                errors::ErrorCode::FUNCTION_NOT_FOUND,
                "Failed to load required functions from OpenMD library"
            );
        }
        
        // Check version compatibility
        Version libraryVersion = getVersion();
        Version requiredVersion = {1, 0, 0, ""};
        
        if (!libraryVersion.isCompatibleWith(requiredVersion)) {
            dlclose(libraryHandle_);
            libraryHandle_ = nullptr;
            throw errors::OpenMDException(
                errors::ErrorCode::INCOMPATIBLE_VERSION,
                "Incompatible OpenMD library version: " + libraryVersion.toString() + 
                ", required: " + requiredVersion.toString()
            );
        }
        
        // Initialize OpenMD with configuration
        if (!initOpenMD(configPath)) {
            dlclose(libraryHandle_);
            libraryHandle_ = nullptr;
            throw errors::OpenMDException(
                errors::ErrorCode::INITIALIZATION_FAILED,
                "Failed to initialize OpenMD with config: " + configPath
            );
        }
        
        initialized_ = true;
        return true;
    }
    
    bool isAvailable() const {
        return initialized_ && libraryHandle_ != nullptr;
    }
    
    OpenMDBinding::Version getVersion() const {
        if (!initialized_ || !versionFn_) {
            throw errors::OpenMDException(
                errors::ErrorCode::BINDING_ERROR,
                "Cannot get version: OpenMD binding not initialized"
            );
        }
        
        int major = 0, minor = 0, patch = 0;
        char suffixBuffer[64] = {0};
        
        versionFn_(&major, &minor, &patch, suffixBuffer, sizeof(suffixBuffer));
        
        return {major, minor, patch, std::string(suffixBuffer)};
    }
    
    bool isFeatureSupported(const std::string& featureName) const {
        if (!initialized_ || !featureSupportedFn_) {
            throw errors::OpenMDException(
                errors::ErrorCode::BINDING_ERROR,
                "Cannot check feature: OpenMD binding not initialized"
            );
        }
        
        return featureSupportedFn_(featureName.c_str()) != 0;
    }
    
    OpenMDBinding::SimulationResult runSimulation(
        const std::string& inputData, 
        const OpenMDBinding::SimulationParams& params) {
        
        if (!initialized_ || !runSimulationFn_) {
            throw errors::OpenMDException(
                errors::ErrorCode::BINDING_ERROR,
                "Cannot run simulation: OpenMD binding not initialized"
            );
        }
        
        // Convert parameters to C-style structures for FFI
        SimulationParamsC paramsC;
        paramsC.iterations = params.iterations;
        paramsC.timeStep = params.timeStep;
        paramsC.temperature = params.temperature;
        paramsC.forceField = params.forceField.c_str();
        
        // Convert additional parameters to C array
        char** additionalParamKeys = new char*[params.additionalParams.size()];
        char** additionalParamValues = new char*[params.additionalParams.size()];
        int additionalParamCount = 0;
        
        for (const auto& [key, value] : params.additionalParams) {
            additionalParamKeys[additionalParamCount] = strdup(key.c_str());
            additionalParamValues[additionalParamCount] = strdup(value.c_str());
            additionalParamCount++;
        }
        
        paramsC.additionalParamKeys = additionalParamKeys;
        paramsC.additionalParamValues = additionalParamValues;
        paramsC.additionalParamCount = additionalParamCount;
        
        // Prepare result structure
        SimulationResultC resultC;
        resultC.success = 0;
        resultC.resultData = nullptr;
        resultC.energy = 0.0;
        resultC.runtime = 0.0;
        resultC.warningCount = 0;
        resultC.warnings = nullptr;
        resultC.errorCount = 0;
        resultC.errors = nullptr;
        
        // Call OpenMD simulation function
        try {
            runSimulationFn_(inputData.c_str(), &paramsC, &resultC);
        } catch (const std::exception& e) {
            // Clean up C structures
            cleanupParams(&paramsC);
            throw errors::OpenMDException(
                errors::ErrorCode::SIMULATION_FAILED,
                "Exception during simulation: " + std::string(e.what())
            );
        }
        
        // Convert C result to C++ result
        OpenMDBinding::SimulationResult result;
        result.success = (resultC.success != 0);
        
        if (resultC.resultData) {
            result.resultData = std::string(resultC.resultData);
            free(resultC.resultData);
        }
        
        result.energy = resultC.energy;
        result.runtime = resultC.runtime;
        
        // Convert warnings
        for (int i = 0; i < resultC.warningCount; i++) {
            if (resultC.warnings[i]) {
                result.warnings.push_back(std::string(resultC.warnings[i]));
                free(resultC.warnings[i]);
            }
        }
        if (resultC.warnings) free(resultC.warnings);
        
        // Convert errors
        for (int i = 0; i < resultC.errorCount; i++) {
            if (resultC.errors[i]) {
                result.errors.push_back(std::string(resultC.errors[i]));
                free(resultC.errors[i]);
            }
        }
        if (resultC.errors) free(resultC.errors);
        
        // Clean up C parameter structures
        cleanupParams(&paramsC);
        
        // Check for simulation failures
        if (!result.success && !result.errors.empty()) {
            throw errors::OpenMDException(
                errors::ErrorCode::SIMULATION_FAILED,
                "Simulation failed: " + result.errors[0],
                "Additional errors: " + std::to_string(result.errors.size() - 1)
            );
        }
        
        return result;
    }
    
    void setProgressCallback(std::function<void(int)> callback) {
        progressCallback_ = callback;
        
        if (initialized_ && setProgressCallbackFn_) {
            // Set up C callback that calls our C++ callback
            setProgressCallbackFn_(progressCallbackTrampoline, this);
        }
    }
    
    bool registerCustomFunction(const std::string& functionName, void* function) {
        if (!initialized_ || !registerCustomFunctionFn_) {
            throw errors::OpenMDException(
                errors::ErrorCode::BINDING_ERROR,
                "Cannot register function: OpenMD binding not initialized"
            );
        }
        
        return registerCustomFunctionFn_(functionName.c_str(), function) != 0;
    }

private:
    void* libraryHandle_;
    bool initialized_;
    std::mutex mutex_;
    std::function<void(int)> progressCallback_;
    
    // Function pointer types for OpenMD API
    typedef void (*VersionFn)(int*, int*, int*, char*, size_t);
    typedef int (*FeatureSupportedFn)(const char*);
    typedef int (*InitFn)(const char*);
    typedef void (*RunSimulationFn)(const char*, const SimulationParamsC*, SimulationResultC*);
    typedef void (*SetProgressCallbackFn)(void (*)(int, void*), void*);
    typedef int (*RegisterCustomFunctionFn)(const char*, void*);
    
    // Function pointers
    VersionFn versionFn_;
    FeatureSupportedFn featureSupportedFn_;
    InitFn initFn_;
    RunSimulationFn runSimulationFn_;
    SetProgressCallbackFn setProgressCallbackFn_;
    RegisterCustomFunctionFn registerCustomFunctionFn_;
    
    // C-style structures for FFI
    struct SimulationParamsC {
        int iterations;
        double timeStep;
        double temperature;
        const char* forceField;
        char** additionalParamKeys;
        char** additionalParamValues;
        int additionalParamCount;
    };
    
    struct SimulationResultC {
        int success;
        char* resultData;
        double energy;
        double runtime;
        int warningCount;
        char** warnings;
        int errorCount;
        char** errors;
    };
    
    bool loadFunctions() {
        // Clear any existing error
        dlerror();
        
        // Load required functions
        versionFn_ = reinterpret_cast<VersionFn>(dlsym(libraryHandle_, "OpenMD_GetVersion"));
        if (!versionFn_) return false;
        
        featureSupportedFn_ = reinterpret_cast<FeatureSupportedFn>(
            dlsym(libraryHandle_, "OpenMD_IsFeatureSupported"));
        if (!featureSupportedFn_) return false;
        
        initFn_ = reinterpret_cast<InitFn>(dlsym(libraryHandle_, "OpenMD_Initialize"));
        if (!initFn_) return false;
        
        runSimulationFn_ = reinterpret_cast<RunSimulationFn>(
            dlsym(libraryHandle_, "OpenMD_RunSimulation"));
        if (!runSimulationFn_) return false;
        
        setProgressCallbackFn_ = reinterpret_cast<SetProgressCallbackFn>(
            dlsym(libraryHandle_, "OpenMD_SetProgressCallback"));
        // This is optional, so we don't check for failure
        
        registerCustomFunctionFn_ = reinterpret_cast<RegisterCustomFunctionFn>(
            dlsym(libraryHandle_, "OpenMD_RegisterCustomFunction"));
        // This is optional, so we don't check for failure
        
        return true;
    }
    
    bool initOpenMD(const std::string& configPath) {
        if (!initFn_) return false;
        
        return initFn_(configPath.c_str()) != 0;
    }
    
    void cleanupParams(SimulationParamsC* params) {
        if (!params) return;
        
        for (int i = 0; i < params->additionalParamCount; i++) {
            if (params->additionalParamKeys[i]) free(params->additionalParamKeys[i]);
            if (params->additionalParamValues[i]) free(params->additionalParamValues[i]);
        }
        
        delete[] params->additionalParamKeys;
        delete[] params->additionalParamValues;
    }
    
    // Static trampoline for C callback
    static void progressCallbackTrampoline(int progress, void* userData) {
        Impl* self = static_cast<Impl*>(userData);
        if (self && self->progressCallback_) {
            self->progressCallback_(progress);
        }
    }
};

// Public interface implementation

OpenMDBinding::OpenMDBinding() : pImpl_(std::make_unique<Impl>()) {}

OpenMDBinding::~OpenMDBinding() = default;

bool OpenMDBinding::initialize(const std::string& libraryPath, const std::string& configPath) {
    return pImpl_->initialize(libraryPath, configPath);
}

bool OpenMDBinding::isAvailable() const {
    return pImpl_->isAvailable();
}

OpenMDBinding::Version OpenMDBinding::getVersion() const {
    return pImpl_->getVersion();
}

bool OpenMDBinding::isFeatureSupported(const std::string& featureName) const {
    return pImpl_->isFeatureSupported(featureName);
}

OpenMDBinding::SimulationResult OpenMDBinding::runSimulation(
    const std::string& inputData, const SimulationParams& params) {
    
    return pImpl_->runSimulation(inputData, params);
}

void OpenMDBinding::setProgressCallback(std::function<void(int)> callback) {
    pImpl_->setProgressCallback(callback);
}

bool OpenMDBinding::registerCustomFunction(const std::string& functionName, void* function) {
    return pImpl_->registerCustomFunction(functionName, function);
}

} // namespace openmd
} // namespace dist_prompt