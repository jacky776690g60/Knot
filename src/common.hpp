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

#include "JsonParser.hpp"

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


std::string getPassword() {
    std::string password;
    std::cout << "Enter password: ";
    std::cin  >> password;
    return password;
}



bool matchesWildcard(const std::string& text, const std::string& pattern) {
    std::string regexPattern = std::regex_replace(pattern, std::regex("\\*"), ".*");
    regexPattern = "^" + regexPattern + "$";
    return std::regex_match(text, std::regex(regexPattern));
}



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
    
    auto options = (maxDepth < 0) ? std::filesystem::directory_options::none
                                  : std::filesystem::directory_options::follow_directory_symlink;
    
    std::filesystem::path parentPath = std::filesystem::current_path().parent_path();
    std::cout << "Searching for files in: " << parentPath << std::endl;
    

    for (const auto& file : config.specific_files) {
        std::filesystem::path filePath = std::filesystem::absolute(file);
        if (std::filesystem::is_regular_file(filePath)) {
            targetFiles.push_back(filePath.string());
            std::cout << "Found specific file: " << filePath << std::endl;
        } else {
            std::cout << "Skipping non-regular file: " << filePath << std::endl;
        }
    }
    
    for (auto it = std::filesystem::recursive_directory_iterator(parentPath, options);
         it != std::filesystem::recursive_directory_iterator();
         ++it) {
        
        if (maxDepth >= 0 && it.depth() > maxDepth) {
            it.disable_recursion_pending();
            continue;
        }
        
        
        
        // In getTargetFiles function:
        bool skip = false;
        std::filesystem::path current_path = it->path();
        for (const auto& skip_pattern : config.skip_folders) {
            if (matchesWildcard(current_path.string(), skip_pattern)) {
                skip = true;
                it.disable_recursion_pending();
                break;
            }
        }
        if (skip) continue;


        
        if (it->is_regular_file()) {
            std::string extension = it->path().extension().string();
            
            // Check if the extension (including the dot) is in the config
            for (const auto& target_ext : config.extensions) {
                if (extension == target_ext) {
                    targetFiles.push_back(it->path().string());
                    // std::cout << "Added to target files: " << it->path().string() << std::endl;
                    break;
                }
            }
        }
    }
    
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


std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}