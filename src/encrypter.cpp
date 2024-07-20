/** ================================================================
| encrypter.cpp  --  encrypter.cpp
|
| Created by Jack on 07/14, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#include <filesystem>
#include <stdexcept>

#include "common.hpp"

void encryptFile(const std::string& filename, const std::string& password) {
    std::filesystem::path filePath = std::filesystem::absolute(filename);
    
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Unable to open input file: " + filePath.string());
    }

    std::filesystem::path outPath = filePath.parent_path() / (filePath.filename().string() + ".knot");
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to create output file: " + outPath.string());
    }
    
    // Write the Knot signature
    outFile.write(KNOT_SIGNATURE.data(), KNOT_SIGNATURE.size());

    char c;
    size_t i = 0;
    while (inFile.get(c)) {
        c ^= password[i % password.length()];
        outFile.put(c);
        i++;
    }
    
    if (inFile.bad()) {
        throw std::runtime_error("Error reading from file: " + filePath.string());
    }
    
    if (outFile.bad()) {
        throw std::runtime_error("Error writing to file: " + outPath.string());
    }

    inFile.close();
    outFile.close();
}


int main() {
    try {
        std::cout << "=== Parsing config ===" << std::endl;
        Config config = parseConfigFile("config.json");
        
        std::cout << "Target Extensions: ";
        for (const auto& ext : config.extensions) {
            std::cout << ext << " ";
        }
        std::cout << std::endl;

        std::vector<std::string> targetFiles = getTargetFiles(config);
        
        std::cout << "Target files to be encrypted:" << std::endl;
        for (const auto& file : targetFiles) {
            std::cout << file << std::endl;
        }
        
        std::string password = getPassword();
        
        for (const auto& file : targetFiles) {
            std::cout << "Processing file: " << file << std::endl;
            if (!isKnotEncryptedFile(file)) {
                try {
                    encryptFile(file, password);
                    std::cout << "Successfully encrypted: " << file << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error encrypting " << file << ": " << e.what() << std::endl;
                }
            } else {
                std::cout << "Skipped (already encrypted)..." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}