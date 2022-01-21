#include <ml_clustering/graphMetrics.h>

struct Volumes {
    int own;
    int complement;
};

float conductance(graph_access& graph, std::vector<PartitionID>& clustering) {
    
    // get all existing cluster ids
    PartitionID clusterCount = 0;
    {
        std::map<PartitionID, PartitionID> uniqueClusterIds;

        for (auto c : clustering) {

            // if we don't know the cluster yet
            if (uniqueClusterIds.find(c) == uniqueClusterIds.end()) {
                uniqueClusterIds[c] = clusterCount;
                clusterCount++;
            }
        }


        // translate old labels to new indices
        for (auto& c : clustering) {
            c = uniqueClusterIds[c];
        }
    }

    std::vector<float> conductances(clusterCount, 0);
    {
        //#pragma omp parallel for
        for (int c = 0; c < clusterCount; c++) {

            // calculate the cut weight
            forall_nodes(graph, startNode)

                if (clustering[startNode] != c)
                    continue;

                forall_out_edges(graph, e, startNode)
                    NodeID targetNode = graph.getEdgeTarget(e);
                    conductances[c] += clustering[targetNode] != c;
                endfor
            endfor
        }

        // calculate the volumes a(C), a(V\C)
        // and divide by the minimum
        //#pragma omp parallel for
        for (int c = 0; c < clusterCount; c++) {
            Volumes v = {0, 0};

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
                continue;
            } else {
                float val = conductances[c] / static_cast<float>(min);
                conductances[c] = val;
            }
        }

        

    }

    return *std::max_element(conductances.begin(), conductances.end());
}