  5 load  SHAPE_CUBE 1
  6 load  SHAPE_SPHERE 2
  7 load  SHAPE_OCTAHEDRON 3
 13 load  MAX_ITER 100
 14 load  MAX_DIST 100.0
 15 load  CAM_ITER 10.0
 17 load  EPSILON 0.001
 19 load  NX 5
 20 load  NY 5
 21 load  NZ 4
 23 load  separation[0] 2.8
 23 load  separation[1] 2.8
 23 load  separation[2] 2.8
 28 load  DISPLAY_DEPTH 10.0
 33 load  %1 NX
 33 dec   %1
 33 mul   %1 0.5 %1
 33 load  %2 NY
 33 dec   %2
 33 mul   %2 0.5 %2
 33 load  %3 NZ
 33 dec   %3
 33 mul   %3 0.5 %3
 33 load  centre[0] %1
 33 load  centre[1] %2
 33 load  centre[2] %3
 44 label $sphere
 44 pop   pos[2]
 44 pop   pos[1]
 44 pop   pos[0]
 44 pop   radius
 45 push  pos[0]
 45 push  pos[1]
 45 push  pos[2]
 45 call  builtin.$len
 45 pop   %4
 45 sub   %5 %4 radius
 45 push  %5
 45 ret
 49 label $box
 49 pop   pos[2]
 49 pop   pos[1]
 49 pop   pos[0]
 49 pop   siz[2]
 49 pop   siz[1]
 49 pop   siz[0]
 50 push  pos[0]
 50 push  pos[1]
 50 push  pos[2]
 50 call  builtin.$abs
 50 pop   %6[2]
 50 pop   %6[1]
 50 pop   %6[0]
 50 sub   %7[0] %6[0] siz
 50 sub   %7[1] %6[1] siz
 50 sub   %7[2] %6[2] siz
 50 max   %8[0] %7[0] 0.0
 50 max   %8[1] %7[1] 0.0
 50 max   %8[2] %7[2] 0.0
 50 push  %8[0]
 50 push  %8[1]
 50 push  %8[2]
 50 ret
 54 label $cube
 54 pop   pos[2]
 54 pop   pos[1]
 54 pop   pos[0]
 54 pop   siz
 55 abs   %6[0] pos[0]
 55 abs   %6[1] pos[1]
 55 abs   %6[2] pos[2]
 55 sub   %7[0] %6[0] siz
 55 sub   %7[1] %6[1] siz
 55 sub   %7[2] %6[2] siz
 55 max   %8[0] %7[0] 0.0
 55 max   %8[1] %7[1] 0.0
 55 max   %8[2] %7[2] 0.0
 55 push  %8[0]
 55 push  %8[1]
 55 push  %8[2]
 55 ret
 59 pop   pos[2]
 59 pop   pos[1]
 59 pop   pos[0]
 59 pop   siz
 60 load  %9[0] 1
 60 load  %9[1] 1
 60 load  %9[2] 1
 60 abs   %10[0] pos[0]
 60 abs   %10[1] pos[1]
 60 abs   %10[2] pos[2]
 60 push  %9[0]
 60 push  %9[1]
 60 push  %9[2]
 60 push  %10[0]
 60 push  %10[1]
 60 push  %10[2]
 60 call  builtin.$dot
 60 pop   %11
 60 sub   %11 %11 siz
 60 push  3.0
 60 call  builtin.$sqrt
 60 pop   %12
 60 rec   %12
 60 mul   %13 %11 %12
 60 push  %13
 65 label $distfunc{1}
 65 pop   origin[2]
 65 pop   origin[1]
 65 pop   origin[0]
 65 pop   dir[2]
 65 pop   dir[1]
 65 pop   dir[0]
 66 load  total 0.0
 67 load  ray[0] origin[0]
 67 load  ray[1] origin[1]
 67 load  ray[2] origin[2]
 68 load  dist EPSILON
 70 load  %14 0
 70 label $1
 71 cmp   dist EPSILON
 71 brlt  $2
 71 cmp   total MAX_DIST
 71 brgt  $2
 74 push  ray[0]
 74 push  ray[1]
 74 push  ray[2]
 74 push  1.0
 74 call  $cube
 74 pop   dist
 76 add   total total dist
 77 mul   %15[0] dist[0] dir[0]
 77 mul   %15[1] dist[1] dir[1]
 77 mul   %15[2] dist[2] dir[2]
 77 add   ray[0] ray[0] %15[0]
 77 add   ray[1] ray[1] %15[1]
 77 add   ray[2] ray[2] %15[2]
 78 inc   %14
 78 cmp   %14 MAX_ITER
 78 brlt  $1
 70 push  total
 70 ret
 85 label $distfunc{2}
 85 pop   origin[2]
 85 pop   origin[1]
 85 pop   origin[0]
 85 pop   dir[2]
 85 pop   dir[1]
 85 pop   dir[0]
 85 pop   shape
 86 load  total 0.0
 87 load  ray[0] origin[0]
 87 load  ray[1] origin[1]
 87 load  ray[2] origin[2]
 88 load  dist EPSILON
 90 load  %16 0
 91 label $3
 92 cmp   dist EPSILON
 92 brlt  $4
 92 cmp   total MAX_DIST
 92 brlt  $4
 94 cmp   shape SHAPE_SPHERE
 94 brne  $5
 94 push  ray[0]
 94 push  ray[1]
 94 push  ray[2]
 94 push  1.0
 94 call  $sphere
 94 pop   dist
 94 bra   $8
 95 label $5
 95 cmp   shape SHAPE_CUBE
 95 brne  $6
 95 push  ray[0]
 95 push  ray[1]
 95 push  ray[2]
 95 push  1.0
 95 call  $cube
 95 pop   dist
 95 bra   $8
 96 label $6
 96 cmp   shape SHAPE_OCTAHEDRON
 96 brne  $8
 97 push  ray[0]
 97 push  ray[1]
 97 push  ray[2]
 97 push  1.0
 97 call  $octahedron
 97 pop   dist
 98 label $8
 98 add   total total dist
 99 mul   %17[0] dist dir[0]
 99 mul   %17[1] dist dir[1]
 99 mul   %17[2] dist dir[2]
 99 add   ray[0] ray[0] %17[0]
 99 add   ray[1] ray[1] %17[1]
 99 add   ray[2] ray[2] %17[2]
100 inc   %16
100 cmp   MAX_ITER
100 brlt  $3
102 label $3
102 push  total
102 ret
108 load  resolution[0] xres
108 load  resolution[1] yres
110 load  %18 resolution[1]
110 rec   %18
110 mul   %19[0] glFragCoord[0] %18
110 mul   %19[1] glFragCoord[1] %18
110 load  pos[0] %19[0]
110 load  pos[1] %19[1]
112 sin   %20 phase
112 mul   %21 2.0 phase
112 cos   %22 %21
112 mul   %23 2.0 phase
112 sin   %24 %23
112 load  %25[0] %20
112 load  %25[1] %22
112 load  %25[2] %24
112 push  %25[0]
112 push  %25[1]
112 push  %25[2]
112 call  builtin.$norm
112 pop   %26[2]
112 pop   %26[1]
112 pop   %26[0]
112 mul   %27[0] CAM_DIST %26[0]
112 mul   %27[1] CAM_DIST %26[1]
112 mul   %27[2] CAM_DIST %26[2]
112 load  cameraOrigin[0] %27[0]
112 load  cameraOrigin[1] %27[1]
112 load  cameraOrigin[2] %27[2]
113 load  cameraTarget[0] 0.0
113 load  cameraTarget[1] 0.0
113 load  cameraTarget[2] 0.0
114 load  upDirection[0] 0.0
114 load  upDirection[1] 1.0
114 load  upDirection[2] 0.0
116 sub   %28[0] cameraTarget[0] cameraOrigin[0]
116 sub   %28[1] cameraTarget[1] cameraOrigin[1]
116 sub   %28[2] cameraTarget[2] cameraOrigin[2]
116 push  %28[0]
116 push  %28[1]
116 push  %28[2]
116 call  builtin.$norm
116 pop   %29[2]
116 pop   %29[1]
116 pop   %29[0]
116 load  cameraDir[0] %29[0]
116 load  cameraDir[1] %29[1]
116 load  cameraDir[2] %29[2]
117 push  upDirection[0]
117 push  upDirection[1]
117 push  upDirection[2]
117 push  camerOrigin[0]
117 push  camerOrigin[1]
117 push  camerOrigin[2]
117 call  $builtin.cross
117 pop   cameraUp[2]
117 pop   cameraUp[1]
117 pop   cameraUp[0]
120 mul   %30 2.0 pos[0]
120 mul   %30 2.0 pos[1]
120 dec   %30[0]
120 dec   %30[1]
120 load  screenPos[0] %30[0]
120 load  screenPos[1] %30[1]
122 load  %31 resolution[1]
122 rec   %31
122 mul   %32 resolution[0] %31
122 mul   %33 screenPos[0] %32
122 load  screenPos[0] %33
124 mul   %34[0] cameraRight[0] screenPos[0]
124 mul   %34[1] cameraRight[1] screenPos[0]
124 mul   %34[2] cameraRight[2] screenPos[0]
124 mul   %35[0] cameraUp[0] screenPos[1]
124 mul   %35[1] cameraUp[1] screenPos[1]
124 mul   %35[2] cameraUp[2] screenPos[1]
124 add   %36[0] %34[0] %35[0]
124 add   %36[1] %34[1] %35[1]
124 add   %36[2] %34[2] %35[2]
124 add   %37[0] %36[0] cameraDir[0]
124 add   %37[1] %36[1] cameraDir[1]
124 add   %37[2] %36[2] cameraDir[2]
124 push  %37[0]
124 push  %37[1]
124 push  %37[2]
124 call  builtin.$norm
124 pop   %38[2]
124 pop   %38[1]
124 pop   %38[0]
124 load  rayDir[0] %38[0]
124 load  rayDir[1] %38[1]
124 load  rayDir[2] %38[2]
126 load  dist MAX_DIST
128 load  shape 1
130 load  %39 0
130 label $9
131 load  %40 0
131 label $10
132 load  %41 0
132 label $11
133 load  %42[0] %39
133 load  %42[1] %40
133 load  %42[2] %41
133 sub   %43[0] %42[0] centre[0]
133 sub   %43[1] %42[1] centre[1]
133 sub   %43[2] %42[2] centre[2]
133 mul   %44[0] separation[0] %43[0]
133 mul   %44[1] separation[1] %43[1]
133 mul   %44[2] separation[2] %43[2]
133 add   %45[0] cameraOrigin[0] %44[0]
133 add   %45[1] cameraOrigin[1] %44[1]
133 add   %45[2] cameraOrigin[2] %44[2]
133 load  rayOrigin[0] %45[0]
133 load  rayOrigin[1] %45[1]
133 load  rayOrigin[2] %45[2]
134 push  rayOrigin[0]
134 push  rayOrigin[1]
134 push  rayOrigin[2]
134 push  rayDir[0]
134 push  rayDir[1]
134 push  rayDir[2]
134 push  shape
134 call  $distfunc{2}
134 pop   dist
137 inc   shape
138 cmp   shape 3
138 brle  $12
138 load  shape 1
139 label $12
139 inc   %41
130 cmp   %41 NZ
130 brlt  $11
130 inc   %40
130 cmp   %40 NY
130 brlt  $10
130 inc   %39
130 cmp   %39
130 brlt  $9
141 cmp   dist MAX_DIST
141 brlt  $13
143 push  0.5
143 push  0.0
143 push  0.0
143 push  1.0
144 bra   $14
144 label $13
146 add   %42 dist DISPLAY_DEPTH
146 load  %43 CAM_DIST
146 neg   %43
146 add   %42 %42 %43
146 load  %44 DISPLAY_DEPTH
146 rec   %44
146 mul   %45 %43 %44
146 mul   %46 0.5 %45
146 neg   %46
146 inc   %46
146 load  shade %46
146 push  shade
146 push  shade
146 push  1.0
146 push  1.0
149 label $14
149 ret
