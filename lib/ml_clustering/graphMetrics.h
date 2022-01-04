#pragma once
#include <graph_io.h>
#include <algorithm>

float conductance(graph_access& graph, std::vector<PartitionID>& clustering);