bnc_name="nbench" ;
lnk_name="$bnc_name.rbc" ;
prf_name="$bnc_name.ibc" ;
obj_name="$bnc_name.o" ;
exe_name="$bnc_name.exe" ;

source_files=("emfloat.c" "misc.c" "nbench0.c" "nbench1.c" "sysspec.c") ;
CXXFLAGS="-DBASE_ITERATIONS=25" ;
RUN_OPTIONS="" ;
