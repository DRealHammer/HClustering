#include <ml_clustering/inOut.h>


void writeGraphFeatureFile(std::string graphFilename, std::string outputFilename, std::string communityFilename, std::string graphletFilename, std::string featureFilename) {

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

		// use a bash command for sorting the file fast
		std::string sortCommand = "cat " + graphletFilename + " | sort -n -k 2 | sort -n -s >> " + graphletFilename + "-s && mv " + graphletFilename + "-s " + graphletFilename;

		system(sortCommand.c_str());
	}

	if (!std::ifstream(graphletFilename).is_open()) {
		std::cout << "couldn't open graphlet file" << std::endl;
		return;
	}

	std::cout << "finished creating graphlet files" << std::endl;

	std::ifstream featureFile(featureFilename);
	if(!featureFile.is_open()) {
		std::cout << "couldn't open feature file" << std::endl;
	}

	std::set<FEATURE> selectedFeaturesSet;
	while (!featureFile.eof()) {
		std::string line;
		std::getline(featureFile, line);


		// add the correct enum value for the specified argument
		// ignore wrong arguments
		if (line == "NODE_DEGREES") {
			selectedFeaturesSet.insert(NODE_DEGREES);
		} else if (line == "NODE_COUNT") {
			selectedFeaturesSet.insert(NODE_COUNT);
		} else if (line == "EDGE_COUNT") {
			selectedFeaturesSet.insert(EDGE_COUNT);
		} else if (line == "SHARED_NEIGHBOR_COUNT") {
			selectedFeaturesSet.insert(SHARED_NEIGHBOR_COUNT);
		} else if (line == "CLUSTERING_COEFFICIENTS_LOCAL") {
			selectedFeaturesSet.insert(CLUSTERING_COEFFICIENTS_LOCAL);
		} else if (line == "GRAPHLETS") {
			selectedFeaturesSet.insert(GRAPHLETS);
		}
	}

	writeFeaturesInFile(graph, outputFilename, comms, graphletFilename, selectedFeaturesSet);

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


void writeFeaturesInFile(graph_access& graph, std::string outputFilename, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::string graphletFilename, std::set<FEATURE> selectedFeatures) {

	// create the data vectors for all small big edges
	std::vector<std::vector<float>> edgeFeatures(graph.number_of_edges() / 2);
	std::vector<FEATURE> features(selectedFeatures.size());

	for (FEATURE f : selectedFeatures) {
		features.push_back(f);
	}
	std::sort(features.begin(), features.end());

	// index of the small->big edges

	for (FEATURE f : features) {
		int i = 0;

		switch (f)
		{
		case NODE_DEGREES:
		{
			forall_nodes(graph, startNode)
				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					edgeFeatures[i].push_back(graph.getNodeDegree(startNode));
					edgeFeatures[i].push_back(graph.getNodeDegree(targetNode));

					i++;
				endfor
			endfor
		}
			break;

		case NODE_COUNT:
		{
			std::size_t count = graph.number_of_nodes();
			forall_nodes(graph, startNode)
				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					edgeFeatures[i].push_back(count);

					i++;
				endfor
			endfor
		}
			break;

		case EDGE_COUNT:
		{
			std::size_t count = graph.number_of_edges();
			forall_nodes(graph, startNode)
				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					edgeFeatures[i].push_back(count);

					i++;
				endfor
			endfor
		}
			break;

		case SHARED_NEIGHBOR_COUNT:
		{
			forall_nodes(graph, startNode)

				std::vector<NodeID> startNeighbors;
				forall_out_edges(graph, e, startNode)
					startNeighbors.push_back(graph.getEdgeTarget(e));
				endfor

				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					std::vector<NodeID> targetNeighbors;
					forall_out_edges(graph, targetEdge, startNode)
						targetNeighbors.push_back(graph.getEdgeTarget(targetEdge));
					endfor

					std::vector<NodeID> sharedNeighbors;
					std::set_intersection(startNeighbors.begin(), startNeighbors.end(), targetNeighbors.begin(), targetNeighbors.end(), std::back_inserter(sharedNeighbors));
					edgeFeatures[i].push_back(sharedNeighbors.size());

					i++;
				endfor
			endfor
		}
			break;

		case CLUSTERING_COEFFICIENTS_LOCAL:
		{
			forall_nodes(graph, startNode)
				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					edgeFeatures[i].push_back(clusteringCoefficient(graph, startNode));
					edgeFeatures[i].push_back(clusteringCoefficient(graph, targetNode));


					i++;
				endfor
			endfor
		}
			break;


		case GRAPHLETS:
		{
			std::ifstream graphletFile(graphletFilename);

			forall_nodes(graph, startNode)
				forall_out_edges(graph, e, startNode)
					NodeID targetNode = graph.getEdgeTarget(e);

					// only use small -> big edges
					if (startNode > targetNode) {
						continue;
					}

					std::string line;
					std::getline(graphletFile, line);
					std::stringstream stream(line);

					float value;
					for (int i = 0; i < 8; i++) {
						stream >> value;
						edgeFeatures[i].push_back(value);
					}

					i++;
				endfor
			endfor
		}
			break;
		
		default:
			break;
		}

	}
	
			
	// create the labels

	std::vector<float> labels(graph.number_of_edges()/2);
	bool hasLabels = nodeCommunites.size() != 0;
	// we have given communities
	if (hasLabels) {

		int i = 0;
		// for all edges
		forall_nodes(graph, startNode)
			forall_out_edges(graph, e, startNode)
				NodeID edgeTarget = graph.getEdgeTarget(e);

				if (startNode > edgeTarget) {
					continue;
				}

				// write the label first
				labels[i] = shareCommunity(nodeCommunites[startNode], nodeCommunites[edgeTarget]);

				i++;
			endfor
		endfor

	}


	// finish the data creation, now fill them into the final features vector
	
	// add the features in a file
	std::ofstream featureFile(outputFilename);
	//std::FILE* f = std::fopen(outputFilename.c_str(), "w");

	//#define XGFeature(nr, value) " " << nr << ":" << static_cast<float>(value)
	//#define featureWrite(file, nr, val) std::fprintf(file, " %d:%f", nr, static_cast<float>(val));

	int i = 0;
	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)
		NodeID edgeTarget = graph.getEdgeTarget(e);

		if (startNode > edgeTarget) {
			continue;
		}

		if (hasLabels) {
			featureFile << labels[i];
		}

		for (auto value : edgeFeatures[i]) {
			featureFile << " " << value;
		}

		featureFile << std::endl;
		i++;
		endfor
	endfor
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