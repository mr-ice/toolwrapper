#include "load_config.h"
#include <iostream>
#include <sstream>
#include <algorithm> // for std::transform
#include <cctype>    // for std::toupper

// Convert a string to uppercase
std::string to_uppercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return result;
}

// remove both leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

std::map<std::string, std::variant<int, float, bool, std::string>> load_config(std::ifstream& file) {
    std::map<std::string, std::variant<int, float, bool, std::string>> keyValueMap;

    if (!file.is_open()) {
        std::cerr << "Error: File stream is not open" << std::endl;
        return keyValueMap;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string processedLine;

        // Remove comment characters outside of quotes
        bool inQuotes = false;
        for (char c : line) {
            if (c == '"' || c == '\'') {
                inQuotes = !inQuotes;
            }
            if (!inQuotes && c == '#') {
                break;
            }
            processedLine.push_back(c);
        }

        // Trim whitespace
        processedLine = trim(processedLine);

        // Skip empty lines or lines that only contain comment characters
        if (processedLine.empty() || processedLine[0] == '#')
            continue;

        // Split the line into key and value
        size_t equalPos = processedLine.find('=');
        if (equalPos == std::string::npos)
            continue; // Invalid line format, skip
        std::string key = trim(processedLine.substr(0, equalPos));
        std::string value = trim(processedLine.substr(equalPos + 1));

        // Remove quotes from the value
        if (!value.empty() && ((value.front() == '"' && value.back() == '"') ||
                               (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        // Attempt to parse the value as integer, float, boolean, or leave as string
        std::istringstream iss(value);
        int intValue;
        float floatValue;

        // make true/false comparisons case insensitive
        std::string uppercaseValue = to_uppercase(value);

        if ((iss >> intValue) && (iss >> std::ws).eof()) {
            keyValueMap[key] = intValue;
        } else if ((iss.clear(), iss.seekg(0), iss >> floatValue) && (iss >> std::ws).eof()) {
            keyValueMap[key] = floatValue;
        } else if (uppercaseValue == "TRUE") {
            keyValueMap[key] = true;
        } else if (uppercaseValue == "FALSE") {
            keyValueMap[key] = false;
        } else {
            keyValueMap[key] = value;
        }

        // Check if we encountered the PATH key
        if (key == "PATH")
            break;
    }

    return keyValueMap;
}
