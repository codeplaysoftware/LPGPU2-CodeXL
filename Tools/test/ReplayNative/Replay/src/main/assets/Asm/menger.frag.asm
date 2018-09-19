  4 var   position[0]
  4 var   position[1]
  4 var   position[2]
 12 load  epsilon 0.004
 14 load  %1 3.0
 14 rec   %1
 14 load  lo %1
 15 load  %2 lo
 15 neg   %2
 15 inc   %2
 15 load  hi %2
 19 label $within
 19 pop   p
 19 pop   eps
 19 load  %3 lo
 19 add   %3 %3 eps
 19 cmp   p %3
 19 brlt  $1
 19 load  %4 eps
 19 neg   %4
 19 add   %5 hi %4
 19 cmp   p %5
 19 brgt  $1
 19 push  1.0
 19 ret
 19 label $1
 19 push  0.0
 19 ret
 23 label $count
 23 pop   p[2]
 23 pop   p[1]
 23 pop   p[0]
 23 pop   eps
 23 push  p[0]
 23 push  eps
 23 call  $within
 23 pop   %6
 23 push  p[1]
 23 push  eps
 23 call  $within
 23 pop   %7
 23 push  p[2]
 23 push  eps
 23 call  $within
 23 pop   %8
 23 add   %9 %6 %7
 23 add   %9 %8 %9
 23 push  %9
 23 ret
 27 label $inside
 27 pop   p[2]
 27 pop   p[1]
 27 pop   p[0]
 28 load  eps epsilon
 30 push  p
 30 push  eps
 30 call  count
 30 pop   %10
 30 cmp   %10 1
 30 brle  $2
 30 push  false
 30 ret
 32 label $2
 32 load  %11 0
 32 label $3
 33 mul   p[0] 3.0 p[0]
 33 label $4
 33 cmp   p[0] 1.0
 33 brle  $5
 33 dec   p[0]
 33 bra   $4
 34 label $5
 34 mul   p[1] 3.0 p.5
 34 label $6
 34 cmp   p[1] 1.0
 34 brle  $7
 34 dec   [1]p
 34 bra   $6
 35 label $7
 35 mul   p[2] 3.0 0.5
 35 label $8
 35 cmp   p[2] 1.0
 35 brle  $9
 35 dec   [2]p
 35 bra   $8
 37 push  p
 37 push  eps
 37 call  $count
 37 pop   %12
 37 cmp   %12 1
 37 brle  $10
 37 push  true
 37 ret
 39 label $10
 39 mul   %13 eps 3.0
 39 load  eps %13
 40 inc   %11
 40 cmp   %11 level
 40 brlt  $3
 42 push  false
 42 ret
 48 pop   gl_FragCoord[2]
 48 pop   gl_FragCoord[1]
 48 pop   gl_FragCoord[0]
 48 load  %14 gl_FragCoord[2]
 48 neg   %14
 48 inc   %14
 48 load  c %14
 52 push  position[0]
 52 push  position[1]
 52 push  position[2]
 52 call  $inside
 52 pop   %15
 52 cmp   %15 true
 52 brne  $11
 52 dscd
 52 ret
 54 label $11
 54 push  c
 54 push  c
 54 push  1.0
 54 push  1.0
 55 ret
