#include <graphFeatures.h>
#include <profiling.h>


float clusteringCoefficient(graph_access& graph, NodeID node) {

	// the node is not connected at all
	// or only to one
	if (graph.getNodeDegree(node) <= 1) {
		return 0;
	}

	// get the amount of edges in the neighborhood

	// first get the neighborhood
	std::vector<NodeID> neighborhood;

	forall_out_edges(graph, e, node)

		neighborhood.push_back(graph.getEdgeTarget(e));

	endfor

	// sort the neighborhood for faster counting
	std::sort(neighborhood.begin(), neighborhood.end());

	// get edge amount of connected
	int edgeCount = 0;

	// for all neighbors
	// find the amount of edges that are inside the neighborhood
	for (int i = 0; i < neighborhood.size(); i++) {

		NodeID currentNode = neighborhood[i];

		// only look for a neighbor with a bigger id
		int searchPos = i + 1;

		// go over all neighbors and check if they are in the neighborhood of node
		forall_out_edges(graph, e, currentNode)

			// the edge of the currentNode is within the neighborhood
			if (std::binary_search(neighborhood.begin() + searchPos, neighborhood.end(), graph.getEdgeTarget(e))) {

				// so we found an edge and increase the count
				edgeCount++;
			}


		endfor

	}

	// return the local clustering coefficent for undirected graphs
	return 2.0 * edgeCount / static_cast<float>(neighborhood.size() * ( neighborhood.size() - 1 ));


}


bool shareCommunity(const std::vector<CommID>& aComms, const std::vector<CommID>& bComms) {

	std::vector<CommID> result;

	std::set_intersection(
		aComms.begin(), aComms.end(),
		bComms.begin(), bComms.end(),
		std::back_inserter(result)
	);

	return result.size() > 0;
}