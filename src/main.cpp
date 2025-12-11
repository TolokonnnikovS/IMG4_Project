#include <iostream>
#include <string>
#include "xml_parser.h"
#include "image_generator.h"
#include "utils.h"
#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    // 1. Создаем парсер аргументов командной строки
    argparse::ArgumentParser program("fbt_to_png", "1.0");
    
    // 2. Добавляем описание программы (будет показано в справке)
    program.add_description("Конвертер FBT-файлов в PNG-изображения функциональных блоков IEC 61499");
    
    // 3. Добавляем аргументы
    program.add_argument("-i", "--input")
        .help("директория с входными .fbt файлами (по умолчанию: xml)")
        .default_value(std::string("xml"))
        .metavar("DIR");
    
    program.add_argument("-o", "--output")
        .help("директория для выходных .png файлов (по умолчанию: xml_png)")
        .default_value(std::string("xml_png"))
        .metavar("DIR");

    try {
        // 4. Парсим аргументы командной строки
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        // 5. Ошибка парсинга (например, пользователь ввел неверный аргумент)
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    
    std::string inputDir = program.get<std::string>("--input");
    std::string outputDir = program.get<std::string>("--output");
    
    std::cout << "FBT to PNG Converter" << std::endl;
    std::cout << "====================" << std::endl;
    
    // Пробуем несколько возможных путей
    std::vector<std::string> possibleInputDirs = {
        "../" + inputDir,
        "./" + inputDir, 
        inputDir,
        "."
    };
    
    std::string foundInputDir;
    std::vector<std::string> files;
    
    // Ищем директорию с файлами
    for (const auto& dir : possibleInputDirs) {
        std::cout << "Checking directory: " << dir << std::endl;
        auto foundFiles = utils::getFilesInDirectory(dir, ".fbt");
        if (!foundFiles.empty()) {
            files = foundFiles;
            foundInputDir = dir;
            break;
        }
    }
    
    // Создаем выходную директорию
    utils::createDirectoryIfNotExists(outputDir);
    
    if (files.empty()) {
        std::cerr << "ERROR: No .fbt files found in any of the expected directories!" << std::endl;
        std::cerr << "Please make sure your .fbt files are in one of these locations:" << std::endl;
        for (const auto& dir : possibleInputDirs) {
            std::cerr << "  - " << dir << std::endl;
        }
        std::cerr << "\nИли укажите правильную директорию с помощью --input" << std::endl;
        return 1;
    }
    
    std::cout << "Found " << files.size() << " .fbt files in: " << foundInputDir << std::endl;
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
                std::cout << "[OK] Created: " << outputFile << std::endl;
                successCount++;
            } else {
                std::cerr << "[ERROR] Failed to create image for: " << file << std::endl;
                errorCount++;
            }
        } else {
            std::cerr << "[ERROR] Failed to parse: " << file << std::endl;
            errorCount++;
        }
    }
    
    std::cout << "\n=== Conversion Summary ===" << std::endl;
    std::cout << "Success: " << successCount << " files" << std::endl;
    std::cout << "Errors: " << errorCount << " files" << std::endl;
    std::cout << "Total: " << files.size() << " files processed" << std::endl;
    std::cout << "Output directory: " << outputDir << std::endl;
    
    return (errorCount > 0) ? 1 : 0;
}