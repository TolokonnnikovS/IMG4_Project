#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <string>
#include <vector>
#include <map>

struct XmlNode {
    std::string name;
    std::string value;
    std::map<std::string, std::string> attributes;
    std::vector<XmlNode> children;
};

class XmlParser {
public:
    XmlParser();
    ~XmlParser() = default;
    
    bool parseFile(const std::string& filePath);
    const XmlNode& getRootNode() const;
    void printTree() const;
    
private:
    XmlNode rootNode_;
    
    void printNode(const XmlNode& node, int depth = 0) const;
    void printParsingDebugInfo(const XmlNode& node, int depth = 0);
};

#endif