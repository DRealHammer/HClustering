#! /bin/bash


create_data() {
    # create the data
    #./build/ml_create_data data/com-amazon.metis --features features --communities data/com-amazon.cmty --graphlet data/com-amazon.metis.graphlet
    #./build/ml_create_data data/com-dblp.metis --features features --communities data/com-dblp.cmty
    #./build/ml_create_data data/com-youtube.metis --features features --communities data/com-youtube.cmty
    #./build/ml_create_data data/test.metis --features features --communities data/test.cmty
    #./build/ml_create_data data/com-lj.metis --features features --communities data/com-lj.cmty

    
    #./build/ml_create_data data/cora.metis --features features --communities data/cora.cluster
    #./build/ml_create_data data/citeseer.metis --features features --communities data/citeseer.cluster --graphlet data/citeseer.metis.graphlet
    #./build/ml_create_data data/pubmed.metis --features features --communities data/pubmed.cluster --graphlet data/pubmed.metis.graphlet
    #./build/ml_create_data data/ppi.metis --features features --communities data/ppi.cluster --graphlet data/ppi.metis.graphlet
    #./build/ml_create_data data/reddit.metis --features features --communities data/reddit.cluster --graphlet data/reddit.metis.graphlet
    #./build/ml_create_data data/3-cluster.metis --features features --communities data/3-cluster.cluster
    #./build/ml_create_data data/4-cluster.metis --features features --communities data/4-cluster.cluster
    #./build/ml_create_data data/3-connected.metis --features features --communities data/3-connected.cluster

    #./build/ml_create_data data/333SP.metis --features features --communities data/333SP.cluster --graphlet data/333SP.metis.graphlet
    ./build/ml_create_data data/luxembourg.metis --features features --communities data/luxembourg.cluster
    # ./build/ml_create_data data/rgg_17.metis --features features --communities data/rgg_17.cluster --graphlet rgg_17.metis.graphlet

}

experiment() {
    #create_data

    #cmake --build build
    #rm booster.json
    #rm 3-cluster.out

    #./build/ml_train data/luxembourg.metis-data --model booster.json --iterations 10

    #./build/ml_train data/train.data --test data/test.data --model booster.json --iterations 5
    #./build/ml_train data/train.data --test data/test.data --model booster.json --iterations 100
    #./build/ml_train data/train.data --test data/test.data --model booster-luxembourg.json --iterations 100 >> luxembourg-split-degrees-not-normalized.out
    #./build/ml_train data/train.data --test data/test.data --model booster-33SP.json --iterations 100 >> 333SP-split.out
    #./build/ml_train data/train.data --test data/test.data --model booster-rgg_17.json --iterations 100 >> rgg_17-split.out
    #./build/ml_train data/train.data --test data/test.data --model booster-rgg_17-luxembourg-333SP.json --iterations 100 >> all-split.out

    #./build/ml_train data/cora_train --test data/cora_test --model booster-cora.json --iterations 1000 >> cora.out
    #./build/ml_train data/cora.metis-data --model booster.json --iterations 50
    #./build/ml_train data/citeseer.metis-data --model booster.json
    #./build/ml_train data/pubmed.metis-data --model booster.json
    #./build/ml_train data/all.data --model booster.json --iterations 100 >> all.out


    #./build/ml_train data/ppi.metis-data --model booster.json
    #./build/ml_train data/reddit.metis-data --model booster.json
    

    #./build/ml_train data/com-amazon.metis-data --model booster.json
    #./build/ml_train data/com-dblp.metis-data --model booster.json
    #./build/ml_train data/com-youtube.metis-data --model booster-youtube.json
    #./build/ml_train data/test.metis-data --model booster.json
    #./build/ml_train data/com-lj.metis-data --model booster.json


    #echo "-------------------------- Testing ----------------------------"



    #echo "-------------------------- Using the predicted weights ----------------------------"
    # perform testwise label propagation
    #/build/ml_cluster data/com-amazon.metis --data data/com-amazon.metis-data
    #./build/ml_cluster data/com-dblp.metis --data data/com-dblp.metis-data
    #./build/ml_cluster data/com-youtube.metis --data data/com-youtube.metis-data
    #./build/ml_cluster data/email-Eu-core.metis --data data/email-Eu-core.metis-data
    #./build/ml_cluster data/com-lj.metis-data --data data/com-lj.metis-data
    #./build/ml_cluster data/test.metis --data data/test.metis-data

    #./build/ml_cluster data/luxembourg.metis --data data/luxembourg.metis-data --model booster.json >> output-luxembourg.txt

    #./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data >> output-3-cluster-not-normalized-luxembourg.txt --model booster-luxembourg.json
    #./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data >> output-3-cluster-not-normalized-333SP.txt --model booster-333SP.json
    #./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data --model booster-rgg_17.json >> output-3-cluster-rgg_17.txt 
    #./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data --model booster.json


    #./build/ml_cluster data/4-cluster.metis --data data/4-cluster.metis-data >> output-4-cluster.txt

    #./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data --model booster-rgg_17-luxembourg-333SP.json >> output-3-cluster-all.txt

    #./build/ml_cluster data/3-connected.metis --data data/3-connected.metis-data --model booster.json
    #./build/ml_cluster data/cora.metis --data data/cora.metis-data --model booster.json >> output-cora.txt
    #./build/ml_cluster data/citeseer.metis --data data/citeseer.metis-data >> output-citeseer-random.txt
    #./build/ml_cluster data/pubmed.metis --data data/pubmed.metis-data >> output-pubmed-random.txt
    #./build/ml_cluster data/ppi.metis --data data/ppi.metis-data >> output-ppi-random.txt
    #./build/ml_cluster data/reddit.metis --data data/reddit.metis-data >> output-reddit-random.txt

    #./build/ml_cluster data/333SP.metis --data data/333SP.metis-data --model booster.json >> output-333SP-normalized.txt   
    #./build/ml_cluster data/rgg_17.metis --data data/rgg_17.metis-data --model booster.json >> output-rgg_17-normalized.txt   

    #./build/ml_create_data data/3-cluster.metis --features features --communities data/3-cluster.cluster
    ./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data --model booster.json
}



experiment