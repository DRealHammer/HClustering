#pragma once

// std lib
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>

// openmp
#include <omp.h>

// ml lib
#include <xgboost/c_api.h>

// graph data structure
#include <graph_io.h>


typedef int CommID;

#define EMPTY_VALUE -1.0f

#define LABEL
#define FEATURE_NODE_DEGREES 0
#define FEATURE_NODE_COUNT 2
#define FEATURE_EDGE_COUNT 3
#define FEATURE_SHARED_NEIGHBOR_COUNT 4
#define FEATURE_CLUSTERING_COEFFICIENTS_LOCAL 5
#define FEATURE_GRAPHLETS 7
// reserve until 20

enum FEATURE {
	NODE_DEGREES,
	NODE_COUNT,
	EDGE_COUNT,
	SHARED_NEIGHBOR_COUNT,
	CLUSTERING_COEFFICIENTS_LOCAL,
	GRAPHLETS,
	SECOND_DEGREE,
	CENTRALITY
};

bool shareCommunity(const std::vector<CommID>& aComms, const std::vector<CommID>& bComms);
float clusteringCoefficient(graph_access& graph, NodeID node);
std::vector<float> nodeCentralities(graph_access& graph);