#pragma once

#include <glm/glm.hpp>
#include <Component/CameraComponent.hpp>

class CoordinateConverter {
public:
    static void Initialize(double refLon, double refLat);

    static void LonLatToMetric(float lon, float lat, float& wx, float& wy);
    static glm::vec2 LonLatToMetric(float lon, float lat);
    static void MetricToLonLat(float wx, float wy, float& lon, float& lat);

    static double GetRefLon() { return sRefLon; }
    static double GetRefLat() { return sRefLat; }
    static double GetMetersPerDegLon() { return sMetersPerDegLon; }
    static double GetMetersPerDegLat() { return sMetersPerDegLat; }
    static glm::vec3 ScreenToWorld(glm::vec2 mousePos, const glm::ivec2& screenSize ,const CameraComponent & camera);
private:
    static double sRefLon;
    static double sRefLat;
    static double sMetersPerDegLon;
    static double sMetersPerDegLat;
};