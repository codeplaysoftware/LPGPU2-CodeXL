  4 uni   opacity
  6 uni   xres
  7 uni   yres
  9 var   texcoord[0]
  9 var   texcoord[1]
 11 uni   ourTexture
 47 push  ourTexture
 47 push  texcoord[0]
 47 push  texcoord[1]
 47 call  builtin.$texture2D
 47 pop   %1[2]
 47 pop   %1[1]
 47 pop   %1[0]
 47 load  col[0] %1[0]
 47 load  col[1] %1[1]
 47 load  col[2] %1[2]
 49 push  col[0]
 49 push  col[1]
 49 push  col[2]
 49 push  opacity
 50 ret
