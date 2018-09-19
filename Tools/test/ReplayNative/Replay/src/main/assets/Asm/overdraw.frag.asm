  3 uni   phase
  4 uni   opacity
  6 uni   xres
  7 uni   yres
 13 load  resolution[0] xres
 13 load  resolution[1] yres
 15 load  width resolution[0]
 16 load  %1 3.0
 16 rec   %1
 16 mul   %2 width %1
 16 load  sep1 %2
 17 load  %3 2.0
 17 mul   %3 %3 %1
 17 mul   %3 width %3
 17 load  sep2 %3
 19 load  v[0] 0.0
 19 load  v[1] 0.0
 19 load  v[2] 0.0
 19 load  v[3] 0.0
 21 cmp   gl_FragCoord[0] sep1
 21 brge  $1
 24 load  v[0] 1.0
 24 load  v[1] 0.0
 24 load  v[2] 0.0
 24 load  v[3] opacity
 24 bra   $2
 25 label $1
 25 cmp   gl_FragCoord[0] sep2
 25 brge  $3
 28 mul   %4 2.0 gl_FragCoord[2]
 28 neg   %4
 28 inc   %4
 28 load  v[0] 0.0
 28 load  v[1] %4
 28 load  v[2] 0.0
 28 load  v[3] opacity
 28 bra   $2
 29 label $3
 32 load  %5 gl_FragCoord[2]
 32 neg   %5
 32 add   %5 phase %5
 32 load  c %5
 34 push  c
 34 call  builtin.$floor
 34 pop   %6
 34 neg   %6
 34 add   %6 c %6
 34 load  c %6
 36 load  v[0] c
 36 load  v[1] c
 36 load  v[2] c
 36 load  v[3] opacity
 39 label $2
 39 push  v[0]
 39 push  v[1]
 39 push  v[2]
 39 push  v[3]
 40 ret
