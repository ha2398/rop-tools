#!/bin/bash

function execute() {

  IFS='/' read -r -a folders <<< "$(pwd)"

  cmd="$PIN_PATH/pin -t $PIN_LIB/obj-intel64/$PINTOOL.so \
       $PIN_FLAGS -o $BASEDIR/overhead_outputs/$PINTOOL/${folders[-1]}.log \
       -- ./$exe_name $RUN_OPTIONS < $STDIN > $STDOUT" ;

  echo "$cmd"
  echo "cd $(pwd) && $cmd" >> $BASEDIR/run.txt ;
  
}
