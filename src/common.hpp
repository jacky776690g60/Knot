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
// #include <openssl/aes.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <array>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include <filesystem>
namespace fs = std::filesystem;

#include "JsonParser.hpp"


const int SALT_SIZE = 16;
const int KEY_SIZE  = 32;//256bits
const int IV_SIZE   = 16;


std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}


std::string getPassword() {
    const char BACKSPACE = 8;
    const char RETURN = 13;

    std::string password;
    int ch = 0;

    std::cout << "Enter password: ";

#ifdef _WIN32
    while ((ch = _getch()) != RETURN) {
        if (ch == BACKSPACE) {
            if (!password.empty()) {
                std::cout << "\b \b";
                password.pop_back();
            }
        } else if (ch >= 32 && ch <= 126) {  // Printable ASCII characters
            password += static_cast<char>(ch);
            std::cout << '*';
        }
    }
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    newt.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while ((ch = getchar()) != RETURN && ch != '\n') {
        if (ch == BACKSPACE || ch == 127) {  // 127 is DEL, often sent by backspace on Unix
            if (!password.empty()) {
                std::cout << "\b \b";
                password.pop_back();
            }
        } else if (ch >= 32 && ch <= 126) {  // Printable ASCII characters
            password += static_cast<char>(ch);
            std::cout << '*';
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    std::cout << std::endl;
    return password;
}



bool matchesWildcard(const std::string& text, const std::string& pattern) {
    /** TODO: '*' and '**' not working properly for windows */ 
    std::string regexPattern = pattern;
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*\\*"), ".*");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*"), "[^/\\\\]*");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\?"), "[^/\\\\]");
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
    /** TODO: Implement proper cryptographic hashing (e.g., SHA-256) and key stretching */ 

    std::vector<uint8_t> key;
    std::vector<uint8_t> combined = salt;
    combined.insert(combined.end(), password.begin(), password.end());

    // Simple key derivation: repeating the combined salt and password
    while (key.size() < KEY_SIZE) {
        key.insert(key.end(), combined.begin(), combined.end());
    }

    // Truncate to ensure the key is exactly KEY_SIZE bytes
    key.resize(KEY_SIZE);

    return key;
}


const std::array<char, 8> KNOT_SIGNATURE = {'K', 'N', 'O', 'T', 'E', 'N', 'C', '1'};


/**
 * This structure holds various configuration parameters used to determine
 * which files should be processed and which should be skipped.
 */
struct Config {
    std::vector<std::string> extensions;
    std::vector<std::string> specific_files;
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


std::vector<std::string> getTargetFiles(const Config& config, int maxDepth = -1) {
    std::vector<std::string> targetFiles;
    
    fs::path currentFilePath = fs::current_path();
    fs::path parentPath = currentFilePath.parent_path();
    std::cout << "Searching for files in: " << parentPath << std::endl;

    std::function<void(const fs::path&, int)> searchDirectory = 
        [&](const fs::path& path, int depth) {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (maxDepth >= 0 && depth > maxDepth) {
                    break;
                }

                if (entry.path() == currentFilePath) {
                    continue;
                }

                if (fs::is_directory(entry)) {
                    bool skip = false;
                    for (const auto& skip_pattern : config.skip_folders) {
                        if (matchesWildcard(entry.path().string(), skip_pattern)) {
                            skip = true;
                            std::cout << "Skipping folder: " << entry.path() << std::endl;
                            break;
                        }
                    }
                    if (!skip) {
                        searchDirectory(entry.path(), depth + 1);
                    }
                } else if (fs::is_regular_file(entry)) {
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

    for (const auto& file : config.specific_files) {
        fs::path filePath = fs::absolute(file);
        if (fs::is_regular_file(filePath)) {
            targetFiles.push_back(filePath.string());
            std::cout << "Found specific file: " << filePath << std::endl;
        } else {
            std::cout << "Skipping non-regular file: " << filePath << std::endl;
        }
    }

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
    fs::path parentPath = fs::current_path().parent_path();
    std::cout << "Searching for .knot files in: " << parentPath << std::endl;

    for (const auto& entry : fs::recursive_directory_iterator(parentPath)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".knot") {
            knotFiles.push_back(entry.path().string());
        }
    }

    return knotFiles;
}

void removeKnotFile(const std::string& filename) {
    if (isKnotEncryptedFile(filename)) {
        fs::remove(filename);
        std::cout << "Removed: " << filename << std::endl;
    } else {
        std::cout << "Skipped (not a Knot encrypted file): " << filename << std::endl;
    }
}