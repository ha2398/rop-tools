#!/bin/bash

function execute() {

  cmd="./$exe_name $RUN_OPTIONS < $STDIN > $STDOUT" ;

  echo "$cmd"
  echo "cd $(pwd) && $cmd" >> $BASEDIR/run.txt ;
  
}
