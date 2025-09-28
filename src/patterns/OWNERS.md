# Patterns Module Owners

## Primary Owner
- **Region**: Program Structure Patterns Module
- **Responsibility**: Structural pattern recognition and application
- **Developer**: [To be assigned]
- **Email**: [To be assigned]

## Module Responsibilities
- Pattern identification using rule-based pattern matcher
- Pattern application using template-based transformers  
- Pattern verification system
- Pattern template management

## Implementation Requirements
- **Pattern Identifier**: `src/patterns/pattern_identifier.cpp`
- **Pattern Transformers**: `src/patterns/transformers/pattern_transformer.cpp`
- **Pattern Verifiers**: `src/patterns/verifiers/pattern_verifier.cpp`
- **Interface**: `include/pattern_interface.h` with `PatternMatcher` class
- **Tests**: Minimum 95% code coverage in `test/patterns/`

## Development Guidelines
- Follow TDD with tests written before implementation
- All commits must reference task IDs and be max 200 lines
- Use MoSCoW prioritization in JIRA
- Create design documents in `design/patterns/`
- Test files must be named `*_test.cpp`

## Integration Points
- **From PCAM**: Receives pattern matching requests via `PatternMatcher` interface
- **To PCAM**: Returns pattern suggestions and transformations
- **To Orchestrator**: Reports module status via `AgentCoordinator` interface

## Git Workflow
- **Branch**: `region/patterns` for main development
- **Feature Branches**: `region/patterns/feature/[feature-name]`
- **Reviews**: All PRs require approval from primary owner