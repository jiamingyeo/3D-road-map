#pragma once

#include <GL/glew.h>
#include "entt/entt.hpp"
#include "MapRenderer.hpp"
#include "Window.hpp"
#include "RoadGraph.hpp"

struct RoadNodeData
{
    long long roadId;
    std::string highwayType;
    long long nodeId;
    double lat;
    double lon;
    int sequence;
};

struct BuildingNodeData
{
    long long buildingId;
    std::string buildingType;
    long long nodeId;
    double lat;
    double lon;
    int sequence;
};

class TestScene
{
public:
    void Load(
        GLuint shaderProgram,
        entt::registry& registry,
        Window& window
    );
    void Update(
        GLuint shaderProgram,
        entt::registry& registry,
        Window& window
    );
    void Refresh(CameraComponent& camera, const glm::ivec2& screenSize);
    void RefreshBuildings(CameraComponent& camera, const glm::ivec2& screenSize);
    void RefreshPOIs(CameraComponent& camera, const glm::ivec2& screenSize);
    const std::vector<Segment>& GetCachedSegments() const { return mCachedSegment; };
    int GetQueryCount() const { return mQueryCount; };
    int GetSegmentCount() const { return mSegmentCount; };
    int GetDrawnCount() const { return mDrawnCount; };
    int GetBuildingCount() const;
    int GetPOICount() const;
    void OnLeftClick(glm::vec2 mousepos, const glm::ivec2& screenSize, const CameraComponent& camera);

private:

    struct CacheBounds
    {
        double MinLon = 0.0;
        double MaxLon = 0.0;
        double MinLat = 0.0;
        double MaxLat = 0.0;
    } RoadBounds, BuildingBounds, POIBounds;

    std::vector<RoadNodeData> CallQuery(
        const std::string& dbPath,
        double minLon, double maxLon,
        double minLat, double maxLat
    );
    std::vector<BuildingNodeData> CallBuildingQuery(
        const std::string& dbPath,
        double minLon, double maxLon,
        double minLat, double maxLat
    );
    std::vector<POIData> CallPOIQuery(
        const std::string& dbPath,
        double minLon, double maxLon,
        double minLat, double maxLat
    );

    std::vector<Segment> NodesToSegments(const std::vector<RoadNodeData>& nodes);
    std::vector<BuildingPolygon> NodesToBuildings(const std::vector<BuildingNodeData>& nodes);
    bool is_CamBound_Outside_CacheBound(CacheBounds& cachebound);
    void UpdateCacheBound(CacheBounds& cachebound);
    void UpdateVisibleBounds(const CameraComponent& camera);

    bool mNeedRefresh = true;
    int mQueryCount = 0;
    int mSegmentCount = 0;

    double visibleMinLon = 0.0;
    double visibleMaxLon = 0.0;
    double visibleMinLat = 0.0;
    double visibleMaxLat = 0.0;

    double mLastAltitude = 0.0;
    int mLastLodLevel = 0;

    static constexpr double SHOW_BUILDINGS_ALTITUDE = 350.0;
    static constexpr double SHOW_POIS_ALTITUDE = 100.0;

    double mQueryMs = 0.0;
    double mBuildMs = 0.0;
    double mDrawnCount = 0.0;
    int mBuildingCount = 0;
    int mPOICount = 0;

    std::vector<Segment> mCachedSegment;

    const double CACHE_MARGIN = 0.2;

    std::string mDBPath = "../projects/jiaming.yeo-project-phase9/map_extended.db";
    //std::string mDBPath = "../projects/YiQian.Lim-project-phase8/map_extended.db";

    RoadGraph mRoadGraph;
    KDTree mKDTree;

    enum class RouteState
    {
        Idle,
        StartSelected,
        RouteDisplayed
    };

    RouteState mRouteState = RouteState::Idle;
    long long mStartNode = -1;
    long long mGoalNode = -1;
};



