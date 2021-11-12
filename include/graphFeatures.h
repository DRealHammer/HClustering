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

typedef int CommID;

#define EMPTY_VALUE -1.0f


#define FEATURE_NODE_DEGREE_START 0
#define FEATURE_NODE_DEGREE_TARGET 1
#define FEATURE_NODE_COUNT 2
#define FEATURE_EDGE_COUNT 3
#define FEATURE_SHARED_NEIGHBOR_COUNT 4
#define FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START 5
#define FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET 6


std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName);

bool writeTrainingDataInFile(graph_access& graph, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string fileName);
std::vector<float> getFeatureVector(graph_access& graph);
void printFeatures(const std::vector<float> features, int rows, int columns);

bool shareCommunity(const std::vector<CommID>& aComms, const std::vector<CommID>& bComms);

std::vector<float> getCommunityLabels(std::string filename, graph_access& graph);

/* Produce a DMatrixHandle out of a graph and if wished a communities file
	\param graphFilename the metisfile of the graph
	\param communityFilename leave empty string if no file is wished or exists
	\param selectedFeatures a vector of all the features that are wished in the correct order
	\return a DMatrixHandle
*/
DMatrixHandle createGraphCommunitiesDMatrix(std::string graphFilename, std::string communityFilename);

float clusteringCoefficient(graph_access& graph, NodeID node);