#pragma once
#include <graph_io.h>
#include <algorithm>
#include <map>

float conductance(graph_access& graph, std::vector<PartitionID>& clustering);