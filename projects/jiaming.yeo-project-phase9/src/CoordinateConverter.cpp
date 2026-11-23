#include "CoordinateConverter.hpp"
#include <glm/gtc/constants.hpp>
#include <cmath>


double CoordinateConverter::sRefLon = 103.905;
double CoordinateConverter::sRefLat = 1.405;
double CoordinateConverter::sMetersPerDegLon = 111320.0;
double CoordinateConverter::sMetersPerDegLat = 111320.0;

void CoordinateConverter::Initialize(double refLon, double refLat)
{
    sRefLon = refLon;
    sRefLat = refLat;
    sMetersPerDegLat = 111320.0;
    sMetersPerDegLon = sMetersPerDegLat * cos(glm::radians(refLat));
}

void CoordinateConverter::LonLatToMetric(float lon, float lat, float& wx, float& wy)
{
    wx = static_cast<float>((lon - sRefLon) * sMetersPerDegLon);
    wy = static_cast<float>((lat - sRefLat) * sMetersPerDegLat);
}

glm::vec2 CoordinateConverter::LonLatToMetric(float lon, float lat)
{
    float wx, wy;
    LonLatToMetric(lon, lat, wx, wy);
    return glm::vec2(wx, wy);
}

void CoordinateConverter::MetricToLonLat(float wx, float wy, float& lon, float& lat) 
{
    lon = static_cast<float>(sRefLon + (wx / sMetersPerDegLon));
    lat = static_cast<float>(sRefLat + (wy / sMetersPerDegLat));
}



glm::vec3 CoordinateConverter::ScreenToWorld(glm::vec2 mousePos, const glm::ivec2& screenSize, const CameraComponent& camera)
{
    // 1. Convert pixel to NDC (Normalized Device Coordinates, range -1..+1)
    float ndcX = (float)(mousePos.x / screenSize.x) * 2.0f - 1.0f;
    float ndcY = 1.0f - (float)(mousePos.y / screenSize.y) * 2.0f;   // y-axis flipped
    // 2. Unproject a point on the near plane back to world space
    glm::mat4 invVP = glm::inverse(camera.viewProjMatrix);
    glm::vec4 clipNear = { ndcX, ndcY, -1.0f, 1.0f };
    glm::vec4 worldNear = invVP * clipNear;
    worldNear /= worldNear.w;   // perspective divide
    // 3. Build a ray from camera position through that point
    glm::vec3 rayOrigin = camera.pos;
    glm::vec3 rayDir = glm::normalize(glm::vec3(worldNear) - rayOrigin);
    // 4. Intersect with z = 0 ground plane: rayOrigin.z + t * rayDir.z = 0
    float t = -rayOrigin.z / rayDir.z;
    return rayOrigin + t * rayDir;
}