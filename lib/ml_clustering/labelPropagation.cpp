#include <ml_clustering/labelPropagation.h>

// returns the new number of toDoNodes
NodeID labelPropagateIterationGreedy(graph_access& graph, std::vector<NodeID>& toDoNodes, std::vector<PartitionID>& labels, const std::vector<float>& edgeProbs, std::vector<NodeID>& labelNodeCounts) {


	std::vector<int> nodeLabelChanged(graph.number_of_nodes(), 0);

	// for every node find the best neighbor
	for (auto currentNode : toDoNodes) {

		if (currentNode == 224) {
			std::cout << "224 detected" << std::endl;
		}


		// find all weighted frequencies of the labels
		std::map<PartitionID, float> gains;
		forall_out_edges(graph, e, currentNode)

			NodeID target = graph.getEdgeTarget(e);
			gains[labels[target]] += edgeProbs[e];

		endfor



		// find the highest frequency label
		std::pair<PartitionID, float> maxPair = {0, 0};
		for (auto pair : gains) {


			if (pair.second > maxPair.second) {
				maxPair = pair;
			}
		}

		
		nodeLabelChanged[currentNode] = labels[currentNode] != maxPair.first;

		if (nodeLabelChanged[currentNode]) {

			// subtract the count of the old label
			labelNodeCounts[labels[currentNode]] -= 1;

			// add it to the new label
			labelNodeCounts[maxPair.first] += 1;
			labels[currentNode] = maxPair.first;

		}
			
	}


	// create new toDo for next iteration
	toDoNodes = std::vector<NodeID>();

	// add the neighbors to the todo as well
	forall_nodes(graph, node)

		// if the node itself changed
		if (nodeLabelChanged[node]){

			// all neighbors will be added
			// but their neighbors will not be added
			forall_out_edges(graph, e, node)
					nodeLabelChanged[graph.getEdgeTarget(e)] = 2;
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


// returns the new number of toDoNodes
NodeID labelPropagateIteration(graph_access& graph, std::vector<NodeID>& toDoNodes, std::vector<PartitionID>& labels, const std::vector<float>& edgeProbs, PartitionID maxClusterSize, std::vector<NodeID>& labelNodeCounts, std::vector<float>& currentValues) {

	


	std::vector<bool> nodeLabelChanged(graph.number_of_nodes(), false);

	// for every node find the best neighbor
	for (auto currentNode : toDoNodes) {


		std::map<PartitionID, float> gains;
		forall_out_edges(graph, e, currentNode)

			auto targetNode = graph.getEdgeTarget(e);

			// add probabilites
			gains[labels[targetNode]] += edgeProbs[e];
			
		endfor

		// find best label
		std::pair<PartitionID, float> maxPair = *(gains.begin());
		for (auto pair : gains) {

			// only allow not full clusters
			if (maxClusterSize != 0 && labelNodeCounts[pair.first] >= maxClusterSize) {
				continue;
			}

			if (pair.second > maxPair.second) {
				maxPair = pair;
			}
		}


		// only allow not full clusters (if "full" exists)
		//bool canChangeLabel = maxClusterSize == 0 || labelNodeCounts[maxPair.first] < maxClusterSize - 1;
		
		// only allow if the gain is positive
		if (maxPair.second + 5 > currentValues[currentNode]) {

			nodeLabelChanged[currentNode] = labels[currentNode] != maxPair.first;

			if (nodeLabelChanged[currentNode]) {

				// subtract the count of the old label
				labelNodeCounts[labels[currentNode]] -= 1;

				// add it to the new label
				labelNodeCounts[maxPair.first] += 1;
				labels[currentNode] = maxPair.first;

				// update the values of all neighbors
				forall_out_edges(graph, e, currentNode)
					NodeID targetNode = graph.getEdgeTarget(e);
					bool hasSameLabel = labels[currentNode] == labels[targetNode];
					currentValues[targetNode] += edgeProbs[e] * (hasSameLabel - (!hasSameLabel));
				endfor
			}
			
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

	


	// initialize the labels with the nodeIDs
	std::vector<PartitionID> labels(toDoNodes);

	// initialize the values of the nodes
	std::vector<float> nodeValues(graph.number_of_nodes(), 0);

	
	std::cout << "starting iterations" << std::endl;

	for (int i = 0; i < iterations; i++) {

		if (randomOrder) {
		std::random_shuffle(toDoNodes.begin(), toDoNodes.end());
		}

		
		if (!labelPropagateIterationGreedy(graph, toDoNodes, labels, edgeProbs, labelNodeCounts)) {
			std::cout << "Finished Label Propagation in Iteration " << i << std::endl; 
			break;
		}



		std::cout << "next iteration with " << toDoNodes.size() << " nodes to propagate over" << std::endl;
	}


	return labels;

}


