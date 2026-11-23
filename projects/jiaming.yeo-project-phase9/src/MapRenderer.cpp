#include <GL/glew.h>
#include <iostream>
#include <algorithm>

#include "Component/CameraComponent.hpp"
#include "MapRenderer.hpp"
#include "CoordinateConverter.hpp"
#include "Vertex.hpp"
#include "shader.hpp"

MapRenderer& GetMapRenderer()
{
	static MapRenderer mapRenderer;
	return mapRenderer;
}

// Sets up renderer with a shader program
void MapRenderer::Initialize(GLuint shaderProgram, const std::string& themePath)
{
	if (!mRoadBuffer)
	{
		mRoadBuffer = std::make_unique<Buffer>();
	}

	if (!mBuildingBuffer)
	{
		mBuildingBuffer = std::make_unique<Buffer>();
	}

	if (!mPOIBuffer)
	{
		mPOIBuffer = std::make_unique<Buffer>();
	}

	mShaderProgram = shaderProgram;

	uViewProjLoc = glGetUniformLocation(mShaderProgram, "uViewProj");
	//uDrawColorLoc = glGetUniformLocation(mShaderProgram, "uDrawColor");
	uLightDirLoc = glGetUniformLocation(mShaderProgram, "uLightDir");
	uLightColorLoc = glGetUniformLocation(mShaderProgram, "uLightColor");
	uAmbientLevelLoc = glGetUniformLocation(mShaderProgram, "uAmbientLevel");
	//uUseUniformColorLoc = glGetUniformLocation(mShaderProgram, "uUseUniformColor");

	/*
	mSkyboxShader = LoadShaders(
		"../projects/YiQian.Lim-project-phase8/shaders/Skybox.vert",
		"../projects/YiQian.Lim-project-phase8/shaders/Skybox.frag"
	);
	*/

	mSkyboxShader = LoadShaders(
		"../projects/jiaming.yeo-project-phase9/shaders/Skybox.vert",
		"../projects/jiaming.yeo-project-phase9/shaders/Skybox.frag"
	);

	mSkybox = std::make_unique<Skybox>();
	mSkybox->SetShaderProgram(mSkyboxShader);

	mThemeManager.Load(themePath);

	const auto& theme = mThemeManager.Current();
	mSunAzimuth = theme.sunAzimuth;
	mSunElevation = theme.sunElevation;
	mAmbientLevel = theme.sunAmbient;

	UpdateLightDirection();
	mRoadsNeedRefresh = mBuildingsNeedRefresh = mPOIsNeedRefresh = true;
}

void MapRenderer::BuildRoadGeometry(const std::vector<Segment>& segments)
{
	if (!mRoadBuffer)
	{
		std::cerr << "MapRenderer not initialized!" << std::endl;
		return;
	}

	if (segments.empty())
	{
		std::cout << "Warning: No segments to build geometry\n";
		return;
	}

	std::vector<Segment> sortedSegments = segments;
	std::sort(sortedSegments.begin(), sortedSegments.end(),
		[](const Segment& a, const Segment& b)
		{
			return a.highwayType < b.highwayType;
		});

	roadTypeGroups.clear();
	std::string currentType;
	int currentOffset = 0;
	std::vector<Vertex> vertices;
	glm::vec3 roadNormal = { 0.0f, 0.0f, 1.0f }; // all facing up

	for (size_t i = 0; i < sortedSegments.size(); ++i)
	{
		const auto& seg = sortedSegments[i];

		// New road type?
		if (i == 0 || seg.highwayType != currentType)
		{
			// Finish previous group
			if (i != 0)
			{
				roadTypeGroups.back().countVertices =
					vertices.size() - currentOffset;
			}

			currentType = seg.highwayType;
			currentOffset = static_cast<int>(vertices.size());

			const auto& theme = mThemeManager.Current();
			auto themeColor = theme.roadColor(seg.highwayType);

			roadTypeGroups.push_back({
				currentOffset,
				0,
				themeColor.r,
				themeColor.g,
				themeColor.b,
				currentType
				});
		}
		float wx1, wy1, wx2, wy2;
        CoordinateConverter::LonLatToMetric(seg.lon1, seg.lat1, wx1, wy1);
        CoordinateConverter::LonLatToMetric(seg.lon2, seg.lat2, wx2, wy2);

		glm::vec2 dir = glm::normalize(glm::vec2(wx2 - wx1, wy2 - wy1)); // direction along the road
		glm::vec2 perp = { -dir.y, dir.x }; // perpendicular, in the ground plane

		float hw = RoadHalfWidth(seg.highwayType);

		glm::vec3 color = {
			roadTypeGroups.back().r,
			roadTypeGroups.back().g,
			roadTypeGroups.back().b
		};

		glm::vec3 A = { wx1 - perp.x * hw, wy1 - perp.y * hw, 0.5f };
		glm::vec3 B = { wx1 + perp.x * hw, wy1 + perp.y * hw, 0.5f };
		glm::vec3 C = { wx2 + perp.x * hw, wy2 + perp.y * hw, 0.5f };
		glm::vec3 D = { wx2 - perp.x * hw, wy2 - perp.y * hw, 0.5f };

		vertices.push_back(Vertex(A, color, roadNormal));
		vertices.push_back(Vertex(C, color, roadNormal));
		vertices.push_back(Vertex(D, color, roadNormal));

		vertices.push_back(Vertex(A, color, roadNormal));
		vertices.push_back(Vertex(B, color, roadNormal));
		vertices.push_back(Vertex(C, color, roadNormal));
	}

	if (!roadTypeGroups.empty())
	{
		roadTypeGroups.back().countVertices =
			static_cast<int>(vertices.size()) - currentOffset;
	}

	std::sort(roadTypeGroups.begin(), roadTypeGroups.end(),
		[](const RoadTypeGroup& a, const RoadTypeGroup& b) {
			return MapRenderer::highwayProminence(a.highwayType) <
				MapRenderer::highwayProminence(b.highwayType);
		});

	mRoadVertexCount = static_cast<GLuint>(vertices.size());

	mRoadBuffer->BindVAO();
	mRoadBuffer->BindVBO();

	mRoadBuffer->SetData(
		vertices.size() * sizeof(Vertex),
		vertices.data(),
		GL_STATIC_DRAW
	);

	// position
	mRoadBuffer->SetAttribute(
		0, 
		3, 
		GL_FLOAT, 
		GL_FALSE,
		sizeof(Vertex), 
		(void*)offsetof(Vertex, position)
	);
	// color
	mRoadBuffer->SetAttribute(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, color)
	);
	// normal
	mRoadBuffer->SetAttribute(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, normal)
	);

	mRoadBuffer->UnbindVBO();
	mRoadBuffer->UnbindVAO();
}

float MapRenderer::RoadHalfWidth(const std::string& t)
{
	if (t == "motorway") return 9.0f;
	if (t == "trunk") return 7.0f;
	if (t == "primary") return 5.5f;
	if (t == "secondary") return 4.0f;
	if (t == "residential") return 3.0f;
	if (t == "living_street") return 2.5f;
	if (t == "footway" || t == "cycleway") return 1.5f;
	return 3.5f; // default
}

void MapRenderer::BuildBuildingGeometry(std::vector<BuildingPolygon> const& buildings)
{
	if (!mBuildingBuffer)
	{
		std::cerr << "Building buffer not initialized!\n";
		return;
	}
	buildingDraws.clear();
	std::vector<Vertex> vertices;

	for (const auto& building : buildings)
	{
		if (building.vertices.size() < 3)
			continue;

		BuildingDraw draw;
		draw.first = static_cast<GLint>(vertices.size());

		int wallStart = static_cast<int>(vertices.size());
		const auto& theme = mThemeManager.Current();
		glm::vec3 wallColor = theme.buildingWallColor(building.buildingType);
		draw.colour.r = wallColor.r;
		draw.colour.g = wallColor.g;
		draw.colour.b = wallColor.b;

		float h = buildingHeight(building.buildingType);
		//walls
		for (size_t i = 0; i < building.vertices.size() ; ++i)
		{
			float wx0, wy0;
			CoordinateConverter::LonLatToMetric(building.vertices.at(i).x, building.vertices.at(i).y, wx0, wy0);
			float wx1, wy1;
			CoordinateConverter::LonLatToMetric(building.vertices.at((i+1)% building.vertices.size()).x, building.vertices.at((i + 1) % building.vertices.size()).y, wx1, wy1);
			
			// wall normal
			glm::vec2 edgeDir = glm::normalize(glm::vec2(wx1 - wx0, wy1 - wy0)); // along the wall, ground - plane
			glm::vec2 outward = { edgeDir.y, -edgeDir.x }; // rotate 90° — points away from polygon interior
			glm::vec3 wallNormal = { outward.x, outward.y, 0.0f };
			
			glm::vec3 v0 = { wx0, wy0, 0.0f };   // bottom-left  (ground level)
			glm::vec3 v1 = { wx1, wy1, 0.0f };   // bottom-right (ground level)
			glm::vec3 v2 = { wx1, wy1, h };	 // top-right (height at 10.0f)
			glm::vec3 v3 = { wx0, wy0, h };	 // top-left (height at 10.0f)

			//triangle 1
			vertices.push_back(Vertex(v0, wallColor, wallNormal));
			vertices.push_back(Vertex(v1, wallColor, wallNormal));
			vertices.push_back(Vertex(v2, wallColor, wallNormal));
			//triangle 2
			vertices.push_back(Vertex(v0, wallColor, wallNormal));
			vertices.push_back(Vertex(v2, wallColor, wallNormal));
			vertices.push_back(Vertex(v3, wallColor, wallNormal));
		}

		int wallCount = static_cast<int>(vertices.size()) - wallStart;
		if (wallCount > 0)
		{
			buildingDraws.push_back({
				wallStart,
				wallCount,
				wallColor
				});
		}

		//roof
		int roofStart = static_cast<int>(vertices.size());
		glm::vec3 roofNormal = { 0.0f, 0.0f, 1.0f };
		glm::vec3 roofColor = mThemeManager.Current().buildingRoofColor();

		for (size_t k = 1; k + 1 < building.vertices.size(); ++k)
		{
			// anchor
			float anchor_x, anchor_y;
			float wx0, wy0;
			float wx1, wy1;
			CoordinateConverter::LonLatToMetric(building.vertices.front().x, building.vertices.front().y, anchor_x, anchor_y);
			CoordinateConverter::LonLatToMetric(building.vertices.at(k).x, building.vertices.at(k).y, wx0, wy0);
			CoordinateConverter::LonLatToMetric(building.vertices.at(k + 1).x, building.vertices.at(k+1).y, wx1, wy1);
			glm::vec3 anchor = { anchor_x, anchor_y,h };
			glm::vec3 v0 = { wx0, wy0, h };
			glm::vec3 v1 = { wx1, wy1, h };
			vertices.push_back(Vertex(anchor, roofColor, roofNormal));
			vertices.push_back(Vertex(v0, roofColor, roofNormal));
			vertices.push_back(Vertex(v1, roofColor, roofNormal));
		}

		int roofCount = static_cast<int>(vertices.size()) - roofStart;
		if (roofCount > 0)
		{
			buildingDraws.push_back({
				roofStart,
				roofCount,
				roofColor
				});
		}
	}

	mBuildingVertexCount = static_cast<GLuint>(vertices.size());

	mBuildingBuffer->BindVAO();
	mBuildingBuffer->BindVBO();

	mBuildingBuffer->SetData(
		vertices.size() * sizeof(Vertex),
		vertices.data(),
		GL_STATIC_DRAW);
	// position
	mBuildingBuffer->SetAttribute(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, position));
	// color
	mBuildingBuffer->SetAttribute(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, color));
	// normal
	mBuildingBuffer->SetAttribute(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, normal));

	mBuildingBuffer->UnbindVBO();
	mBuildingBuffer->UnbindVAO();
}

void MapRenderer::BuildPOIGeometry(std::vector<POIData> const& pois)
{
	if (!mPOIBuffer)
	{
		std::cerr << "POI buffer not initialized!\n";
		return;
	}

	std::vector<Vertex> vertices;
	const auto& theme = mThemeManager.Current();
	//glm::vec3 poiColor = theme.poiColor("default");
	glm::vec3 poiNormal = { 0.0f, 0.0f, 0.0f };

	for (const auto& poi : pois)
	{
		glm::vec3 poiColor = poiCategoryColor(poi.poiType, poi.poiValue);
		float wx, wy;
		CoordinateConverter::LonLatToMetric(poi.lon, poi.lat, wx, wy);
		glm::vec3 bottom = { wx, wy, 0.0f };
		glm::vec3 top = { wx,wy,25.0f };
		vertices.emplace_back(bottom, poiColor, poiNormal);
		vertices.emplace_back(top, poiColor, poiNormal);
	}

	mPOIVertexCount = static_cast<GLuint>(vertices.size());

	mPOIBuffer->BindVAO();
	mPOIBuffer->BindVBO();

	mPOIBuffer->SetData(
		vertices.size() * sizeof(Vertex), 
		vertices.data(), 
		GL_STATIC_DRAW);
	// position
	mPOIBuffer->SetAttribute(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, position));
	// color
	mPOIBuffer->SetAttribute(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, color));
	// normal
	mPOIBuffer->SetAttribute(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, normal));

	mPOIBuffer->UnbindVBO();
	mPOIBuffer->UnbindVAO();


}

void MapRenderer::BuildRibbonGeometry(const std::vector<int>& path , const RoadGraph& graph)
{
	if (path.size() < 2)
		return;

	if (!mRibbonBuffer) {
		mRibbonBuffer = std::make_unique<Buffer>();
	}

	std::vector<Vertex> vertices;
	float z = 0.9f;
	glm::vec3 innerribbonColor = { 0.3f, 0.1f, 0.1f }; 
	glm::vec3 outerribbonColor = { 0.0f, 0.0f, 1.0f }; 

	//inner ribbon
	for (size_t i = 0; i + 1 < path.size(); ++i) {

		float halfWidth = 3.5f;
		const NodeInfo& node0 = graph.getNode(path[i]);
		const NodeInfo& node1 = graph.getNode(path[i + 1]);
		glm::vec3 A{ node0.wx ,node0.wy,1.1f};
		glm::vec3 B{ node1.wx ,node1.wy,1.1f};
		glm::vec2 dir = glm::normalize(glm::vec2(B - A));
		glm::vec2 perp = { -dir.y, dir.x };   // rotate 90°
		glm::vec3 n = { 0, 0, 1 };
		glm::vec3 AL = { A.x + perp.x * halfWidth, A.y + perp.y * halfWidth, A.z };
		glm::vec3 AR = { A.x - perp.x * halfWidth, A.y - perp.y * halfWidth, A.z };
		glm::vec3 BL = { B.x + perp.x * halfWidth, B.y + perp.y * halfWidth, B.z };
		glm::vec3 BR = { B.x - perp.x * halfWidth, B.y - perp.y * halfWidth, B.z };
		// Triangle 1: AL, AR, BR
		vertices.push_back(Vertex(AL, innerribbonColor, n));
		vertices.push_back(Vertex(AR, innerribbonColor, n));
		vertices.push_back(Vertex(BR, innerribbonColor, n));
		// Triangle 2: AL, BR, BL
		vertices.push_back(Vertex(AL, innerribbonColor, n));
		vertices.push_back(Vertex(BR, innerribbonColor, n));
		vertices.push_back(Vertex(BL, innerribbonColor, n));
	}
	//outer ribbon
	for (size_t i = 0; i + 1 < path.size(); ++i) {
		float halfWidth = 5.5f;
		const NodeInfo& node0 = graph.getNode(path[i]);
		const NodeInfo& node1 = graph.getNode(path[i + 1]);
		glm::vec3 A{ node0.wx ,node0.wy,0.9f };
		glm::vec3 B{ node1.wx ,node1.wy,0.9f };
		glm::vec2 dir = glm::normalize(glm::vec2(B - A));
		glm::vec2 perp = { -dir.y, dir.x };   // rotate 90°
		glm::vec3 n = { 0, 0, 1 };
		glm::vec3 AL = { A.x + perp.x * halfWidth, A.y + perp.y * halfWidth, A.z };
		glm::vec3 AR = { A.x - perp.x * halfWidth, A.y - perp.y * halfWidth, A.z };
		glm::vec3 BL = { B.x + perp.x * halfWidth, B.y + perp.y * halfWidth, B.z };
		glm::vec3 BR = { B.x - perp.x * halfWidth, B.y - perp.y * halfWidth, B.z };
		// Triangle 1: AL, AR, BR
		vertices.push_back(Vertex(AL, outerribbonColor, n));
		vertices.push_back(Vertex(AR, outerribbonColor, n));
		vertices.push_back(Vertex(BR, outerribbonColor, n));
		// Triangle 2: AL, BR, BL
		vertices.push_back(Vertex(AL, outerribbonColor, n));
		vertices.push_back(Vertex(BR, outerribbonColor, n));
		vertices.push_back(Vertex(BL, outerribbonColor, n));
	}

	mRibbonBuffer->BindVAO();
	mRibbonBuffer->BindVBO();
	mRibbonBuffer->SetData(vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	// position
	mRibbonBuffer->SetAttribute(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, position)
	);
	// color
	mRibbonBuffer->SetAttribute(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, color)
	);
	// normal
	mRibbonBuffer->SetAttribute(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, normal)
	);
	mRibbonBuffer->UnbindVAO();
	mRibbonVertexCount = static_cast<GLuint>(vertices.size());
}

void MapRenderer::ClearBuildingGeometry()
{
	buildingDraws.clear();
	mBuildingVertexCount = 0;
}

void MapRenderer::ClearPOIGeometry()
{
	mPOIVertexCount = 0;
}

void MapRenderer::ClearRibbonGeometry()
{
	mRibbonVertexCount = 0;
}

void MapRenderer::Render(
	CameraComponent& camera,
	Window& window,
	float dt
)
{
	if (!mRoadBuffer || mRoadVertexCount == 0 || mShaderProgram == 0)
	{
		return;
	}

	const auto& theme = mThemeManager.Current();
	const auto& bg = theme.background;
	glClearColor(bg.r, bg.g, bg.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mSkybox && mSkyboxShader)
	{
		glm::vec3 skyColor(bg.r, bg.g, bg.b);
		glm::vec3 groundColor = glm::vec3(theme.skyground.r, theme.skyground.g, theme.skyground.b);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		mSkybox->Render(skyColor, groundColor);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}

	mThemeManager.HandleHotkeys(window.GetGLFWWindow());

	if (mThemeManager.Changed()) 
	{
		const auto& theme = mThemeManager.Current();
		mSunAzimuth = theme.sunAzimuth;
		mSunElevation = theme.sunElevation;
		mAmbientLevel = theme.sunAmbient;
		mRoadsNeedRefresh = mBuildingsNeedRefresh = mPOIsNeedRefresh = true;
	}

	if (glfwGetKey(window.GetGLFWWindow(), GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
		mSunAzimuth -= SUN_ROT_SPEED * dt;
	if (glfwGetKey(window.GetGLFWWindow(), GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
		mSunAzimuth += SUN_ROT_SPEED * dt;

	UpdateLightDirection();

	auto size = window.GetWindowSize();
	camera.UpdateMatrices(size.x, size.y);

	glUseProgram(mShaderProgram);

	glUniformMatrix4fv(uViewProjLoc, 1, GL_FALSE, &camera.viewProjMatrix[0][0]);

	glUniform3fv(uLightDirLoc, 1, &mLightDirection[0]);
	glUniform3fv(uLightColorLoc, 1, &mLightColor[0]);
	glUniform1f(uAmbientLevelLoc, mAmbientLevel);

	//draw road
	mRoadBuffer->BindVAO();
	glEnable(GL_POLYGON_OFFSET_FILL);
	for (const auto& group : roadTypeGroups)
	{
		float offset = static_cast<float>(highwayProminence(group.highwayType)) * 0.5f;
		glPolygonOffset(offset, offset);
		glDrawArrays(
			GL_TRIANGLES,
			group.offsetVertices,
			group.countVertices);
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	mRoadBuffer->UnbindVAO();
	//draw buildings
	if (mBuildingVertexCount > 0)
	{
		mBuildingBuffer->BindVAO();
		for (const auto& draw : buildingDraws)
		{
			glDrawArrays(GL_TRIANGLES, draw.first, draw.count);
		}
		mBuildingBuffer->UnbindVAO();
	}
	//draw POIs
	if (mPOIVertexCount > 0)
	{
		mPOIBuffer->BindVAO();
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, mPOIVertexCount);
		mPOIBuffer->UnbindVAO();
	}
	//draw ribbon
	if (mRibbonVertexCount > 0) {
		mRibbonBuffer->BindVAO();
		glDrawArrays(GL_TRIANGLES, 0, mRibbonVertexCount);
		mRibbonBuffer->UnbindVAO();
	}
	
	glUseProgram(0);
}

void MapRenderer::roadTypeColor(const std::string& t, float& r, float& g, float& b) {
	if (t == "motorway")
	{
		r = 1.00f; g = 0.24f; b = 0.24f;
	}  // red
	else if (t == "trunk")
	{
		r = 1.00f; g = 0.55f; b = 0.20f;
	}  // orange
	else if (t == "primary")
	{
		r = 1.00f; g = 0.86f; b = 0.00f;
	}  // yellow
	else if (t == "secondary")
	{
		r = 0.78f; g = 0.78f; b = 0.00f;
	}  // lime
	else if (t == "tertiary")
	{
		r = 0.70f; g = 0.70f; b = 0.70f;
	}  // light gray
	else if (t == "residential") { r = 0.63f; g = 0.63f; b = 0.63f; }  // gray
	else if (t == "service")
	{
		r = 0.47f; g = 0.47f; b = 0.47f;
	}  // dark gray
	else if (t == "footway")
	{
		r = 0.31f; g = 0.78f; b = 0.31f;
	}  // green
	else if (t == "path")
	{
		r = 0.31f; g = 0.78f; b = 0.31f;
	}  // green
	else if (t == "cycleway")
	{
		r = 0.24f; g = 0.55f; b = 0.86f;
	}  // blue
	else if (t == "living_street") { r = 0.55f; g = 0.71f; b = 0.55f; }  // muted green
	else if (t == "steps")
	{
		r = 0.63f; g = 0.47f; b = 0.31f;
	}  // brown
	else
	{
		r = 0.39f; g = 0.39f; b = 0.39f;
	}  // neutral gray (unknown)
}

bool MapRenderer::IsTypeVisible(const std::string& highwayType, double altitude)
{
	int lod = GetLodLevelFromAltitude(altitude);
	if (lod >= 3) return true; // highest detail tier: show everything
	if (highwayType == "motorway" || highwayType == "trunk") return true;
	if (lod >= 1 && (highwayType == "primary" || highwayType == "secondary"))
		return true;
	if (lod >= 2 && (highwayType == "tertiary" || highwayType == "residential" ||
		highwayType == "unclassified" || highwayType == "living_street"))
		return true;
	// service, footway, cycleway, steps, path — visible only at tier 3
	return false;
}

int MapRenderer::GetLodLevelFromAltitude(double altitude)
{
	if (altitude < 200.0) return 3;   // Very low - show everything
	if (altitude < 500.0) return 2;   // Low - show most roads
	if (altitude < 1500.0) return 1;  // Medium - show major roads only
	return 0;                         // High - show motorways only
}

glm::vec3 MapRenderer::buildingColor(const std::string& t) {
	if
		(t == "residential" || t == "house" || t == "detached")
		return { 0.35f, 0.51f, 0.71f };   // muted blue
	else if (t == "apartments")
		return { 0.27f, 0.43f, 0.63f };   // darker blue
	else if (t == "commercial" || t == "retail")
		return { 0.71f, 0.51f, 0.27f };   // orange-brown
	else if (t == "industrial")
		return { 0.63f, 0.39f, 0.31f };   // reddish-brown
	else if (t == "school" || t == "university")
		return { 0.43f, 0.71f, 0.35f };   // muted green
	else if (t == "hospital")
		return { 0.86f, 0.31f, 0.31f };   // red
	else return{ 0.5f,0.5f,0.5f };
	// ... additional types fall back to a neutral gray
}

float MapRenderer::buildingHeight(const std::string& t)
{
	if (t == "apartments")
		return 25.0f;
	if (t == "hospital")
		return 15.0f;
	if (t == "commercial" || t == "retail")
		return 12.0f;
	if (t == "school" || t == "university")
		return 10.0f;
	if (t == "residential" || t == "house" || t == "detached")
		return 8.0f;
	return 6.0f;
}

void MapRenderer::UpdateLightDirection()
{
	float azimuthRad = glm::radians(mSunAzimuth);
	float elevationRad = glm::radians(mSunElevation);
	mLightDirection = glm::vec3(cos(elevationRad) * sin(azimuthRad), cos(elevationRad) * cos(azimuthRad), sin(elevationRad));
	mLightDirection = glm::normalize(mLightDirection);
}

int MapRenderer::highwayProminence(const std::string& hw)
{
	if (hw == "motorway" || hw == "motorway_link") return 5;
	if (hw == "trunk" || hw == "trunk_link") return 4;
	if (hw == "primary" || hw == "primary_link") return 3;
	if (hw == "secondary" || hw == "secondary_link") return 2;
	if (hw == "residential" || hw == "living_street") return 1;
	return 0; // footway, cycleway, service
}


glm::vec3 MapRenderer::poiCategoryColor(std::string poi_type, std::string poi_value)
{
	// amenity types
	if ((poi_value == "hospital") ||
		(poi_value == "clinic") ||
		(poi_value == "pharmacy"))  return { 0.90f, 0.15f, 0.15f };  // red

	if ((poi_value == "school") ||
		(poi_value == "university") ||
		(poi_value == "kindergarten")) return { 0.20f, 0.50f, 0.90f }; // blue

	if ((poi_value == "restaurant") ||
		(poi_value == "cafe") ||
		(poi_value == "food_court")) return { 1.00f, 0.60f, 0.10f }; // orange

	if ((poi_value == "bus_stop") ||
		(poi_value == "subway") ||
		(poi_value == "ferry")) return { 0.70f, 0.20f, 0.90f }; // purple

	//  types
	if ((poi_type == "leisure") ||
		(poi_value == "park")) return { 0.20f, 0.80f, 0.30f }; // green
	// shop pes
	if ((poi_type == "shop")) return { 0.90f, 0.80f, 0.10f }; // yellow
	// default
	return { 0.70f, 0.70f, 0.70f };  // grey
}


