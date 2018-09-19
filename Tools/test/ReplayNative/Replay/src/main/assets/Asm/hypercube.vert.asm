  2 attr  vPosition[0]
  2 attr  vPosition[1]
  2 attr  vPosition[2]
  2 attr  vPosition[3]
  3 attr  vColor[0]
  3 attr  vColor[1]
  3 attr  vColor[2]
  5 var   col
  7 uni   xres
  8 uni   yres
 12 load  %1 xres
 12 inc   %1
 12 load  %2 yres
 12 inc   %2
 12 load  resolution %1 %2
 14 load  col vPositon[3]
 15 add   %3 vPosition[3] 2.0
 15 rec   %3
 15 mul   %3[0] vPosition[0] %3[0]
 15 mul   %3[1] vPosition[1] %3[1]
 15 mul   %3[2] vPosition[2] %3[2]
 15 load  p %3[0] %3[1] %3[2]
 16 mul   %4 p.x resoluton[1]
 16 load  %5 resolution[0]
 16 rec   %5
 16 mul   %5 %4 %5
 16 push  %5
 16 push  p[1]
 16 push  p[2]
 16 push  1.0
 17 ret
