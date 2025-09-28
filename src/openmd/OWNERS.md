# OpenMD Integration Layer Owners

## Primary Owner
- **Region**: OpenMD Integration Layer
- **Responsibility**: Interface with OpenMD for parallel processing
- **Developer**: [To be assigned]
- **Email**: [To be assigned]

## Module Responsibilities
- API binding using SWIG-generated FFI with version checking
- Data transformation with bidirectional mappers
- Comprehensive error handling and taxonomy
- OpenMD simulation coordination

## Implementation Requirements
- **API Binding**: `src/openmd/binding.cpp` with SWIG-generated FFI and version checking
- **Data Mappers**: Bidirectional mappers in `src/openmd/mappers/data_mapper.cpp`
- **Error Handling**: Comprehensive error taxonomy in `src/openmd/errors/error_codes.cpp`
- **Main Interface**: `include/openmd_interface.h` with `SimulationConnector` class
- **Tests**: Minimum 95% code coverage in `test/openmd/`

## Development Guidelines
- Follow TDD with tests written before implementation
- All commits must reference task IDs and be max 200 lines
- Use MoSCoW prioritization in JIRA
- Create design documents in `design/openmd/`
- Test files must be named `*_test.cpp`

## Integration Points
- **From PCAM**: Receives simulation requests via `SimulationConnector` interface
- **To OpenMD**: Executes parallel simulations and data processing
- **To PCAM**: Returns simulation results and status updates
- **To Orchestrator**: Reports integration status via `AgentCoordinator` interface

## Git Workflow
- **Branch**: `region/openmd` for main development
- **Feature Branches**: `region/openmd/feature/[feature-name]`
- **Reviews**: All PRs require approval from primary owner

## Dependencies
- SWIG for FFI generation
- OpenMD library and headers
- Standard C++ libraries for data transformation
- Error handling frameworks