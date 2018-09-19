/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Hypercube: public AppRenderer {

 std::vector<float> vtx,vtx2;

 float* gfx_vtx() {return vtx.data();}

 void gfx_rotate(double& v1, double& v2, double& v3, double& v4, double a, int i, int j) {
  double s = std::sin(a);
  double c = std::cos(a);

  double v[] {v1,v2,v3,v4};

  double x = v[j-1]*s+v[i-1]*c;
  double y = v[j-1]*c-v[i-1]*s;

  v[i-1]=x;
  v[j-1]=y;

  v1=v[1-1];
  v2=v[2-1];
  v3=v[3-1];
  v4=v[4-1];
 }

 int gfx_dim  () {return 4;}
 int gfx_count() {return vtx.size()/gfx_dim();}

 void gfx_set(int i, double v1, double v2, double v3, double v4) {
  if (vtx.size()<gfx_dim()*i) vtx.resize(gfx_dim()*i);

  auto n=(i-1)*gfx_dim();

  vtx[n+1-1]=v1;
  vtx[n+2-1]=v2;
  vtx[n+3-1]=v3;
  vtx[n+4-1]=v4;
 }

 void gfx_get(int i, double& v1, double& v2, double& v3, double& v4) {
  int d = gfx_dim();

  v1 = vtx2[d*(i-1)+1-1];
  v2 = vtx2[d*(i-1)+2-1];
  v3 = vtx2[d*(i-1)+3-1];
  v4 = vtx2[d*(i-1)+4-1];
 }

 void gfx_new(double v1, double v2, double v3, double v4) {
  int n = vtx.size();

  vtx.resize(n+gfx_dim());

  vtx[n+1-1]=v1;
  vtx[n+2-1]=v2;
  vtx[n+3-1]=v3;
  vtx[n+4-1]=v4;
 }

 GLuint vShader;
 GLuint fShader;

 GLuint sProgram;

 GLuint screenXLocation;
 GLuint screenYLocation;

 int ScreenXRes;
 int ScreenYRes;

 double delta;
 double scale;

 int iframe;


 void quad(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4, double su, double sv)
 {
  double t1,t2,t3,t4;

  t1=p1            ;
  t2=p2            ;
  t3=p3            ;
  t4=p4            ; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1      ;
  t2=p2+su*u2      ;
  t3=p3+su*u3      ;
  t4=p4+su*u4      ; gfx_new(t1,t2,t3,t4);

  t4=p4      +sv*v4;
  t3=p3      +sv*v3;
  t2=p2      +sv*v2;
  t1=p1      +sv*v1; gfx_new(t1,t2,t3,t4);


  t1=p1      +sv*v1;
  t2=p2      +sv*v2;
  t3=p3      +sv*v3;
  t4=p4      +sv*v4; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1      ;
  t2=p2+su*u2      ;
  t3=p3+su*u3      ;
  t4=p4+su*u4      ; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1+sv*v1;
  t2=p2+su*u2+sv*v2;
  t3=p3+su*u3+sv*v3;
  t4=p4+su*u4+sv*v4; gfx_new(t1,t2,t3,t4);
 }



 void cube(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4,
  double w1, double w2, double w3, double w4, double su, double sv, double sw)
 {
  double q1,q2,q3,q4;

  q1=p1;
  q2=p2;
  q3=p3;
  q4=p4;

  quad(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            , su, sv    );
  quad(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4, su    , sw);
  quad(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4    , sv, sw);

  q1+=su*u1+sv*v1+sw*w1;
  q2+=su*u2+sv*v2+sw*w2;
  q3+=su*u3+sv*v3+sw*w3;
  q4+=su*u4+sv*v4+sw*w4;

  quad(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,-su,-sv    );
  quad(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,-su    ,-sw);
  quad(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4    ,-sv,-sw);
}


 void hypercube(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4,
  double w1, double w2, double w3, double w4,
  double t1, double t2, double t3, double t4, double su, double sv, double sw, double st)
 {
  double q1,q2,q3,q4;

  q1=p1-su*u1/2-sv*v1/2-sw*w1/2-st*t1/2;
  q2=p2-su*u2/2-sv*v2/2-sw*w2/2-st*t2/2;
  q3=p3-su*u3/2-sv*v3/2-sw*w3/2-st*t3/2;
  q4=p4-su*u4/2-sv*v4/2-sw*w4/2-st*t4/2;

  cube(q1,q2,q3,q4,u1,u2,u4,u4,v1,v2,v3,v4,w1,w2,w3,w4            , su, sv, sw    );
  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,t1,t2,t3,t4, su, sv    , st);
  cube(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,t1,t2,t3,t4, su    , sw, st);
  cube(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4,t1,t2,t3,t4    , sv, sw, st);

  q1=p1+su*u1/2+sv*v1/2+sw*w1/2+st*t1/2;
  q2=p2+su*u2/2+sv*v2/2+sw*w2/2+st*t2/2;
  q3=p3+su*u3/2+sv*v3/2+sw*w3/2+st*t3/2;
  q4=p4+su*u4/2+sv*v4/2+sw*w4/2+st*t4/2;

  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4,w1,w2,w3,w4            ,-su,-sv,-sw    );
  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,t1,t2,t3,t4,-su,-sv    ,-st);
  cube(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,t1,t2,t3,t4,-su    ,-sw,-st);
  cube(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4,t1,t2,t3,t4    ,-sv,-sw,-st);
 }


 void hypercubes(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4,
  double w1, double w2, double w3, double w4,
  double t1, double t2, double t3, double t4, double su, double sv, double sw, double st)
 {
  double q1,q2,q3,q4;

  for(int i=1;i<=2;i++)
  for(int j=1;j<=2;j++)
  for(int k=1;k<=2;k++) {
   q1=p1+0.5*(i-0.5);
   q2=p2+0.5*(j-0.5);
   q3=p3+0.5*(k-0.5);
   q4=p4;

   hypercube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4,w1,w2,w3,w4,t1,t2,t3,t4,su,sv,sw,st);
  }
 }

public:
 void ReplayInit() {

  const char* vSource =
   "#version 100\n"
   "attribute vec4 vPosition;\n"
   "attribute vec3 vColor;\n"
   "\n"
   "varying float col;\n"
   "\n"
   "uniform int xres;    // screen X resolution\n"
   "uniform int yres;    // screen Y resolution\n"
   "\n"
   "void main() { // VERT_HYPERCUBE\n"
   "\n"
   " vec2 resolution = vec2(xres+1,yres+1);\n"
   "\n"
   " col = vPosition.w;\n"
   " vec3 p = vPosition.xyz/(2.0+vPosition.w);\n"
   " gl_Position = vec4(p.x*resolution.y/resolution.x,p.yz,1.0);\n"
// " gl_Position = vec4(vPosition.xyz/(2.0+vPosition.w),1.0);"
   "}\n";

  const char* fSource =
   "precision mediump float;\n"
   "\n"
   "varying float col;\n"
   "\n"
   "void main() { // FRAG_HYPERCUBE\n"
   " float c = 1.0 - gl_FragCoord.z;\n"
   " gl_FragColor = vec4(c, col, 1.0-col, 0.5);\n"
   "}\n";


  vtx.clear();


  scale = 1.4;


  double offset[] {0.0,0.0,0.0,0.0};

  double origin[] {0.0,0.0,0.0,0.0};

  double u_unit[] {1.0,0.0,0.0,0.0};
  double v_unit[] {0.0,1.0,0.0,0.0};
  double w_unit[] {0.0,0.0,1.0,0.0};
  double t_unit[] {0.0,0.0,0.0,1.0};

// quad      ("geom",origin,u_unit,v_unit              ,scale,scale            )
// cube      ("geom",origin,u_unit,v_unit,w_unit       ,scale,scale,scale      )
   hypercube (
    origin[0],origin[1],origin[2],origin[3],
    u_unit[0],u_unit[1],u_unit[2],u_unit[3],
    v_unit[0],v_unit[1],v_unit[2],v_unit[3],
    w_unit[0],w_unit[1],w_unit[2],w_unit[3],
    t_unit[0],t_unit[1],t_unit[2],t_unit[3],scale,scale,scale,scale);

   vtx2 = vtx;

// hypercubes("geom",origin,u_unit,v_unit,w_unit,t_unit,scale,scale,scale,scale)


  iframe = 0;


  vShader = LoadShader(GL_VERTEX_SHADER  , vSource);
  fShader = LoadShader(GL_FRAGMENT_SHADER, fSource);

  sProgram = glCreateProgram();


  glAttachShader(sProgram, vShader);
  glAttachShader(sProgram, fShader);

  glBindAttribLocation(sProgram, 0, "vPosition");

  glLinkProgram(sProgram);

  // Screen Resolution
  screenXLocation = glGetUniformLocation(sProgram,"xres");
  screenYLocation = glGetUniformLocation(sProgram,"yres");

  ScreenXRes = GetScreenWidth ();
  ScreenYRes = GetScreenHeight();



  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
 }

 void ReplayRender(double tim) {
  glClearColor(0.75,0.75,0.75,1.0);

  iframe++;

  for(int i=1;i<=gfx_count();i++) {
   double v1,v2,v3,v4;

   gfx_get(i,v1,v2,v3,v4);

   double a1 = tim * 1.4;
   double a2 = tim * 0.8;
   double a3 = tim * 1.7;
   double a4 = tim * 2.5;

   gfx_rotate(v1,v2,v3,v4,a1,1,2);
   gfx_rotate(v1,v2,v3,v4,a2,2,3);
   gfx_rotate(v1,v2,v3,v4,a3,3,1);
   gfx_rotate(v1,v2,v3,v4,a4,1,4);

   gfx_set(i,v1,v2,v3,v4);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  glUseProgram(sProgram);

  // Set Screen Resolution uniforms
  glUniform1i(screenXLocation,ScreenXRes);
  glUniform1i(screenYLocation,ScreenYRes);

  glVertexAttribPointer( 0, gfx_dim(), GL_FLOAT, GL_FALSE, 4*gfx_dim(), gfx_vtx());

  glEnableVertexAttribArray(0);


  glDrawArrays(GL_TRIANGLES, 0, gfx_count());

//glFinish();
 }
}; // Hypercube
