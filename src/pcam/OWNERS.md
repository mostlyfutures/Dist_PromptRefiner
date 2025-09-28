# PCAM Core Engine Owners

## Primary Owner
- **Region**: PCAM Core Engine
- **Responsibility**: Core transformation logic for Plan-Code-Architect-Model steps
- **Developer**: [To be assigned]
- **Email**: [To be assigned]

## Module Responsibilities
- Plan decomposition using PCAM-DX algorithm
- Code generation with Mustache templates (v4.1+)
- Module architecture following hexagonal pattern
- Data modeling with Protocol Buffers v3.19.4

## Implementation Requirements
- **Plan Decomposer**: `src/pcam/plan_decomposer.cpp` with PCAM-DX algorithm
- **Code Generator**: `src/pcam/code_generator/` with Mustache templates and custom hooks
- **Architecture**: `src/pcam/architecture/` following hexagonal architecture with ports and adapters
- **Data Models**: `proto/pcam_models.proto` using Protocol Buffers v3.19.4
- **Interface**: Central hub connecting to all other modules
- **Tests**: Minimum 95% code coverage in `test/pcam/`

## Development Guidelines
- Follow TDD with tests written before implementation
- All commits must reference task IDs and be max 200 lines
- Use MoSCoW prioritization in JIRA
- Create design documents in `design/pcam/`
- Test files must be named `*_test.cpp`

## Integration Points
- **From CLI**: Receives commands via `CommandProcessor` interface
- **To Geometric**: Sends decomposition requests via `RegionDecomposer` interface
- **To Patterns**: Sends pattern requests via `PatternMatcher` interface
- **To OpenMD**: Sends simulation jobs via `SimulationConnector` interface
- **To Orchestrator**: Coordinates workflow via `AgentCoordinator` interface

## Git Workflow
- **Branch**: `region/pcam` for main development
- **Feature Branches**: `region/pcam/feature/[feature-name]`
- **Reviews**: All PRs require approval from primary owner

## Dependencies
- Mustache template engine v4.1+
- Protocol Buffers v3.19.4
- Boost libraries for core functionality