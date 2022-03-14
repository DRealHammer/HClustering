#include <random>
#include <argtable3.h>
#include <iostream>
#include <math.h>
#include <regex.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <fstream>


// custom functions and definitions
#include <ml_clustering/graphFeatures.h>
#include <ml_clustering/inOut.h>

#include "parse_parameters.h"

void evaluateBooster(BoosterHandle& booster, DMatrixHandle& dmat) {

	std::cout << "-------------------------- booster evaluation --------------------------" << std::endl;
	bst_ulong len_predict = 0;
	const float* y_predict = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &len_predict, &y_predict);

	std::ofstream file("predicts");

	for (int i = 0; i < len_predict; i++) {
		file << y_predict[i] << "\n";
	}

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

void printFeatureImportance(BoosterHandle booster) {

	bst_ulong out_n_features;
	const char **out_features;
	bst_ulong out_dim;
	const bst_ulong *out_shape;
	const float *out_scores;
	XGBoosterFeatureScore(booster, "{\"importance_type\": \"total_gain\"}", &out_n_features, &out_features, &out_dim, &out_shape, &out_scores);

	std::cout << XGBGetLastError() << std::endl;


	std::cout << "----------------- feature importance ----------" << std::endl;
	for (int i = 0; i < out_n_features; i++) {
		std::cout << out_features[i] << "\t" << out_scores[i] << std::endl;
	}
}

void train(std::string trainFilename, std::string testFilename, std::string boosterFilename, int iters) {

	if (boosterFilename.size() == 0) {
		boosterFilename = "booster.json";
	}

	std::cout << "reading file" << std::endl;
	DMatrixHandle dtrain;
	
	std::cout << "train filename: " << trainFilename << std::endl;
	std::cout << "test filename: " <<  testFilename << std::endl;


	if(XGDMatrixCreateFromFile(trainFilename.c_str(), 1, &dtrain)) {
		std::cout << "could not read the training file" << std::endl;
		return;
	}

	DMatrixHandle dtest;
	bool testGiven = testFilename.size() != 0;
	if (testGiven) {
		if(XGDMatrixCreateFromFile(testFilename.c_str(), 1, &dtest)) {
		std::cout << "could not read the test file" << std::endl;
		return;
		}
	}
	

	std::cout << "finished reading the files"  << std::endl;

	BoosterHandle booster;
	std::vector<DMatrixHandle> dmats;
	dmats.push_back(dtrain);

	if (testGiven) {
		dmats.push_back(dtest);
	}

	XGBoosterCreate(dmats.data(), dmats.size(), &booster);
	//XGBoosterSetParam(booster, "learning_rate", "0.01");
	//XGBoosterSetParam(booster, "n_estimators", "50");
	//XGBoosterSetParam(booster, "max_depth", "5");
	//XGBoosterSetParam(booster, "subsample", "0.8");
	//XGBoosterSetParam(booster, "colsample_bytree", "1");
	//XGBoosterSetParam(booster, "gamma", "1");

	
	// test if booster exists
	if (std::ifstream(boosterFilename).is_open()) {
		XGBoosterLoadModel(booster, boosterFilename.c_str());
	} else {
		std::cout << "booster file not found, creating new model" << std::endl;
	}


	std::cout << "start training" << std::endl;
	std::cout << "loading " << trainFilename << std::endl;

	const char* name = trainFilename.c_str();
	const char* names[] = {"training", "testing"};
	const char* eval_result = NULL;

	for (int iter = 0; iter < iters; iter++) {
		if(XGBoosterUpdateOneIter(booster, iter, dtrain)) {
			std::cout << XGBGetLastError() << std::endl;
			std::cout << "could not train" << std::endl;
			return;
		}

		//std::cout << "saving model" << std::endl;
		XGBoosterSaveModel(booster, boosterFilename.c_str());

		if(XGBoosterEvalOneIter(booster, iter, dmats.data(), names, dmats.size(), &eval_result)) {
			std::cerr << "[" << iter << "] Error!" << std::endl;
			return;
		}
		
		std::cout << eval_result << std::endl;
	}

	std::cout << "custom evaluation" << std::endl;
	evaluateBooster(booster, dtrain);
	printFeatureImportance(booster);

	std::cout << "freeing matrix of " << trainFilename << std::endl;
	XGDMatrixFree(dtrain);

	if (testGiven) {
		std::cout << "freeing matrix of " << testFilename << std::endl;
		XGDMatrixFree(dtest);
	}


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

	
	train(dataFilename, partition_config.testFilename, partition_config.modelFilename, partition_config.iterations);

	

	return 0;
}