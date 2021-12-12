#pragma once

#include <graph_io.h>
#include <data_structure/graph_access.h>
#include <fstream>
#include <sstream>


void graphToMTX(graph_access& graph, const std::string outputFilename);