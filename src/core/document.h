#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace badcad {

// Simple in-memory document representation
// This will be saved to .bCAD files
class Document {
public:
    struct Plane {
        std::string name;
        bool visible;
        bool selected;
        
        Plane(const std::string& n, bool v = true) : name(n), visible(v), selected(false) {}
    };
    
    Document();
    ~Document();
    
    // Serialize to text format (for .bCAD file)
    std::string serialize() const;
    
    // Deserialize from text format
    bool deserialize(const std::string& content);
    
    // Getters
    const std::vector<Plane>& getPlanes() const { return m_planes; }
    
    // Plane management
    void setPlaneVisibility(const std::string& name, bool visible);
    bool isPlaneVisible(const std::string& name) const;
    
    // Selection management
    void selectPlane(const std::string& name, bool addToSelection = false);
    void deselectAll();
    bool isPlaneSelected(const std::string& name) const;
    
private:
    std::vector<Plane> m_planes;
    std::vector<std::string> m_sketches;
    std::vector<std::string> m_features;
};

} // namespace badcad
