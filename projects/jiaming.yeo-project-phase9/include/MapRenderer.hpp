#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Component/CameraComponent.hpp"
#include "buffer.hpp"
#include "ThemeManager.hpp"
#include "Window.hpp"
#include "RoadGraph.hpp"
#include "Skybox.hpp"

struct Segment
{
    double lon1, lat1;                // start coordinates
    double lon2, lat2;                // end coordinates
    std::string highwayType;          // e.g. "motorway", "residential"
};

struct RoadTypeGroup
{
    int offsetVertices;               // starting index in the vertex buffer
    int countVertices;                // number of vertices for this road type
    float r, g, b;                    // color for this road type
    std::string highwayType;
};

struct BuildingPolygon
{
    long long buildingId;
    std::string buildingType;
    std::vector<glm::vec2> vertices;  //store vertex data instead as later we might want to fill them
    // Vertices stored in map coordinates:
    // x = longitude
    // y = latitude
};

struct POIData
{
    long long poiId;
    std::string poiType;
    std::string poiValue;
    std::string name;
    double lat;
    double lon;
};


struct PoiRecord {
    float wx, wy;
    char  poi_type[32];
    char  poi_value[32];
    char  name[64];
    int   id;
};


class MapRenderer 
{
public:
    MapRenderer() = default;

    void Initialize(GLuint shaderProgram, const std::string& themePath);
    void BuildRoadGeometry(const std::vector<Segment>& segments);
    float RoadHalfWidth(const std::string& t);
    void BuildBuildingGeometry(std::vector<BuildingPolygon> const& buildings);
    void BuildPOIGeometry(std::vector<POIData> const& pois);
    void BuildRibbonGeometry(const std::vector<int>& path, const RoadGraph& graph);
    void ClearBuildingGeometry();
    void ClearPOIGeometry();
    void ClearRibbonGeometry();
    void Render(CameraComponent& camera, Window& window, float dt);
    void roadTypeColor(const std::string& t, float& r, float& g, float& b);
    bool IsTypeVisible(const std::string& highway, double zoom);
    static int GetLodLevelFromAltitude(double altitude);
    glm::vec3 buildingColor(const std::string& t);
    float buildingHeight(const std::string& t);
    void UpdateLightDirection();
    bool RoadsNeedRefresh() const { return mRoadsNeedRefresh; }
    void ResetRoadsRefresh() { mRoadsNeedRefresh = false; }
    bool BuildingsNeedRefresh() const { return mBuildingsNeedRefresh; }
    void ResetBuildingsRefresh() { mBuildingsNeedRefresh = false; }
    bool POIsNeedRefresh() const { return mPOIsNeedRefresh; }
    void ResetPoiRefresh() { mPOIsNeedRefresh = false; }
    static int highwayProminence(const std::string& hw);
    glm::vec3 poiCategoryColor(std::string poi_type, std::string poi_value);
private:
    std::unique_ptr<Buffer> mRoadBuffer;
    std::unique_ptr<Buffer> mBuildingBuffer;
    std::unique_ptr<Buffer> mPOIBuffer;
    std::vector<RoadTypeGroup> roadTypeGroups;
    std::unique_ptr<Buffer> mRibbonBuffer;
    GLuint mShaderProgram = 0;
    GLuint mRoadVertexCount = 0;
    GLuint mPOIVertexCount = 0;
    GLuint mRibbonVertexCount = 0;

    struct BuildingDraw
    {
        GLint first;
        GLsizei count;
        glm::vec3 colour;
    };
    std::vector<BuildingDraw> buildingDraws;
    GLuint mBuildingVertexCount = 0;

    GLint uViewProjLoc = -1;
    //GLuint uDrawColorLoc = -1;

    GLuint uLightDirLoc = -1;
    GLuint uLightColorLoc = -1;
    GLuint uAmbientLevelLoc = -1;
    //GLint uUseUniformColorLoc = -1;
    glm::vec3 mLightDirection = glm::normalize(glm::vec3(0.4f, 0.6f, 0.8f));
    glm::vec3 mLightColor = { 1.0f, 0.95f, 0.85f };
    float mSunAzimuth;
    float mSunElevation;
    float mAmbientLevel;

    ThemeManager mThemeManager;
    bool mRoadsNeedRefresh = false;
    bool mBuildingsNeedRefresh = false;
    bool mPOIsNeedRefresh = false;
    float SUN_ROT_SPEED = 30.0f;

    std::unique_ptr<Skybox> mSkybox;
    GLuint mSkyboxShader;

    // index into original WirePOI array
    std::vector<PoiRecord> poiRecords_;
    KDTree poiKDTree_;
};

MapRenderer& GetMapRenderer();