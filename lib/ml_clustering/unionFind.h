#pragma once

#include <vector>
#include <definitions.h>



struct UnionFind {

	std::vector<NodeID> parents;
	std::vector<int> sizes;

	UnionFind(int nodeCount)
	: parents(nodeCount), sizes(nodeCount, 1) {

		// init the parents
		for (int i = 0; i < parents.size(); i++) {
			parents[i] = i;
		}
	}

	NodeID find_set(NodeID node) {
		if (node == parents[node]) {
			return node;
		}

		return parents[node] = find_set(parents[node]);
	}

	void unite(NodeID a, NodeID b) {
		NodeID parent_a = find_set(a);
		NodeID parent_b = find_set(b);

		// already together
		if ( a == b ) {
			return;
		}

		if (sizes[parent_a] < sizes[parent_b]) {
			std::swap(parent_a, parent_b);
		}

		// append smaller to bigger set
		parents[parent_b] = parent_a;

		// add sizes
		sizes[parent_a] += sizes[parent_b];
	}

};