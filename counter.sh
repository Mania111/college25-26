# counter.sh
# Author: Marta Fryczke

(while ! ln numbers.txt numbers.lock 2>/dev/null; do sleep 0.1; done; echo "Access granted!" ) &

while true; do

    if [ -z "$NUMBER" ]; then
        NUMBER=1
    else
        NUMBER=$((NUMBER + 1))
    fi

    sleep 0.5
    echo "$NUMBER"


done


