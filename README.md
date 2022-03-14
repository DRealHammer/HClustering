# HClustering

This project is a code basis for creating high quality graph clusterings with the help of machine learning. 
The target is to use the local structure of an edge inside of a graph to determine a score of how likely the edge is to be contained inside of a cluster.
Graphs are assumed to be in the [metis graph format](https://people.sc.fsu.edu/~jburkardt/data/metis_graph/metis_graph.html)

# Installation

## Downloading

The project makes use of different libraries.
To include them automatically while cloning

```bash
git clone https://github.com/DRealHammer/HClustering.git --recurse-submodules
```

## Compilation

first add a build directory

```bash
mkdir build
```

then configure cmake and build the project

```bash
cmake -S . -B build
cmake --build build
```

The compiled executables are now in the build directory (ml_create_data, ml_train, ml_cluster).

# Usage

The features for the usage need to be defined beforehand in a file. An example is the file `features`.

## Preprocessing

The data of a graph (features and labels if available) needs to be extracted first

```bash
./build/ml_create_data data/3-cluster.metis --features features --communities data/3-cluster.cluster
```

With this we first specify the graph and the features we want to use.
The `--communities` option is only used for graphs with a known clustering. From this the labels for the training are generated.
If no clustering is known or should be used this option can be omitted.

## Training

A model needs to be fitted before clustering.

```bash
./build/ml_train data/3-cluster.metis-data --model booster.json --iterations 10
```
Here we use the before created data file of the edges. We also specify the filename of the model (always as .json).
The iterations is the number of trees created to fit the model (for more see [XGBoost](https://github.com/dmlc/xgboost)).

## Clustering

The model can now score an edge given it's features.
First create the folder for the clusterings.

```bash
mkdir clustering
```

Then create a data file containing the edge features for the target graph (no option `--communities` needed) as before.

Now use the fitted model:

```bash
./build/ml_cluster data/3-cluster.metis --data data/3-cluster.metis-data --model booster.json
```

This command will create clusters.
For every individual score apprearing  for the edges a clustering will be created in the folder `clustering`.
A clustering is named `threshold-clustering-X`, with X a float. 
The clustering has clusters with all edges contracted with a score higher or equal X.

# Licence
The program is licenced under MIT licence.
If you publish results using our algorithms, please acknowledge our work by linking this page.

# Acknowledgements

- Code basis and graph datastructure [KaHIP](https://github.com/KaHIP/KaHIP)
- Machine learning library [XGBoost](https://github.com/dmlc/xgboost)
- Grahplet extraction [PGD](https://github.com/nkahmed/PGD/) Nesreen K. Ahmed, Jennifer Neville, Ryan A. Rossi, Nick Duffield, Efficient Graphlet Counting for Large Networks, IEEE International Conference on Data Mining (ICDM), pages 10, 2015.
- Conversation for different graph formats https://github.com/guowentian/SubgraphMatchGPU
