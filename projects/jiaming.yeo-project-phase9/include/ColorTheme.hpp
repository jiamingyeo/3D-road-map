#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct RGB 
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    const float* data() const { return &r; }
    static RGB rgbFrom255(const json& arr) 
    {
        return { arr[0].get<int>() / 255.0f,
        arr[1].get<int>() / 255.0f,
        arr[2].get<int>() / 255.0f };
    }

    operator glm::vec3() const { return glm::vec3(r, g, b); }
};

struct ColorTheme 
{
    int id = 0;
    std::string name;
    std::string hotkey;
    std::string inspiredBy;

    RGB background; // sky
    RGB skyground;
    // Keyed by OSM type string: "motorway", "primary", "residential"
    std::unordered_map<std::string, RGB> roads;
    // Keys: "default", "roof", "commercial", "industrial", "residential", "school", "retail"
    std::unordered_map<std::string, RGB> buildings;
    // Keys: "default", "amenity", "shop", "leisure", "tourism"
    std::unordered_map<std::string, RGB> pois;
    RGB pathInner; // animated inner A* ribbon color
    RGB pathOutline; // outer border A* ribbon color
    float sunAmbient = 0.25f; // ambient strength for this theme's mood
    float sunElevation = 45.0f; // sun elevation angle (degrees)
    float sunAzimuth = 135.0f; // sun azimuth angle (degrees)
    RGB roadColor(const std::string& highway_type) const;
    RGB buildingWallColor(const std::string& building_type) const;
    RGB buildingRoofColor() const;
    RGB poiColor(const std::string& poi_type) const;
};