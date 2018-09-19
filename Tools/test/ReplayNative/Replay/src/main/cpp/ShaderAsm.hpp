#ifndef INCLUDE_ShaderAsm_hpp
#define INCLUDE_ShaderAsm_hpp

#include<cassert>
#include<sstream>
#include<vector>
#include<iostream>
#include<string>
#include<map>


 class AsmDict {
 protected:
  void init    (double c);
  void branches(double c);
  void hardmath(double c);
  void easymath(double c);
  void quick   (double c);
  void function(double c);
  void lppc    (double x, double y, double z, double w);

 public:
  AsmDict() {weight["label"] = 0.0;}

  std::map<std::string,double> weight;

  bool ok(std::string s) const {return weight.find(s)!=weight.end();}

  double operator[](std::string cmd) const {return weight.find(cmd)->second;}
 }; // AsmDict



 class ShaderAsmLine {
 public:
  ShaderAsmLine(AsmDict const& dict, int al, std::istream&);

  void display(double tot) const;

  int line,asmline; std::string cmd,args; double weight;

  bool ok() const {return cmd.size();}
 }; // ShaderAsmLine



 class ShaderAsm {
  int shaderId;

  double total;

  AsmDict const& dict;

  std::string src;

  std::map<int,std::vector<ShaderAsmLine>> lines;

  int* blob = nullptr;

  int blobsiz = 0;

 public:
  ShaderAsm(int id, AsmDict const& d, std::string f);

 ~ShaderAsm() {delete[] blob;}

  void display() const;

  std::string source() const {return src;}

  void* debug() {return blob;}

  int debug_size() const {return blobsiz;}
 }; // ShaderAsm



 class ShaderAsmSet {
  std::map<std::string,std::string> dat;

 protected:
  void insert(std::string k, std::string v) {dat[k]=v;}

 public:
  ShaderAsmSet() {}

  bool ok(std::string k) const {return dat.find(k) != dat.end();}

  std::string operator[](std::string k) const {return dat.find(k)->second;}

  std::string getasmfile(std::string src);
 }; // ShaderAsmSet



 class AsmDictOne: public AsmDict {
 public:
  AsmDictOne() {
   init    (0.7);
   branches(0.5);
   hardmath(0.2);
   easymath(0.5);
   quick   (0.2);
   function(1.6);

   lppc(1.3,0.9,0.8,0.6);
  }
 }; // AsmDictOne



 class AsmDictTwo: public AsmDict {
 public:
  AsmDictTwo() {
   init    (0.5);
   branches(0.8);
   hardmath(0.3);
   easymath(0.4);
   quick   (0.3);
   function(2.6);

   lppc(1.1,0.6,0.9,0.7);
  }
 }; // AsmDictTwo



 class ShaderAsmSetDemo: public ShaderAsmSet {
 public:
  ShaderAsmSetDemo() {
   insert("VERT_RAYMARCHING","raymarching.vert.asm");
   insert("FRAG_RAYMARCHING","raymarching.frag.asm");
   insert("VERT_OVERDRAW"   ,   "overdraw.vert.asm");
   insert("FRAG_OVERDRAW"   ,   "overdraw.frag.asm");
   insert("VERT_TEXOVERDRAW","overdrawTex.vert.asm");
   insert("FRAG_TEXOVERDRAW","overdrawTex.frag.asm");
   insert("VERT_MENGER"     ,     "menger.vert.asm");
   insert("FRAG_MENGER"     ,     "menger.frag.asm");
   insert("VERT_HYPERCUBE"  ,  "hypercube.vert.asm");
   insert("FRAG_HYPERCUBE"  ,  "hypercube.frag.asm");
   insert("VERT_TRIANGLE"   ,   "triangle.vert.asm");
   insert("FRAG_TRIANGLE"   ,   "triangle.frag.asm");
  }
 }; // ShaderAsmSetDemo

#endif
