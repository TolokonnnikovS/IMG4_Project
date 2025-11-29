#include "utils.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace utils {
    std::vector<std::string> getFilesInDirectory(const std::string& directoryPath, const std::string& extension) {
        std::vector<std::string> files;
        
        std::cout << "DEBUG: Looking for files in: " << directoryPath << std::endl;
        std::cout << "DEBUG: Looking for extension: " << extension << std::endl;
        
        // Проверяем существование директории
        if (!std::filesystem::exists(directoryPath)) {
            std::cerr << "Directory does not exist: " << directoryPath << std::endl;
            return files;
        }
        
        if (!std::filesystem::is_directory(directoryPath)) {
            std::cerr << "Path is not a directory: " << directoryPath << std::endl;
            return files;
        }
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    std::string fileExtension = entry.path().extension().string();
                    
                    // Сравниваем расширения без учета регистра
                    std::string extLower = extension;
                    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
                    std::transform(extLower.begin(), extLower.end(), extLower.begin(), ::tolower);
                    
                    if (fileExtension == extLower) {
                        files.push_back(entry.path().string());
                    }
                }
            }
            
            // Сортируем файлы по имени для консистентности
            std::sort(files.begin(), files.end());
            
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error accessing directory: " << directoryPath << " - " << e.what() << std::endl;
        }
        
        std::cout << "DEBUG: Total " << extension << " files found: " << files.size() << std::endl;
        return files;
    }

    bool fileExists(const std::string& filePath) {
        return std::filesystem::exists(filePath);
    }

    std::string getFileNameWithoutExtension(const std::string& filePath) {
        std::filesystem::path path(filePath);
        return path.stem().string();
    }

    void createDirectoryIfNotExists(const std::string& directoryPath) {
        if (!std::filesystem::exists(directoryPath)) {
            std::cout << "Creating directory: " << directoryPath << std::endl;
            std::filesystem::create_directories(directoryPath);
        }
    }
}