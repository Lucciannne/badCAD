#include "document.h"
#include <iostream>

namespace badcad {

Document::Document() {
    // Initialize default construction planes
    m_planes.push_back(Plane("planexy", true));
    m_planes.push_back(Plane("planexz", true));
    m_planes.push_back(Plane("planeyz", true));
}

Document::~Document() {
}

std::string Document::serialize() const {
    std::stringstream ss;
    
    // Serialize planes
    for (const auto& plane : m_planes) {
        ss << plane.name << " " << (plane.visible ? "1" : "0") << "\n";
    }
    
    // Serialize sketches (TODO)
    for (const auto& sketch : m_sketches) {
        ss << "sketch " << sketch << "\n";
    }
    
    // Serialize features (TODO)
    for (const auto& feature : m_features) {
        ss << "feature " << feature << "\n";
    }
    
    return ss.str();
}

bool Document::deserialize(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    
    m_planes.clear();
    m_sketches.clear();
    m_features.clear();
    
    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string keyword;
        lineStream >> keyword;
        
        if (keyword == "planexy" || keyword == "planexz" || keyword == "planeyz") {
            int visible;
            lineStream >> visible;
            m_planes.push_back(Plane(keyword, visible == 1));
        } else if (keyword == "sketch") {
            std::string sketchData;
            std::getline(lineStream, sketchData);
            m_sketches.push_back(sketchData);
        } else if (keyword == "feature") {
            std::string featureData;
            std::getline(lineStream, featureData);
            m_features.push_back(featureData);
        }
    }
    
    return true;
}

void Document::setPlaneVisibility(const std::string& name, bool visible) {
    for (auto& plane : m_planes) {
        if (plane.name == name) {
            plane.visible = visible;
            break;
        }
    }
}

bool Document::isPlaneVisible(const std::string& name) const {
    for (const auto& plane : m_planes) {
        if (plane.name == name) {
            return plane.visible;
        }
    }
    return false;
}

void Document::selectPlane(const std::string& name, bool addToSelection) {
    // If not adding to selection, clear all selections first
    if (!addToSelection) {
        for (auto& plane : m_planes) {
            plane.selected = false;
        }
    }
    
    // Select the specified plane
    for (auto& plane : m_planes) {
        if (plane.name == name) {
            plane.selected = true;
            break;
        }
    }
}

void Document::deselectAll() {
    for (auto& plane : m_planes) {
        plane.selected = false;
    }
}

bool Document::isPlaneSelected(const std::string& name) const {
    for (const auto& plane : m_planes) {
        if (plane.name == name) {
            return plane.selected;
        }
    }
    return false;
}

} // namespace badcad
