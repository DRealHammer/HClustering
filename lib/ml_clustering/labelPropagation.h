#pragma once

#include <graph_io.h>
#include <map>
#include <numeric>
#include <algorithm>


// receive an graph and the edge weights (maybe move the weights in the graph)
std::vector<PartitionID> labelPropagate(graph_access& graph, const std::vector<float>& edgeProbs, int iterations, bool randomOrder = false, PartitionID maxClusterSize = 0);
NodeID labelPropagateIteration(graph_access& graph, std::vector<NodeID>& toDoNodes, std::vector<PartitionID>& labels, const std::vector<float>& edgeProbs,  std::vector<float>& currentValues);