#ifndef IMAGE_GENERATOR_H
#define IMAGE_GENERATOR_H

#include "xml_parser.h"
#include <string>
#include <vector>
#include <utility>
#include <ft2build.h>
#include FT_FREETYPE_H

class ImageGenerator {
public:
    
    ImageGenerator();
    ~ImageGenerator();
    
    bool generateImageFromXml(const XmlNode& rootNode, const std::string& outputPath);
    
private:
    int imageWidth_;
    int imageHeight_;
    FT_Library ftLibrary_;
    FT_Face ftFace_;
    
    bool initFreeType(); // Инициализация шрифта
    bool createFBImage(const XmlNode& rootNode, const std::string& outputPath); // Создание изображения
    void drawFBDiagram(const XmlNode& rootNode, unsigned char* image_data); // Отрисовка диаграммы
    void drawText(const std::string& text, unsigned char* image_data, int x, int y, 
                  unsigned char r, unsigned char g, unsigned char b, 
                  int fontSize = 10, bool italic = false, bool bold = false); // Отрисовка текста
    void drawLine(unsigned char* image_data, int x1, int y1, int x2, int y2, 
                  unsigned char r, unsigned char g, unsigned char b, int thickness = 1); // Отрисовка линий
    void drawRectangle(unsigned char* image_data, int x, int y, int width, int height,
                      unsigned char r, unsigned char g, unsigned char b, bool fill = false); // Отрисовка прямоугольника
    void drawSquare(unsigned char* image_data, int x, int y, int size,
                   unsigned char r, unsigned char g, unsigned char b, bool fill = false); // Отрисовка квадрата
    void drawTriangle(unsigned char* image_data, int x, int y, int size,
                     unsigned char r, unsigned char g, unsigned char b); // Отрисовка треугольника
    int getTextWidth(const std::string& text, int fontSize); // Ширина текста
};

#endif