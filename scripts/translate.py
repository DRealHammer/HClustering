import re
import sys




def shiftIndex_not_needed(): 
	with open("testGraph.txt", "r") as file:
		content = file.read().replace("\n", " ").split(" ")

		with open("testGraphBase0.txt", "w") as out:
			out.write(f"{content[0]} {content[1]}\n")

			# go through all other, starting with index 2
			for i in range(2, len(content), 3):
				out.write(f"{int(content[i]) - 1} {int(content[i+1]) - 1} {content[i+2]}\n")


# comments are marked with hashtags(#)
# node and edge count are in the comments like: # Nodes: 334863 Edges: 35566
# edges are described like: <nodefrom> <nodeto>
def to_metis_from_amazon(graphFilename, communityFilename):

	graph = dict()

	# read the graph
	with open(graphFilename, "r") as file:

		for line in file:

			# skip all comments and find nodecount and edgecount in the comments
			if line[0] == "#":
				continue

			# get the start and end node
			edgeStrings = re.findall(r"(\d*).(\d*)", line)[0]
			if edgeStrings:
				fromNode = int(edgeStrings[0])
				toNode = int(edgeStrings[1])

				if fromNode in graph.keys():
					graph[fromNode].append(toNode)
				else:
					graph[fromNode] = [toNode]

				if toNode in graph.keys():
					graph[toNode].append(fromNode)
				else:
					graph[toNode] = [fromNode]


	# translate the graph to dense indices
	nodeCount = len(graph.keys())
	edgeCount = 0
	for edges in graph.values():
		edgeCount += len(edges)

	# we have an undirected graph
	edgeCount = int(edgeCount/ 2)
		
	# we have the graph in a dict
	# there could be missin indices so we create a translation table to start from index 1
	translation = dict()
	for index, nodeid in enumerate(graph.keys()):
		translation[nodeid] = index + 1


	communities = []
	# read the communities file and translate it in the new indices
	with open(communityFilename, 'r') as file:
		for line in file:
			communities.append([translation[int(id)] for id in line.split("\t")])



	# write the metis file
	with open(graphFilename + "-metis", 'w+') as outfile:
		# first write a comment
		outfile.write("% metis graph format: https://people.sc.fsu.edu/~jburkardt/data/metis_graph/metis_graph.html\n")
		outfile.write("% auto-translated by script of Daniel Hammer\n")
	
		# then write the node and edge count
		outfile.writelines(f"{nodeCount} {edgeCount}\n")

		# for a list of connected nodes
		for edges in graph.values():
			writeLine = ""

			# get all new node ids and write them in a line
			for toNode in edges:
				writeLine += str(translation[toNode]) + " "

			writeLine += "\n"
			outfile.write(writeLine)
	


	# write the communities
	with open(communityFilename + "-new", 'w+') as outfile:
		for community in communities:
				outfile.write("\t".join([str(node) for node in community]) + "\n")




if __name__ == "__main__":

	if len(sys.argv) != 3:
		print("start translation with arguments graphfile and community file")
		sys.exit()
	graphFile = sys.argv[1]
	communityFile = sys.argv[2]
	to_metis_from_amazon(graphFile, communityFile)
