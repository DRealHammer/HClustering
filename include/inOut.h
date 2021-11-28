#pragma once

#include <graphFeatures.h>
#include <cstdio>
#include <PGD/graphlet_core.h>
#include <translation/graphFormats.h>


std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName);
bool writeFeaturesInFile(graph_access& graph, std::string outputFilename, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string graphletFilename = std::string());
void writeGraphFeatureFile(std::string graphFilename, std::string outputFilename, std::string communityFilename = std::string(), std::string graphletFilename = std::string());
std::vector<float> getCommunityLabels(std::string filename, graph_access& graph);

void createGraphletFile(std::string mtxGraphFilename, std::string outputFilename);