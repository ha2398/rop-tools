#!/bin/bash

function compile() {

  # source_files is the variable with all the files we're gonna compile

  CXXFLAGS="$CXXFLAGS -w -O2 -fno-strict-aliasing -fcf-protection=branch"

  for file_name in "${source_files[@]}"; do
    $COMPILER $file_name -o ${file_name::-2}.o -c $CXXFLAGS;
  done

  $COMPILER -o $exe_name *.o $CXXFLAGS ;
}
