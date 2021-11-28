#! /bin/bash

FILE=${1}

# cut the first line away | sort for the second number | sort stable for the first number
tail ${FILE} -n +2 | sort -n -k 2 | sort -n -s >> ${FILE}-translated