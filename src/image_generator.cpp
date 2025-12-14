#include "image_generator.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <vector>
#include <cmath>

// Конструктор - инициализация размеров изображения и FreeType
ImageGenerator::ImageGenerator() : imageWidth_(800), imageHeight_(600), ftLibrary_(nullptr), ftFace_(nullptr) {
    if (!initFreeType()) {
        std::cerr << "Failed to initialize FreeType" << std::endl;
    }
}

// Деструктор - освобождение ресурсов FreeType
ImageGenerator::~ImageGenerator() {
    if (ftFace_) {
        FT_Done_Face(ftFace_);
    }
    if (ftLibrary_) {
        FT_Done_FreeType(ftLibrary_);
    }
}

// Инициализация библиотеки FreeType и загрузка шрифта
bool ImageGenerator::initFreeType() {
    if (FT_Init_FreeType(&ftLibrary_)) {
        std::cerr << "ERROR: Could not initialize FreeType library" << std::endl;
        return false;
    }

    // Список путей к шрифтам для разных операционных систем
    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/times.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/Library/Fonts/Arial.ttf",
        nullptr
    };

    // Попытка загрузить шрифт из списка
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        if (FT_New_Face(ftLibrary_, fontPaths[i], 0, &ftFace_) == 0) {
            std::cout << "Successfully loaded font: " << fontPaths[i] << std::endl;
            return true;
        }
    }

    std::cerr << "WARNING: Could not load any system font" << std::endl;
    return false;
}

bool ImageGenerator::generateImageFromXml(const XmlNode& rootNode, const std::string& outputPath) {
    return createFBImage(rootNode, outputPath);
}

// Создание PNG изображения функционального блока
bool ImageGenerator::createFBImage(const XmlNode& rootNode, const std::string& outputPath) {
    std::cout << "Creating Functional Block diagram: " << outputPath << std::endl;

    // 1. Создаем белый фон в памяти
    std::vector<unsigned char> image_data(imageWidth_ * imageHeight_ * 3);
    for (size_t i = 0; i < image_data.size(); i++) {
        image_data[i] = 255;
    }

    // 2. Отрисовка диаграммы функционального блока
    drawFBDiagram(rootNode, image_data.data());

    // 3. Сохраняем в PNG
    bool success = stbi_write_png( // 176 Строка stb_image_write.h
        outputPath.c_str(),     // имя файла
        imageWidth_,           // ширина
        imageHeight_,          // высота
        3,                     // RGB (3 компонента)
        image_data.data(),     // данные изображения
        imageWidth_ * 3        // stride = ширина * 3
    );

    if (success) {
        std::cout << "Successfully created: " << outputPath << std::endl;
        return true;
    } else {
        std::cerr << "Failed to create PNG: " << outputPath << std::endl;
        return false;
    }
}

// Отрисовка текста с использованием FreeType
void ImageGenerator::drawText(const std::string& text, unsigned char* image_data, int x, int y, 
                              unsigned char r, unsigned char g, unsigned char b, 
                              int fontSize, bool italic, bool bold) {
    if (!ftFace_) {
        return;
    }

    /* Устанавливам размер шрифта в пикселях (растровый размер),
     который будет использоваться для последующего рендеринга*/
    FT_Set_Pixel_Sizes(ftFace_, 0, fontSize);

    // Применение курсивного наклона
    if (italic) {
        FT_Matrix italic_matrix;
        italic_matrix.xx = 0x10000L;
        italic_matrix.xy = 0x06000L;
        italic_matrix.yx = 0x00000L;
        italic_matrix.yy = 0x10000L;
        FT_Set_Transform(ftFace_, &italic_matrix, nullptr);
    } else {
        FT_Set_Transform(ftFace_, nullptr, nullptr);
    }

    // Эффект жирного шрифта через многократную отрисовку
    if (bold) { // Если требуется жирный шрифт
        int pen_x = x; // Начальная позиция "пера" (курсора) по горизонтали
        
        // Цикл по всем символам строки
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i]; // Текущий символ
            
            // Загружаем и рендерим символ в битмап
            // FT_LOAD_RENDER означает: загрузить + сразу растеризовать
            if (FT_Load_Char(ftFace_, c, FT_LOAD_RENDER)) {
                continue; // Если ошибка загрузки, пропускаем этот символ
            }

            // Получаем указатель на растровое изображение символа
            FT_Bitmap* bitmap = &ftFace_->glyph->bitmap;
            
            // Рисуем с смещениями для жирного эффекта
            // Жирность достигается многократной отрисовкой со смещениями
            for (int offset_x = -1; offset_x <= 1; offset_x++) { // Смещение по X: -1, 0, 1
                for (int offset_y = -1; offset_y <= 1; offset_y++) { // Смещение по Y: -1, 0, 1
                    if (offset_x == 0 && offset_y == 0) continue; // Пропускаем центральную позицию
                    // Отрисовываем 8 раз вокруг центра (все комбинации кроме 0,0)
                    
                    // Проходим по всем строкам (пикселям по вертикали) битмапа
                    for (unsigned int row = 0; row < bitmap->rows; ++row) {
                        // Проходим по всем столбцам (пикселям по горизонтали) битмапа
                        for (unsigned int col = 0; col < bitmap->width; ++col) {
                            // Получаем значение альфа-канала (прозрачности) пикселя
                            // pitch - количество байт от начала одной строки до начала следующей
                            unsigned char alpha = bitmap->buffer[row * bitmap->pitch + col];
                            
                            // Если пиксель не полностью прозрачный
                            if (alpha > 0) {
                                // Вычисляем координаты пикселя на целевом изображении
                                // pen_x - текущая позиция базовой линии
                                // bitmap_left - левый отступ глифа (может быть отрицательным для букв типа 'j')
                                // col - текущий столбец в битмапе
                                // offset_x - смещение для жирности
                                int px = pen_x + ftFace_->glyph->bitmap_left + col + offset_x;
                                
                                // y - базовая линия текста
                                // bitmap_top - верхний отступ глифа (расстояние от базовой линии до верха)
                                // row - текущая строка в битмапе
                                // fontSize/2 - эмпирическая коррекция (автор обнаружил, что текст рисуется высоко)
                                // offset_y - смещение для жирности
                                int py = y - ftFace_->glyph->bitmap_top + row + fontSize / 2 + offset_y;
                                
                                // Проверяем, не выходит ли пиксель за границы изображения
                                if (px >= 0 && px < imageWidth_ && py >= 0 && py < imageHeight_) {
                                    // Вычисляем индекс в массиве image_data
                                    // Каждый пиксель занимает 3 байта (RGB)
                                    int index = (py * imageWidth_ + px) * 3;
                                    
                                    // Преобразуем альфа-значение (0-255) в коэффициент смешивания (0.0-1.0)
                                    float blend = alpha / 255.0f;
                                    
                                    // Смешиваем цвет текста с существующим цветом фона (альфа-блендинг)
                                    // Формула: новый_цвет = (1 - alpha) * старый_цвет + alpha * цвет_текста
                                    image_data[index] = static_cast<unsigned char>(
                                        (1 - blend) * image_data[index] + blend * r);
                                    image_data[index + 1] = static_cast<unsigned char>(
                                        (1 - blend) * image_data[index + 1] + blend * g);
                                    image_data[index + 2] = static_cast<unsigned char>(
                                        (1 - blend) * image_data[index + 2] + blend * b);
                                }
                            }
                        }
                    }
                }
            }

            // Сдвигаем "перо" для следующего символа
            // advance.x - расстояние до следующего символа в формате 26.6 фиксированной точки
            // >> 6 - сдвиг вправо на 6 битов = деление на 64 (преобразование в пиксели)
            pen_x += (ftFace_->glyph->advance.x >> 6);
        }
    }

    // Основная отрисовка текста (выполняется всегда, независимо от bold)
    // Если bold=true, это центральная отрисовка поверх "тени"
    // Если bold=false, это единственная отрисовка
    int pen_x = x; // Начальная позиция "пера" (сбрасываем для новой отрисовки)
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i]; // Текущий символ
        
        // Загружаем и рендерим символ
        if (FT_Load_Char(ftFace_, c, FT_LOAD_RENDER)) {
            continue; // Пропускаем при ошибке
        }

        // Получаем битмап символа
        FT_Bitmap* bitmap = &ftFace_->glyph->bitmap;
        
        // Проходим по всем пикселям битмапа
        for (unsigned int row = 0; row < bitmap->rows; ++row) {
            for (unsigned int col = 0; col < bitmap->width; ++col) {
                unsigned char alpha = bitmap->buffer[row * bitmap->pitch + col];
                if (alpha > 0) { // Если пиксель не прозрачный
                    // Координаты БЕЗ смещений (основная отрисовка)
                    int px = pen_x + ftFace_->glyph->bitmap_left + col; // Без offset_x
                    int py = y - ftFace_->glyph->bitmap_top + row + fontSize / 2; // Без offset_y
                    
                    // Проверка границ
                    if (px >= 0 && px < imageWidth_ && py >= 0 && py < imageHeight_) {
                        int index = (py * imageWidth_ + px) * 3;
                        float blend = alpha / 255.0f;
                        
                        // Альфа-блендинг
                        image_data[index] = static_cast<unsigned char>(
                            (1 - blend) * image_data[index] + blend * r);
                        image_data[index + 1] = static_cast<unsigned char>(
                            (1 - blend) * image_data[index + 1] + blend * g);
                        image_data[index + 2] = static_cast<unsigned char>(
                            (1 - blend) * image_data[index + 2] + blend * b);
                    }
                }
            }
        }

        // Сдвигаем перо для следующего символа
        pen_x += (ftFace_->glyph->advance.x >> 6);
    }

    // Сбрасываем матрицу преобразования (курсив) в исходное состояние
    // nullptr в качестве матрицы означает единичную матрицу [1 0; 0 1]
    // Важно: не оставлять преобразование активным для последующих операций
    FT_Set_Transform(ftFace_, nullptr, nullptr);
}
// Вычисление ширины текста в пикселях
int ImageGenerator::getTextWidth(const std::string& text, int fontSize) {
    if (!ftFace_) {
        return text.length() * fontSize * 0.6;
    }

    FT_Set_Pixel_Sizes(ftFace_, 0, fontSize);
    
    int width = 0;
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        if (FT_Load_Char(ftFace_, c, FT_LOAD_DEFAULT)) {
            continue;
        }
        width += ftFace_->glyph->advance.x >> 6;
    }
    
    return width;
}

// Отрисовка квадрата
void ImageGenerator::drawSquare(unsigned char* image_data, int x, int y, int size,
                               unsigned char r, unsigned char g, unsigned char b, bool fill) {
    drawRectangle(image_data, x - size/2, y - size/2, size, size, r, g, b, fill);
}

// Отрисовка треугольника (только направленного вправо)
void ImageGenerator::drawTriangle(unsigned char* image_data, int x, int y, int size,
                                unsigned char r, unsigned char g, unsigned char b) {
    for (int py = y - size/2; py <= y + size/2; py++) {
        for (int px = x - size/2; px <= x + size/2; px++) {
            int dx = px - x;
            int dy = py - y;
            // Проверка принадлежности точки треугольнику
            if (dx >= -size/2 && dx <= size/2 && 
                std::abs(dy) <= size/2 - std::abs(dx) && 
                dx >= 0) {
                if (px >= 0 && px < imageWidth_ && py >= 0 && py < imageHeight_) {
                    int index = (py * imageWidth_ + px) * 3;
                    image_data[index] = r;
                    image_data[index + 1] = g;
                    image_data[index + 2] = b;
                }
            }
        }
    }
}

// Основная функция отрисовки диаграммы функционального блока
void ImageGenerator::drawFBDiagram(const XmlNode& rootNode, unsigned char* image_data) {
    std::string fbName = "Unknown";
    auto nameIt = rootNode.attributes.find("Name");
    if (nameIt != rootNode.attributes.end()) {
        fbName = nameIt->second;
    }

    std::string version = "1.0";
    for (const auto& child : rootNode.children) {
        if (child.name == "VersionInfo") {
            auto versionIt = child.attributes.find("Version");
            if (versionIt != child.attributes.end()) {
                version = versionIt->second;
            }
        }
    }

    std::cout << "DEBUG: Drawing FB: " << fbName << " Version: " << version << std::endl;

    // Сбор информации о интерфейсах
    std::vector<std::string> eventInputs;
    std::vector<std::string> eventOutputs;
    std::vector<std::pair<std::string, std::string>> inputVars;
    std::vector<std::pair<std::string, std::string>> outputVars;
    
    // Парсинг XML для извлечения информации об интерфейсах
    for (const auto& child : rootNode.children) {
        if (child.name == "InterfaceList") {
            for (const auto& interfaceChild : child.children) {
                if (interfaceChild.name == "EventInputs") {
                    for (const auto& event : interfaceChild.children) {
                        if (event.name == "Event") {
                            auto eventNameIt = event.attributes.find("Name");
                            std::string eventName = (eventNameIt != event.attributes.end()) ? eventNameIt->second : "Unnamed";
                            eventInputs.push_back(eventName);
                        }
                    }
                } else if (interfaceChild.name == "EventOutputs") {
                    for (const auto& event : interfaceChild.children) {
                        if (event.name == "Event") {
                            auto eventNameIt = event.attributes.find("Name");
                            std::string eventName = (eventNameIt != event.attributes.end()) ? eventNameIt->second : "Unnamed";
                            eventOutputs.push_back(eventName);
                        }
                    }
                } else if (interfaceChild.name == "InputVars") {
                    for (const auto& var : interfaceChild.children) {
                        if (var.name == "VarDeclaration") {
                            auto nameIt = var.attributes.find("Name");
                            auto typeIt = var.attributes.find("Type");
                            std::string varName = (nameIt != var.attributes.end()) ? nameIt->second : "Unnamed";
                            std::string varType = (typeIt != var.attributes.end()) ? typeIt->second : "Unknown";
                            inputVars.push_back({varName, varType});
                        }
                    }
                } else if (interfaceChild.name == "OutputVars") {
                    for (const auto& var : interfaceChild.children) {
                        if (var.name == "VarDeclaration") {
                            auto nameIt = var.attributes.find("Name");
                            auto typeIt = var.attributes.find("Type");
                            std::string varName = (nameIt != var.attributes.end()) ? nameIt->second : "Unnamed";
                            std::string varType = (typeIt != var.attributes.end()) ? typeIt->second : "Unknown";
                            outputVars.push_back({varName, varType});
                        }
                    }
                }
            }
        }
    }

    // Расчет размеров текста
    int nameWidth = getTextWidth(fbName, 12);
    int versionWidth = getTextWidth("v" + version, 8);
    
    // Расчет размеров основного блока
    int maxEvents = std::max(eventInputs.size(), eventOutputs.size());
    int maxVars = std::max(inputVars.size(), outputVars.size());
    
    int mainBlockWidth = std::max(200, std::max(nameWidth, versionWidth) + 40);
    int mainBlockHeight = 80 + (maxEvents * 25) + (maxVars * 20);
    
    // Минимальные размеры блока
    mainBlockWidth = std::max(mainBlockWidth, 200);
    mainBlockHeight = std::max(mainBlockHeight, 100);

    // Центрирование блока
    int mainBlockX = (imageWidth_ - mainBlockWidth) / 2;
    int mainBlockY = (imageHeight_ - mainBlockHeight) / 2;

    // Отрисовка основного прямоугольника
    drawRectangle(image_data, mainBlockX, mainBlockY, mainBlockWidth, mainBlockHeight, 0, 0, 0);
    
    // Отрисовка названия функционального блока
    drawText(fbName, image_data, mainBlockX + mainBlockWidth/2 - nameWidth/2, 
             mainBlockY + mainBlockHeight/2 - 8, 0, 0, 0, 12, true);
    
    // Отрисовка версии
    drawText("v" + version, image_data, mainBlockX + mainBlockWidth/2 - versionWidth/2, 
             mainBlockY + mainBlockHeight/2 + 8, 0, 0, 0, 8, false);

    // Координата для квадратиков слева
    int squareX = mainBlockX - 15;

    // Отрисовка входных событий (левая сторона)
    int eventInputY = mainBlockY + 25;
    for (size_t i = 0; i < eventInputs.size(); i++) {
        int currentY = eventInputY + i * 22;
        
        // Квадратик
        drawSquare(image_data, squareX, currentY, 8, 0, 0, 0, false);
        
        // Линия от квадратика к блоку
        drawLine(image_data, squareX, currentY, mainBlockX, currentY, 0, 0, 0, 1);
        
        // Линия наружу
        drawLine(image_data, mainBlockX - 30, currentY, squareX, currentY, 0, 0, 0, 1);
        
        // Зеленый треугольник для первой линии
        if (i == 0) {
            drawTriangle(image_data, mainBlockX, currentY, 10, 0, 255, 0);
        }
        
        // Текст "Event"
        drawText("Event", image_data, mainBlockX - 70, currentY - 4, 0, 0, 0, 8, false);
        
        // Название события
        drawText(eventInputs[i], image_data, mainBlockX + 8, currentY - 4, 0, 0, 0, 9, false);
    }

    // Отрисовка выходных событий (правая сторона)
    int eventOutputY = mainBlockY + 25;
    for (size_t i = 0; i < eventOutputs.size(); i++) {
        int currentY = eventOutputY + i * 22;
        
        // Зеленый треугольник для первой линии
        if (i == 0) {
            drawTriangle(image_data, mainBlockX + mainBlockWidth - 5, currentY, 10, 0, 255, 0);
        }
        
        // Линия наружу
        drawLine(image_data, mainBlockX + mainBlockWidth, currentY, mainBlockX + mainBlockWidth + 30, currentY, 0, 0, 0, 1);
        
        // Текст "Event"
        drawText("Event", image_data, mainBlockX + mainBlockWidth + 35, currentY - 4, 0, 0, 0, 8, false);
        
        // Название события
        drawText(eventOutputs[i], image_data, mainBlockX + mainBlockWidth - 40, currentY - 4, 0, 0, 0, 9, false);
    }

    // Отрисовка входных переменных (левая сторона)
    int inputStartY = mainBlockY + mainBlockHeight/2 + 20;
    for (size_t i = 0; i < inputVars.size(); i++) {
        int yPos = inputStartY + i * 18;
        
        // Квадратик
        drawSquare(image_data, squareX, yPos, 8, 0, 0, 0, false);
        
        // Линия от квадратика к блоку
        drawLine(image_data, squareX, yPos, mainBlockX, yPos, 0, 0, 0, 1);
        
        // Линия наружу
        drawLine(image_data, mainBlockX - 45, yPos, squareX, yPos, 0, 0, 0, 1);
        
        // Синий треугольник
        drawTriangle(image_data, mainBlockX, yPos, 8, 0, 0, 255);
        
        // Имя переменной
        drawText(inputVars[i].first, image_data, mainBlockX + 8, yPos - 4, 0, 0, 0, 9, false);
        
        // Тип переменной
        drawText(inputVars[i].second, image_data, mainBlockX - 110, yPos - 4, 0, 0, 0, 7, false);
    }

    // Отрисовка выходных переменных (правая сторона)
    int outputStartY = mainBlockY + mainBlockHeight/2 + 20;
    for (size_t i = 0; i < outputVars.size(); i++) {
        int yPos = outputStartY + i * 18;
        
        // Синий треугольник
        drawTriangle(image_data, mainBlockX + mainBlockWidth - 5, yPos, 8, 0, 0, 255);
        
        // Линия наружу
        drawLine(image_data, mainBlockX + mainBlockWidth, yPos, mainBlockX + mainBlockWidth + 45, yPos, 0, 0, 0, 1);
        
        // Имя переменной
        drawText(outputVars[i].first, image_data, mainBlockX + mainBlockWidth - 40, yPos - 4, 0, 0, 0, 9, false);
        
        // Тип переменной
        drawText(outputVars[i].second, image_data, mainBlockX + mainBlockWidth + 50, yPos - 4, 0, 0, 0, 7, false);
    }

    // Отрисовка вертикальной линии, соединяющей квадратики
    if (!eventInputs.empty() || !inputVars.empty()) {
        int firstLineY = 0;
        int lastLineY = 0;
        
        // Определение первой линии
        if (!eventInputs.empty()) {
            firstLineY = mainBlockY + 25;
        } else if (!inputVars.empty()) {
            firstLineY = mainBlockY + mainBlockHeight/2 + 20;
        }
        
        // Определение последней линии
        if (!inputVars.empty()) {
            lastLineY = mainBlockY + mainBlockHeight/2 + 20 + (inputVars.size() - 1) * 18;
        } else if (!eventInputs.empty()) {
            lastLineY = mainBlockY + 25 + (eventInputs.size() - 1) * 22;
        }
        
        // Отрисовка вертикальной линии
        if (firstLineY > 0 && lastLineY > 0 && firstLineY != lastLineY) {
            drawLine(image_data, squareX, firstLineY, squareX, lastLineY, 0, 0, 0, 1);
            
            // Квадратики на концах
            drawSquare(image_data, squareX, firstLineY, 8, 0, 0, 0, false);
            drawSquare(image_data, squareX, lastLineY, 8, 0, 0, 0, false);
        }
    }
}

// Отрисовка линии алгоритмом Брезенхэма
void ImageGenerator::drawLine(unsigned char* image_data, int x1, int y1, int x2, int y2, 
                              unsigned char r, unsigned char g, unsigned char b, int thickness) {
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // Отрисовка пикселей с учетом толщины
        for (int tx = -thickness/2; tx <= thickness/2; tx++) {
            for (int ty = -thickness/2; ty <= thickness/2; ty++) {
                int px = x1 + tx;
                int py = y1 + ty;
                if (px >= 0 && px < imageWidth_ && py >= 0 && py < imageHeight_) {
                    int index = (py * imageWidth_ + px) * 3;
                    image_data[index] = r;
                    image_data[index + 1] = g;
                    image_data[index + 2] = b;
                }
            }
        }

        if (x1 == x2 && y1 == y2) break;
        
        // Коррекция ошибки алгоритма Брезенхэма
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Отрисовка прямоугольника
void ImageGenerator::drawRectangle(unsigned char* image_data, int x, int y, int width, int height,
                                  unsigned char r, unsigned char g, unsigned char b, bool fill) {
    if (fill) {
        // Заливка прямоугольника
        for (int py = y; py < y + height; py++) {
            for (int px = x; px < x + width; px++) {
                if (px >= 0 && px < imageWidth_ && py >= 0 && py < imageHeight_) {
                    int index = (py * imageWidth_ + px) * 3;
                    image_data[index] = r;
                    image_data[index + 1] = g;
                    image_data[index + 2] = b;
                }
            }
        }
    } else {
        // Отрисовка контура
        // Верхняя и нижняя границы
        for (int px = x; px < x + width; px++) {
            if (px >= 0 && px < imageWidth_ && y >= 0 && y < imageHeight_) {
                int index = (y * imageWidth_ + px) * 3;
                image_data[index] = r;
                image_data[index + 1] = g;
                image_data[index + 2] = b;
            }
            if (px >= 0 && px < imageWidth_ && (y + height - 1) >= 0 && (y + height - 1) < imageHeight_) {
                int index = ((y + height - 1) * imageWidth_ + px) * 3;
                image_data[index] = r;
                image_data[index + 1] = g;
                image_data[index + 2] = b;
            }
        }
        
        // Левая и правая границы
        for (int py = y; py < y + height; py++) {
            if (x >= 0 && x < imageWidth_ && py >= 0 && py < imageHeight_) {
                int index = (py * imageWidth_ + x) * 3;
                image_data[index] = r;
                image_data[index + 1] = g;
                image_data[index + 2] = b;
            }
            if ((x + width - 1) >= 0 && (x + width - 1) < imageWidth_ && py >= 0 && py < imageHeight_) {
                int index = (py * imageWidth_ + (x + width - 1)) * 3;
                image_data[index] = r;
                image_data[index + 1] = g;
                image_data[index + 2] = b;
            }
        }
    }
}