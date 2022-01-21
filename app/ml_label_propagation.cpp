#include <random>
#include <argtable3.h>
#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h>


// custom functions and definitions
#include <ml_clustering/graphFeatures.h>
#include <ml_clustering/inOut.h>
#include <ml_clustering/labelPropagation.h>
#include <ml_clustering/graphMetrics.h>


	

#include "parse_parameters.h"


/*


./programm --test --booster <boosterFilename> {<graphNames>}+
./programm --predict --data <graphName>



// global arg_xxx structs
struct arg_lit *createFeatures;
struct arg_lit *train;
struct arg_end *ending;

*/


std::vector<float> getEdgeProbs(std::string graphFile, std::string boosterFilename, DMatrixHandle dmat) {

	BoosterHandle booster;
	XGBoosterCreate(&dmat, 1, &booster);
	
	// test if booster exists
	if (!std::ifstream(boosterFilename).is_open()) {
		std::cout << "Error: no model found, train your booster first" << std::endl;
		return std::vector<float>();
	}

	XGBoosterLoadModel(booster, boosterFilename.c_str());

	// predictions

	bst_ulong out_len = 0;
	float* out_result = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &out_len, &out_result);

	// set all negative edges to 0 and 
	for (int i = 0; i < out_len; i++) {
		if (out_result[i] < 0) {
			out_result[i] = 0;
		}
		out_result[i] += 0.0000001;
	}

	

	// propagation

	std::cout << "finished prediction, starting to propagate through the graph" << std::endl;
	graph_io graphIO;
	graph_access graph;
	graphIO.readGraphWeighted(graph, graphFile);



	std::vector<float> probs(graph.number_of_edges());

	int i = 0;
	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)
			NodeID targetNode = graph.getEdgeTarget(e);

			if (startNode > targetNode) {
				continue;
			}
			
			//std::cout << startNode << " -> " << targetNode << " weight: " << out_result[i] << std::endl;
			probs[e] = out_result[i];

			forall_out_edges(graph, inverse_e, targetNode)
				if (graph.getEdgeTarget(inverse_e) == startNode) {
					probs[inverse_e] = out_result[i];
					break;
				}
			endfor
			i++;
		endfor
	endfor

	for (auto& p : probs) {
		//p = 1;
	}

	return probs;
}

void printPrediction(std::string graphFile, std::string dataFilename) {


	// loading of the data and model


	std::cout << "graph file: " << graphFile << std::endl;
	std::cout << "data file: " << dataFilename << std::endl;
	std::string boosterFilename = "booster.json";

	if (!std::ifstream(dataFilename).good()) {
		std::cout << "Error: no data file found, extract your features beforehand" << std::endl;
		return;
	}


	DMatrixHandle dmat;
	XGDMatrixCreateFromFile(dataFilename.c_str(), 0, &dmat);


	BoosterHandle booster;
	XGBoosterCreate(&dmat, 1, &booster);
	

	XGBoosterLoadModel(booster, boosterFilename.c_str());

	// predictions

	bst_ulong out_len = 0;
	const float* out_result = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &out_len, &out_result);


	for (int i = 0; i < out_len; i++) {
		std::cout << out_result[i] << '\n';
	}
}

void performLabelPropagation(std::string graphFile, std::string dataFilename, float threshold = 0.5, bool compare = false) {


	// loading of the data and model


	std::cout << "graph file: " << graphFile << std::endl;
	std::cout << "data file: " << dataFilename << std::endl;
	std::string boosterFilename = "booster.json";

	if (!std::ifstream(dataFilename).good()) {
		std::cout << "Error: no data file found, extract your features beforehand" << std::endl;
		return;
	}


	DMatrixHandle dmat;
	XGDMatrixCreateFromFile(dataFilename.c_str(), 0, &dmat);

	auto probs = getEdgeProbs(graphFile, boosterFilename, dmat);

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> val(0, 1);

	for (auto& p : probs) {
		p = 1;
	}

	
	graph_io graphIO;
	graph_access graph;
	graphIO.readGraphWeighted(graph, graphFile);

	forall_nodes(graph, start)
		forall_out_edges(graph, e, start)
			NodeID target = graph.getEdgeTarget(e);
			std::cout << start + 1 << " -> " << target + 1 << ": " << probs[e] << std::endl; 
		endfor
	endfor
	
	std::cout << "starting label propagation" << std::endl;
	auto res = labelPropagate(graph, probs, 200, true);

	std::cout << "finished propagation" << std::endl;



	// evaluation


	std::vector<PartitionID> labelCounts(graph.number_of_nodes());
	
	// count the different clusters created
	for (auto nodeLabel : res) {
		labelCounts[nodeLabel] += 1;
	}

	std::size_t matchedEdges = 0;

	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)
			NodeID targetNode = graph.getEdgeTarget(e);

			// skip big -> small edges
			if (startNode > targetNode) {
				continue;
			}

			// if have the same resulting label
			// the edge was matched
			if (res[startNode] == res[targetNode]) {
				++matchedEdges;
			}


		endfor
	endfor

	int labelAmount = 0;
	int maxClustersize = 0;
	int minClustersize = std::numeric_limits<int>::max();
	for(auto count : labelCounts) {
		if (count > 0 ) {
			labelAmount++;

			if (count < minClustersize) {
				minClustersize = count;
			}

			if (count > maxClustersize) {
				maxClustersize = count;
			}
		}
	}

	

	std::cout << "Matched " << matchedEdges << " edges" << std::endl;
	std::cout << "Created " << labelAmount << " clusters" << std::endl;
	std::cout << "Smallest cluster: " << minClustersize << std::endl;
	std::cout << "Largest cluster: " << maxClustersize << std::endl;
	std::cout << "Conductance: " << conductance(graph, res) << std::endl;

	std::cout << "propagated labels:" << std::endl;
	forall_nodes(graph, node)
		std::cout << node + 1 << " " << res[node] << std::endl;
	endfor
}



int main(int argc, char** argv) {

	PartitionConfig partition_config;
	std::string graph_filename;

	bool is_graph_weighted = false;
	bool suppress_output   = false;
	bool recursive         = false;

	int ret_code = parse_parameters(argc, argv, 
					partition_config, 
					graph_filename, 
					is_graph_weighted, 
					suppress_output, recursive); 

	if(ret_code) {
			return 0;
	}

	//std::cout << "edge weight predictions" << std::endl;
	//printPrediction(graph_filename, partition_config.dataFilename);

	std::cout << "------------------------" << std::endl;
	std::cout << "performing label propagation" << std::endl;

	//printPrediction(graph_filename, partition_config.dataFilename);
	performLabelPropagation(graph_filename, partition_config.dataFilename, 0.142724, true);

	return 0;
}