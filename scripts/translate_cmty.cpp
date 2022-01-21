#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>


int main(int argc, char** argv) {

	if (argc == 1) {
		std::cout << "./a.out <snap file> <cmty file>" << std::endl;
		return 1;
	}

	if (argc != 3) {
		std::cout << "wrong number of filenames" << std::endl;
		return 1;
	}

	
	std::string snapfile(argv[1]);

	std::ifstream snapstream(snapfile);

	std::map<int , int> translationTable;

	std::string line;
	int nodes[2];

	int currentNode = 0;
	while(getline(snapstream, line)) {
		if (line.length() == 0 || !std::isdigit(line[0])) continue;

		std::stringstream linestream(line);
		linestream >> nodes[0];
		linestream >> nodes[1];

		for (int i = 0; i < 2; i++) {
			// not yet seen
			if (translationTable.find(nodes[i]) == translationTable.end()) {
				translationTable[nodes[i]] = currentNode++;
			}
		}
	}

	snapstream.close();




	// translating the communityfile
	std::string cmtyfile(argv[2]);

	std::ifstream cmtystream(cmtyfile);
	

	int node;
	int currentCmty = 0;
	std::vector<std::vector<int>> nodeCmtys(translationTable.size());
	while(getline(cmtystream, line)) {
		if (line.length() == 0 || !std::isdigit(line[0])) continue;

		std::stringstream linestream(line);
		while(linestream >> node) {
			if (translationTable.find(node) == translationTable.end()) continue;
			
			nodeCmtys[translationTable[node]].push_back(currentCmty);
		}
		currentCmty++;
	}
	cmtystream.close();

	std::ofstream newCmtyStream(cmtyfile + "-nodes");

	// for every node
	for (int currentNode = 0; currentNode < nodeCmtys.size(); currentNode++) {

		// skip not clustered nodes
		if (nodeCmtys[currentNode].size() == 0) {
			continue;
		}

		newCmtyStream << currentNode + 1;

		// for every cmty
		for (auto cmty : nodeCmtys[currentNode]) {
			newCmtyStream << " " << cmty;
		}

		newCmtyStream << std::endl;
	}

	newCmtyStream.close();

}
