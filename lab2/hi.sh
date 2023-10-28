#!/bin/bash

for i in {1..100}
do
    ./rr test.txt $i >> outputj.txt
done