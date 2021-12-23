import sys
from typing import OrderedDict

# correct incorrect indices
# comments are marked with hashtags(#)
# edges are described like: <nodefrom> <nodeto>
def correct_missing_indices(graphFilename, communityFilename = None):

	unique_ids:set[int] = set()
	wrongIndices = 0

	print("starting reading the file for the translation table")
	# read the graph
	with open(graphFilename, "r") as file:

		for line in file:

			# skip all comments
			if line[0] == "#":
				continue


			fromNode, toNode = line.split("	")
			fromNode = int(fromNode)
			toNode = int(toNode)

			unique_ids.add(fromNode)
			unique_ids.add(toNode)

	
	nodeCount = len(unique_ids)

	print("finished reading")

	print("constructing the table")
		
	# there could be missin indices so we create a translation table to start from index 1
	translation = dict()
	ids = sorted(sorted(list(unique_ids)))
	for index, nodeid in enumerate(ids):
		translation[nodeid] = index + 1
		if nodeid != index + 1:
			wrongIndices += 1
			

	print("finished construction!")

	if communityFilename != None:
		print("writing the correct community file")

		# write the correct community file
		with open(communityFilename, "r") as cmtFile, open(communityFilename + "-correct", "w") as outFile:
			for line in cmtFile:
				nodes = [str(translation[int(node)]) for node in line.split("	")]
				outFile.write(" ".join(nodes) + "\n")

	print("writing the correct graph file")

	with open(graphFilename, "r") as grFile, open(graphFilename + "-correct", "w") as outFile:
		for line in grFile:

			# skip all comments
			if line[0] == "#":
				continue


			fromNode, toNode = line.split("	")
			fromNode = int(fromNode)
			toNode = int(toNode)

			outFile.write(f"{translation[fromNode]} {translation[toNode]}\n")

	print(f"Fixed {wrongIndices} wrong Indices")


def correct_missing_indices_nodebased_cmty(graphFilename, communityFilename = None):

	unique_ids:set[int] = set()
	wrongIndices = 0

	print("starting reading the file for the translation table")
	# read the graph
	with open(graphFilename, "r") as file:

		for line in file:

			# skip all comments
			if line[0] == "#":
				continue


			fromNode, toNode = line.split(" ")
			fromNode = int(fromNode)
			toNode = int(toNode)

			unique_ids.add(fromNode)
			unique_ids.add(toNode)

	
	nodeCount = len(unique_ids)

	print("finished reading")

	print("constructing the table")
		
	# there could be missin indices so we create a translation table to start from index 1
	translation = dict()
	ids = sorted(sorted(list(unique_ids)))
	for index, nodeid in enumerate(ids):
		translation[nodeid] = index + 1
		if nodeid != index + 1:
			wrongIndices += 1
			

	print("finished construction!")

	if communityFilename != None:
		print("writing the correct community file")

		# write the correct community file
		with open(communityFilename, "r") as cmtFile, open(communityFilename + "-correct", "w") as outFile:
			for line in cmtFile:
				node, department = line.split(" ")
				node = int(node)
				department = int(department)

				outFile.write(f"{translation[node]} {department}\n")

	print("writing the correct graph file")

	with open(graphFilename, "r") as grFile, open(graphFilename + "-correct", "w") as outFile:
		for line in grFile:

			# skip all comments
			if line[0] == "#":
				continue


			fromNode, toNode = line.split(" ")
			fromNode = int(fromNode)
			toNode = int(toNode)

			outFile.write(f"{translation[fromNode]} {translation[toNode]}\n")

	print(f"Fixed {wrongIndices} wrong Indices")

# assume every edge appearing only once (smallNode bigNode)
# assume ids starting with 1
# assume normal space as seperator
def ungraph_to_metis(graphFilename):

	# read once for node count and edge count
	#nodeCount = 65608366
	
	uniqueIDs = set()
	edgeCount = 0

	with open(graphFilename, "r") as inFile:
	
		
		for line in inFile:
			# every line is an edge
			startNode, toNode = line.split(" ")
			startNode = int(startNode)
			toNode = int(toNode)

			if startNode != toNode:
				edgeCount += 1

			uniqueIDs.add(startNode)
			uniqueIDs.add(toNode)

	nodeCount = len(uniqueIDs) + 1

	print(f"found {nodeCount} Nodes and {edgeCount} edges")

	# create list of nodes for every node
	graph = [set() for _ in range(nodeCount + 1)]
	with open(graphFilename, "r") as inFile:
		for line in inFile:
			startNode, toNode = line.split(" ")
			startNode = int(startNode)
			
			toNode = int(toNode)

			if startNode == toNode:
				continue

			graph[startNode].add(toNode)
			graph[toNode].add(startNode)

	sum = 0
	for s in graph:
		sum += len(s)

	print(f"created the graph with {len(graph)} nodes and {sum} edges")

	with open(graphFilename + ".metis", "w") as outFile:
		# reduce by 1 again, because we added 1 for the indices to be correct
		outFile.write(f"{nodeCount - 1} {edgeCount}")
		for neighbors in graph:
			neighborStrings = [str(neighbor) for neighbor in neighbors]
			outFile.write(" ".join(neighborStrings) + "\n")

	print("finished writing the metis file")



#correct_missing_indices_nodebased_cmty("email-Eu-core.ungraph.txt", "email-Eu-core.cmty.txt")

ungraph_to_metis("email-Eu-core.ungraph.txt-correct-sorted")
