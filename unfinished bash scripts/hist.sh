#hist.sh
#Author : Marta Fryczke

bean=$1

file /bin/* |
grep -F ELF | 
cut -f1 -d":" | 
xargs du -b | 
cut -f1 | 
sort -n |
(
    while read f_size
    do
        echo $(($f_size/$bean*$bean))
    done 
) | uniq -c
