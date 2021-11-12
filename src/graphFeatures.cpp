#include <graphFeatures.h>

std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName) {

	std::map<NodeID, std::vector<CommID>> comms;


	std::ifstream file(fileName, std::ios::in);

	if (!file.is_open()) {
		std::cout << "Alert: File " << fileName << " could not be opened!" << std::endl;
	}

	// read the file
	while (!file.eof()) {
		std::string line;
		std::getline(file, line, '\n');

		std::stringstream lineStream(line);

		NodeID nodeID;
		lineStream >> nodeID;
		//std::cout << nodeID << "'s Communities found" << std::endl;

		while (!lineStream.eof()) {
			CommID id;
			lineStream >> id;
			comms[nodeID].push_back(id);
		}
	}
	
	return comms;

}

// TODO add the communities and the label of the edge
bool writeTrainingDataInFile(graph_access& graph, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string fileName) {

	// create the data vectors if needed

	#if defined(FEATURE_NODE_DEGREE_START) || defined(FEATURE_NODE_DEGREE_TARGET)

		std::vector<unsigned long> nodeDegrees(graph.number_of_nodes());
		
		forall_nodes(graph, n)
			nodeDegrees[n] = graph.getNodeDegree(n);
		endfor

	#endif

	#ifdef FEATURE_NODE_COUNT
		unsigned long nodeCount = graph.number_of_nodes();
	#endif

	#ifdef FEATURE_EDGE_COUNT
		unsigned long edgeCount = graph.number_of_edges();
	#endif

	
	

	// TODO improve performance
	#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
		std::vector<std::vector<unsigned long>> neighbors(graph.number_of_nodes());

		forall_nodes(graph, n)
			forall_out_edges(graph, e, n)

				neighbors[n].push_back(graph.getEdgeTarget(e));

			endfor
		endfor

		// create the shared neighbor amounts
		std::vector<unsigned long> sharedNeighbors;

		std::vector<unsigned long> sharedNeighborCounts(graph.number_of_edges());
		forall_nodes(graph, n)
			forall_out_edges(graph, e, n)
				sharedNeighbors.clear();
				auto& startNeighbors = neighbors[n];
				auto& targetNeighbors = neighbors[graph.getEdgeTarget(e)];
				
				std::set_intersection(startNeighbors.begin(), startNeighbors.end(), targetNeighbors.begin(), targetNeighbors.end(), std::back_inserter(sharedNeighbors));
				sharedNeighborCounts[e] = sharedNeighbors.size();
			endfor
		endfor
	#endif

	#if defined(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START) || defined(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET)
		std::vector<float> localClusteringCoefficients(graph.number_of_nodes());

		forall_nodes(graph, node)
			localClusteringCoefficients[node] = clusteringCoefficient(graph, node);
		endfor
	#endif

	// finish the data creation, now fill them into the final features vector
	

	// add the features in a file
	std::ofstream featureFile(fileName, std::ios::out);

	#define XGFeature(nr, value) " " << nr << ":" << static_cast<float>(value)

	// for all edges
	forall_nodes(graph, n)
		forall_out_edges(graph, e, n)

			NodeID edgeTarget = graph.getEdgeTarget(e);

			// write the label first
			if ( shareCommunity(nodeCommunites[n], nodeCommunites[edgeTarget]) ) {
				featureFile << 1.0f;
			} else {
				featureFile << 0.0f;
			}

			#ifdef FEATURE_NODE_DEGREE_START
				// add both side degrees
				featureFile << XGFeature(FEATURE_NODE_DEGREE_START, nodeDegrees[n]);
			#endif

			#ifdef FEATURE_NODE_DEGREE_TARGET
				// add both side degrees
				featureFile << XGFeature(FEATURE_NODE_DEGREE_TARGET, nodeDegrees[edgeTarget]);

			#endif

			#ifdef FEATURE_NODE_COUNT
				featureFile << XGFeature(FEATURE_NODE_COUNT, nodeCount);

			#endif

			#ifdef FEATURE_EDGE_COUNT
				featureFile << XGFeature(FEATURE_EDGE_COUNT, edgeCount);

			#endif

			#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
				featureFile << XGFeature(FEATURE_SHARED_NEIGHBOR_COUNT, sharedNeighborCounts[e]);
			#endif

			#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START
				featureFile << XGFeature(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START, localClusteringCoefficients[n]);
			#endif

			#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET
				featureFile << XGFeature(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET, localClusteringCoefficients[edgeTarget]);
			#endif

			// at the end of the file add a new line for the next edge
			featureFile << std::endl;

		endfor
	endfor

	return true;
}


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

std::vector<float> getFeatureVector(graph_access& graph) {

	// create the data vectors if needed

	#ifdef FEATURE_NODE_DEGREES

		std::vector<unsigned long> nodeDegrees(graph.number_of_nodes());
		
		forall_nodes(graph, n)
			nodeDegrees[n] = graph.getNodeDegree(n);
		endfor

	#endif


	#ifdef FEATURE_NODE_COUNT
		unsigned long nodeCount = graph.number_of_nodes();
	#endif

	#ifdef FEATURE_EDGE_COUNT
		unsigned long edgeCount = graph.number_of_edges();
	#endif
	

	#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
		std::vector<std::vector<unsigned long>> neighbors(graph.number_of_nodes());

		forall_nodes(graph, n)
			forall_out_edges(graph, e, n)

				neighbors[n].push_back(graph.getEdgeTarget(e));

			endfor
		endfor

		// create the shared neighbor amounts
		std::vector<unsigned long> sharedNeighbors;

		std::vector<unsigned long> sharedNeighborCounts(graph.number_of_edges());
		forall_nodes(graph, n)
			forall_out_edges(graph, e, n)
				sharedNeighbors.clear();
				auto& startNeighbors = neighbors[n];
				auto& targetNeighbors = neighbors[graph.getEdgeTarget(e)];
				
				std::set_intersection(startNeighbors.begin(), startNeighbors.end(), targetNeighbors.begin(), targetNeighbors.end(), std::back_inserter(sharedNeighbors));
				sharedNeighborCounts[e] = sharedNeighbors.size();
			endfor
		endfor
	#endif

	// finish the data creation, now fill them into the final features vector
	

	// get the vectors of informations for all edges
	std::vector<float> features;

	// for all edges
	forall_nodes(graph, n)
		forall_out_edges(graph, e, n)


			#ifdef FEATURE_NODE_DEGREES
				// add both side degrees
				features.push_back(nodeDegrees[n]);
				features.push_back(nodeDegrees[graph.getEdgeTarget(e)]);
			#else
				//features.push_back(EMPTY_VALUE);
			#endif

			#ifdef FEATURE_NODE_COUNT
				features.push_back(nodeCount);
			#else
				//features.push_back(EMPTY_VALUE);
			#endif

			#ifdef FEATURE_EDGE_COUNT
				features.push_back(edgeCount);
			#else
				//features.push_back(EMPTY_VALUE);
			#endif

			#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
				features.push_back(sharedNeighborCounts[e]);
				
			#else
				//features.push_back(EMPTY_VALUE);
			#endif

		endfor
	endfor

	return features;

}

void printFeatures(const std::vector<float> features, int rows, int columns) {
	for (int i = 0; i < features.size(); i++) {
		std::cout << features[i];

		if ((i + 1) % columns == 0) {
			std::cout << std::endl;
		} else {
			std::cout << " ";
		}
	}
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

std::vector<float> getCommunityLabels(std::string filename, graph_access& graph) {

	std::ifstream communityFile(filename, std::ios::in);

	if (!communityFile.is_open()) {
		std::cout << "Alert: the communities file could not be opened!" << std::endl;
	}


	// save all communities of one node
	// by construction the sub arrays will be sorted (so easy finding of intersections) 
	std::vector<std::vector<CommID>> nodeCommunities(graph.number_of_nodes() + 1);


	std::string line;
	CommID communityNr = 0;
	while (!communityFile.eof()){
		std::getline(communityFile, line, '\n');
		
		std::stringstream stream(line);

		while(!stream.eof()) {

			// for every node of a community
			NodeID node;
			if(stream >> node) {

				// index shift as KaHIP uses 0 based indices
				node  -= 1;
				// add the community to the communitys of this node
				nodeCommunities[node].push_back(communityNr);

			}
		}

		// the next iteration will be the next community
		communityNr++;
	}


	// save all labels for the return
	std::vector<float> labels(graph.number_of_edges());

	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)

		NodeID targetNode = graph.getEdgeTarget(e);

			if (shareCommunity(nodeCommunities[startNode], nodeCommunities[targetNode])) {
				labels[e] = 1.0f;
				// std::cout << "community edge: " << startNode << " to " << targetNode << std::endl;
			} else {
				labels[e] = 0.0f;
			}

		endfor
	endfor

	return labels;

}

DMatrixHandle createGraphCommunitiesDMatrix(std::string graphFilename, std::string communityFilename) {

	// build a graph
	graph_io GraphIO;
	graph_access graph;
	GraphIO.readGraphWeighted(graph, graphFilename);

	auto features = getFeatureVector(graph);

	// calculate row and columns of the feature array
	int rows = graph.number_of_edges();
	int columns = 0;

	#ifdef FEATURE_NODE_DEGREES
		columns += 2;
	#endif

	#ifdef FEATURE_NODE_COUNT
		columns += 1;
	#endif

	#ifdef FEATURE_EDGE_COUNT
		columns += 1;
	#endif

	#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
		columns += 1;
	#endif


	// create a matrix for training
	DMatrixHandle dmat;
	XGDMatrixCreateFromMat(features.data(), rows, columns, EMPTY_VALUE, &dmat);

	// if the community file is given
	// create the labels and add them
	if (communityFilename.length()) {
		// get the communities as a float array
		std::vector<float> labels = getCommunityLabels(communityFilename, graph);

		
		XGDMatrixSetFloatInfo(dmat, "label", labels.data(), labels.size());
	}

	return dmat;

}