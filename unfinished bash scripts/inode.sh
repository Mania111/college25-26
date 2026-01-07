#inode.sh
#Author: Marta Fryczke

find / -mount -type f -links +1 -printf "%i %p\n" 2> /dev/null | sort -n -k 2 | uniq -c -w 7 | awk -F' ' '{print}'


