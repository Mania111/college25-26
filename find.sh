# Author: Marta Fryczke
# find.sh name path1 path2
# using du and grep command

read -p filename
read -p path
# 1. find file of same name in path
# 2. check if file size is the same as what we're looking for
sizeOfFile=$(wc -c filename)
if [ (wc -c grep -r filename path) -eq sizeOfFile ]; then
	echo


