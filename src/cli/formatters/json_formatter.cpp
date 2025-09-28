#include "cli/formatters/json_formatter.h"
#include <stdexcept>

namespace dist_prompt {
namespace cli {
namespace formatters {

JsonFormatter::JsonFormatter() : indentLevel_(4) {}

std::string JsonFormatter::format(const nlohmann::json& data) {
    try {
        return data.dump(indentLevel_);
    }
    catch (const std::exception& e) {
        return "{\"error\": \"Failed to format JSON: " + std::string(e.what()) + "\"}";
    }
}

void JsonFormatter::setIndent(int indent) {
    if (indent >= 0 && indent <= 8) {
        indentLevel_ = indent;
    }
}

bool JsonFormatter::isValidJson(const std::string& jsonStr) {
    try {
        nlohmann::json::parse(jsonStr);
        return true;
    }
    catch (...) {
        return false;
    }
}

} // namespace formatters
} // namespace cli
} // namespace dist_prompt