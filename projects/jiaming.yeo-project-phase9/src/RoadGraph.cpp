#include "RoadGraph.hpp"
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <algorithm>
#include <limits>
#include <CoordinateConverter.hpp>

void RoadGraph::load(const std::string& dbPath)
{
	nodes_.clear();
	adjacency_.clear();
	nodeIdList_.clear();

	sqlite3* db = nullptr;
	int rc = sqlite3_open(dbPath.c_str(), &db);

	if (rc != SQLITE_OK) {
		std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return;
	}

	std::string sql = "SELECT rn.road_id, rn.node_id, rn.sequence, n.lat, n.lon "
		"FROM	  road_nodes AS rn "
		"JOIN   nodes      AS n  ON n.node_id = rn.node_id "
		"ORDER  BY rn.road_id, rn.sequence";
	sqlite3_stmt* stmt = nullptr;
	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		std::cerr << "Failed to load RoadGraph: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}

	std::vector<int> currentRoadNodes;
	int currentRoadId = -1;

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int roadId = sqlite3_column_int64(stmt, 0);
		int nodeId = sqlite3_column_int64(stmt, 1);
		int sequence = sqlite3_column_int(stmt, 2);
		double lat = sqlite3_column_double(stmt, 3);
		double lon = sqlite3_column_double(stmt, 4);

		// Add node if not already exists
		if (nodes_.find(nodeId) == nodes_.end()) {
			NodeInfo node;
			node.node_id = nodeId;
			node.lat = static_cast<float>(lat);
			node.lon = static_cast<float>(lon);
			node.wx = static_cast<float>((lon - CoordinateConverter::GetRefLon()) * CoordinateConverter::GetMetersPerDegLon());
			node.wy = static_cast<float>((lat - CoordinateConverter::GetRefLat()) * CoordinateConverter::GetMetersPerDegLat());
			nodes_[nodeId] = node;
			nodeIdList_.push_back(nodeId);
		}

		// Process road edges when road changes
		if (roadId != currentRoadId) {
			for (size_t i = 0; i + 1 < currentRoadNodes.size(); ++i) {
				int from = currentRoadNodes.at(i);
				int to = currentRoadNodes.at(i + 1);

				if (nodes_.find(from) != nodes_.end() && nodes_.find(to) != nodes_.end()) {
					const NodeInfo& fromNode = nodes_.at(from);
					const NodeInfo& toNode = nodes_.at(to);

					float distance = haversine(fromNode.lat, fromNode.lon, toNode.lat, toNode.lon);

					adjacency_[from].push_back({ to, distance });
					adjacency_[to].push_back({ from, distance });
				}
			}
			currentRoadNodes.clear();
			currentRoadId = roadId;
		}

		currentRoadNodes.push_back(nodeId);
	}

	// Process last road
	for (size_t i = 0; i + 1 < currentRoadNodes.size(); ++i) {
		int from = currentRoadNodes.at(i);
		int to = currentRoadNodes.at(i + 1);

		if (nodes_.find(from) != nodes_.end() && nodes_.find(to) != nodes_.end()) {
			const NodeInfo& fromNode = nodes_.at(from);
			const NodeInfo& toNode = nodes_.at(to);

			float distance = haversine(fromNode.lat, fromNode.lon, toNode.lat, toNode.lon);

			adjacency_[from].push_back({ to, distance });
			adjacency_[to].push_back({ from, distance });
		}
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error executing RoadGraph query: " << sqlite3_errmsg(db) << std::endl;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	std::cout << "RoadGraph loaded: " << nodes_.size() << " nodes, " << adjacency_.size() << " connected nodes" << std::endl;

}

const NodeInfo& RoadGraph::getNode(int id) const
{
	static NodeInfo emptyNode{ 0, 0.0f, 0.0f, 0.0f, 0.0f };
	auto it = nodes_.find(id);
	if (it != nodes_.end()) {
		return it->second;
	}
	return emptyNode;
}

const std::vector<Edge>& RoadGraph::getEdges(int id) const
{
	static std::vector<Edge> emptyEdges;
	auto it = adjacency_.find(id);
	if (it != adjacency_.end()) {
		return it->second;
	}
	return emptyEdges;
}

const std::vector<int>& RoadGraph::allNodeIds() const
{
	return nodeIdList_;
}

const std::unordered_map<int, NodeInfo>& RoadGraph::allNodeInfo() const
{
	return nodes_;
}


static float haversine(float lat1, float lon1, float lat2, float lon2) {
	const float R = 6371000.0f;   // Earth radius in metres
	float dLat = glm::radians(lat2 - lat1);
	float dLon = glm::radians(lon2 - lon1);
	float a = std::sin(dLat / 2) * std::sin(dLat / 2)
		+ std::cos(glm::radians(lat1)) * std::cos(glm::radians(lat2))
		* std::sin(dLon / 2) * std::sin(dLon / 2);
	return R * 2.0f * std::atan2(std::sqrt(a), std::sqrt(1.0f - a));
}


int KDTree::buildNode(std::vector<int>& indices, int lo, int hi, int depth) {
	if (lo >= hi) return -1;
	int axis = depth % 2;
	// sort [lo, hi) by the axis coordinate
	std::sort(indices.begin() + lo, indices.begin() + hi,
		[&](int a, int b) { return pts[a].coord(axis) < pts[b].coord(axis); });
	int mid = (lo + hi) / 2;
	int nodeIdx = static_cast<int>(tree.size());
	tree.push_back({ indices[mid], -1, -1, axis, pts[indices[mid]].coord(axis) });
	tree[nodeIdx].left = buildNode(indices, lo, mid, depth + 1);
	tree[nodeIdx].right = buildNode(indices, mid + 1, hi, depth + 1);
	return nodeIdx;
}

void KDTree::searchNode(int idx, float qx, float qy, int& bestId, float& bestDist2) const{
	if (idx == -1) return;
	const KDNode& kn = tree[idx];
	// Check the split point itself
	const Point& p = pts[kn.point_idx];
	float d2 = (p.x - qx) * (p.x - qx) + (p.y - qy) * (p.y - qy);
	if (d2 < bestDist2) { bestDist2 = d2; bestId = p.node_id; }
	// Which side is the query on?
	float diff = (kn.axis == 0) ? (qx - kn.split_val) : (qy - kn.split_val);
	int near = (diff <= 0) ? kn.left : kn.right;
	int far = (diff <= 0) ? kn.right : kn.left;
	searchNode(near, qx, qy, bestId, bestDist2);
	// Only visit the far side if the splitting plane is closer than current best
	if (diff * diff < bestDist2)
		searchNode(far, qx, qy, bestId, bestDist2);
}


int KDTree::nearestNode(float x, float y) const{
	int bestId = -1;
	float bestDist2 = std::numeric_limits<float>::max();
	searchNode(0, x, y, bestId, bestDist2);
	return bestId;
}


void KDTree::build(const std::vector<int>& nodeIds, const std::unordered_map<int, NodeInfo>& nodes) {

	pts.clear();
	tree.clear();

	// Copy nodes to pts
	for (int id : nodeIds) {
		const NodeInfo& node = nodes.at(id);
		pts.push_back({ id, node.wx, node.wy });
	}

	// Create index array
	std::vector<int> indices(nodeIds.size());
	for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;

	// Build tree
	buildNode(indices, 0, indices.size(), 0);

}



static float heuristic(const RoadGraph& graph, int from_id, int to_id) {
    const NodeInfo& fromNode = graph.getNode(from_id);
    const NodeInfo& toNode = graph.getNode(to_id);
    return haversine(fromNode.lat, fromNode.lon, toNode.lat, toNode.lon);
}

std::vector<int> AstarPathFinding(const RoadGraph& graph, int start_id, int goal_id) {
	using PQEntry = std::pair<float, int>;   // (f_value, node_id)
	std::priority_queue<PQEntry,
		std::vector<PQEntry>,
		std::greater<PQEntry>> open;   // min-heap
	std::unordered_map<int, float> g_cost;
	std::unordered_map<int, int>   came_from;
	std::unordered_set<int>        closed;
	g_cost[start_id] = 0.0f;
	open.push({ heuristic(graph, start_id, goal_id), start_id });
	while (!open.empty()) {
		auto [f, cur] = open.top();
		open.pop();
		if (closed.count(cur)) continue;   // stale entry skip
		closed.insert(cur);
		if (cur == goal_id) break;         // found it
		for (const Edge& e : graph.getEdges(cur)) {
			if (closed.count(e.neighbour_id)) continue;
			float tentative_g = g_cost[cur] + e.distance_m;
			if (!g_cost.count(e.neighbour_id) ||
				tentative_g < g_cost[e.neighbour_id]) {
				g_cost[e.neighbour_id] = tentative_g;
				came_from[e.neighbour_id] = cur;
				float f_new = tentative_g
					+ heuristic(graph, e.neighbour_id, goal_id);
				open.push({ f_new, e.neighbour_id });
			}
		}
	}
	// Reconstruct path
	if (!came_from.count(goal_id) && goal_id != start_id) return {};
	std::vector<int> path;
	for (int n = goal_id; n != start_id; n = came_from[n])
		path.push_back(n);
	path.push_back(start_id);
	std::reverse(path.begin(), path.end());
	return path;
}


void KDTree::radiusQuery(float qx, float qy, float radius,
	std::vector<int>& outIds) const {
	outIds.clear();
	if (tree.empty()) return;
	radiusSearchNode(tree, 0, qx, qy, radius * radius, outIds);
}

void KDTree::radiusSearchNode(const std::vector<KDNode>& nodes, int idx,
	float qx, float qy, float r2,
	std::vector<int>& out) const {
	if (idx < 0 || idx >= (int)nodes.size()) return;
	const KDNode& n = nodes[idx];
	float dx = qx - pts.at(n.point_idx).x;
	float dy = qy - pts.at(n.point_idx).y;
	if (dx * dx + dy * dy <= r2)
		out.push_back(pts.at(n.point_idx).node_id);
	// Which axis did this level split on?
	bool splitOnX = (n.axis == 0);  // or however your KDNode stores split axis
	float diff = splitOnX ? dx : dy;
	int near = (diff < 0) ? n.left : n.right;
	int far_ = (diff < 0) ? n.right : n.left;
	radiusSearchNode(nodes, near, qx, qy, r2, out);
	// Only visit far child if it could contain points within radius
	if (diff * diff <= r2)
		radiusSearchNode(nodes, far_, qx, qy, r2, out);
}




