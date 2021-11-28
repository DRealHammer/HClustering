#pragma once

#include <KaHIP/graph_io.h>
#include <KaHIP/graph_access.h>
#include <fstream>
#include <sstream>


void graphToMTX(graph_access& graph, const std::string outputFilename);