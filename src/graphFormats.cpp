#include <translation/graphFormats.h>


void graphToMTX(graph_access& graph, const std::string outputFilename) {

	// write the first line (nodecount nodecount edgecount)
	// the edge count is the undirected edge count
	// every edge appears only once (smallnode bignode), the other direction is only implied

	std::ofstream out(outputFilename);
	const char* delim = " ";

	// graph_access has number of unweighted edges -> / 2 
	out << graph.number_of_nodes() << delim << graph.number_of_nodes() << delim << graph.number_of_edges()/2 << std::endl;

	forall_nodes(graph, startNode) 

		forall_out_edges(graph, e, startNode)

			NodeID targetNode = graph.getEdgeTarget(e);

			if (targetNode < startNode) {
				continue;
			}

			out << startNode + 1 << delim << targetNode + 1 << std::endl;


		endfor
	endfor

}