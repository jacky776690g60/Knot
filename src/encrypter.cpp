/** ================================================================
| encrypter.cpp  --  encrypter.cpp
|
| Created by Jack on 07/14, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#include "common.hpp"

#ifdef _WIN32
#include <direct.h>
#endif

void encryptFile(const std::string& filename, const std::string& password) {
    std::cout << "Starting encryption of file: " << filename << std::endl;
    std::filesystem::path filePath = std::filesystem::absolute(filename);
    
    // std::cout << "Opening input file..." << std::endl;
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Unable to open input file: " + filePath.string());
    }

    // std::cout << "Creating output file..." << std::endl;
    std::filesystem::path outPath = filePath.parent_path() / (filePath.filename().string() + ".knot");
    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to create output file: " + outPath.string());
    }
    
    // std::cout << "Writing signature..." << std::endl;
    outFile.write(KNOT_SIGNATURE.data(), KNOT_SIGNATURE.size());

    // std::cout << "Generating salt and IV..." << std::endl;
    auto salt = generateRandomBytes(SALT_SIZE);
    outFile.write(reinterpret_cast<char*>(salt.data()), salt.size());

    auto iv = generateRandomBytes(IV_SIZE);
    outFile.write(reinterpret_cast<char*>(iv.data()), iv.size());

    // std::cout << "Deriving key..." << std::endl;
    auto key = deriveKey(password, salt);
    // std::cout << "Key derived. Key size: " << key.size() << std::endl;

    // std::cout << "Starting encryption loop..." << std::endl;
    std::vector<uint8_t> buffer(1024);
    size_t position = 0;
    while (inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size())) {
        size_t bytesRead = inFile.gcount();
        for (size_t i = 0; i < bytesRead; ++i) {
            buffer[i] ^= key[position % key.size()] ^ iv[position % iv.size()];
            position++;
        }
        outFile.write(reinterpret_cast<char*>(buffer.data()), bytesRead);
    }
    
    // std::cout << "Handling remaining bytes..." << std::endl;
    if (inFile.gcount() > 0) {
        size_t bytesRead = inFile.gcount();
        for (size_t i = 0; i < bytesRead; ++i) {
            buffer[i] ^= key[position % key.size()] ^ iv[position % iv.size()];
            position++;
        }
        outFile.write(reinterpret_cast<char*>(buffer.data()), bytesRead);
    }

    if (inFile.bad()) {
        throw std::runtime_error("Error reading from file: " + filePath.string());
    }
    
    if (outFile.bad()) {
        throw std::runtime_error("Error writing to file: " + outPath.string());
    }

    inFile.close();
    outFile.close();

    // Get the executable's directory
    std::filesystem::path executablePath = std::filesystem::current_path();
    // Create refs folder in the executable's directory if it doesn't exist
    std::filesystem::path refsPath = executablePath / "refs";
    
    #ifdef _WIN32
        _mkdir(refsPath.string().c_str());
    #else
        std::filesystem::create_directories(refsPath);
    #endif

    // Create empty file in refs folder
    std::filesystem::path emptyFilePath = refsPath / filePath.filename();
    std::ofstream emptyFile(emptyFilePath);
    if (!emptyFile) {
        throw std::runtime_error("Unable to create empty file: " + emptyFilePath.string());
    }

    int repetitions = 5;
    for (int i = 0; i < repetitions; ++i) {
        emptyFile << "reference" << (i < repetitions-1 ? "\n" : "");
    }
    emptyFile.close();

    if (!emptyFile) {
        throw std::runtime_error("Error writing to reference file: " + emptyFilePath.string());
    }
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
            try {
                encryptFile(file, password);
                std::cout << "Successfully encrypted: " << file << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error encrypting " << file << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error occurred while encrypting " << file << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}