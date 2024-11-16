/** ================================================================
| cleaner.cpp  --  src/cleaner.cpp
|
| Created by Jack on 07/20, 2024
| Copyright © 2024 jacktogon. All rights reserved.
================================================================= */
#include "common.hpp"

int main() {
    try {
        std::vector<std::string> knotFiles = findKnotFiles();
        
        std::cout << "Found the following .knot files:" << std::endl;
        for (const auto& file : knotFiles) {
            std::cout << file << std::endl;
        }
        
        if (knotFiles.empty()) {
            std::cout << "No .knot files found." << std::endl;
            return 0;
        }

        std::string confirmation;
        std::cout << "Are you sure you want to remove these files? (yes/no): ";
        std::cin >> confirmation;
        toLower(confirmation);
        strip(confirmation);
        
        if (confirmation == "yes" || confirmation == "y") {
            for (const auto& file : knotFiles) {
                try {
                    removeKnotFile(file);
                } catch (const std::exception& e) {
                    std::cerr << "Error removing " << file << ": " << e.what() << std::endl;
                }
            }
            std::cout << "Removal process completed." << std::endl;
        } else {
            std::cout << "Operation cancelled." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}