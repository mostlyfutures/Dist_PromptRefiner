# Geometric Decomposition Module Owners

## Primary Owner
- **Region**: Geometric Decomposition Module
- **Responsibility**: Spatial problem decomposition algorithms
- **Developer**: [To be assigned]
- **Email**: [To be assigned]

## Module Responsibilities
- Spatial partitioning using k-d tree with configurable depth
- Region assignment using graph coloring algorithm with 4-color theorem
- Interface definition with abstract base classes
- Geometric problem space analysis

## Implementation Requirements
- **Spatial Partitioner**: `src/geometric/spatial_partitioner.cpp` with k-d tree implementation
- **Region Assigner**: `src/geometric/region_assigner.cpp` with graph coloring algorithm
- **Interfaces**: Abstract base classes in `include/geometric/interfaces/`
- **Main Interface**: `include/geometric_interface.h` with `RegionDecomposer` class
- **Tests**: Minimum 95% code coverage in `test/geometric/`

## Development Guidelines
- Follow TDD with tests written before implementation
- All commits must reference task IDs and be max 200 lines
- Use MoSCoW prioritization in JIRA
- Create design documents in `design/geometric/`
- Test files must be named `*_test.cpp`

## Integration Points
- **From PCAM**: Receives decomposition requests via `RegionDecomposer` interface
- **To PCAM**: Returns spatial partitioning results and region assignments
- **To Orchestrator**: Reports module status via `AgentCoordinator` interface

## Git Workflow
- **Branch**: `region/geometric` for main development
- **Feature Branches**: `region/geometric/feature/[feature-name]`
- **Reviews**: All PRs require approval from primary owner

## Dependencies
- Standard C++ libraries for mathematical operations
- Graph theory algorithms for coloring
- Spatial data structures (k-d trees)