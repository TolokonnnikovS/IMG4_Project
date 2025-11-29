#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils {
    std::vector<std::string> getFilesInDirectory(const std::string& directoryPath, const std::string& extension = ".xml");
    bool fileExists(const std::string& filePath);
    std::string getFileNameWithoutExtension(const std::string& filePath);
    void createDirectoryIfNotExists(const std::string& directoryPath);
}

#endif