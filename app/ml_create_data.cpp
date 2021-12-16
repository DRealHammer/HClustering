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

#include "parse_parameters.h"

/*

./build/ml_create_data <graphFilename> [--output <outputFilename>] [--communities <communityFilename>] [--graphlets <graphletFilename>] [--features <features>] [--featureFile <featureFile>]

*/

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

	//--createData <graphFilename> <outputFilename> [<communityFilename>] [<graphletFilename>]

	// find a outputfilename for the file if not specified
	std::string outputFilename = partition_config.filename_output.empty() ? graph_filename + "-data" : partition_config.filename_output;
	
	std::string communityFilename(partition_config.communityFilename);
	std::string graphletFilename(partition_config.graphletFilename);
	std::string featureFilename(partition_config.featureFilename);

	std::cout << "graph filename: " << graph_filename << std::endl;
	std::cout << "output filename: " << outputFilename << std::endl;
	std::cout << "comm filename: " << communityFilename << std::endl;
	std::cout << "graphlet filename: " << graphletFilename << std::endl;

	writeGraphFeatureFile(graph_filename, outputFilename, communityFilename, graphletFilename, featureFilename);

	std::cout << "sucessfull" << std::endl;
	return 0;
}