/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "App.hpp"
#include <string.h>
#include <stdlib.h>



 DrawApp::~DrawApp() {
  destroy_pipeline       (info,pipeline);
  destroy_shaders        (info,shaderStages);
  destroy_descriptor_pool(info,desc_pool);
  destroy_descriptor_and_pipeline_layouts(info,desc_layout,pipeline_layout);
  destroy_uniform_buffer (info,vert_uniform_data,frag_uniform_data);
  destroy_vertex_buffer  (info,vertex_buffer);
 }



 void DrawApp::update(std::size_t it) {
  update_uniforms(it);

  vert_uniform_data.update(info,vud);
  frag_uniform_data.update(info,fud);

  vkCmdBindPipeline       (info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets (info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, NUM_DESCRIPTOR_SETS, desc_set.data(), 0, NULL);
 }



 void DrawApp::init_vertices(Vertices& vertices) {
  init_vertices(
   vertices.size()/vertices.stride,
   (void*)vertices.data(),
   vertices.size()*sizeof(vertices[0]),
   vertices.stride*sizeof(vertices[0])
  );
 }



 void DrawApp::init_vertices(size_t vcount, void* data, size_t total, size_t stride) {
  vertex_count = vcount;

  init_uniform_buffer(info,vud,vus,fud,fus,vert_uniform_data,frag_uniform_data);

  init_descriptor_and_pipeline_layouts(info,desc_layout,pipeline_layout,vert_uniform_data,frag_uniform_data,texture_data);

  init_vertex_buffer(info, data, total, stride, vi_binding, vi_attribs, vertex_buffer, texture_data.textures.size());

  init_descriptor_pool(info, desc_pool, texture_data);
  init_descriptor_set (info, desc_pool, desc_set, desc_layout, vert_uniform_data, frag_uniform_data, texture_data);

  init_shaders (info, shaderStages, getVertShader(), getFragShader());
  init_pipeline(info, pipeline, shaderStages, pipeline_layout, vi_binding, vi_attribs, info.depthPresent);
 }



 void DirectDrawApp::draw(std::size_t it) {
  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(info.cmd, 0, 1, &vertex_buffer.buf, offsets);
  vkCmdDraw(info.cmd, vertex_count, 1, 0, 0);
 }



 IndirectDrawApp::~IndirectDrawApp() {
  vkDestroyBuffer(info.device,index_buffer.buffer,nullptr);
  vkFreeMemory   (info.device,index_buffer.memory,nullptr);
 }



 void IndirectDrawApp::draw(std::size_t it) {
  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(info.cmd, 0, 1, &vertex_buffer.buf, offsets);
  vkCmdBindIndexBuffer(info.cmd, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(info.cmd, index_buffer.count, 1, 0, 0, 1);
 }



 void IndirectDrawApp::init_indices(Indices& indices) {
  init_indices(indices.size(),(void*)indices.data(),indices.size()*sizeof(indices[0]));
 }



 void IndirectDrawApp::init_indices(size_t count, void* triangle_indices, size_t indexBufferSize) {
  index_buffer.count = count;

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  VkMemoryRequirements memReqs;

  VkBufferCreateInfo indexbufferInfo = {};
  indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  indexbufferInfo.size = indexBufferSize;
  indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VkResult res;

  // Copy index data to a buffer visible to the host
  res = vkCreateBuffer(info.device, &indexbufferInfo, nullptr, &index_buffer.buffer);
  vkGetBufferMemoryRequirements(info.device, index_buffer.buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex = getMemoryTypeIndex(info,memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  res = vkAllocateMemory(info.device, &memAlloc, nullptr, &index_buffer.memory);

  void* data;
  res = vkMapMemory(info.device, index_buffer.memory, 0, indexBufferSize, 0, &data);
  memcpy(data, triangle_indices, indexBufferSize);
  vkUnmapMemory(info.device, index_buffer.memory);
  res = vkBindBufferMemory(info.device, index_buffer.buffer, index_buffer.memory, 0);
 }



 uint32_t IndirectDrawApp::getMemoryTypeIndex(struct sample_info& info, uint32_t typeBits, VkMemoryPropertyFlags properties) {
  // Iterate over all memory types available for the device used in this example
  for (uint32_t i = 0; i < info.memory_properties.memoryTypeCount; i++) {
   if ((typeBits & 1) == 1) {
    if ((info.memory_properties.memoryTypes[i].propertyFlags & properties) == properties) return i;
   }
   typeBits >>= 1;
  }

  throw "Could not find a suitable memory type!";
 }
