#! /bin/bash

create_data() {
    # create the data
    time ./build/ml_create_data data/com-amazon.metis --features features --communities data/com-amazon.cmty-nodes
    #time ./build/ml_create_data data/com-dblp.metis --features features --communities data/com-dblp.top5000.cmty.txt-nodes --graphlets data/com-dblp.metis.graphlet
    #time ./build/ml_create_data data/com-youtube.metis --features features --communities data/com-youtube.cmty-nodes --graphlets data/com-youtube.metis.graphlet
    #./build/ml_create_data data/test.metis --features features --communities data/test.cmty
    #time ./build/ml_create_data data/com-lj.metis --features features --communities data/com-lj.cmty-nodes
}

experiment() {

    #create_data

    rm booster.json

    #./build/ml_train data/com-amazon.metis-data --model booster.json
    ./build/ml_train data/com-dblp.metis-data --model booster.json
    ./build/ml_train data/com-youtube.metis-data --model booster.json
    #./build/ml_train data/test.metis-data --model booster.json


    echo "-------------------------- Testing ----------------------------"



    echo "-------------------------- Using the predicted weights ----------------------------"
    # perform testwise label propagation
    ./build/ml_label_propagation data/com-amazon.metis --data data/com-amazon.metis-data
    #./build/ml_label_propagation data/com-dblp.metis --data data/com-dblp.metis-data
    #./build/ml_label_propagation data/com-youtube.metis --data data/com-youtube.metis-data
    #./build/ml_label_propagation data/email-Eu-core.metis --data data/email-Eu-core.metis-data
    #./build/ml_label_propagation data/com-lj.metis-data --data data/com-lj.metis-data
    #./build/ml_label_propagation data/test.metis --data data/test.metis-data
}

experiment

#for feat in NODE_DEGREES SHARED_NEIGHBOR_COUNT CLUSTERING_COEFFICIENTS_LOCAL;
#do
#    echo NODE_COUNT > features
#    echo EDGE_COUNT >> features
#    echo GRAPHLETS >> features
#
#    echo $feat >> features
#    echo "Working on feature $feat"
#    experiment
#done


