#include "openmd/binding.h"
#include "openmd/errors/error_codes.h"
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include <mutex>

namespace dist_prompt {
namespace openmd {

// Version implementation
std::string OpenMDBinding::Version::toString() const {
    std::ostringstream oss;
    oss << major << "." << minor << "." << patch;
    if (!suffix.empty()) {
        oss << "-" << suffix;
    }
    return oss.str();
}

bool OpenMDBinding::Version::isCompatibleWith(const Version& other) const {
    // Major version must match for compatibility
    if (major != other.major) {
        return false;
    }
    
    // If major versions match but our minor version is less than other, not compatible
    if (minor < other.minor) {
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
        
        // Close existing library handle if any
        if (libraryHandle_) {
            dlclose(libraryHandle_);
            libraryHandle_ = nullptr;
        }
        
        // Load the shared library
        libraryHandle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!libraryHandle_) {
            throw errors::OpenMDException(
                errors::ErrorCode::LIBRARY_NOT_FOUND,
                "Failed to load OpenMD library",
                dlerror()
            );
        }
        
        // Load required symbols
        loadSymbols();
        
        // Check version compatibility
        Version minVersion = {3, 0, 0, ""};
        if (!version_.isCompatibleWith(minVersion)) {
            throw errors::OpenMDException(
                errors::ErrorCode::INCOMPATIBLE_VERSION,
                "Incompatible OpenMD version",
                "Required version 3.0.0 or higher, found " + version_.toString()
            );
        }
        
        // Initialize the library with configuration
        if (!initializeLibrary(configPath)) {
            throw errors::OpenMDException(
                errors::ErrorCode::INITIALIZATION_FAILED,
                "Failed to initialize OpenMD library",
                "Configuration error with " + configPath
            );
        }
        
        initialized_ = true;
        return true;
    }
    
    bool isAvailable() const {
        return initialized_ && libraryHandle_ != nullptr;
    }
    
    Version getVersion() const {
        if (!initialized_) {
            throw errors::OpenMDException(
                errors::ErrorCode::INITIALIZATION_FAILED,
                "OpenMD binding not initialized",
                "Call initialize() before getVersion()"
            );
        }
        
        return version_;
    }
    
    bool isFeatureSupported(const std::string& featureName) const {
        if (!initialized_) {
            return false;
        }
        
        // Use dlsym to check if the feature function exists
        std::string featureSymbol = "OpenMD_feature_" + featureName;
        void* featurePtr = dlsym(libraryHandle_, featureSymbol.c_str());
        return featurePtr != nullptr;
    }
    
    OpenMDBinding::SimulationResult runSimulation(
        const std::string& inputData, 
        const OpenMDBinding::SimulationParams& params) {
        
        if (!initialized_) {
            throw errors::OpenMDException(
                errors::ErrorCode::INITIALIZATION_FAILED,
                "OpenMD binding not initialized",
                "Call initialize() before runSimulation()"
            );
        }
        
        // In a real implementation, this would call the actual OpenMD simulation function
        // For this example, we'll create a simulated result
        
        OpenMDBinding::SimulationResult result;
        result.success = true;
        result.resultData = "{ \"simulation\": \"result data\" }";
        result.energy = -123.456;
        result.runtime = 2.5;
        
        // Report progress at intervals
        if (progressCallback_) {
            for (int progress = 0; progress <= 100; progress += 10) {
                progressCallback_(progress);
                // In a real implementation, this would be called during the actual simulation
                // rather than in a simple loop
            }
        }
        
        return result;
    }
    
    void setProgressCallback(std::function<void(int)> callback) {
        progressCallback_ = callback;
    }
    
    bool registerCustomFunction(const std::string& functionName, void* function) {
        if (!initialized_) {
            return false;
        }
        
        // In a real implementation, this would register the function with OpenMD
        // For this example, we'll just store it in our map
        customFunctions_[functionName] = function;
        return true;
    }

private:
    void* libraryHandle_;
    bool initialized_;
    Version version_;
    std::mutex mutex_;
    std::map<std::string, void*> customFunctions_;
    std::function<void(int)> progressCallback_;
    
    // Function pointer types for the OpenMD API
    typedef int (*InitFuncType)(const char*);
    typedef int (*VersionFuncType)(int*, int*, int*);
    typedef int (*SimulateFuncType)(const char*, void*, char*, int);
    
    // Function pointers for OpenMD API functions
    InitFuncType initFunc_;
    VersionFuncType versionFunc_;
    SimulateFuncType simulateFunc_;
    
    void loadSymbols() {
        // Clear any existing error
        dlerror();
        
        // Load initialization function
        initFunc_ = reinterpret_cast<InitFuncType>(dlsym(libraryHandle_, "OpenMD_initialize"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            throw errors::OpenMDException(
                errors::ErrorCode::FUNCTION_NOT_FOUND,
                "Failed to load OpenMD_initialize function",
                dlsymError
            );
        }
        
        // Load version function
        versionFunc_ = reinterpret_cast<VersionFuncType>(dlsym(libraryHandle_, "OpenMD_version"));
        dlsymError = dlerror();
        if (dlsymError) {
            throw errors::OpenMDException(
                errors::ErrorCode::FUNCTION_NOT_FOUND,
                "Failed to load OpenMD_version function",
                dlsymError
            );
        }
        
        // Load simulate function
        simulateFunc_ = reinterpret_cast<SimulateFuncType>(dlsym(libraryHandle_, "OpenMD_simulate"));
        dlsymError = dlerror();
        if (dlsymError) {
            throw errors::OpenMDException(
                errors::ErrorCode::FUNCTION_NOT_FOUND,
                "Failed to load OpenMD_simulate function",
                dlsymError
            );
        }
        
        // Get version information
        int major, minor, patch;
        int result = versionFunc_(&major, &minor, &patch);
        if (result != 0) {
            throw errors::OpenMDException(
                errors::ErrorCode::BINDING_ERROR,
                "Failed to get OpenMD version",
                "Error code: " + std::to_string(result)
            );
        }
        
        version_.major = major;
        version_.minor = minor;
        version_.patch = patch;
        
        // Add suffix if this is a development version
        if (major == 0) {
            version_.suffix = "dev";
        }
    }
    
    bool initializeLibrary(const std::string& configPath) {
        if (!initFunc_) {
            return false;
        }
        
        int result = initFunc_(configPath.c_str());
        return result == 0;
    }
};

// OpenMDBinding implementation

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
    const std::string& inputData, 
    const SimulationParams& params) {
    
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