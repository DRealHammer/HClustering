if __name__ == "__main__":
	filename = "com-amazon.top5000.cmty.txt-correct"
	outfile = "com-amazon.cmty-nodes"


	nodeCommunities = dict()

	with open(filename, "r") as inFile:

		
		# create a list of communities for every node 

		communityNr = 0
		for line in inFile:
			if len(line) == 0 or line == "\n":
				continue
			for node in [int(n) for n in line.split(" ")]:
				if node not in nodeCommunities.keys():
					nodeCommunities[node] = list()
				nodeCommunities[node].append(communityNr)

			# increase the communityNr as we finished this community
			communityNr += 1

	with open(outfile, "w") as outFile:
		for nodeID in nodeCommunities.keys():

			outFile.write(str(nodeID) + " " + " ".join([str(comm) for comm in nodeCommunities[nodeID]]) + "\n")
