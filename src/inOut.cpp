#include <inOut.h>

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
	std::vector<unsigned long> nodeDegrees(graph.number_of_nodes());

	{
		Timer t("node degrees");
		#if defined(FEATURE_NODE_DEGREE_START) || defined(FEATURE_NODE_DEGREE_TARGET)

			
			forall_nodes(graph, n)
				nodeDegrees[n] = graph.getNodeDegree(n);
			endfor

		#endif
	}

	#ifdef FEATURE_NODE_COUNT
		unsigned long nodeCount = graph.number_of_nodes();
	#endif

	#ifdef FEATURE_EDGE_COUNT
		unsigned long edgeCount = graph.number_of_edges();
	#endif

	
	std::vector<unsigned long> sharedNeighborCounts(graph.number_of_edges());
	{
		Timer t("shared Neighbor Counts");
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


			forall_nodes(graph, n)
				forall_out_edges(graph, e, n)
					NodeID targetNode = graph.getEdgeTarget(e);

					// we already calculated the size of the shared neighbors of the edge targetNode -> n
					// we only need to find the correct position
					if (n > targetNode) {

						forall_out_edges(graph, e_, targetNode)
							if (graph.getEdgeTarget(e_) == n) {
								sharedNeighborCounts[e] = sharedNeighborCounts[e_];
								continue;
							}

						endfor
						continue;
					}

					sharedNeighbors.clear();
					auto& startNeighbors = neighbors[n];
					auto& targetNeighbors = neighbors[targetNode];
					
					std::set_intersection(startNeighbors.begin(), startNeighbors.end(), targetNeighbors.begin(), targetNeighbors.end(), std::back_inserter(sharedNeighbors));
					sharedNeighborCounts[e] = sharedNeighbors.size();

				endfor
			endfor
		#endif

	}

	std::vector<float> localClusteringCoefficients(graph.number_of_nodes());
	{
		Timer t("local clustering coefficient");

		#if defined(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START) || defined(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET) | defined(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_MEAN)
			

			forall_nodes(graph, node)
				localClusteringCoefficients[node] = clusteringCoefficient(graph, node);
			endfor
		#endif
	}

	#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_MEAN

		float localClusteringCoefficientMean = 0;
		for (auto coeff : localClusteringCoefficients) {
			localClusteringCoefficientMean += coeff;
		}
		localClusteringCoefficientMean /= localClusteringCoefficients.size();


	#endif

	// finish the data creation, now fill them into the final features vector
	
	{

		Timer t("writing the file (should be rather fast)");
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

				#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_MEAN
					featureFile << XGFeature(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_MEAN, localClusteringCoefficientMean);
				#endif

				// at the end of the file add a new line for the next edge
				featureFile << std::endl;

			endfor
		endfor
	}

	return true;
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

void writeGraphFile(std::string graphName) {

	std::string graphFile = "data/" + graphName + "/" + graphName + ".metis";
	std::string communityFile = "data/" + graphName + "/" + graphName + ".top5000.cmty.txt-nodes";
	std::string dataFile = "data/" + graphName + "-data";


	graph_io IO;
	graph_access graph;
	IO.readGraphWeighted(graph, graphFile);

	std::cout << graphName << std::endl;

	std::map<NodeID, std::vector<CommID>> comms;
	{
		Timer t("reading of Community file");
		comms = readCommunityFile(communityFile);
	}
	
	{
		Timer t("writing the data in the file");
		writeTrainingDataInFile(graph, comms, dataFile);
	}
}