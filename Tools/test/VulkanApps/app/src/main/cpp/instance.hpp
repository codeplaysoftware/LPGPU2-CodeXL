/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_instance_hpp
#define INCLUDE_instance_hpp

#include <util_init.hpp>

// This class helps to manage the lifetime of the apps, basically it ensures the
// destroy_* functions are called after the apps have gone out of scope

class Instance {
public:
 sample_info& info;

 Instance(struct sample_info& c, bool depthPresent):info(c) {
  info.depthPresent = depthPresent;

  init_global_layer_properties(info);
  init_instance_extension_names(info);
  init_device_extension_names(info);
  init_instance(info, "LPGPU2 Vulkan Apps");
  init_enumerate_device(info);
  init_window_size(info, 500, 500);
  init_connection(info);
  init_window(info);
  init_swapchain_extension(info);
  init_device(info);
  init_command_pool(info);
  init_command_buffer(info);
  init_device_queue(info);
  init_swap_chain(info);
  init_depth_buffer(info);

  execute_begin_command_buffer(info);

  init_renderpass(info, info.depthPresent);
  init_pipeline_cache(info);
  init_framebuffers(info, info.depthPresent);
 }

~Instance() {
  destroy_pipeline_cache(info);
  destroy_textures(info);
//destroy_vertex_buffer(info);
  destroy_framebuffers(info);
  destroy_renderpass(info);
  destroy_depth_buffer(info);
  destroy_swap_chain(info);
  destroy_command_buffer(info);
  destroy_command_pool(info);
  destroy_device(info);
  destroy_window(info);
  destroy_instance(info);
 }

 int   getXRes  () {return info.width ;}
 int   getYRes  () {return info.height;}

 float getAspect() {return static_cast<float>(getXRes())
                         / static_cast<float>(getYRes());}
}; // Instance

#endif
