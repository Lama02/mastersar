#!/bin/bash

CMD=$1
CWD=$PWD

for F in $(find . -mindepth 1 -maxdepth 1 -type d); do
		cd $F
	  ant $CMD
		cd $CWD
done

