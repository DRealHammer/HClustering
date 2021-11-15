// custom functions and definitions
#include <graphFeatures.h>
#include <profiling.h>
#include <inOut.h>


static const std::vector<std::string> trainingGraphs = {
	//"com-amazon",
	"com-dblp",
	"com-lj",
	//"com-orkut"
	"com-youtube"
};

static const std::vector<std::string> testGraphs = {
	"com-amazon"
};


void updateDataFiles() {
	for (auto graphName : trainingGraphs) {
		writeGraphFile(graphName);
	}

	for (auto graphName : testGraphs) {
		writeGraphFile(graphName);
	}
}

int main(int argc, char** argv) {

	// load training data
	std::vector<DMatrixHandle> dtrains(trainingGraphs.size());

	for (int i = 0; i < trainingGraphs.size(); i++) {
		XGDMatrixCreateFromFile(std::string("data/" + trainingGraphs[i] + "-data").c_str(), 0, &dtrains[i]);
	}

	
	// create / load the model
	BoosterHandle booster;
	XGBoosterCreate(dtrains.data(), dtrains.size(), &booster);
	XGBoosterLoadModel(booster, "booster.json");

	XGBoosterSetParam(booster, "learning_rate", "0.2");

	// training
	std::cout << "starting training" << std::endl;
	
	for (auto& dmat : dtrains) {

		
		const char* name = "training";
		const char* eval_result = NULL;

		for (int i = 0; i < 10; i++) {
			XGBoosterUpdateOneIter(booster, i, dmat);
			if(XGBoosterEvalOneIter(booster, i, &dmat, &name, 1, &eval_result)) {
				std::cerr << "[" << i << "] Error!" << std::endl;
			}
			
			std::cout << eval_result << std::endl;
		}
	}

	std::cout << "freeing matrices" << std::endl;
	for (auto& dmat : dtrains) {
		XGDMatrixFree(dmat);
	}



	// testing
	const char* name = "com-amazon";
	const char* eval_result = NULL;

	DMatrixHandle dtest;
	XGDMatrixCreateFromFile("data/com-amazon-data", 0, &dtest);
	

	XGBoosterEvalOneIter(booster, 0, &dtest, &name, 1, &eval_result);
	std::cout << eval_result << std::endl;

	XGDMatrixFree(dtest);
	std::cout << "freeing complete" << std::endl;
	

	std::cout << "saving model" << std::endl;
	// write the model in the json
	XGBoosterSaveModel(booster, "booster.json");
	std::cout << "saving complete!" << std::endl;
/*

	bst_ulong out_len = 0;
	const float* out_result = nullptr;


	XGBoosterPredict(booster, dtest, 0, 0, 0, &out_len, &out_result);

	for (int i = 0; i < out_len; i++) {
		std::cout << "[Edge " << i << "]" << out_result[i] << std::endl;
	}
*/
	
	return 0;
}























/*
std::vector<float> getHGraphFeatures(const HGraph& graph) {

	int samples = 0;
	int featureAmount = 2;

	std::vector<float> features;

	// get all edge weights
	std::vector<HWeight> edgeWeights = HGraphGetEdgeWeights(graph);

	// get all max vertex edge weights
	std::vector<HWeight> maxVertexEdgeWeights = HGraphGetNodeMaxEdgeWeights(graph);

	FOR_VERTEX(v, graph) {
		FOR_EDGE_OF_VERTEX(e, v, graph) {

			//std::cout << "add edgeWeight " << edgeWeights[e] << std::endl;
			features.push_back(edgeWeights[e]/maxVertexEdgeWeights[v]);

			features.push_back(edgeWeights[e]/maxVertexEdgeWeights[EDGE(e, graph).to_]);
		}
	}

		

	return features;
}


int main() {

	//add(2, 3);


	//std::fstream graphFile;
	//graphFile.open("test.gra", std::ios::in);
	//HGraph graph = HGraphReadFromStreamUndirected(graphFile);

	//HGraph graph = HGraphCreateRandom(200, 29, 50);

	std::fstream graphFile("training.gra", std::ios::in);
	HGraph graph = HGraphReadFromStreamUndirected(graphFile);

	DMatrixHandle dtrain;
	std::vector<float> features = getHGraphFeatures(graph);



	if (XGDMatrixCreateFromMat(features.data(), EDGE_COUNT(graph), 2, -1.0, &dtrain)) {
		std::cout << "Error!!!" << std::endl;
	}


	// matching a training label
	std::vector<float> hemlabel;
	HMatching hemMatching = HGraphCreateMatchingHEM(graph);

	


	FOR_VERTEX(v, graph) {
		FOR_EDGE_OF_VERTEX(e, v, graph) {

			// if we match this edge
			// if the matched node is the edges to value
			if (hemMatching[v] == EDGE(e, graph).to_) {
				hemlabel.push_back(1.0f);
			} else {
				hemlabel.push_back(0.0f);
			}
		}
	}

	XGDMatrixSetFloatInfo(dtrain, "label", hemlabel.data(), hemlabel.size());
	


	BoosterHandle booster;

	XGBoosterCreate(&dtrain, 1, &booster);

	XGBoosterLoadModel(booster, "booster.json");




	const char* name = "training";
	const char* eval_result = NULL;

	for (int i = 0; i < 10; i++) {
		XGBoosterUpdateOneIter(booster, i, dtrain);
		if(XGBoosterEvalOneIter(booster, i, &dtrain, &name, 1, &eval_result)) {
			std::cerr << "[" << i << "] Error!" << std::endl;
		}

		std::cout << eval_result << std::endl;
	}

	XGBoosterSaveModel(booster, "booster.json");


	// predict for a small example
	std::fstream testGraphFile("test2.gra", std::ios::in);
	HGraph testGraph = HGraphReadFromStreamUndirected(testGraphFile);



	DMatrixHandle dtest;
	
	// generate features
	std::vector<float> testFeatures = getHGraphFeatures(testGraph);


	if (XGDMatrixCreateFromMat(features.data(), EDGE_COUNT(testGraph), 2, -1.0, &dtest)) {
		std::cout << "Error!!!" << std::endl;
	}


	bst_ulong out_len = 0;
	const float* out_result = NULL;


	XGBoosterPredict(booster, dtest, 0, 0, 0, &out_len, &out_result);
	std::cout << "edge prections" << std::endl;

	for (int i = 0; i < testGraph.edges_.size(); i++) {
		printf("%1.3f ", out_result[i]);
	}

	std::cout << std::endl;

   



	//HGraphWriteForXGBoost();

*/


/*

	DMatrixHandle dtrain;
	DMatrixHandle dtest;

	XGDMatrixCreateFromFile("data/partners.txt.train", 0, &dtrain);
	XGDMatrixCreateFromFile("data/partners.txt.test", 0, &dtest);

	

	BoosterHandle booster;

	XGBoosterCreate(&dtrain, 1, &booster);

	XGBoosterSetParam(booster, "eta", "0.3");

	const char* name = "training";
	const char* eval_result = NULL;

	for (int i = 0; i < 100; i++) {
		XGBoosterUpdateOneIter(booster, i, dtrain);
		XGBoosterEvalOneIter(booster, i, &dtrain, &name, 1, &eval_result);

		std::cout << eval_result << std::endl;
	}

	bst_ulong out_len = 0;
	const float* out_result = NULL;


	XGBoosterPredict(booster, dtest, 0, 0, 0, &out_len, &out_result);
	std::cout << "y_predict" << std::endl;

	for (int i = 0; i < 100; i++) {
		printf("%1.3f ", out_result[i]);
	}
	std::cout << std::endl;

	XGDMatrixGetFloatInfo(dtest, "label", &out_len, &out_result);
	std::cout << "y_test" << std::endl;

	for (int i = 0; i < 100; i++) {
		printf("%1.3f ", out_result[i]);
	}



	XGDMatrixFree(dtrain);
	XGDMatrixFree(dtest);

	XGBoosterFree(booster);



}


	
*/