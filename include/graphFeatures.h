#pragma once

// std lib
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>

// ml lib
#include <xgboost/c_api.h>

// graph data structure
#include <KaHIP/graph_io.h>

// profiling
#include <profiling.h>

typedef int CommID;

#define EMPTY_VALUE -1.0f


#define FEATURE_NODE_DEGREE_START 0
#define FEATURE_NODE_DEGREE_TARGET 1
#define FEATURE_NODE_COUNT 2
#define FEATURE_EDGE_COUNT 3
#define FEATURE_SHARED_NEIGHBOR_COUNT 4
#define FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START 5
#define FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET 6
#define FEATURE_CLUSTERING_COEFFICIENT_LOCAL_MEAN 7



bool shareCommunity(const std::vector<CommID>& aComms, const std::vector<CommID>& bComms);
float clusteringCoefficient(graph_access& graph, NodeID node);