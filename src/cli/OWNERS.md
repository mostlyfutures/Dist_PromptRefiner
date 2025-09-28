# CLI Interface Owners

## Primary Owner
- **Region**: CLI Interface
- **Responsibility**: All command-line interactions and argument parsing
- **Developer**: [To be assigned]
- **Email**: [To be assigned]

## Module Responsibilities
- Command-line parsing using Boost.Program_options v1.78.0
- Input validation with 12 validator classes
- Output rendering with JSON and plain-text formatters
- User interaction management

## Implementation Requirements
- **CLI Parser**: `src/cli/cli_parser.cpp` and `src/cli/cli_parser.h`
- **Command Processor**: `src/cli/command_processor_impl.cpp` and `src/cli/command_processor_impl.h`
- **Validators**: 12 validator classes in `src/cli/validators/`
- **Formatters**: JSON (nlohmann/json v3.11.2) and plain-text in `src/cli/formatters/`
- **Interface**: `include/cli_pcam_interface.h` with `CommandProcessor` class
- **Tests**: Minimum 95% code coverage in `test/cli/`

## Development Guidelines
- Follow TDD with tests written before implementation
- All commits must reference task IDs and be max 200 lines
- Use MoSCoW prioritization in JIRA
- Create design documents in `design/cli/`
- Test files must be named `*_test.cpp`

## Integration Points
- **To PCAM**: Sends parsed commands via `CommandProcessor` interface
- **To Orchestrator**: Reports CLI status via `AgentCoordinator` interface
- **From Users**: Receives command-line input and displays formatted output

## Git Workflow
- **Branch**: `region/cli` for main development
- **Feature Branches**: `region/cli/feature/[feature-name]`
- **Reviews**: All PRs require approval from primary owner

## Dependencies
- Boost.Program_options v1.78.0
- nlohmann/json v3.11.2
- Standard C++ libraries