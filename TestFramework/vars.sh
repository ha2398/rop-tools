#!/bash/bin

# if 0, redirect benchmark output to /dev/null
# if 1, print benchmark output to stdout
[[ -n $DEBUG ]] || DEBUG=0 ;

# Specify the timeout
[[ -n $RUNTIME ]] || RUNTIME=8m ;

# Execute the benchmark
[[ -n $EXEC ]] || EXEC=1 ;

# Compile
[[ -n $COMPILE ]] || COMPILE=1 ;

# JOBS
[[ -n $JOBS ]] || JOBS=1 ;

# Perf repeat number
[[ -n $REPEAT ]] || REPEAT=5 ;

# Set the lib suffix.
suffix="dylib" ;
if [[ $(uname -s) == "Linux" ]]; then
  suffix="so" ;
fi

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

# LLVM_PATH  => The place where I have all the LLVM tools
LLVM_PATH="$HOME/llvm38/build/Debug+Asserts/bin"

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

# THIS PART IS LEFT AS AN EXAMPLE FOR THE PEOPLE WORKING WITH PIN!

# PIN 
[[ -n $PIN ]] || PIN=0 ;

if [[ $PIN -eq 1 ]]; then
  # PIN_PATH   => The place where I keep the pin source code
  PIN_PATH="$HOME/pin-3.4"
  PIN_LIB="$PIN_PATH/source/tools/SimpleExamples"

  echo "PIN_PATH is set to $PIN_PATH"

  cp ../Pintools/Overhead/complete.cpp ../Pintools/Overhead/pruned.cpp $PIN_PATH/source/tools/SimpleExamples
fi

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

BASEDIR="$(pwd)"

TESTDIR="$BASEDIR/Benchs/"

# -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- # -- 

echo "#########################"
echo "DEBUG is set to $DEBUG"
echo "RUNTIME is set to $RUNTIME"
echo "PIN is set to $PIN"
echo "EXEC is set to $EXEC"
echo "COMPILE is set to $COMPILE"
echo "suffix is set to $suffix"
echo "BASEDIR is set to $BASEDIR"
echo "TESTDIR is set to $TESTDIR"
echo "#########################"
