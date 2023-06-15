#! /bin/bash

for i in 1 2 3
do
    for algorithm in fifo lfu mfu lru
    do
        echo $i $algorithm
        ./project3 test/test$i $algorithm && mv result test/test$i/$algorithm
    done
done
