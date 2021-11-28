// custom functions and definitions
#include <graphFeatures.h>
#include <profiling.h>
#include <inOut.h>
#include <labelPropagation.h>



#include <argtable/argtable3.h>


/*
./programm --createFeatures <folderName> (writeGraph to a data file)
./programm --train --data <graphName> [<graphNames>] [--test <graphName> ] [--booster <boosterName> ]
./programm --test --booster <boosterFilename> {<graphNames>}+
./programm --predict --data <graphName>



// global arg_xxx structs
struct arg_lit *createFeatures;
struct arg_lit *train;
struct arg_end *ending;

*/


void train(std::string dataFilename) {

	std::string boosterFilename = "booster.json";

	std::cout << "reading file" << std::endl;
	DMatrixHandle dtrain;
	if(XGDMatrixCreateFromFile(dataFilename.c_str(), 1, &dtrain)) {
		std::cout << "could not read the data file" << std::endl;
		return;
	}

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

	std::cout << "freeing matrix of " << dataFilename << std::endl;
	XGDMatrixFree(dtrain);


	std::cout << "saving model" << std::endl;
	XGBoosterSaveModel(booster, "booster.json");

}

void performLabelPropagation(std::string graphFile, std::string dataFilename) {

	std::string boosterFilename = "booster.json";


	DMatrixHandle dmat;
	XGDMatrixCreateFromFile(dataFilename.c_str(), 0, &dmat);



	BoosterHandle booster;
	XGBoosterCreate(&dmat, 1, &booster);
	
	// test if booster exists
	if (std::ifstream(boosterFilename).is_open()) {
		XGBoosterLoadModel(booster, boosterFilename.c_str());
		return;
	}



	bst_ulong out_len = 0;
	const float* out_result = nullptr;

	XGBoosterPredict(booster, dmat, 0, 0, 0, &out_len, &out_result);

	std::vector<float> probs(out_len);
	for (int i = 0; i < out_len; i++) {
		//std::cout << "[Edge " << i << "] " << out_result[i] << std::endl;
		probs[i] = out_result[i];
	}


	graph_io graphIO;
	graph_access graph;
	graphIO.readGraphWeighted(graph, "data/testFiles/test.metis");

	
	auto res = labelPropagate(graph, probs, 200);

	for (auto r : res) {
		//std::cout << r << std::endl;
	}
	

}

int main(int argc, char** argv) {

	if (argc < 3) {
		std::cout << "2 arguments needed: ./build/HClustering --train <dataFilename>" << std::endl;
		std::cout << "./build/HClustering --labelPropagation <graphFilename> <dataFilename>" << std::endl;
		std::cout << "./build/HClustering --createData <graphFilename> <outputFilename> [<communityFilename>] [<graphletFilename>]" << std::endl;
	}

	std::string option(argv[1]);



	if (option == std::string("--train")) {
		if (argc != 3) {
			std::cout << "too less parameters" << std::endl;
			return 1;
		}
		train(argv[2]);
	}
	else if (option == std::string("--labelPropagation")) {
		if (argc != 4) {
			std::cout << "too less parameters" << std::endl;
			return 1;
		}
		performLabelPropagation(argv[2], argv[3]);
	}
	else if (option == std::string("--createData")) {
		if (argc < 4) {
			std::cout << "too less parameters" << std::endl;
			return 1;
		}

		std::string graphFilename(argv[2]);
		std::string outputFilename(argv[3]);
		
		std::string communityFilename;
		std::string graphletFilename;
		if(argc >= 5) {
			communityFilename = argv[4];
		}
		if(argc == 6) {
			graphFilename = argv[5];
		}
		writeGraphFeatureFile(graphFilename, outputFilename, communityFilename, graphletFilename);
	}

	
	return 0;
}