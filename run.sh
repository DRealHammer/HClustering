#! /bin/bash

program=./build/ml_clustering

if [ -z ${1} ]; then
echo "a graph name is required"
exit 1
fi

graphFile=${1}


# create the data
#${program} --createData --graphFile data/${graphFile}/${graphFile}.metis --communityFilename data/${graphFile}/${graphFile}.top5000.cmty.txt-nodes --outFilename data/${graphFile}-data --graphletFile data/${graphFile}/${graphFile}.metis.graphlet

# train with the created data
#${program} --train --dataFile data/${graphFile}-data

echo "-------------------------- Testing ----------------------------"

if [ -z ${2} ]; then
echo "no test graph specified"
exit
fi

testGraph=${2}

echo "-------------------------- Using the predicted weights ----------------------------"
# perform testwise label propagation
${program} --labelPropagation --graphFile data/${testGraph}/${testGraph}.metis --dataFile data/${testGraph}-data --check


#echo "-------------------------- Using random weights ----------------------------"
# perform testwise label propagation with random weights
#${program} --labelPropagation --graphFile data/${graphFile}/${graphFile}.metis --dataFile data/${graphFile}-data --check --random



