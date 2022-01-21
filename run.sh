#! /bin/bash

create_data() {
    # create the data
    #./build/ml_create_data data/com-amazon.metis --features features --communities data/com-amazon.cmty --graphlet data/com-amazon.metis.graphlet
    #./build/ml_create_data data/com-dblp.metis --features features --communities data/com-dblp.cmty
    #./build/ml_create_data data/com-youtube.metis --features features --communities data/com-youtube.cmty
    #./build/ml_create_data data/test.metis --features features --communities data/test.cmty
    #./build/ml_create_data data/com-lj.metis --features features --communities data/com-lj.cmty

    #./build/ml_create_data data/cora.metis --features features --communities data/cora.cluster --graphlet data/cora.metis.graphlet
    ./build/ml_create_data data/citeseer.metis --features features --communities data/citeseer.cluster --graphlet data/citeseer.metis.graphlet
    ./build/ml_create_data data/pubmed.metis --features features --communities data/pubmed.cluster --graphlet data/pubmed.metis.graphlet
    ./build/ml_create_data data/ppi.metis --features features --communities data/ppi.cluster --graphlet data/ppi.metis.graphlet
    ./build/ml_create_data data/reddit.metis --features features --communities data/reddit.cluster --graphlet data/reddit.metis.graphlet
}

experiment() {
    create_data

    #rm booster.json

    #./build/ml_train data/cora.metis-data --model booster.json
    #./build/ml_train data/citeseer.metis-data --model booster.json
    #./build/ml_train data/pubmed.metis-data --model booster.json
    #./build/ml_train data/ppi.metis-data --model booster.json
    #./build/ml_train data/reddit.metis-data --model booster.json
    #./build/ml_train data/all.metis-data --model booster.json

    #./build/ml_train data/com-amazon.metis-data --model booster.json
    #./build/ml_train data/com-dblp.metis-data --model booster.json
    #./build/ml_train data/com-youtube.metis-data --model booster-youtube.json
    #./build/ml_train data/test.metis-data --model booster.json
    #./build/ml_train data/com-lj.metis-data --model booster.json


    echo "-------------------------- Testing ----------------------------"



    echo "-------------------------- Using the predicted weights ----------------------------"
    # perform testwise label propagation
    #/build/ml_label_propagation data/com-amazon.metis --data data/com-amazon.metis-data
    #./build/ml_label_propagation data/com-dblp.metis --data data/com-dblp.metis-data
    #./build/ml_label_propagation data/com-youtube.metis --data data/com-youtube.metis-data
    #./build/ml_label_propagation data/email-Eu-core.metis --data data/email-Eu-core.metis-data
    #./build/ml_label_propagation data/com-lj.metis-data --data data/com-lj.metis-data
    #./build/ml_label_propagation data/test.metis --data data/test.metis-data

    ./build/ml_label_propagation data/cora.metis --data data/cora.metis-data >> output-cora-random.txt
    ./build/ml_label_propagation data/citeseer.metis --data data/citeseer.metis-data >> output-citeseer-random.txt
    ./build/ml_label_propagation data/pubmed.metis --data data/pubmed.metis-data >> output-pubmed-random.txt
    ./build/ml_label_propagation data/ppi.metis --data data/ppi.metis-data >> output-ppi-random.txt
    ./build/ml_label_propagation data/reddit.metis --data data/reddit.metis-data >> output-reddit-random.txt
    
}



experiment