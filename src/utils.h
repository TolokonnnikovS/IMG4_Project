#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils {
    // Ищет файлы с заданным расширением в указанной директории
    // Проверяет несколько возможных мест расположения файлов .fbt
    // Возвращает вектор с полными путями к найденным файлам
    std::vector<std::string> getFilesInDirectory(const std::string& directoryPath, const std::string& extension = ".xml");
    
    // Проверяет, существует ли файл по указанному пути
    bool fileExists(const std::string& filePath);
    
    // Извлекает имя файла без расширения из полного пути
    // Нужно для генерации имен выходных PNG-файлов
    std::string getFileNameWithoutExtension(const std::string& filePath);
    
    // Создает директорию, если она не существует
    void createDirectoryIfNotExists(const std::string& directoryPath);
}

#endif