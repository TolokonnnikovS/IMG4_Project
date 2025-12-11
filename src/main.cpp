#include <iostream>
#include <string>
#include "xml_parser.h"
#include "image_generator.h"
#include "utils.h"

int main() {
    // Вывод заголовка программы
    std::cout << "FBT to PNG Converter" << std::endl;
    std::cout << "====================" << std::endl;
    
    // Список возможных директорий для поиска входных файлов
    // Программа проверяет несколько стандартных расположений
    std::vector<std::string> possibleInputDirs = {
        "../xml",  // Директория на уровень выше
        "./xml",   // Директория в текущей папке
        "xml",     // Поддиректория xml
        "."        // Текущая директория
    };
    
    std::string inputDir;  // Найденная директория с файлами
    std::vector<std::string> files;  // Список найденных .fbt файлов
    
    // Поиск директории с файлами .fbt
    // Проверяем каждую возможную директорию по порядку
    for (const auto& dir : possibleInputDirs) {
        std::cout << "Checking directory: " << dir << std::endl;
        auto foundFiles = utils::getFilesInDirectory(dir, ".fbt");
        if (!foundFiles.empty()) {
            files = foundFiles;        // Сохраняем найденные файлы
            inputDir = dir;            // Запоминаем директорию
            break;                     // Прекращаем поиск после первой находки
        }
    }
    
    // Имя выходной директории для PNG файлов
    std::string outputDir = "xml_png";
    
    // Создаем выходную директорию, если она не существует
    utils::createDirectoryIfNotExists(outputDir);
    
    // Проверка: найдены ли файлы для обработки
    if (files.empty()) {
        std::cerr << "ERROR: No .fbt files found in any of the expected directories!" << std::endl;
        std::cerr << "Please make sure your .fbt files are in one of these locations:" << std::endl;
        for (const auto& dir : possibleInputDirs) {
            std::cerr << "  - " << dir << std::endl;
        }
        return 1;  // Возвращаем код ошибки
    }
    
    // Вывод информации о найденных файлах
    std::cout << "Found " << files.size() << " .fbt files in: " << inputDir << std::endl;
    for (const auto& file : files) {
        std::cout << "  - " << file << std::endl;  // Печать имени каждого файла
    }
    
    // Создание объектов для парсинга XML и генерации изображений
    XmlParser parser;            // Парсер XML файлов
    ImageGenerator generator;    // Генератор PNG изображений
    
    int successCount = 0;  // Счетчик успешно обработанных файлов
    int errorCount = 0;    // Счетчик файлов с ошибками
    
    // Основной цикл обработки файлов
    for (const auto& file : files) {
        std::cout << "\nProcessing: " << file << std::endl;
        
        // Проверка существования файла
        if (!utils::fileExists(file)) {
            std::cerr << "File does not exist: " << file << std::endl;
            errorCount++;  // Увеличиваем счетчик ошибок
            continue;      // Переходим к следующему файлу
        }
        
        // Парсинг XML файла
        if (parser.parseFile(file)) {
            // Получаем имя файла без расширения для создания выходного имени
            std::string baseName = utils::getFileNameWithoutExtension(file);
            // Формируем полный путь для выходного PNG файла
            std::string outputFile = outputDir + "/" + baseName + ".png";
            
            // Генерация PNG изображения из распарсенных данных
            if (generator.generateImageFromXml(parser.getRootNode(), outputFile)) {
                std::cout << "✓ Created: " << outputFile << std::endl;
                successCount++;  // Увеличиваем счетчик успехов
            } else {
                std::cerr << "✗ Failed to create image for: " << file << std::endl;
                errorCount++;  // Увеличиваем счетчик ошибок
            }
        } else {
            // Ошибка парсинга XML файла
            std::cerr << "✗ Failed to parse: " << file << std::endl;
            errorCount++;  // Увеличиваем счетчик ошибок
        }
    }
    
    // Вывод итоговой статистики обработки
    std::cout << "\n=== Conversion Summary ===" << std::endl;
    std::cout << "Success: " << successCount << " files" << std::endl;
    std::cout << "Errors: " << errorCount << " files" << std::endl;
    std::cout << "Total: " << files.size() << " files processed" << std::endl;
    
    // Возвращаем код возврата: 0 - если нет ошибок, 1 - если были ошибки
    return (errorCount > 0) ? 1 : 0;
}