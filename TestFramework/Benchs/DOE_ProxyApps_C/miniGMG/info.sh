bnc_name="miniGMG" ;
lnk_name="$bnc_name.rbc" ;
prf_name="$bnc_name.ibc" ;
obj_name="$bnc_name.o" ;
exe_name="$bnc_name.exe" ;
source_files=($(ls *.c)) ;
CXXFLAGS=" -lm " ;
RUN_OPTIONS=" 6  2 2 2  1 1 1 " ;
