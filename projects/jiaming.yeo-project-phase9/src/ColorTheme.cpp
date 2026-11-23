#include "ColorTheme.hpp"

RGB ColorTheme::roadColor(const std::string& highway_type) const
{
    auto it = roads.find(highway_type);
    if (it != roads.end()) return it->second;
    static const std::unordered_map<std::string, std::string> fallbacks =
    {
    {"trunk", "motorway"},
    {"primary_link", "primary"},
    {"secondary_link", "secondary"},
    {"living_street", "residential"},
    {"service", "residential"},
    {"unclassified", "residential"},
    {"path", "footway"},
    {"steps", "footway"},
    };
    auto fb = fallbacks.find(highway_type);
    if (fb != fallbacks.end())
    {
        auto it2 = roads.find(fb->second);
        if (it2 != roads.end()) return it2->second;
    }
    auto def = roads.find("residential");
    return def != roads.end() ? def->second : RGB{ 0.78f, 0.78f, 0.78f };
}

RGB ColorTheme::buildingWallColor(const std::string& building_type) const
{
    auto it = buildings.find(building_type);
    if (it != buildings.end())
    {
        return it->second;
    }

    auto def = buildings.find("default");
    if (def != buildings.end())
    {
        return def->second;
    }

    return RGB(0.5f, 0.5f, 0.5f); // neutral grey
}

RGB ColorTheme::buildingRoofColor() const
{
    return RGB(0.5f, 0.5f, 0.5f); // neutral grey
}

RGB ColorTheme::poiColor(const std::string& poi_type) const
{
    auto it = pois.find(poi_type);
    if (it != pois.end())
    {
        return it->second;
    }

    auto def = pois.find("default");
    if (def != pois.end())
    {
        return def->second;
    }

    return RGB(0.5f, 0.5f, 0.5f); // neutral grey
}
