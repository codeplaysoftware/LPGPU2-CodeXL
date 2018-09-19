  1 attr  vPosition[0]
  1 attr  vPosition[1]
  1 attr  vPosition[2]
  1 attr  vPosition[3]
  2 attr  vTex[0]
  2 attr  vTex[1]
  4 uni   phase
  6 var   texcoord
  6 var   texcoord
  9 push  vPosition[0]
  9 push  vPosition[1]
  9 push  vPosition[2]
  9 push  vPosition[3]
 11 load  %1 phase
 11 neg   %1
 11 add   %2 vTex[0] %1
 11 load  %3 vTex[1]
 11 neg   %3
 11 inc   %3
 11 load  texcoord[0] %2
 11 load  texcoord[1] %3
 12 ret
