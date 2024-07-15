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

const std::array<char, 8> KNOT_SIGNATURE = {'K', 'N', 'O', 'T', 'E', 'N', 'C', '1'};

struct Config {
    std::vector<std::string> extensions;
    std::vector<std::string> specific_files;
    std::vector<std::string> skip_folders;
};


std::string getPassword() {
    std::string password;
    std::cout << "Enter password: ";
    std::cin >> password;
    return password;
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

    // Simple JSON parsing (without a 3rd-party library)
    size_t pos = 0;
    auto parseArray = [&](const std::string& key, std::vector<std::string>& vector) {
        pos = content.find("\"" + key + "\"", pos);
        if (pos == std::string::npos) return;
        pos = content.find('[', pos);
        if (pos == std::string::npos) return;
        size_t end = content.find(']', pos);
        if (end == std::string::npos) return;
        std::string arrayContent = content.substr(pos + 1, end - pos - 1);
        std::istringstream iss(arrayContent);
        std::string item;
        while (std::getline(iss, item, ',')) {
            item.erase(0, item.find_first_not_of(" \t\""));
            item.erase(item.find_last_not_of(" \t\"") + 1);
            vector.push_back(item);
        }
        pos = end;
    };

    parseArray("extensions", config.extensions);
    parseArray("specific_files", config.specific_files);
    parseArray("skip_folders", config.skip_folders);

    return config;
}


std::vector<std::string> getTargetFiles(const Config& config, int maxDepth = -1) {
    std::vector<std::string> targetFiles;
    
    auto options = (maxDepth < 0) ? std::filesystem::directory_options::none
                                  : std::filesystem::directory_options::follow_directory_symlink;
    
    std::filesystem::path parentPath = std::filesystem::current_path().parent_path();
    std::cout << "Searching for files in: " << parentPath << std::endl;
    
    // Process specific files
    for (const auto& file : config.specific_files) {
        std::filesystem::path filePath = std::filesystem::absolute(file);
        if (std::filesystem::is_regular_file(filePath)) {
            targetFiles.push_back(filePath.string());
            std::cout << "Added specific file: " << filePath << std::endl;
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
        
        // Check if the current path should be skipped
        bool skip = false;
        for (const auto& skip_folder : config.skip_folders) {
            if (it->path().string().find(skip_folder) != std::string::npos) {
                skip = true;
                it.disable_recursion_pending();
                break;
            }
        }
        if (skip) continue;
        
        if (it->is_regular_file()) {
            std::string extension = it->path().extension().string();
            // std::cout << "Found file: " << it->path().string() << " with extension: " << extension << std::endl;
            
            // Check if the extension (including the dot) is in the config
            for (const auto& target_ext : config.extensions) {
                if (extension == target_ext) {
                    targetFiles.push_back(it->path().string());
                    std::cout << "Added to target files: " << it->path().string() << std::endl;
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