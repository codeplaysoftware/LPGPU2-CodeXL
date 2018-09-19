  3 var   col
  6 pop   glFragCoord[2]
  6 pop   glFragCoord[1]
  6 pop   glFragCoord[0]
  6 load  %1 glFragCoord[2]
  6 neg   %1
  6 inc   %1
  6 load  c %1
  7 push  c
  7 push  col
  7 load  %2 col
  7 neg   %2
  7 inc   %2
  7 push  %2
  7 push  0.5
  8 ret
