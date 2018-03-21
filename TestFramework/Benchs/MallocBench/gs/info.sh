bnc_name="$(basename $(pwd))" ;
lnk_name="$bnc_name.rbc" ;
prf_name="$bnc_name.ibc" ;
obj_name="$bnc_name.o" ;
exe_name="$bnc_name.exe" ;

PROJ_SRC_DIR=$(pwd) ;

source_files=("gs.c" "gsmain.c" "gp_unix.c" "utrace.c" \
           "ialloc.c" "idebug.c" "idict.c" "iinit.c" "iname.c" \
           "interp.c" "iscan.c" "iutil.c" "stream.c" \
           "zarith.c" "zarray.c" "zcontrol.c" "zdict.c" "zfile.c" \
           "zgeneric.c" "zmath.c" "zmisc.c" "zpacked.c" "zrelbit.c" \
           "zstack.c" "zstring.c" "ztype.c" "zvmem.c" \
           "zchar.c" "zcolor.c" "zfont.c" "zdevice.c" "zgstate.c" \
           "zht.c" "zmatrix.c" "zpaint.c" "zpath.c" "zpath2.c" \
          "gschar.c" "gscolor.c" "gscoord.c" "gsdevice.c" \
          "gsfile.c" "gsfont.c" "gsimage.c" "gsim2out.c" \
          "gsline.c" "gsmatrix.c" "gsmisc.c" \
          "gspaint.c" "gspath.c" "gspath2.c" "gsstate.c" "gstype1.c" \
          "gxcache.c" "gxcolor.c" "gxdither.c" "gxdraw.c" "gxfill.c" \
          "gxht.c" "gxpath.c" "gxpath2.c" "gxstroke.c" \
          "gdevmem.c" "gdevs.c") ;

CXXFLAGS=" -lm -DNOMEMOPT \
           -DGS_LIB_DEFAULT=\"$PROJ_SRC_DIR:$PROJ_SRC_DIR/fonts\"" ;

RUN_OPTIONS="-DNODISPLAY $PROJ_SRC_DIR/INPUT/large.ps" ;



