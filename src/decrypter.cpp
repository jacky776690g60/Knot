/** ================================================================
| decrypter.cpp  --  decrypter.cpp
|
| Created by Jack on 07/14, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#include "common.hpp"

#ifdef _WIN32
#include <direct.h>
#endif

void decryptFile(const std::string& filename, const std::string& password) {
    std::filesystem::path filePath = std::filesystem::absolute(filename);
    if (!isKnotEncryptedFile(filename))
        throw std::runtime_error("Invalid file format: " + filePath.string());

    std::ifstream inFile(filePath, std::ios::binary);
    std::array<char, 8> signature;
    inFile.read(signature.data(), KNOT_SIGNATURE.size()); // Skip the signature bytes.
    

    std::vector<uint8_t> salt(SALT_SIZE);
    inFile.read(reinterpret_cast<char*>(salt.data()), SALT_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);
    inFile.read(reinterpret_cast<char*>(iv.data()), IV_SIZE);

    auto key = deriveKey(password, salt);

    std::filesystem::path outPath = filePath.parent_path() / filePath.stem();
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to create output file: " + outPath.string());
    }

    std::vector<uint8_t> buffer(1024);
    size_t position = 0;
    while (inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size()) || inFile.gcount() > 0) {
        size_t bytesRead = inFile.gcount();
        for (size_t i = 0; i < bytesRead; ++i) {
            buffer[i] ^= key[position % key.size()] ^ iv[position % iv.size()];
            position++;
        }
        outFile.write(reinterpret_cast<char*>(buffer.data()), bytesRead);
    }
    /** Ensure overall integrity at the end. */
    if (inFile.bad())  throw std::runtime_error("Error reading from file: " + filePath.string());
    if (outFile.bad()) throw std::runtime_error("Error writing to file: " + outPath.string());

    inFile.close();
    outFile.close();
}



int main() {
    try {
        std::vector<std::string> knotFiles;
        std::filesystem::path    startPath = std::filesystem::current_path().parent_path();
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(startPath)) {
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
            try {
                decryptFile(file, password);
                std::cout << "Successfully decrypted: " << file << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error decrypting " << file << ": " << e.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}