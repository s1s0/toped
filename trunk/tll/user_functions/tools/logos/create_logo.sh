#!/bin/bash

# creates a TELL script which represents the information of a bitmap
# the workflow needs several external program --> please refer to README

# (C) Copyright 2010 by Armin Taschwer
#

## keep the temporary files?
#KEEP_FILES=1


if [ $# -ne 1 ] ; then
    echo "usage: create_logo.sh <file.pnm>"
    exit 1
fi

DIR=`dirname $1`
FILE=`basename $1 .pnm`
TEMPFILE=$$_temp_file.${RANDOM}

POTRACE=potrace
SED=sed
GREP=grep
FIG2DEV=fig2dev
OCTAVE="octave write_tell_script.m" 

if [ -f ${DIR}/${FILE}.pnm ] ; then
    
    FULLNAME=${DIR}/${FILE}.pnm
    
    echo "vectorizing ${FULLNAME} .."
    ${POTRACE} -b xfig -a -1 -o ${TEMPFILE}.fig ${FULLNAME}
    
    if ! [ -f ${TEMPFILE}.fig ] ; then
	echo "ERROR: problem vectorizing your file ${FULLNAME}"
	exit 1
    fi
    
    echo "converting to TK-file .."
    ${FIG2DEV} -L tk ${TEMPFILE}.fig ${TEMPFILE}.tcltk

    if ! [ -f ${TEMPFILE}.tcltk ] ; then
	echo "ERROR: problem converting the vector file to a TK file"
	exit 1
    fi
    
    echo "converting to .m file .."
    cat ${TEMPFILE}.tcltk | awk '/\\/{printf "%s",$0;next}{print}' | grep "create polygon" | sed -e 's/[^0-9 \.]//g' | tr -d '\\' > ${TEMPFILE}.dat    
    cat -b ${TEMPFILE}.dat | sed -e 's/\([0-9]\+\)\(.*\)/A(\1).poly = \[\2\];/' > ${TEMPFILE}.m 
    
    
    echo "calling octave .."
    ${OCTAVE} ${TEMPFILE}.m ${DIR}/${FILE}.tll
    
    if [ "x$KEEP_FILES" == "x" ] ; then
	rm -f ${TEMPFILE}.fig
	rm -f ${TEMPFILE}.tcltk
	rm -f ${TEMPFILE}.dat
	rm -f ${TEMPFILE}.m
    fi

else
    echo "ERROR: PNM version of your file doesn't exist"
    exit 1
fi
