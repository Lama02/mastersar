#!/bin/sh

VER=33
NSHOME=/usr/local/ns-allinone-2.$VER
# test if NSHOME directory exists
#if [ ! -e $NSHOME ]; then
#        #we are on a configured system
#	tmp=$(which ns)
#	NSHOME=${tmp%/bin/ns}
#	tmp=${tmp#*/ns-allinone-*}
#	tmp=${tmp%%/*}
#	VER=${tmp#*.}
#fi

export PATH=$PATH:$NSHOME/bin:$NSHOME/tcl*/unix:/$NSHOME/tk*/unix
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NSHOME/lib:$NSHOME/otcl-*
export TCL_LIBRARY=$NSHOME/tcl*/library


if [ ! -h ~/ns-2 ] ; then
	ln -s $NSHOME/ns-2.$VER ~/ns-2
fi

export LANG=POSIX
export LC_ALL=C
export NS2=~/ns-2/bin
export NSHOME
export TP=$(pwd)
export NAM=$(echo  $NSHOME/nam-*)/bin

pathns=$TP:$NS2:$NAM
export PATH=$PATH:.:$pathns
export LIBSUITE=$TP/0-Lib

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NSHOME/otcl-*:$NSHOME/lib
#export TCL_LIBRARY=$NSHOME/tcl*/library
