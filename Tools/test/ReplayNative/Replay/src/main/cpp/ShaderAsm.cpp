#include "ShaderAsm.hpp"

#include <cstdlib>
#include <JNIHelper.h>


 void AsmDict::init(double c) {
   weight["attr" ] = c;
   weight["var"  ] = c;
   weight["uni"  ] = c;
  }

  void AsmDict::branches(double c) {
   weight["brlt" ] = c;
   weight["brle" ] = c;
   weight["brgt" ] = c;
   weight["brge" ] = c;
   weight["brne" ] = c;
   weight["bra"  ] = c;
  }

  void AsmDict::hardmath(double c) {
   weight["neg"  ] =   c;
   weight["rec"  ] = 3*c;
   weight["sin"  ] = 5*c;
   weight["cos"  ] = 5*c;
   weight["sqrt" ] = 3*c;
   weight["norm" ] = 8*c;
  }

  void AsmDict::easymath(double c) {
   weight["mul"  ] = c;
   weight["add"  ] = c;
   weight["sub"  ] = c;
   weight["max"  ] = c+0.2;
   weight["abs"  ] = c+0.1;
  }

  void AsmDict::quick(double c) {
   weight["inc"  ] = c;
   weight["dec"  ] = c;
  }

  void AsmDict::function(double c) {
   weight["call" ] = c;
   weight["dscd" ] = c;
   weight["ret"  ] = c;
  }

  void AsmDict::lppc(double x, double y, double z, double w) {
   weight["load" ] = x;
   weight["push" ] = y;
   weight["pop"  ] = z;
   weight["cmp"  ] = w;
  }



  ShaderAsmLine::ShaderAsmLine(AsmDict const& dict, int al, std::istream& s) {
   s >> line >> cmd; std::getline(s,args); asmline=al;

   while(args.size()&&args[0]==' ') args=args.substr(1);

   weight = dict[cmd];
  }

  void ShaderAsmLine::display(double tot) const {
// std::cout << line    << "\t";
   std::cout << asmline << "\t";
   printf("%5.2f%%\t",100*weight/tot);
   std::cout << cmd     << "\t";
   std::cout << args    << "\n";
  }



  std::string strfromfile(std::string f) {
   std::vector<std::uint8_t> data;

   if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(f.c_str(), &data)) {
    LOGI("LPGPU2: Can not open a file:%s", f.c_str());
   }

   std::string datastr; datastr.resize(data.size());

   for(auto i=0;i<data.size();i++) datastr[i]=(char)data[i];

   return datastr;
  }



#if 0
  class dbShaderSrc {
  public:
   dbShaderSrc(                  ):asmStart(     ),asmEnd(   ) {}
   dbShaderSrc(int start, int end):asmStart(start),asmEnd(end) {}

   int asmStart;
   int asmEnd;
  }; // dbShaderAsm



  class dbShaderAsm {
  public:
   dbShaderAsm(double weight = 0):percentage(100*weight) {}

   int percentage;
  }; // dbShaderAsm
#endif



  ShaderAsm::ShaderAsm(int id, AsmDict const& d, std::string f):shaderId(id),dict(d) {
   int asmline = 1;

   {
    std::istringstream str(strfromfile(f));

    LOGI("LPGPU2: total:%s",f.c_str());

    while(!str.eof()) {
     ShaderAsmLine line(dict,asmline,str);

     if (line.ok()) {
      asmline++;

      if (!dict.ok(line.cmd)) assert(0);

      lines[line.line].push_back(line);
     } else
      asmline--;
    }

    total = 0.0;

    for(auto const& line:lines) for(auto const& cmd:line.second) total+=cmd.weight;

    LOGI("LPGPU2: total:%g", total);
   }

   {
    std::istringstream str(strfromfile(f));

    while(!str.eof()) {
     std::string s; str >> s; if (!s.size()) break;

     std::getline(str,s); while(s.size()&&s[0]==' ') s=s.substr(1);

     src += s + "\n";
    }
   }

   blob = new int[1+3*lines.size()+1*asmline];

   LOGI("LPGPU2: src lines: %d",(int)lines.size());
   LOGI("LPGPU2: asm lines: %d",asmline);

   int* cur = blob;

   *cur++ = (int)lines.size();

   for(auto const& line:lines) {
    auto const& cmds = line.second;

    cur[0] = line.first;
    cur[1] = cmds.front().asmline;
    cur[2] = cmds.back ().asmline;

    LOGI("LPGPU2: data: %d\t%d\t%d",cur[0],cur[1],cur[2]);

    cur+=3;
   }

   *cur++ = asmline;

   for(auto const& line:lines) for(auto const& cmd:line.second) *cur++ = (int) (10000 * cmd.weight / total);

   blobsiz = sizeof(int) * (cur-blob);

   LOGI("LPGPU2: diff: %d",blobsiz);
  }



  void ShaderAsm::display() const {
   for(auto const& line:lines) {
    std::cout << "Shader line:" << line.first << ", total = " << total << "\n";
    for(auto const& instr:line.second) {
     instr.display(total);
    }
    std::cout << "\n";
   }
  }



  std::string ShaderAsmSet::getasmfile(std::string s) {
   for(auto const& shader:dat) if (s.find(shader.first)!=std::string::npos) return shader.second;

   return "";
  }
