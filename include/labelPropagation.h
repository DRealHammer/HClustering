#pragma once

#include <KaHIP/graph_io.h>
#include <map>
#include <numeric>


// receive an graph and the edge weights (maybe move the weights in the graph)
// TODO add random ordering functionality
std::vector<PartitionID> labelPropagate(graph_access& graph, const std::vector<float>& edgeProbs, int iterations, bool randomOrder = false);
NodeID labelPropagateIteration(graph_access& graph, std::vector<NodeID>& toDoNodes, std::vector<PartitionID>& labels, const std::vector<float>& edgeProbs);