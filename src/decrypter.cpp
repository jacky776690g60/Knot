/** ================================================================
| decrypter.cpp  --  decrypter.cpp
|
| Created by Jack on 07/14, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#include "common.hpp"

void decryptFile(const std::string& filename, const std::string& password) {
    std::filesystem::path filePath = std::filesystem::absolute(filename);
    
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Unable to open input file: " + filePath.string());
    }

    // Skip the Knot signature
    inFile.seekg(KNOT_SIGNATURE.size());

    std::filesystem::path outPath = filePath.parent_path() / filePath.stem();
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to create output file: " + outPath.string());
    }

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
        std::vector<std::string> knotFiles;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path().parent_path())) {
            if (entry.is_regular_file() && entry.path().extension() == ".knot") {
                knotFiles.push_back(entry.path().string());
            }
        }

        std::cout << "Files to be decrypted:" << std::endl;
        for (const auto& file : knotFiles) {
            std::cout << file << std::endl;
        }
        
        std::string password = getPassword();
        
        for (const auto& file : knotFiles) {
            std::cout << "Processing file: " << file << std::endl;
            if (isKnotEncryptedFile(file)) {
                try {
                    decryptFile(file, password);
                    std::cout << "Successfully decrypted: " << file << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error decrypting " << file << ": " << e.what() << std::endl;
                }
            } else {
                std::cout << "Skipped (not a valid Knot encrypted file)..." << file << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}