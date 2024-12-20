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
#include <openssl/evp.h>
#include <openssl/sha.h>

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

const int SALT_SIZE = 16, 
          KEY_SIZE  = 32, // 256bits
          IV_SIZE   = 16;

/**
 * Transform `std::string` to lowercase.
 * @note unsigned char to properly handle extended ASCII characters (>127)
 */
void toLower(std::string& s) {
    std::transform(
        s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); }
    );
}

/** 
 * Strip the string 
 */
void strip(std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        str.clear();
        return;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    str.erase(last + 1); // Erase trailing whitespace
    str.erase(0, first); // Erase leading whitespace
}


std::string getPassword() {
    const char BACKSPACE = 8;
    const char RETURN    = 13;

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
    /** TODO: test if working on windows */ 
    std::string regexPattern = pattern;
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*\\*"), ".*");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\*"), "[^/\\\\]*");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\?"), "[^/\\\\]");
    return std::regex_search(text, std::regex(regexPattern));
}


/** 
 * Generates a vector of random (0 ~ 255) bytes.
 * @note Uses hardware random device as seed for Mersenne Twister
 * @example
 * auto ten_random_bytes = generateRandomBytes(10);
 */
std::vector<uint8_t> generateRandomBytes(size_t size) {
    std::vector<uint8_t> bytes(size);
    std::random_device   rd;                     // hardware rand-num generator src
    std::mt19937         gen(rd());              // mt pseudorandom generator
    std::uniform_int_distribution<> dis(0, 255); // get uniform random numbers (0 ~ 255)
    std::generate(bytes.begin(), bytes.end(), [&]() { return static_cast<uint8_t>(dis(gen)); });
    return bytes;
}


/**
 * Derive a key from combining `password` and `salt`
 * @deprecated Use SHA256
 */
[[deprecated("Simple derviekey function.")]]
std::vector<uint8_t> deriveKey_d(const std::string& password, const std::vector<uint8_t>& salt) {
    std::vector<uint8_t> key;             // new vector
    std::vector<uint8_t> combined = salt; // new vector initialized with salt
    combined.insert(combined.end(), password.begin(), password.end()); // append to end
    // Simple key derivation: repeating the combined salt and password
    while (key.size() < KEY_SIZE) {
        key.insert(key.end(), combined.begin(), combined.end());
    }
    key.resize(KEY_SIZE); // Truncate key to size of KEY_SIZE bytes
    return key;
}
/** Derive a key from combining `password` and `salt` with SHA256 */
std::vector<uint8_t> deriveKey(const std::string& password, const std::vector<uint8_t>& salt) {
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), 
            password.length(),
            salt.data(),      
            salt.size(),
            10000,        // 10000 iterations
            EVP_sha256(),
            KEY_SIZE,
            key.data()
        ) != 1
    ) {
        throw std::runtime_error("PBKDF2 key derivation failed");
    }
    
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
    fs::path parentPath      = currentFilePath.parent_path();
    std::cout << "Searching for files with targeted extension(s) in: " << parentPath << std::endl;

    std::function<void(const fs::path&, int)> searchDirectory = 
        [&](const fs::path& path, int depth) {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (maxDepth >= 0 && depth > maxDepth)    break;
                else if (entry.path() == currentFilePath) continue;

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

/**
 * Locate files with the extension .knot
 */
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

/**
 * First check if it is really encrypted by Knot. If so, remove it.
 */
void removeKnotFile(const std::string& filename) {
    if (isKnotEncryptedFile(filename)) {
        fs::remove(filename);
        std::cout << "Removed: " << filename << std::endl;
    } else {
        std::cout << "Skipped (not a Knot encrypted file): " << filename << std::endl;
    }
}