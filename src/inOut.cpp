#include <inOut.h>


void writeGraphFeatureFile(std::string graphFilename, std::string outputFilename, std::string communityFilename, std::string graphletFilename) {

	graph_io IO;
	graph_access graph;
	IO.readGraphWeighted(graph, graphFilename);

	

	std::map<NodeID, std::vector<CommID>> comms;

	if (communityFilename.size() != 0) {
		comms = readCommunityFile(communityFilename);
	}

	// check if we have to create the graphlet file
	if (graphletFilename.size() == 0) {
		
		std::cout << "no graphlet file given:\ncreating mtx file..." << std::endl;
		std::string mtxGraphFilename = graphFilename + ".mtx";
		graphToMTX(graph, mtxGraphFilename);

		std::cout << "creating graphlet file..." << std::endl;
		graphletFilename = graphFilename + ".graphlet";
		createGraphletFile(mtxGraphFilename, graphletFilename);

		// TODO correctly order the file (currently with translate_graphlet.sh) and delete first line
		// use a bash command for sorting the file fast
		std::string sortCommand = "cat " + graphletFilename + " | sort -n -k 2 | sort -n -s >> " + graphletFilename + "-s && mv " + graphletFilename + "-s " + graphletFilename;

		system(sortCommand.c_str());
	}

	std::cout << "finished creating graphlet files" << std::endl;



	writeFeaturesInFile(graph, outputFilename, comms, graphletFilename);

}



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


bool writeFeaturesInFile(graph_access& graph, std::string outputFilename, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string graphletFilename) {

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
		#ifdef FEATURE_SHARED_NEIGHBOR_COUNT
			std::vector<std::vector<unsigned long>> neighbors(graph.number_of_nodes());

			forall_nodes(graph, n)
				forall_out_edges(graph, e, n)

					neighbors[n].push_back(graph.getEdgeTarget(e));

				endfor
			endfor



			{
				#pragma omp parallel for
				for(NodeID n = 0; n < graph.number_of_nodes(); ++n) {
					forall_out_edges(graph, e, n)
						NodeID targetNode = graph.getEdgeTarget(e);
						
						std::vector<unsigned long> sharedNeighbors;


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
			
			
			{ 
				#pragma omp parallel for
				for(NodeID node = 0; node < graph.number_of_nodes(); ++node) {
				localClusteringCoefficients[node] = clusteringCoefficient(graph, node);
			endfor
		#endif
	}


	// create the labels

	#ifdef LABEL
		std::vector<bool> labels(graph.number_of_edges());
		bool hasLabels = nodeCommunites.size() != 0;
		// we have given communities
		if (hasLabels) {

			// for all edges
			{ 
				#pragma omp parallel for
				for(NodeID n = 0; n < graph.number_of_nodes(); ++n) {
					forall_out_edges(graph, e, n)

						NodeID edgeTarget = graph.getEdgeTarget(e);

						// write the label first
						labels[e] = shareCommunity(nodeCommunites[n], nodeCommunites[edgeTarget]);

					endfor
			endfor

		}

		


	#endif

	#ifdef FEATURE_GRAPHLETS
		std::ifstream graphletFile(graphletFilename);

	#endif

	// finish the data creation, now fill them into the final features vector
	
	{

		Timer t("writing the file");
		// add the features in a file
		//std::ofstream featureFile(fileName, std::ios::out | std::ios::binary);
		std::FILE* f = std::fopen(outputFilename.c_str(), "w");

		#define XGFeature(nr, value) " " << nr << ":" << static_cast<float>(value)
		#define featureWrite(file, nr, val) std::fprintf(file, " %d:%.3f", nr, static_cast<float>(val));

		// for all edges
		forall_nodes(graph, n)
			forall_out_edges(graph, e, n)


				NodeID edgeTarget = graph.getEdgeTarget(e);

				// write only small - big edges
				if (n > edgeTarget) {
					continue;
				}


				#ifdef LABEL
					//featureFile << labels[e] * 1.0f;
					//auto label = std::to_string(labels[e]);
					//featureFile.write(label.c_str(), label.size());
					if (hasLabels) {
						std::fprintf(f, "%d", static_cast<int>(labels[e]));
					}
					
		
				#endif

				#ifdef FEATURE_NODE_DEGREE_START
					// add both side degrees
					//featureFile << XGFeature(FEATURE_NODE_DEGREE_START, nodeDegrees[n]);
					//auto featureNodeDegreeStart = XGFeatureString(FEATURE_NODE_DEGREE_START, nodeDegrees[n]);
					//featureFile.write(featureNodeDegreeStart.c_str(), featureNodeDegreeStart.size());
					featureWrite(f, FEATURE_NODE_DEGREE_START, nodeDegrees[n]);
				#endif

				#ifdef FEATURE_NODE_DEGREE_TARGET
					// add both side degrees
					//featureFile << XGFeature(FEATURE_NODE_DEGREE_TARGET, nodeDegrees[edgeTarget]);
					//auto featureNodeDegreeEnd = XGFeatureString(FEATURE_NODE_DEGREE_TARGET, nodeDegrees[edgeTarget]);
					//featureFile.write(featureNodeDegreeEnd.c_str(), featureNodeDegreeEnd.size());
					featureWrite(f, FEATURE_NODE_DEGREE_TARGET, nodeDegrees[edgeTarget]);
				#endif

				#ifdef FEATURE_NODE_COUNT
					//featureFile << XGFeature(FEATURE_NODE_COUNT, nodeCount);
					//auto featureNodeCount = XGFeatureString(FEATURE_NODE_COUNT, nodeCount);
					//featureFile.write(featureNodeCount.c_str(), featureNodeCount.size());
					featureWrite(f, FEATURE_NODE_COUNT, nodeCount);
				#endif

				#ifdef FEATURE_EDGE_COUNT
					//featureFile << XGFeature(FEATURE_EDGE_COUNT, edgeCount);
					//auto featureEdgeCount = XGFeatureString(FEATURE_EDGE_COUNT, edgeCount);
					//featureFile.write(featureEdgeCount.c_str(), featureEdgeCount.size());
					featureWrite(f, FEATURE_EDGE_COUNT, edgeCount);
				#endif

				#ifdef FEATURE_SHARED_NEIGHBOR_COUNT

					//featureFile << XGFeature(FEATURE_SHARED_NEIGHBOR_COUNT, sharedNeighborCounts[e]);
					//auto featureSharedNCount = XGFeatureString(FEATURE_SHARED_NEIGHBOR_COUNT, sharedNeighborCounts[edgeTarget]);
					//featureFile.write(featureSharedNCount.c_str(), featureSharedNCount.size());
					featureWrite(f, FEATURE_SHARED_NEIGHBOR_COUNT, sharedNeighborCounts[e]);
				#endif

				#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START
					//featureFile << XGFeature(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START, localClusteringCoefficients[n]);
					//auto featureClustCoeffStart = XGFeatureString(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START, localClusteringCoefficients[n]);
					//featureFile.write(featureClustCoeffStart.c_str(), featureClustCoeffStart.size());
					featureWrite(f, FEATURE_CLUSTERING_COEFFICIENT_LOCAL_START, localClusteringCoefficients[n]);
				#endif

				#ifdef FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET
					//featureFile << XGFeature(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET, localClusteringCoefficients[edgeTarget]);
					//auto featureClustCoeffTarget = XGFeatureString(FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET, localClusteringCoefficients[edgeTarget]);
					//featureFile.write(featureClustCoeffTarget.c_str(), featureClustCoeffTarget.size());
					featureWrite(f, FEATURE_CLUSTERING_COEFFICIENT_LOCAL_TARGET, localClusteringCoefficients[edgeTarget]);
				#endif

				#ifdef FEATURE_GRAPHLETS
					// read the current line of the graphlet file
					std::size_t value;

					// skip start node
					graphletFile >> value;
					// skip target node
					graphletFile >> value;

					// read and write the values

					for (int i = 0; i < 8; i++) {
						graphletFile >> value;
						featureWrite(f, FEATURE_GRAPHLETS + i, value);
					}


				#endif


				// at the end of the file add a new line for the next edge
				//featureFile << "\n";
				//featureFile.write("\n", 1);
				std::fprintf(f, "\n");

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

/*

	procudes a file outputFile with an (currently unordered) listing for every edge (u, v) with u < v
	of graphlets

	triangle
	2-star
	4-clique
	4-chordal-cycle
	4-tailed-triangle
	4-cycle
	3-star
	4-path

*/
void createGraphletFile(std::string mtxGraphFilename, std::string outputFilename) {

	std::ifstream testFile(mtxGraphFilename);

	if(!testFile.is_open()) {
		std::cout << "Error opening file " << mtxGraphFilename << std::endl; 
		return;
	}
	
	params p;

	p.algorithm = "exact";
	p.verbose = false;
	p.graph_stats = false;
	p.help = false;
	p.graph = mtxGraphFilename;
	p.macro_stats_filename = "";
	p.micro_stats_filename = "";
	p.workers = omp_get_max_threads();
	p.block_size = 64;
	p.ordering = "";
	p.is_small_to_large = false;
	p.ordering_csc_neighbor = "";
	p.is_small_to_large_csc_neighbor = false;
	p.graph_representation = "";
	p.adj_limit = 10000;
	p.density_cutoff = 0.80;


	graphlet::graphlet_core G(p);
	G.graphlet_decomposition_micro(p.workers);
	std::string outFile = outputFilename;

	// we want the ids of the nodes
	G.write_micro_stats(outFile, true);
}