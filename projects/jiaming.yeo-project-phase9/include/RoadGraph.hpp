#pragma once

#include <unordered_map>
#include <vector>
#include <sqlite3.h>
#include <string>
#include <glm/glm.hpp>
#include <queue>
#include <unordered_set>


struct NodeInfo {
	int   node_id;
	float lat, lon;
	float wx, wy;
};

struct Edge {
	int   neighbour_id;
	float distance_m;
};




class RoadGraph {
public:
	void load(const std::string& dbPath);
	const NodeInfo& getNode(int id) const;
	const std::vector<Edge>& getEdges(int id) const;
	const std::vector<int>& allNodeIds() const;
	const std::unordered_map<int, NodeInfo>& allNodeInfo() const;
private:
	std::unordered_map<int, NodeInfo> nodes_;
	std::unordered_map<int, std::vector<Edge>> adjacency_;
	std::vector<int> nodeIdList_;
};

struct Point {
	int node_id;
	float x, y;   // wx, wy
	float coord(int axis) const { return (axis == 0) ? x : y; }
};

struct KDNode {
	int point_idx; // index into pts[] (-1 if this is an internal split node)
	int left = -1;
	int right = -1;
	int axis; // 0 = split on X, 1 = split on Y
	float split_val;
};

class KDTree {
public:
	int nearestNode(float x, float y) const;
	void build(const std::vector<int>& nodeIds, const std::unordered_map<int, NodeInfo>& nodes);
	void radiusQuery(float qx, float qy, float radius, std::vector<int>& outIds) const;
private:
	int buildNode(std::vector<int>& indices, int lo, int hi, int depth);
	void searchNode(int idx, float qx, float qy, int& bestId, float& bestDist2) const;
	void radiusSearchNode(const std::vector<KDNode>& nodes, int idx, float qx, float qy, float r2, std::vector<int>& out) const;
	void build(const std::vector<Point>& points);
	std::vector<Point> pts;      // All points
	std::vector<KDNode> tree;    // KD-tree nodes stored in array
};


static float haversine(float lat1, float lon1, float lat2, float lon2);
std::vector<int> AstarPathFinding(const RoadGraph& graph, int start_id, int goal_id);