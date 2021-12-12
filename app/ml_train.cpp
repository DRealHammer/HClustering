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

void evaluateBooster(BoosterHandle& booster, DMatrixHandle& dmat) {

	std::cout << "-------------------------- booster evaluation --------------------------" << std::endl;
	bst_ulong len_predict = 0;
	const float* y_predict = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &len_predict, &y_predict);

	bst_ulong len_true = 0;
	const float* y_true = nullptr;
	XGDMatrixGetFloatInfo(dmat, "label", &len_true, &y_true);

	if (len_predict != len_true) {
		std::cout << "error with the amount of edges!" << std::endl;
		return;
	}

	std::size_t errorMatch = 0;
	std::size_t errorNoMatch = 0;
	std::size_t matchedEdges = 0;

	for (int i = 0; i < len_predict; i++) {

		
		bool propagateMatched = y_predict[i] > 0.5 ? true : false;
		
		if (propagateMatched) {
			matchedEdges++;
		}

		bool shouldMatch = y_true[i] > 0.5 ? true : false;

		errorMatch += propagateMatched && !shouldMatch;
		errorNoMatch += !propagateMatched && shouldMatch;

	}

	std::cout << "Overall matched edges: " << matchedEdges << std::endl;

	std::cout << "There were " << errorMatch << " edges wrong clustered." << std::endl;
	std::cout << "There were " << errorNoMatch << " edges wrong not clustered." << std::endl;
	std::cout << "Error: " << static_cast<float>(errorNoMatch + errorMatch)/(len_predict) << std::endl;
}

void train(std::string dataFilename, std::string boosterFilename) {

	if (boosterFilename.size() == 0) {
		boosterFilename = "booster.json";
	}


	std::cout << "reading file" << std::endl;
	DMatrixHandle dtrain;
	std::cout << "aaa" << std::endl;
	std::cout << "data filename; " << dataFilename << std::endl;
	if(XGDMatrixCreateFromFile(dataFilename.c_str(), 1, &dtrain)) {
		std::cout << "could not read the data file" << std::endl;
		return;
	}

	std::cout << "finished reading the file"  << std::endl;

	BoosterHandle booster;
	XGBoosterCreate(&dtrain, 1, &booster);
	
	// test if booster exists
	if (std::ifstream(boosterFilename).is_open()) {
		XGBoosterLoadModel(booster, boosterFilename.c_str());
	}

	
	std::cout << "start training" << std::endl;
	std::cout << "loading " << dataFilename << std::endl;

	const char* name = dataFilename.c_str();
	const char* eval_result = NULL;

	for (int iter = 0; iter < 10; iter++) {
		if(XGBoosterUpdateOneIter(booster, iter, dtrain)) {
			std::cout << XGBGetLastError() << std::endl;
			std::cout << "could not train" << std::endl;
			return;
		}
		if(XGBoosterEvalOneIter(booster, iter, &dtrain, &name, 1, &eval_result)) {
			std::cerr << "[" << iter << "] Error!" << std::endl;
			return;
		}
		
		std::cout << eval_result << std::endl;
	}

	std::cout << "custom evaluation" << std::endl;
	evaluateBooster(booster, dtrain);

	std::cout << "freeing matrix of " << dataFilename << std::endl;
	XGDMatrixFree(dtrain);


	std::cout << "saving model" << std::endl;
	XGBoosterSaveModel(booster, boosterFilename.c_str());

}

int main(int argc, char** argv) {

	PartitionConfig partition_config;
	std::string dataFilename;

	bool is_graph_weighted = false;
	bool suppress_output   = false;
	bool recursive         = false;

	int ret_code = parse_parameters(argc, argv, 
					partition_config, 
					dataFilename, 
					is_graph_weighted, 
					suppress_output, recursive); 

	if(ret_code) {
			return 0;
	}


	train(dataFilename, partition_config.modelFilename);

	

	return 0;
}