#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <array>
#include <memory>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sqlite3.h>

#include "TestScene.hpp"
#include "Vertex.hpp"
#include "Component/CameraComponent.hpp"
#include "CoordinateConverter.hpp"

std::vector<RoadNodeData> TestScene::CallQuery(
	const std::string& dbPath,
	double minLon,
	double maxLon,
	double minLat,
	double maxLat
)
{
	std::vector<RoadNodeData> nodes;

	sqlite3* db = nullptr;
	int rc = sqlite3_open(dbPath.c_str(), &db);

	if (rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return nodes;
	}

	std::string sql =
		"SELECT r.road_id, r.highway_type, n.node_id, n.lat, n.lon, rn.sequence "
		"FROM nodes_rtree AS idx "
		"JOIN nodes n ON n.node_id = idx.node_id "
		"JOIN road_nodes rn ON rn.node_id = n.node_id "
		"JOIN roads r ON r.road_id = rn.road_id "
		"WHERE idx.min_lon >= ? AND idx.max_lon <= ? "
		"AND idx.min_lat >= ? AND idx.max_lat <= ? "
		"ORDER BY r.road_id, rn.sequence";

	sqlite3_stmt* stmt = nullptr;
	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nodes;
	}

	sqlite3_bind_double(stmt, 1, minLon);
	sqlite3_bind_double(stmt, 2, maxLon);
	sqlite3_bind_double(stmt, 3, minLat);
	sqlite3_bind_double(stmt, 4, maxLat);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		RoadNodeData node;

		node.roadId = sqlite3_column_int64(stmt, 0);
		node.highwayType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		node.nodeId = sqlite3_column_int64(stmt, 2);
		node.lat = sqlite3_column_double(stmt, 3);
		node.lon = sqlite3_column_double(stmt, 4);
		node.sequence = sqlite3_column_int(stmt, 5);

		nodes.push_back(node);
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error executing query: " << sqlite3_errmsg(db) << std::endl;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	++mQueryCount;
	return nodes;
}

std::vector<BuildingNodeData> TestScene::CallBuildingQuery(
	const std::string& dbPath,
	double minLon, double maxLon,
	double minLat, double maxLat
)
{
	std::vector<BuildingNodeData> nodes;

	sqlite3* db = nullptr;
	int rc = sqlite3_open(dbPath.c_str(), &db);

	if (rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return nodes;
	}

	std::string sql =
		"SELECT b.building_id, b.building_type, bn.node_id, bn.lat, bn.lon, bn.sequence "
		"FROM buildings_rtree idx "
		"JOIN buildings b ON b.building_id = idx.building_id "
		"JOIN building_nodes bn ON bn.building_id = b.building_id "
		"WHERE idx.min_lon >= ? AND idx.max_lon <= ? "
		"AND idx.min_lat >= ? AND idx.max_lat <= ? "
		"ORDER BY b.building_id, bn.sequence; ";


	sqlite3_stmt* stmt = nullptr;
	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return nodes;
	}

	sqlite3_bind_double(stmt, 1, minLon);
	sqlite3_bind_double(stmt, 2, maxLon);
	sqlite3_bind_double(stmt, 3, minLat);
	sqlite3_bind_double(stmt, 4, maxLat);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		BuildingNodeData node;

		node.buildingId = sqlite3_column_int64(stmt, 0);
		node.buildingType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		node.nodeId = sqlite3_column_int64(stmt, 2);
		node.lat = sqlite3_column_double(stmt, 3);
		node.lon = sqlite3_column_double(stmt, 4);
		node.sequence = sqlite3_column_int(stmt, 5);

		nodes.push_back(node);
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error executing buildings query: " << sqlite3_errmsg(db) << std::endl;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return nodes;
}


std::vector<POIData> TestScene::CallPOIQuery(
	const std::string& dbPath,
	double minLon, double maxLon,
	double minLat, double maxLat
)
{
	std::vector<POIData> pois;

	sqlite3* db = nullptr;
	int rc = sqlite3_open(dbPath.c_str(), &db);

	if (rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return pois;
	}

	std::string sql =
		"SELECT p.poi_id, p.lat, p.lon, p.poi_type, p.poi_value, p.name "
		"FROM pois_rtree idx "
		"JOIN pois p ON p.poi_id = idx.poi_id "
		"WHERE idx.min_lon >= ? "
		"AND idx.max_lon <= ? "
		"AND idx.min_lat >= ? "
		"AND idx.max_lat <= ? ";


	sqlite3_stmt* stmt = nullptr;
	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return pois;
	}

	sqlite3_bind_double(stmt, 1, minLon);
	sqlite3_bind_double(stmt, 2, maxLon);
	sqlite3_bind_double(stmt, 3, minLat);
	sqlite3_bind_double(stmt, 4, maxLat);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		POIData poi;

		poi.poiId = sqlite3_column_int64(stmt, 0);
		poi.lat = sqlite3_column_double(stmt, 1);
		poi.lon = sqlite3_column_double(stmt, 2);
		const unsigned char* type = sqlite3_column_text(stmt, 3);
		poi.poiType = type ? reinterpret_cast<const char*>(type) : "";
		const unsigned char* value = sqlite3_column_text(stmt, 4);
		poi.poiValue = value ? reinterpret_cast<const char*>(value) : "";
		const unsigned char* name = sqlite3_column_text(stmt, 5);
		poi.name = name ? reinterpret_cast<const char*>(name) : "";
		pois.push_back(poi);
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error executing POIData query: " << sqlite3_errmsg(db) << std::endl;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return pois;
}

std::vector<Segment> TestScene::NodesToSegments(const std::vector<RoadNodeData>& nodes)
{
	std::vector<Segment> segments;

	if (nodes.empty())
	{
		return segments;
	}

	std::map<int, std::vector<RoadNodeData>> roadGroups;
	for (const auto& node : nodes) {
		roadGroups[node.roadId].push_back(node);
	}

	for (auto& roadPair : roadGroups) {
		auto& roadNodes = roadPair.second;

		std::sort(roadNodes.begin(), roadNodes.end(), [](const RoadNodeData& a, const RoadNodeData& b)
			{
				return a.sequence < b.sequence;
			});

		// Create segments between consecutive nodes
		for (size_t i = 0; i < roadNodes.size() - 1; ++i) {
			if (roadNodes[i + 1].sequence <= roadNodes[i].sequence) {
				continue;
			}
			Segment seg;
			seg.lon1 = static_cast<float>(roadNodes[i].lon);
			seg.lat1 = static_cast<float>(roadNodes[i].lat);
			seg.lon2 = static_cast<float>(roadNodes[i + 1].lon);
			seg.lat2 = static_cast<float>(roadNodes[i + 1].lat);
			seg.highwayType = roadNodes[i].highwayType;
			segments.push_back(seg);
		}
	}
	return segments;
}

std::vector<BuildingPolygon> TestScene::NodesToBuildings(const std::vector<BuildingNodeData>& nodes)
{
	std::vector<BuildingPolygon> buildings;
	if (nodes.empty())
		return buildings;
	std::map<long long, std::vector<BuildingNodeData>> buildingGroups;
	for (const auto& node : nodes)
	{
		buildingGroups[node.buildingId].push_back(node);
	}
	for (auto& pair : buildingGroups)
	{
		auto& buildingNodes = pair.second;
		std::sort(buildingNodes.begin(), buildingNodes.end(),
			[](const BuildingNodeData& a, const BuildingNodeData& b)
			{
				return a.sequence < b.sequence;
			});
		BuildingPolygon polygon;
		polygon.buildingId = pair.first;
		polygon.buildingType = buildingNodes.front().buildingType;
		for (const auto& node : buildingNodes)
		{
			polygon.vertices.emplace_back(static_cast<float>(node.lon), static_cast<float>(node.lat));
		}
		buildings.push_back(polygon);
	}
	return buildings;
}

void TestScene::Load(
	GLuint shaderProgram,
	entt::registry& registry,
	Window& window
)
{
	registry.clear();

	auto camera = registry.create();
	CameraComponent cameraComp = CameraComponent();
	float wx, wy;
	CoordinateConverter::LonLatToMetric(
		static_cast<float>(CoordinateConverter::GetRefLon()),
		static_cast<float>(CoordinateConverter::GetRefLat()),
		wx, wy
	);
	registry.emplace<CameraComponent>(camera, cameraComp);

	MapRenderer& mapRenderer = GetMapRenderer();
	//mapRenderer.Initialize(shaderProgram, "../projects/YiQian.Lim-project-phase8/themes.json");
	mapRenderer.Initialize(shaderProgram, "../projects/jiaming.yeo-project-phase9/themes.json");

	auto size = window.GetWindowSize();
	cameraComp.UpdateMatrices(size.x, size.y);

	// Load road graph
	mRoadGraph.load(mDBPath);
	mKDTree.build(mRoadGraph.allNodeIds(), mRoadGraph.allNodeInfo());


	UpdateVisibleBounds(cameraComp);

	UpdateCacheBound(RoadBounds);
	UpdateCacheBound(BuildingBounds);
	UpdateCacheBound(POIBounds);

	Refresh(cameraComp, size);
	RefreshBuildings(cameraComp, size);
	RefreshPOIs(cameraComp, size);

	std::vector<int> testIds;
	// Use a known road intersection position in your city
	mKDTree.radiusQuery(cameraComp.pos.x, cameraComp.pos.y, 100.0f, testIds);
	printf("Radius test: %d nodes within 100m of origin", (int)testIds.size());
}

void TestScene::Update(GLuint shaderProgram, entt::registry& registry, Window& window)
{
	auto view = registry.view<CameraComponent>();
	auto entity = *view.begin();
	auto& camera = registry.get<CameraComponent>(entity);
	auto size = window.GetWindowSize();
	camera.UpdateMatrices(size.x, size.y);

	UpdateVisibleBounds(camera);
	double currentAltitude = camera.pos.z;

	int currentLodLevel = MapRenderer::GetLodLevelFromAltitude(currentAltitude);

	MapRenderer& mapRenderer = GetMapRenderer();
	if (is_CamBound_Outside_CacheBound(RoadBounds) || currentLodLevel != mLastLodLevel || mapRenderer.RoadsNeedRefresh())
	{
		Refresh(camera, size);
	}
	if (is_CamBound_Outside_CacheBound(BuildingBounds) ||
		(currentAltitude <= SHOW_BUILDINGS_ALTITUDE && mLastAltitude > SHOW_BUILDINGS_ALTITUDE) ||
		(currentAltitude > SHOW_BUILDINGS_ALTITUDE && mLastAltitude <= SHOW_BUILDINGS_ALTITUDE) ||
		mapRenderer.BuildingsNeedRefresh()
		)
	{
		RefreshBuildings(camera, size);
	}

	if (is_CamBound_Outside_CacheBound(POIBounds) ||
		(currentAltitude <= SHOW_POIS_ALTITUDE && mLastAltitude > SHOW_POIS_ALTITUDE) ||
		(currentAltitude > SHOW_POIS_ALTITUDE && mLastAltitude <= SHOW_POIS_ALTITUDE) ||
		mapRenderer.POIsNeedRefresh()
		)
	{
		RefreshPOIs(camera, size);
	}

	mLastAltitude = currentAltitude;
	mLastLodLevel = currentLodLevel;
}

void TestScene::Refresh(CameraComponent& camera, const glm::ivec2& screenSize)
{
	MapRenderer& mapRenderer = GetMapRenderer();
	mapRenderer.ResetRoadsRefresh();

	auto t0 = std::chrono::high_resolution_clock::now();
	std::vector<RoadNodeData> roadNodes = CallQuery(mDBPath, RoadBounds.MinLon, RoadBounds.MaxLon, RoadBounds.MinLat, RoadBounds.MaxLat);
	auto t1 = std::chrono::high_resolution_clock::now();

	if (!roadNodes.empty())
	{
		std::vector<Segment> segments = NodesToSegments(roadNodes);
		mSegmentCount = segments.size();
		mCachedSegment = segments;
		mDrawnCount = 0.0;

		if (!segments.empty())
		{
			std::vector<Segment> renderSegments;
			double currentAltitude = camera.pos.z;

			for (const auto& seg : segments)
			{
				if (!mapRenderer.IsTypeVisible(seg.highwayType, currentAltitude))
				{
					continue;
				}
				Segment s;
				s.lon1 = seg.lon1;
				s.lat1 = seg.lat1;
				s.lon2 = seg.lon2;
				s.lat2 = seg.lat2;
				s.highwayType = seg.highwayType;
				renderSegments.push_back(s);
				mDrawnCount++;
			}
			mapRenderer.BuildRoadGeometry(renderSegments);
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		mQueryMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
		mBuildMs = std::chrono::duration<double, std::milli>(t2 - t1).count();

		std::cout << "Query #" << mQueryCount << ": " << mQueryMs << " ms Query "
			<< mBuildMs << " ms Build total " << mDrawnCount << " / "
			<< mSegmentCount << " altitude=" << camera.pos.z << " lod="
			<< MapRenderer::GetLodLevelFromAltitude(camera.pos.z) << "\n";


	}

	else
	{
		std::cerr << "No road nodes found!" << std::endl;
	}
}

void TestScene::RefreshBuildings(CameraComponent& camera, const glm::ivec2& screenSize)
{
	MapRenderer& mapRenderer = GetMapRenderer();
	mapRenderer.ResetBuildingsRefresh();

	if (camera.pos.z > SHOW_BUILDINGS_ALTITUDE)
	{
		mapRenderer.ClearBuildingGeometry();
		mBuildingCount = 0;
		return;
	}

	std::vector<BuildingNodeData> buildingNodes = CallBuildingQuery(mDBPath, BuildingBounds.MinLon, BuildingBounds.MaxLon, BuildingBounds.MinLat, BuildingBounds.MaxLat);
	std::cout << "Buildings returned: " << buildingNodes.size() << std::endl;

	if (!buildingNodes.empty())
	{
		std::vector<BuildingPolygon> buildings = NodesToBuildings(buildingNodes);
		if (!buildings.empty()) {
			mapRenderer.BuildBuildingGeometry(buildings);
		}
		mBuildingCount = static_cast<int>(buildings.size());
	}
}

void TestScene::RefreshPOIs(CameraComponent& camera, const glm::ivec2& screenSize)
{
	MapRenderer& mapRenderer = GetMapRenderer();
	mapRenderer.ResetPoiRefresh();

	if (camera.pos.z > SHOW_POIS_ALTITUDE)
	{
		mapRenderer.ClearPOIGeometry();
		mPOICount = 0;
		return;
	}

	std::vector<POIData> pois = CallPOIQuery(mDBPath, POIBounds.MinLon, POIBounds.MaxLon, POIBounds.MinLat, POIBounds.MaxLat);
	std::cout << "POIs returned: " << pois.size() << '\n';

	if (!pois.empty())
	{
		mapRenderer.BuildPOIGeometry(pois);
		mPOICount = static_cast<int>(pois.size());
	}

}

bool TestScene::is_CamBound_Outside_CacheBound(CacheBounds& cachebound)
{
	if (visibleMinLon < cachebound.MinLon ||
		visibleMaxLon > cachebound.MaxLon ||
		visibleMinLat < cachebound.MinLat ||
		visibleMaxLat > cachebound.MaxLat)
	{
		UpdateCacheBound(cachebound);
		return true;
	}
	else return false;
}

void TestScene::UpdateCacheBound(CacheBounds& cachebound)
{
	double lonSpan = visibleMaxLon - visibleMinLon;
	double latSpan = visibleMaxLat - visibleMinLat;
	cachebound.MinLon = visibleMinLon - lonSpan * CACHE_MARGIN;
	cachebound.MaxLon = visibleMaxLon + lonSpan * CACHE_MARGIN;
	cachebound.MinLat = visibleMinLat - latSpan * CACHE_MARGIN;
	cachebound.MaxLat = visibleMaxLat + latSpan * CACHE_MARGIN;
}

void TestScene::UpdateVisibleBounds(const CameraComponent& camera)
{
	double visibleRadius = camera.pos.z * 1.5;

	float camLon, camLat;
	CoordinateConverter::MetricToLonLat(camera.pos.x, camera.pos.y, camLon, camLat);

	double radiusDegLon = visibleRadius / CoordinateConverter::GetMetersPerDegLon();
	double radiusDegLat = visibleRadius / CoordinateConverter::GetMetersPerDegLat();

	visibleMinLon = camLon - radiusDegLon;
	visibleMaxLon = camLon + radiusDegLon;
	visibleMinLat = camLat - radiusDegLat;
	visibleMaxLat = camLat + radiusDegLat;
}

int TestScene::GetBuildingCount() const
{
	return mBuildingCount;
}

int TestScene::GetPOICount() const
{
	return mPOICount;
}

void TestScene::OnLeftClick(glm::vec2 mousepos , const glm::ivec2& screenSize, const CameraComponent& camera)
{
	glm::vec2 world = CoordinateConverter::ScreenToWorld(mousepos , screenSize , camera);
	int nearest = mKDTree.nearestNode(world.x, world.y);
	if (nearest == -1)
	{
		return;
	}
	MapRenderer& mapRenderer = GetMapRenderer();
	switch (mRouteState)
	{
	case RouteState::Idle:
	{
		mStartNode = nearest;
		mRouteState = RouteState::StartSelected;
		break;
	}
	case RouteState::StartSelected:
	{
		mGoalNode = nearest;
		std::vector<int> path = AstarPathFinding(mRoadGraph,mStartNode,mGoalNode);
		if (!path.empty())
		{
			mapRenderer.BuildRibbonGeometry(path, mRoadGraph);
			mRouteState = RouteState::RouteDisplayed;
		}
		break;
	}
	case RouteState::RouteDisplayed:
	{
		mapRenderer.ClearRibbonGeometry();
		mStartNode = nearest;
		mGoalNode = -1;
		mRouteState = RouteState::StartSelected;
		break;
	}
	}
}
