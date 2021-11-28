# HClustering

to build the project:
- add the xgboost library in the so called folder (or clone with git clone --recurse-submodules)
- create a folder for the build
- build the project with cmake

## Usage

to create initial data from a graph and if available a community file (you can specify a graphlet file, if already calculated)
`./build/HClustering --createData <graphFilename> <outputFilename> [<communityFilename>] [<graphletFilename>]`
*remark: the order of the arguments is fixed*

to train the model in the directory (currently uses always "booster.json", will create one if not existing)
`./build/HClustering --train <dataFilename>`
  
to use a simple labelPropagation to cluster a graph (first use createData to create the data file)
`./build/HClustering --labelPropagation <graphFilename> <dataFilename>`

### Note

for faster reading speeds, the community files were transformed to a node-centric version instead of community-centric
(so every line are first the node and then the communities for this node)

to create such a file one can use the communityNodesList_nodesCommunityList.py script

### Example

for a clean start remove the old booster
`rm booster.json`

```bash

# first create the data file we can you for the learning step
# important: use always the *-nodes files (see Note for more)
./build/HClustering --createData data/com-dblp/com-dblp.metis data/com-dblp-data data/com-dblp/com-dblp.top5000.cmty.txt-nodes

# now we can use this file to learn
./build/HClustering --train data/com-dblp-data

# now we can perform a label propagation on this graph
# here the edges are weighted by the model we trained
# for this the edges are being processed by the model
./build/HClustering --labelPropagation data/com-dblp/com-dblp.metis data/com-dblp-data


```
