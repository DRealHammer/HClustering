#! /bin/bash

echo "create the data file"
./build/HClustering --createData data/com-dblp/com-dblp.metis data/com-dblp-data data/com-dblp/com-dblp.top5000.cmty.txt-nodes

echo "use this file to learn"
./build/HClustering --train data/com-dblp-data

echo "perform a label propagation on this graph"
./build/HClustering --labelPropagation data/com-dblp/com-dblp.metis data/com-dblp-data