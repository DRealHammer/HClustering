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
	const float* out_result = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &out_len, &out_result);

	

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

	return probs;
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


	// ********* Experimental ***********



	// subtract the threshold from all probs
	for (auto& val : probs) {
		val -= threshold;
	}



	// ************ end of experimental ************

	graph_io graphIO;
	graph_access graph;
	graphIO.readGraphWeighted(graph, graphFile);

	
	std::cout << "starting label propagation" << std::endl;
	auto res = labelPropagate(graph, probs, 200, false);

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



	bst_ulong out_len = 0;
	const float* out_result = nullptr;

	std::size_t errorMatch = 0;
	std::size_t errorNoMatch = 0;
	XGDMatrixGetFloatInfo(dmat, "label", &out_len, &out_result);

	int i = 0;
	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)
			NodeID targetNode = graph.getEdgeTarget(e);

			// skip big -> small edges
			if (startNode > targetNode) {
				continue;
			}
			i++;

			// if have the same resulting label
			// the edge was matched

			bool wasMatched = res[startNode] == res[targetNode];
			bool shouldMatch = out_result[i] == 0 ? false : true;


		errorMatch += wasMatched && !shouldMatch;
		errorNoMatch += !wasMatched && shouldMatch;

		endfor
	endfor

	std::cout << "There were " << errorMatch << " edges wrong clustered." << std::endl;
	std::cout << "There were " << errorNoMatch << " edges wrong not clustered." << std::endl;
	std::cout << "Error: " << static_cast<float>(errorNoMatch + errorMatch)/(i + 1) << std::endl;

	std::cout << "FP Rate: " << errorMatch / static_cast<float>(matchedEdges) << std::endl;

	//std::cout << "Conductance: " << conductance(graph, res);
}

void performRandomLabelPropagation(std::string graphFile, std::string dataFilename, bool compare = true) {

	std::cout << "graph file: " << graphFile << std::endl;
	std::cout << "data file: " << dataFilename << std::endl;

	if (!std::ifstream(dataFilename).good()) {
		std::cout << "Error: no data file found, extract your features beforehand" << std::endl;
		return;
	}



	graph_io graphIO;
	graph_access graph;
	graphIO.readGraphWeighted(graph, graphFile);

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> val(0, 1);

	std::vector<float> probs(graph.number_of_edges());
	for (int i = 0; i < graph.number_of_nodes(); i++) {
		probs[i] = val(e2);
	}



	// ********* Experimental ***********


	// find mean of the probs
	float sum = 0;

	for (auto val : probs) {
		sum += val;
	}
	float mean = sum/probs.size();


	// subtract it from all
	for (auto& val : probs) {
		val -= mean;
	}



	// ************ end of experimental ************


	std::cout << "starting label propagation" << std::endl;
	auto res = labelPropagate(graph, probs, 200, true);

	std::vector<PartitionID> labelCounts(graph.number_of_nodes());
	
	// count the different clusters created
	for (auto nodeLabel : res) {
		labelCounts[nodeLabel] += 1;
	}

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

	std::cout << "Created " << labelAmount << " clusters" << std::endl;
	std::cout << "Smallest cluster: " << minClustersize << std::endl;
	std::cout << "Largest cluster: " << maxClustersize << std::endl;


	std::size_t errorMatch = 0;
	std::size_t errorNoMatch = 0;
	std::ifstream compareFile(dataFilename);

	forall_nodes(graph, startNode)
		forall_out_edges(graph, e, startNode)

		NodeID targetNode = graph.getEdgeTarget(e);

		if (targetNode < startNode) {
			continue;
		}

		bool propagateMatched = res[startNode] == res[targetNode];
		
		std::string line;
		std::getline(compareFile, line);
		

		bool shouldMatch = line[0] == '1';

		errorMatch += propagateMatched && !shouldMatch;
		errorNoMatch += !propagateMatched && shouldMatch;

		endfor

	endfor


	std::cout << "There were " << errorMatch << " edges wrong clustered." << std::endl;
	std::cout << "There were " << errorNoMatch << " edges wrong not clustered." << std::endl;
	std::cout << "Error: " << static_cast<float>(errorNoMatch + errorMatch)/(graph.number_of_edges()/2.0f) << std::endl;

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


	std::cout << "performing label propagation" << std::endl;


	performLabelPropagation(graph_filename, partition_config.dataFilename, 0.5, false);

	return 0;
}