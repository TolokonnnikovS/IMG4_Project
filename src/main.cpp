#include <iostream>
#include <string>
#include "xml_parser.h"
#include "image_generator.h"
#include "utils.h"

int main() {
    std::cout << "FBT to PNG Converter" << std::endl;
    std::cout << "====================" << std::endl;
    
    // Пробуем несколько возможных путей
    std::vector<std::string> possibleInputDirs = {
        "../xml",
        "./xml", 
        "xml",
        "."
    };
    
    std::string inputDir;
    std::vector<std::string> files;
    
    // Ищем директорию с файлами
    for (const auto& dir : possibleInputDirs) {
        std::cout << "Checking directory: " << dir << std::endl;
        auto foundFiles = utils::getFilesInDirectory(dir, ".fbt");
        if (!foundFiles.empty()) {
            files = foundFiles;
            inputDir = dir;
            break;
        }
    }
    
    std::string outputDir = "xml_png";
    
    // Создаем выходную директорию
    utils::createDirectoryIfNotExists(outputDir);
    
    if (files.empty()) {
        std::cerr << "ERROR: No .fbt files found in any of the expected directories!" << std::endl;
        std::cerr << "Please make sure your .fbt files are in one of these locations:" << std::endl;
        for (const auto& dir : possibleInputDirs) {
            std::cerr << "  - " << dir << std::endl;
        }
        return 1;
    }
    
    std::cout << "Found " << files.size() << " .fbt files in: " << inputDir << std::endl;
    for (const auto& file : files) {
        std::cout << "  - " << file << std::endl;
    }
    
    XmlParser parser;
    ImageGenerator generator;
    
    int successCount = 0;
    int errorCount = 0;
    
    for (const auto& file : files) {
        std::cout << "\nProcessing: " << file << std::endl;
        
        if (!utils::fileExists(file)) {
            std::cerr << "File does not exist: " << file << std::endl;
            errorCount++;
            continue;
        }
        
        if (parser.parseFile(file)) {
            std::string baseName = utils::getFileNameWithoutExtension(file);
            std::string outputFile = outputDir + "/" + baseName + ".png";
            
            if (generator.generateImageFromXml(parser.getRootNode(), outputFile)) {
                std::cout << "✓ Created: " << outputFile << std::endl;
                successCount++;
            } else {
                std::cerr << "✗ Failed to create image for: " << file << std::endl;
                errorCount++;
            }
        } else {
            std::cerr << "✗ Failed to parse: " << file << std::endl;
            errorCount++;
        }
    }
    
    std::cout << "\n=== Conversion Summary ===" << std::endl;
    std::cout << "Success: " << successCount << " files" << std::endl;
    std::cout << "Errors: " << errorCount << " files" << std::endl;
    std::cout << "Total: " << files.size() << " files processed" << std::endl;
    
    return (errorCount > 0) ? 1 : 0;
}