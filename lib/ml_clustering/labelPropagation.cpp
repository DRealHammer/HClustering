#include <ml_clustering/labelPropagation.h>


// returns the new number of toDoNodes
NodeID labelPropagateIteration(graph_access& graph, std::vector<NodeID>& toDoNodes, std::vector<PartitionID>& labels, const std::vector<float>& edgeProbs, PartitionID maxClusterSize, std::vector<NodeID>& labelNodeCounts) {

	std::vector<bool> nodeLabelChanged(graph.number_of_nodes(), false);

	// for every node find the best neighbor
	for (auto startNode : toDoNodes) {

		std::map<PartitionID, float> labelValues;
		forall_out_edges(graph, e, startNode)

			auto targetNode = graph.getEdgeTarget(e);

			// add probabilites
			labelValues[labels[targetNode]] += edgeProbs[e];
			
		endfor

		// find best label
		std::pair<PartitionID, float> maxPair = *(labelValues.begin());
		for (auto pair : labelValues) {

			// only allow not full clusters
			if (maxClusterSize != 0 && labelNodeCounts[pair.first] >= maxClusterSize) {
				continue;
			}

			if (pair.second > maxPair.second) {
				maxPair = pair;
			}
		}

		// only allow not full clusters (if "full" exists)
		bool canChangeLabel = maxClusterSize == 0 || labelNodeCounts[maxPair.first] < maxClusterSize - 1;
		
		// only allow if the gain is positive
		if (canChangeLabel && maxPair.second > 0) {

			nodeLabelChanged[startNode] = labels[startNode] != maxPair.first || nodeLabelChanged[startNode];

			// subtract the count of the old label
			labelNodeCounts[labels[startNode]] -= 1;

			// add it to the new label
			labelNodeCounts[maxPair.first] += 1;
			labels[startNode] = maxPair.first;
		}

	}


	// create new toDo for next iteration
	toDoNodes = std::vector<NodeID>();

	// first set every neighbor of a changed node to have changed too
	forall_nodes(graph, node)

		if (nodeLabelChanged[node]){

			forall_out_edges(graph, e, node)
					nodeLabelChanged[graph.getEdgeTarget(e)] = true;
			endfor
		}

	endfor

	// add all nodes to the list
	forall_nodes(graph, node) 

		if (nodeLabelChanged[node]) {
			toDoNodes.push_back(node);
		}

	endfor

	return toDoNodes.size();
}

std::vector<PartitionID> labelPropagate(graph_access& graph, const std::vector<float>& edgeProbs, int iterations, bool randomOrder, PartitionID maxClusterSize) {

	std::vector<NodeID> toDoNodes(graph.number_of_nodes());
	std::iota(std::begin(toDoNodes), std::end(toDoNodes), 0);

	// number of nodes per label/cluster
	std::vector<NodeID> labelNodeCounts(graph.number_of_nodes(), 0);

	if (randomOrder) {
		std::random_shuffle(toDoNodes.begin(), toDoNodes.end());
	}


	// initialize the labels with the nodeIDs
	std::vector<PartitionID> labels(toDoNodes);

	
	std::cout << "starting iterations" << std::endl;

	for (int i = 0; i < iterations; i++) {

		

		if (!labelPropagateIteration(graph, toDoNodes, labels, edgeProbs, maxClusterSize, labelNodeCounts)) {
			std::cout << "Finished Label Propagation in Iteration " << i << std::endl; 
			break;
		}

		std::cout << "changed " << toDoNodes.size() << " labels" << std::endl;
	}


	return labels;

}


