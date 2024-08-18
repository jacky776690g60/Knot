/** ================================================================
| JsonParser.hpp  --  JsonParser.hpp
|
| Created by Jack on 07/19, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */

#include <string>
#include <variant>
#include <vector>
#include <map>
#include <stdexcept>
#include <cctype>
#include <cstdint>

class JsonParser {
public:
    class JsonValue; // Forward declaration to solve circular dependency

    using JsonArray  = std::vector<JsonValue>;
    using JsonObject = std::map<std::string, JsonValue>;

    class JsonValue : public std::variant<
                                std::nullptr_t, bool, 
                                int_fast64_t, 
                                double, 
                                std::string, JsonArray, JsonObject>
    {
    public:
        using variant::variant;  // Inherit constructors
    };

    static JsonValue parse(const std::string& json) {
        size_t pos = 0;
        return parseValue(json, pos);
    }


private:
    static JsonValue parseValue(const std::string& json, size_t& pos) {
        skipWhitespace(json, pos);
        
        if (pos >= json.length()) {
            throw std::runtime_error("Unexpected end of input");
        }

        switch (json[pos]) {
            case 'n': return parseNull(json, pos);
            case 't':
            case 'f': return parseBoolean(json, pos);
            case '"': return parseString(json, pos);
            case '{': return parseObject(json, pos);
            case '[': return parseArray(json, pos);
            default:
                if (std::isdigit(json[pos]) || json[pos] == '-') {
                    return parseNumber(json, pos);
                }
                throw std::runtime_error("Invalid JSON syntax");
        }
    }

    
    static void skipWhitespace(const std::string& json, size_t& pos) {
        while (pos < json.length() && std::isspace(json[pos])) {
            ++pos;
        }
    }

    static std::nullptr_t parseNull(const std::string& json, size_t& pos) {
        if (json.substr(pos, 4) == "null") {
            pos += 4;
            return nullptr;
        }
        throw std::runtime_error("Invalid null value");
    }

    static bool parseBoolean(const std::string& json, size_t& pos) {
        if (json.substr(pos, 4) == "true") {
            pos += 4;
            return true;
        }
        if (json.substr(pos, 5) == "false") {
            pos += 5;
            return false;
        }
        throw std::runtime_error("Invalid boolean value");
    }

    static std::string parseString(const std::string& json, size_t& pos) {
        std::string result;
        ++pos; // Skip opening quote
        while (pos < json.length() && json[pos] != '"') {
            if (json[pos] == '\\') {
                ++pos;
                if (pos >= json.length()) {
                    throw std::runtime_error("Unexpected end of input in string");
                }
                switch (json[pos]) {
                    case '"':
                    case '\\':
                    case '/': result += json[pos]; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw std::runtime_error("Invalid escape sequence in string");
                }
            } else {
                result += json[pos];
            }
            ++pos;
        }
        if (pos >= json.length() || json[pos] != '"') {
            throw std::runtime_error("Unterminated string");
        }
        ++pos; // Skip closing quote
        return result;
    }

    static JsonValue parseNumber(const std::string& json, size_t& pos) {
        size_t start = pos;
        bool isFloat = false;
        while (pos < json.length() && (std::isdigit(json[pos]) || json[pos] == '-' || json[pos] == '.' || json[pos] == 'e' || json[pos] == 'E')) {
            if (json[pos] == '.' || json[pos] == 'e' || json[pos] == 'E') {
                isFloat = true;
            }
            ++pos;
        }
        std::string numStr = json.substr(start, pos - start);
        if (isFloat) {
            return std::stod(numStr);
        } else {
            return static_cast<int_fast64_t>(std::stoll(numStr)); // Cast to int_fast64_t
        }
    }

    static JsonArray parseArray(const std::string& json, size_t& pos) {
        JsonArray result;
        ++pos; // Skip opening bracket
        skipWhitespace(json, pos);
        if (json[pos] == ']') {
            ++pos;
            return result;
        }
        while (true) {
            result.push_back(parseValue(json, pos));
            skipWhitespace(json, pos);
            if (json[pos] == ']') {
                ++pos;
                return result;
            }
            if (json[pos] != ',') {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
            ++pos;
        }
    }

    static JsonObject parseObject(const std::string& json, size_t& pos) {
        JsonObject result;
        ++pos; // Skip opening brace
        skipWhitespace(json, pos);
        if (json[pos] == '}') {
            ++pos;
            return result;
        }
        while (true) {
            std::string key = parseString(json, pos);
            skipWhitespace(json, pos);
            if (json[pos] != ':') {
                throw std::runtime_error("Expected ':' in object");
            }
            ++pos;
            result[key] = parseValue(json, pos);
            skipWhitespace(json, pos);
            if (json[pos] == '}') {
                ++pos;
                return result;
            }
            if (json[pos] != ',') {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
            ++pos;
            skipWhitespace(json, pos);
        }
    }
};