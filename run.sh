#! /bin/bash




# create the data
#./build/ml_create_data data/com-amazon.metis --features features --communities data/com-amazon.top5000.cmty.txt-nodes --graphlets data/com-amazon.metis.graphlet
#./build/ml_create_data data/com-dblp.metis --features features --communities data/com-dblp.top5000.cmty.txt-nodes
#./build/ml_create_data data/com-youtube.metis --features features --communities data/com-youtube.top5000.cmty.txt-nodes --graphlets data/com-youtube.metis.graphlet
#./build/ml_create_data data/email-Eu-core.metis --communities data/email-Eu-core.cmty.txt-nodes --features features

./build/ml_create_data data/com-lj.metis --features features --communities data/com-lj.top5000.cmty.txt-nodes --graphlets data/com-lj.metis.graphlet
# train with the created data
#./build/ml_train data/com-amazon.metis-data --model booster.json

#echo "-------------------------- Testing ----------------------------"



#echo "-------------------------- Using the predicted weights ----------------------------"
# perform testwise label propagation
#./build/ml_label_propagation data/com-amazon.metis --data data/com-amazon.metis-data


#echo "-------------------------- Using random weights ----------------------------"
# perform testwise label propagation with random weights
#${program} --labelPropagation --graphFile data/${graphFile}/${graphFile}.metis --dataFile data/${graphFile}-data --check --random



# create the data
#./build/ml_create_data data/com-dblp.metis --features features --communities data/com-dblp.top5000.cmty.txt-nodes


# train with the created data
#./build/ml_train data/com-dblp.metis-data --model booster.json

#echo "-------------------------- Testing ----------------------------"



#echo "-------------------------- Using the predicted weights ----------------------------"
# perform testwise label propagation
#./build/ml_label_propagation data/com-dblp.metis --data data/com-dblp.metis-data

