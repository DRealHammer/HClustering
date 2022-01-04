#include <ml_clustering/graphMetrics.h>

struct Volumes {
    int own;
    int complement;
};

float conductance(graph_access& graph, std::vector<PartitionID>& clustering) {
    
    // get all existing cluster ids
    std::vector<PartitionID> clusterIds;
    {
        std::vector<bool> clusterExists(clustering.size(), false);

        for (auto c : clustering) {
            clusterExists[c] = true;
        }

        for (int i = 0; i < clusterExists.size(); i++) {
            if (clusterExists[i]) {
                clusterIds.push_back(i);
            }
        }
    }

    std::vector<float> conductances(clusterIds.size(), 0);
    {
        #pragma omp parallel for
        for (int i = 0; i < clusterIds.size(); i++) {
            PartitionID c = clusterIds[i];

            // calculate the cut weight
            forall_nodes(graph, startNode)

                if (clustering[startNode] != c)
                    continue;

                forall_out_edges(graph, e, startNode)
                    NodeID targetNode = graph.getEdgeTarget(e);
                    conductances[i] += clustering[targetNode] != c;
                endfor
            endfor
        }

        // calculate the volumes a(C), a(V\C)
        // and divide by the minimum
        #pragma omp parallel for
        for (int i = 0; i < clusterIds.size(); i++) {
            Volumes v = {0, 0};
            PartitionID c = clusterIds[i];

            forall_nodes(graph, startNode)
                int deg = graph.getNodeDegree(startNode);
                if (clustering[startNode] == c) {
                    v.own += deg;
                } else {
                    v.complement += deg;
                }
            endfor

            int min = std::min(v.own, v.complement);

            if (min == 0) {
                conductances[i] = std::numeric_limits<float>::infinity();
            } else {
                float val = conductances[i] / static_cast<float>(min);
                conductances[i] = val;
            }

        }

        

    }

    return *std::max_element(conductances.begin(), conductances.end());
}