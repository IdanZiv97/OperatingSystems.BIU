#!/bin/bash

# global variable for pwd

# check the number of of args
if [ $# -lt 2 ]; 
    then
        echo "Not enough parameters"
elif [[ $# -ge 3 ]] && [[ $3 == "-r" ]];
    then
    cd $1
    word=$2
    # recursive invokation
    # delete all the .out files
    find . -type f -name "*.out" -exec rm {} +;
    # search files with the word and compile them
    for file in $(grep -niwl $word $(find . -type f -name "*c"));
    do
        gcc $file -w -o ${file%.*}.out
    done
else
    cd $1
    word=$2
    find . -maxdepth 1 -type f -name "*.out" -exec rm {} +;
    for file in $(grep -niwl $word $(find . -maxdepth 1 -type f -name "*.c"));
    do
        gcc $file -w -o ${file%.*}.out
    done
fi
