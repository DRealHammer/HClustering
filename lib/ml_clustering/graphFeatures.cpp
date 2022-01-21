#include <ml_clustering/graphFeatures.h>


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

struct BFSVisitNode {
	NodeID id;
	NodeID parent;
};

void performBFS(graph_access& graph, NodeID currentNode, std::queue<BFSVisitNode>& queue, std::vector<NodeID>& visited, std::vector<float>& visitCounts, std::vector<NodeID>& parents) {

	visited[currentNode] = currentNode;
	// add all neighbors to the queue
	forall_out_edges(graph, e, currentNode)
		queue.push({graph.getEdgeTarget(e), currentNode});
	endfor

	BFSVisitNode visitNode;
	// visit all nodes
	while(!queue.empty()) {
		visitNode = queue.front();
		queue.pop();

		// we have seen this node before, skip it
		if (visited[visitNode.id] == currentNode) {
			continue;
		}

		// we see this node for the first time
		visited[visitNode.id] = currentNode;
		parents[visitNode.id] = visitNode.parent;

		// add all possible children
		forall_out_edges(graph, e, visitNode.id)
			queue.push({graph.getEdgeTarget(e), visitNode.id});
		endfor;
	}

	// we have a shortest path tree from the current node to all others
	// we go from all nodes to the currentNode and add 1 on each visited node on the way

	NodeID runningNode;
	forall_nodes(graph, node)
		runningNode = node;

		// skip unconnected nodes
		if (visited[runningNode] != currentNode) {
			continue;
		}

		while(runningNode != currentNode) {
			visitCounts[runningNode] += 1;
			runningNode = parents[runningNode];
		}

	endfor
}

std::vector<float> nodeCentralities(graph_access& graph) {

	std::vector<float> result(graph.number_of_nodes());
	std::queue<BFSVisitNode> queue;
	std::vector<NodeID> visited(graph.number_of_nodes(), -1);
	std::vector<NodeID> parents(graph.number_of_nodes());

	forall_nodes(graph, node)

		//std::cout << "bfs for node " << node << std::endl;
		performBFS(graph, node, queue, visited, result, parents);

	endfor

	float divider = 2 * (graph.number_of_nodes() - 1 ) * (graph.number_of_nodes() - 2);

	for (auto& c : result) {
		c = c / divider;
	}
	
	return result;
}