#include "xml_parser.h"
#include "pugixml.hpp"
#include <iostream>

XmlParser::XmlParser() {}

// Рекурсивная функция для парсинга узла
XmlNode parseNodeRecursive(pugi::xml_node xmlNode) {
    XmlNode node;
    node.name = xmlNode.name();
    node.value = xmlNode.child_value();
    
    // Парсим атрибуты
    for (auto attr : xmlNode.attributes()) {
        node.attributes[attr.name()] = attr.value();
    }
    
    // Рекурсивно парсим дочерние узлы
    for (auto child : xmlNode.children()) {
        XmlNode childNode = parseNodeRecursive(child);
        node.children.push_back(childNode);
    }
    
    return node;
}

bool XmlParser::parseFile(const std::string& filePath) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filePath.c_str());
    
    if (!result) {
        std::cerr << "XML parsing error: " << result.description() << std::endl;
        return false;
    }
    
    // Очищаем предыдущее дерево
    rootNode_ = XmlNode();
    
    // Рекурсивно парсим XML с корневого элемента
    auto root = doc.document_element();
    if (root) {
        rootNode_ = parseNodeRecursive(root);
        std::cout << "DEBUG PARSER: Successfully parsed XML with root: " << rootNode_.name << std::endl;
        std::cout << "DEBUG PARSER: Root has " << rootNode_.children.size() << " direct children" << std::endl;
        
        // Отладочный вывод структуры
        printParsingDebugInfo(rootNode_);
    }
    
    return true;
}

// Вспомогательная функция для отладочного вывода структуры
void XmlParser::printParsingDebugInfo(const XmlNode& node, int depth) {
    std::string indent(depth * 2, ' ');
    std::cout << "DEBUG PARSER: " << indent << "Node: " << node.name;
    
    if (!node.attributes.empty()) {
        std::cout << " [";
        for (const auto& attr : node.attributes) {
            std::cout << attr.first << "=" << attr.second << " ";
        }
        std::cout << "]";
    }
    
    if (!node.value.empty()) {
        std::cout << " Value: " << node.value;
    }
    
    std::cout << " Children: " << node.children.size() << std::endl;
    
    for (const auto& child : node.children) {
        printParsingDebugInfo(child, depth + 1);
    }
}

const XmlNode& XmlParser::getRootNode() const {
    return rootNode_;
}

void XmlParser::printTree() const {
    std::cout << "XML Tree Structure:" << std::endl;
    printNode(rootNode_);
}

void XmlParser::printNode(const XmlNode& node, int depth) const {
    std::string indent(depth * 2, ' ');
    std::cout << indent << "Node: " << node.name << std::endl;
    
    if (!node.value.empty()) {
        std::cout << indent << "  Value: " << node.value << std::endl;
    }
    
    for (const auto& attr : node.attributes) {
        std::cout << indent << "  Attribute: " << attr.first << " = " << attr.second << std::endl;
    }
    
    for (const auto& child : node.children) {
        printNode(child, depth + 1);
    }
}