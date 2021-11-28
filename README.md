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
