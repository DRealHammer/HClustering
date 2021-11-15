#pragma once

#include <graphFeatures.h>


std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName);
bool writeTrainingDataInFile(graph_access& graph, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string fileName);
void writeGraphFile(std::string graphName);
std::vector<float> getCommunityLabels(std::string filename, graph_access& graph);