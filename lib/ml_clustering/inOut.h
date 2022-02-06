#pragma once

#include <set>

#include <ml_clustering/graphFeatures.h>
#include <cstdio>
#include <PGD/graphlet_core.h>
#include <ml_clustering/graphFormats.h>
#include <ml_clustering/graphMetrics.h>


std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName);
void writeFeaturesInFile(graph_access& graph, std::string outputFilename, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::set<FEATURE>& selectedFeatures, std::string graphletFilename = std::string());
void writeGraphFeatureFile(std::string graphFilename, std::string outputFilename, const std::string communityFilename = std::string(), std::string graphletFilename = std::string(), std::string featureFilename = std::string());
std::vector<float> getCommunityLabels(std::string filename, graph_access& graph);

void createGraphletFile(std::string mtxGraphFilename, std::string outputFilename);