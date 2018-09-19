/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_App_hpp
#define INCLUDE_apps_App_hpp



#include "../instance.hpp"

#include "Buffers.hpp"



// Handy Model View Project matrix class...
class MVP {
public:
 // Model matrix...
 glm::mat4 Model = glm::mat4(1.0f);

 // Vulkan clip space has inverted Y and half Z...
 glm::mat4 Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

 // Orthogonal Projection with depth between 0.1 and 100...
 glm::mat4 Proj = glm::ortho(-1.0f,1.0f,-1.0f,1.0f,0.1f,100.0f);

 // Observer Position and Orientation...
 glm::mat4 View = glm::lookAt(glm::vec3(0, 0, -10),  // Camera location
                              glm::vec3(0, 0,   0),  // Camera focus
                              glm::vec3(0, 1,   0)); // Up vector

 // MVP matrix...
 operator glm::mat4() const {return Clip * Proj * View * Model;}
}; // MVP



class App {
public:
 Instance& instance;

 sample_info& info;

 size_t num;

 virtual void update(std::size_t it) {}
 virtual void draw  (std::size_t it) {}

 virtual void render(std::size_t it) {update(it); draw(it);}

 void renderall() {for(std::size_t it=0;it<num;it++) render(it);}

protected:
 App(Instance& instance, int n = 1):instance(instance),info(instance.info),num(n) {}

public:
 float getAspect() {return instance.getAspect();}

 int   getXRes  () {return instance.getXRes  ();}
 int   getYRes  () {return instance.getYRes  ();}

 std::size_t repeats() const {return num;}

 virtual bool escape() const {return false;}
}; // App



class DrawApp: public App {
 virtual void update_uniforms(std::size_t it) {}

protected:
 std::string vertShader,fragShader;

 template<typename VU, typename FU>
 DrawApp(Instance& instance, VU& vu, FU& fu, std::string vsfile, std::string fsfile, int binding)
  :App(instance),vud(&vu),fud(&fu),vus(sizeof(vu)),fus(sizeof(fu))
 {
  vertShader = load_shader_source(vsfile);
  fragShader = load_shader_source(fsfile,binding); // binding for texture, if present
 }

~DrawApp();

public:
 const char* getVertShader()  {return vertShader.c_str();}
 const char* getFragShader()  {return fragShader.c_str();}

 VkVertexInputBindingDescription vi_binding;
 VkVertexInputAttributeDescription vi_attribs[2];

 VkPipelineLayout pipeline_layout;
 std::vector<VkDescriptorSetLayout> desc_layout;

 VkPipeline pipeline;
 VkPipelineShaderStageCreateInfo shaderStages[2];
 VkDescriptorPool desc_pool;
 std::vector<VkDescriptorSet> desc_set;

 UniformData vert_uniform_data;
 UniformData frag_uniform_data;

 void*  vud;
 void*  fud;

 size_t vus;
 size_t fus;

 Textures texture_data;

 vertex_buffer_t vertex_buffer;

 size_t vertex_count;

 void update(std::size_t it);

 void init_vertices(Vertices& vertices);

 void init_vertices(size_t vcount, void* data, size_t total, size_t stride);
}; // DrawApp



class DirectDrawApp: public DrawApp {
protected:
 template<typename VU, typename FU>
 DirectDrawApp(Instance& c, VU& vuni, FU& funi, std::string vsfile, std::string fsfile, int binding = -1):DrawApp(c,vuni,funi,vsfile,fsfile,binding) {}

 void draw(std::size_t it);
}; // DirectDrawApp



class IndirectDrawApp: public DrawApp {
 index_buffer_t index_buffer;

protected:
 template<typename VU, typename FU>
 IndirectDrawApp(Instance& c, VU& vuni, FU& funi, std::string vsfile, std::string fsfile, int binding = -1):DrawApp(c,vuni,funi,vsfile,fsfile,binding) {}

~IndirectDrawApp();

 void draw(std::size_t it);

 void init_indices(Indices& indices);

 void init_indices(size_t count, void* triangle_indices, size_t indexBufferSize);

 uint32_t getMemoryTypeIndex(struct sample_info& info, uint32_t typeBits, VkMemoryPropertyFlags properties);
}; // IndirectDrawApp



#endif
