#include <ml_clustering/inOut.h>


void writeGraphFeatureFile(std::string graphFilename, std::string outputFilename, const std::string communityFilename, std::string graphletFilename, std::string featureFilename) {

	graph_io IO;
	graph_access graph;
	IO.readGraphWeighted(graph, graphFilename);

	std::map<NodeID, std::vector<CommID>> comms;

	if (communityFilename.size() != 0) {
		comms = readCommunityFile(communityFilename);
	}

	std::ifstream featureFile(featureFilename);
	if(!featureFile.is_open()) {
		std::cout << "couldn't open feature file" << std::endl;
		return;
	}

	bool needGraphlet = false;
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
			needGraphlet = true;
		} else if (line == "SECOND_DEGREE") {
			selectedFeaturesSet.insert(SECOND_DEGREE);
		} else if (line == "CENTRALITY") {
			selectedFeaturesSet.insert(CENTRALITY);
		}
	}

	// check if we have to create the graphlet file
	if (needGraphlet && graphletFilename.size() == 0) {
		
		std::cout << "no graphlet file given:\ncreating mtx file..." << std::endl;
		std::string mtxGraphFilename = graphFilename + ".mtx";
		graphToMTX(graph, mtxGraphFilename);

		std::cout << "creating graphlet file..." << std::endl;
		graphletFilename = graphFilename + ".graphlet";
		createGraphletFile(mtxGraphFilename, graphletFilename);

		// use a bash command for sorting the file fast
		std::string sortCommand = "cat " + graphletFilename + " | sort -n -k 2 | sort -n -s >> " + graphletFilename + "-s && mv " + graphletFilename + "-s " + graphletFilename;

		system(sortCommand.c_str());

		std::cout << "finished creating graphlet files" << std::endl;
	}
	

	if (needGraphlet && !std::ifstream(graphletFilename).is_open()) {
		std::cout << "couldn't open graphlet file" << std::endl;
		return;
	}

	

	std::cout << "start writing" << std::endl;
	writeFeaturesInFile(graph, outputFilename, comms, selectedFeaturesSet, graphletFilename);
	std::cout << "cmtyfiel: " << communityFilename << std::endl;
}



std::map<NodeID, std::vector<CommID>> readCommunityFile(std::string fileName) {

	std::map<NodeID, std::vector<CommID>> comms;


	std::ifstream file(fileName, std::ios::in);

	if (!file.is_open()) {
		std::cout << "Alert: File " << fileName << " could not be opened!" << std::endl;
	}

	// read the file
	NodeID nodeID;
	while (!file.eof()) {
		std::string line;
		std::getline(file, line, '\n');

		std::stringstream lineStream(line);

		lineStream >> nodeID;
		nodeID--;
		//std::cout << nodeID << "'s Communities found" << std::endl;

		while (!lineStream.eof()) {
			CommID id;
			lineStream >> id;
			comms[nodeID].push_back(id);
		}
	}
	
	return comms;

}

struct FullEdge {
	NodeID startNode;
	NodeID targetNode;
	EdgeID id;
};


void writeFeaturesInFile(graph_access& graph, std::string outputFilename, std::map<NodeID, std::vector<CommID>>& nodeCommunites, std::set<FEATURE>& selectedFeatures, std::string graphletFilename) {

	// create vector of all relevant edges and their information once
	// to iterate less in the data creating

	std::vector<FullEdge> relevantEdges(graph.number_of_edges() / 2);

	{
		int i = 0;
		forall_nodes(graph, startNode)
			forall_out_edges(graph, e, startNode)

				NodeID targetNode = graph.getEdgeTarget(e);

				if (startNode > targetNode) {
					continue;
				}

				relevantEdges[i].id = e;
				relevantEdges[i].startNode = startNode;
				relevantEdges[i].targetNode = targetNode;

				i++;

			endfor
		endfor
	}



	// create the data vectors for all small big edges
	std::vector<std::vector<float>> edgeFeatures(graph.number_of_edges() / 2);
	std::vector<FEATURE> features;

	for (FEATURE f : selectedFeatures) {
		features.push_back(f);
	}
	std::sort(features.begin(), features.end());

	std::ifstream graphletFile;
	if (std::find(features.begin(), features.end(), GRAPHLETS) != features.end()) {
		graphletFile.open(graphletFilename);
	}

	std::vector<float> clusterCoefficents(graph.number_of_nodes());

	#pragma omp parallel for
	for (NodeID n = 0; n < graph.number_of_nodes(); n++) {
		clusterCoefficents[n] = clusteringCoefficient(graph, n);
	}

	std::vector<bool> isNeighbor(graph.number_of_nodes(), false);


	// precalculating 2nd order degrees once for all nodes
	std::vector<float> node2ndDegrees(graph.number_of_nodes(), 0);
	if (std::find(features.begin(), features.end(), SECOND_DEGREE) != features.end() ) {
		forall_nodes(graph, n)

					
				// mark all nodes in the second degree
				forall_out_edges(graph, e, n) 
					forall_out_edges(graph, second_edge, graph.getEdgeTarget(e))
						isNeighbor[graph.getEdgeTarget(second_edge)] = true;
					endfor
				endfor

				for (const auto& b : isNeighbor) {
					node2ndDegrees[n] += b;
				}

				// unmark all nodes in the second degree
				forall_out_edges(graph, e, n) 
					forall_out_edges(graph, second_edge, graph.getEdgeTarget(e))
						isNeighbor[graph.getEdgeTarget(second_edge)] = false;
					endfor
				endfor

				node2ndDegrees[n] = node2ndDegrees[n] / static_cast<float>(graph.number_of_nodes());

		endfor
		std::cout << "finished second degrees" << std::endl;
	}

	std::vector<float> centralities;
	if (std::find(features.begin(), features.end(), CENTRALITY) != features.end() ) {
		std::cout << "starting calculating centralities" << std::endl;
		centralities = nodeCentralities(graph);

		std::cout << "finished centralities" << std::endl;

		//std::cout << "cents" << std::endl;
		//for(auto c : centralities) {
		//	std::cout << c << std::endl;
		//}
	}





	// index of the small->big edges

	// bool array for every thread
	//std::vector<std::vector<bool>> isNeighbor(omp_get_max_threads(), std::vector<bool>(graph.number_of_nodes()));

	

	for (int i = 0; i < relevantEdges.size(); i++) {
		FullEdge& edge = relevantEdges[i];

		for (FEATURE f : features) {
			
			switch (f){

				case NODE_DEGREES:
				{
					
					edgeFeatures[i].push_back(graph.getNodeDegree(edge.startNode) /static_cast<float>(graph.number_of_nodes()));
					edgeFeatures[i].push_back(graph.getNodeDegree(edge.targetNode)/static_cast<float>(graph.number_of_nodes()));
					break;
				}


				case NODE_COUNT:
				{
					edgeFeatures[i].push_back(graph.number_of_nodes());
					break;
				}


				case EDGE_COUNT:
				{
					edgeFeatures[i].push_back(graph.number_of_edges());
					break;
				}


				case SHARED_NEIGHBOR_COUNT:
				{
					int count = 0;
					forall_out_edges(graph, e, edge.startNode)
						isNeighbor[graph.getEdgeTarget(e)] = true;
					endfor

					forall_out_edges(graph, e, edge.targetNode)
						count += isNeighbor[graph.getEdgeTarget(e)];
					endfor

					edgeFeatures[i].push_back(2 * count/static_cast<float>(std::min(graph.getNodeDegree(edge.startNode), graph.getNodeDegree(edge.targetNode))));

					forall_out_edges(graph, e, edge.startNode)
						isNeighbor[graph.getEdgeTarget(e)] = false;
					endfor
					break;
				}

				case CLUSTERING_COEFFICIENTS_LOCAL:
				{

					edgeFeatures[i].push_back(clusterCoefficents[edge.startNode]);
					edgeFeatures[i].push_back(clusterCoefficents[edge.targetNode]);
					break;
				}



				case GRAPHLETS:
				{
					std::string line;
					std::getline(graphletFile, line);
					std::stringstream stream(line);

					float value;

					// get the first two numbers out as they are the node informations
					NodeID node;
					stream >> node;
					stream >> node;

					while (stream >> value) {
						edgeFeatures[i].push_back(value/static_cast<float>(std::min(graph.getNodeDegree(edge.startNode), graph.getNodeDegree(edge.targetNode))));
					}

					break;

					
				}
					
				
				case SECOND_DEGREE:
				{

					edgeFeatures[i].push_back(node2ndDegrees[edge.startNode]);
					edgeFeatures[i].push_back(node2ndDegrees[edge.targetNode]);
					break;
				}

				case CENTRALITY:
				{

					edgeFeatures[i].push_back(centralities[edge.startNode]);
					edgeFeatures[i].push_back(centralities[edge.targetNode]);
					break;
				}
				
				
				default:
					break;
			}
		}

	}
	graphletFile.close();

	
	// create the labels

	std::vector<float> labels(graph.number_of_edges()/2);
	bool hasLabels = nodeCommunites.size() != 0;

	// we have given communities
	if (hasLabels) {

		// create clustering array from the communities
		std::vector<PartitionID> clustering;

		for (int i = 0; i < graph.number_of_nodes(); i++) {
			clustering.push_back(nodeCommunites[i][0]);
		}

		conductance(graph, clustering);

		// for all edges
		for (int i = 0; i < relevantEdges.size(); i++) {
			FullEdge& edge = relevantEdges[i];

			// write the label first
			labels[i] = shareCommunity(nodeCommunites[edge.startNode], nodeCommunites[edge.targetNode]);
		}

	}


	// finish the data creation


	std::vector<float> means(edgeFeatures[0].size());
	// normalize the means
	for (auto features : edgeFeatures) {
		for (int featureIdx = 0; featureIdx < features.size(); featureIdx++) {
			means[featureIdx] += features[featureIdx];
		}
	}

	for (auto& m : means) {
		m /= edgeFeatures.size();
	}

	
	for (auto features : edgeFeatures) {
		for (int featureIdx = 0; featureIdx < features.size(); featureIdx++) {
			features[featureIdx] -= means[featureIdx];
		}
	}

	std::vector<float> stddevs(edgeFeatures[0].size());
	// normalize the stddev
	for (auto features : edgeFeatures) {
		for (int featureIdx = 0; featureIdx < features.size(); featureIdx++) {
			stddevs[featureIdx] += features[featureIdx] * features[featureIdx];
		}
	}

	for (auto& dev : stddevs) {
		dev = std::sqrt(dev / edgeFeatures.size());
	}

	for (auto features : edgeFeatures) {
		for (int featureIdx = 0; featureIdx < features.size(); featureIdx++) {
			features[featureIdx] /= stddevs[featureIdx];
		}
	}

	
	// add the features in a file
	std::ofstream featureFile(outputFilename);
	//std::FILE* f = std::fopen(outputFilename.c_str(), "w");

	//#define XGFeature(nr, value) " " << nr << ":" << static_cast<float>(value)
	//#define featureWrite(file, nr, val) std::fprintf(file, " %d:%f", nr, static_cast<float>(val));

	for (int i = 0; i < relevantEdges.size(); i++) {


		if (hasLabels) {
			featureFile << labels[i];
		}

		for (int idx = 0; idx < edgeFeatures[i].size(); idx++) {
			featureFile << " " << idx << ":" << edgeFeatures[i][idx];
		}

		featureFile << std::endl;
	}
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