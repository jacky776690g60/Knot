/** ================================================================
| common.hpp  --  common.hpp
|
| Created by Jack on 07/14, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <regex>
#include <stdexcept>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#include <termios.h>
#include <unistd.h>

#include "JsonParser.hpp"


const int SALT_SIZE = 16;
const int KEY_SIZE  = 32;
const int IV_SIZE   = 16;


std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::string getPassword() {
    const char BACKSPACE = 127;
    const char RETURN = 10;

    std::string password;
    char ch = 0;

    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "Enter password: ";
    while ((ch = getchar()) != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                std::cout << "\b \b";
                password.resize(password.length() - 1);
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << std::endl;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return password;
}

bool matchesWildcard(const std::string& text, const std::string& pattern) {
    std::string regexPattern = pattern;
    // Handle ** (match any including path separators)
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*\\*"), ".*");
    // Handle * (within a path segment)
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*"), "[^/]*");
    // Handle ? (match any single character within a path segment)
    regexPattern = std::regex_replace(regexPattern, std::regex("\\?"), "[^/]");
    //regexPattern = "^" + regexPattern + "$";
    return std::regex_search(text, std::regex(regexPattern));
}

std::vector<uint8_t> generateRandomBytes(size_t size) {
    std::vector<uint8_t> bytes(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::generate(bytes.begin(), bytes.end(), [&]() { return static_cast<uint8_t>(dis(gen)); });
    return bytes;
}



std::vector<uint8_t> deriveKey(const std::string& password, const std::vector<uint8_t>& salt) {
    std::vector<uint8_t> key(KEY_SIZE);
    std::vector<uint8_t> data = salt;
    data.insert(data.end(), password.begin(), password.end());
    
    for (int i = 0; i < 10000; ++i) {
        std::hash<std::string> hasher;
        auto hash = hasher(std::string(data.begin(), data.end()));
        std::copy_n(reinterpret_cast<uint8_t*>(&hash), sizeof(hash), data.begin());
    }
    
    std::copy_n(data.begin(), KEY_SIZE, key.begin());
    return key;
}




const std::array<char, 8> KNOT_SIGNATURE = {'K', 'N', 'O', 'T', 'E', 'N', 'C', '1'};
/**
 * This structure holds various configuration parameters used to determine
 * which files should be processed and which should be skipped.
 */
struct Config {
    /** A vector of file extensions to be processed. */
    std::vector<std::string> extensions;
    /** A vector of specific filenames or paths to be processed. */
    std::vector<std::string> specific_files;
    /** A vector of folder names or paths to be skipped during processing. */
    std::vector<std::string> skip_folders;
};


Config parseConfigFile(const std::string& filename) {
    Config config;
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open config file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    JsonParser::JsonValue jsonConfig = JsonParser::parse(content);

    if (auto* obj = std::get_if<JsonParser::JsonObject>(&jsonConfig)) {
        if (auto* extensions = std::get_if<JsonParser::JsonArray>(&(*obj)["extensions"])) {
            for (const auto& ext : *extensions) {
                if (auto* str = std::get_if<std::string>(&ext)) {
                    config.extensions.push_back(*str);
                }
            }
        }

        if (auto* specific_files = std::get_if<JsonParser::JsonArray>(&(*obj)["specific_files"])) {
            for (const auto& file : *specific_files) {
                if (auto* str = std::get_if<std::string>(&file)) {
                    config.specific_files.push_back(*str);
                }
            }
        }

        if (auto* skip_folders = std::get_if<JsonParser::JsonArray>(&(*obj)["skip_folders"])) {
            for (const auto& folder : *skip_folders) {
                if (auto* str = std::get_if<std::string>(&folder)) {
                    config.skip_folders.push_back(*str);
                }
            }
        }
    } else {
        throw std::runtime_error("Invalid JSON format in config file");
    }

    return config;
}


std::vector<std::string> getTargetFiles(
    const Config& config, 
    int maxDepth = -1
) {
    std::vector<std::string> targetFiles;
    
    std::filesystem::path currentFilePath = std::filesystem::current_path();
    std::filesystem::path parentPath      = currentFilePath.parent_path();
    std::cout << "Searching for files in: " << parentPath << std::endl;

    // Function to recursively search directories
    std::function<void(const std::filesystem::path&, int)> searchDirectory = 
        [&](const std::filesystem::path& path, int depth) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (maxDepth >= 0 && depth > maxDepth) {
                    break;
                }

                // Skip the current executable's directory
                if (entry.path() == currentFilePath) {
                    continue;
                }

                if (entry.is_directory()) {
                    bool skip = false;
                    for (const auto& skip_pattern : config.skip_folders) {
                        if (matchesWildcard(entry.path().string(), skip_pattern)) {
                            skip = true;
                            std::cout << "\033[33m" << "Skipping folder: " << entry.path() << "\033[0m" << std::endl;
                            break;
                        }
                    }
                    if (!skip) {
                        searchDirectory(entry.path(), depth + 1);
                    }
                } else if (entry.is_regular_file()) {
                    std::string extension = entry.path().extension().string();
                    for (const auto& target_ext : config.extensions) {
                        if (extension == target_ext) {
                            targetFiles.push_back(entry.path().string());
                            break;
                        }
                    }
                }
            }
        };

    // Add specific files
    for (const auto& file : config.specific_files) {
        std::filesystem::path filePath = std::filesystem::absolute(file);
        if (std::filesystem::is_regular_file(filePath)) {
            targetFiles.push_back(filePath.string());
            std::cout << "Found specific file: " << filePath << std::endl;
        } else {
            std::cout << "Skipping non-regular file: " << filePath << std::endl;
        }
    }

    // Start the recursive search
    searchDirectory(parentPath, 0);

    std::cout << "Total target files found: " << targetFiles.size() << std::endl;
    return targetFiles;
}


bool isKnotEncryptedFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;

    std::array<char, 8> signature;
    file.read(signature.data(), signature.size());

    return (file && signature == KNOT_SIGNATURE);
}



std::vector<std::string> findKnotFiles() {
    std::vector<std::string> knotFiles;
    std::filesystem::path parentPath = std::filesystem::current_path().parent_path();
    std::cout << "Searching for .knot files in: " << parentPath << std::endl;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(parentPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".knot") {
            knotFiles.push_back(entry.path().string());
        }
    }

    return knotFiles;
}


void removeKnotFile(const std::string& filename) {
    if (isKnotEncryptedFile(filename)) {
        std::filesystem::remove(filename);
        std::cout << "Removed: " << filename << std::endl;
    } else {
        std::cout << "Skipped (not a Knot encrypted file): " << filename << std::endl;
    }
}