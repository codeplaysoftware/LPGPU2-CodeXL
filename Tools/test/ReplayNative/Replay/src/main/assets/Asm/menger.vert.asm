  2 attr  vPosition[0]
  2 attr  vPosition[1]
  2 attr  vPosition[2]
  4 var   position[0]
  4 var   position[1]
  4 var   position[2]
  7 load  %1 3.0
  7 sqrt  %2 %1
  7 rec   %2
  7 mul   %3 2.0 %2
  7 load  distance %3
 12 uni   frame
 14 uni   xres
 15 uni   yres
 19 load  resolution[0] xres
 19 load  resolution[1] yres
 21 add   %4[0] vPosition[0] 0.5
 21 add   %4[1] vPosition[1] 0.5
 21 add   %4[2] vPosition[2] 0.5
 21 load  position %4
 23 load  %5 frame
 23 mul   %5 %5 0.001
 23 load  p %5
 25 sin   %6 p
 25 load  sy %6
 26 cos   %7 p
 26 load  sc %7
 28 load  ry[0][0] 1.0
 28 load  ry[0][1] 0.0
 28 load  ry[0][2] 0.0
 28 load  ry[1][0] 0.0
 28 load  ry[1][1] 1.0
 28 load  ry[1][2] 0.0
 28 load  ry[2][0] 0.0
 28 load  ry[2][1] 0.0
 28 load  ry[2][2] 1.0
 30 load  %8[0] cy
 30 load  %8[1] ry[0][1]
 30 load  %8[2] ry[0][2]
 30 load  %9 sy
 30 neg   %9
 30 load  %8[2] %9
 31 load  %10[0] sy
 31 load  %10[1] ry[2][1]
 31 load  %10[2] cy
 34 mul   %11 2.0 p
 34 sin   %12 %11
 34 load  sz %12
 35 cos   %13 %11
 35 load  cz %13
 37 load  rz[0][0] 1.0
 37 load  rz[0][1] 0.0
 37 load  rz[0][2] 0.0
 37 load  rz[1][0] 0.0
 37 load  rz[1][1] 1.0
 37 load  rz[1][2] 0.0
 37 load  rz[2][0] 0.0
 37 load  rz[2][1] 0.0
 37 load  rz[2][2] 1.0
 39 load  %14[0] cz
 39 load  %14[1] rz[0][1]
 39 load  %14[2] rz[0][2]
 39 load  %15 sz
 39 neg   %15
 39 load  %14[1] %15
 40 load  %16[0] sz
 40 load  %16[1] cz
 40 load  %16[2] rz[1][2]
 43 mul   %17 p 3.0
 43 sin   %18 %17
 43 load  sx %18
 44 cos   %19 %17
 44 load  cx %19
 46 load  rx[0][0] 1.0
 46 load  rx[0][1] 0.0
 46 load  rx[0][2] 0.0
 46 load  rx[1][0] 0.0
 46 load  rx[1][1] 1.0
 46 load  rx[1][2] 0.0
 46 load  rx[2][0] 0.0
 46 load  rx[2][1] 0.0
 46 load  rx[2][2] 1.0
 48 load  %20[0] rx[1][0]
 48 load  %20[1] cx
 48 load  %20[2] rx[1][2]
 48 load  %21 sx
 48 neg   %21
 48 load  %20[2] %21
 49 load  %22[0] rx[2][0]
 49 load  %22[1] sx
 49 load  %22[2] cx
 52 mul   %23[0] rx[0] ry[0]
 52 mul   %23[1] rx[1] ry[1]
 52 mul   %23[2] rx[2] ry[2]
 52 mul   %23[0] %23[0] rz[0]
 52 mul   %23[1] %23[1] rz[1]
 52 mul   %23[2] %23[2] rz[2]
 52 mul   %23[0] vPosition[0]
 52 mul   %23[1] vPosition[1]
 52 mul   %23[2] vPosition[2]
 52 mul   %23[0] distance %23[0]
 52 mul   %23[1] distance %23[1]
 52 mul   %23[2] distance %23[2]
 52 load  pos[0] %23[0]
 52 load  pos[1] %23[1]
 52 load  pos[2] %23[2]
 54 load  %24 resolution[0]
 54 rec   %24
 54 mul   %24 resolution[1] %24
 54 mul   %24 pos[0] %24
 54 load  %25[0] %24
 54 load  %25[1] pos[1]
 54 load  %25[2] pos[2]
 54 load  %25[3] 0.0
 54 push  %25[0]
 54 push  %25[1]
 54 push  %25[2]
 54 push  %25[3]
 55 ret
