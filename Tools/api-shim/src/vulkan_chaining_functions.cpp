/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "vulkan/vulkan.h"
#include "vulkan/vk_layer.h"

#include <cstring> // memset

#include "unordered_map"

#include "generated.h"
#include "logger.h"
#include "export.h"
#include "api_id.h"

#include "vulkan_dispatch.h"

// #include "vk_layer_table.h"
// #include "vk_layer_data.h" vkGetPhysicalDeviceQueueFamilyProperties
// #include "vk_layer_logging.h"
// #include "vk_layer_extension_utils.h"
// #include "vk_layer_utils.h"

extern void EnableVulkanDebugCallback(VkInstance instance);
extern void DestroyVulkanDebugCallback(VkInstance instance);

namespace {

VkResult vulkanError(VkResult error)
{
  Log(LogError, "Vulkan Error: %d", error);
  return error;
}

VkLayerInstanceCreateInfo* FindLayerInstanceCreateInfo(const VkInstanceCreateInfo *pCreateInfo, const VkLayerFunction func)
{
  VkLayerInstanceCreateInfo *info = NULL;

  if(pCreateInfo)
  {
    info = reinterpret_cast<VkLayerInstanceCreateInfo *>(const_cast<void*>(pCreateInfo->pNext));
    if(info)
    {
      while (!(info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && info->function == func))
      {
        info = (VkLayerInstanceCreateInfo *)info->pNext;
      }
    }
  }

  return info;
}

VkLayerDeviceCreateInfo* FindDeviceCreateInfo(const VkDeviceCreateInfo *pCreateInfo, const VkLayerFunction func)
{
  VkLayerDeviceCreateInfo *info = NULL;

  if(pCreateInfo)
  {
    info = reinterpret_cast<VkLayerDeviceCreateInfo *>( const_cast<void*>(pCreateInfo->pNext));
    if(info)
    {
      while (!(info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && info->function == func))
      {
        info = (VkLayerDeviceCreateInfo *)info->pNext;
      }
    }
  }

  return info;
}

void LayerInitInstanceDispatchTable(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa)
{
  std::memset(table, 0, sizeof(*table));
  // Instance function pointers
  table->DestroyInstance = (PFN_vkDestroyInstance) gpa(instance, "vkDestroyInstance");
  table->EnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) gpa(instance, "vkEnumeratePhysicalDevices");
  table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) gpa(instance, "vkGetPhysicalDeviceFeatures");
  table->GetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) gpa(instance, "vkGetPhysicalDeviceFormatProperties");
  table->GetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
  table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) gpa(instance, "vkGetPhysicalDeviceProperties");
  table->GetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
  table->GetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
  table->GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) gpa(instance, "vkGetInstanceProcAddr");
  table->EnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) gpa(instance, "vkEnumerateDeviceExtensionProperties");
  table->EnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) gpa(instance, "vkEnumerateDeviceLayerProperties");
  table->GetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
  table->DestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) gpa(instance, "vkDestroySurfaceKHR");
  table->GetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
  table->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  table->GetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  table->GetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) gpa(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  table->GetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) gpa(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
  table->GetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) gpa(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
  table->GetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) gpa(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
  table->GetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) gpa(instance, "vkGetDisplayModePropertiesKHR");
  table->CreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) gpa(instance, "vkCreateDisplayModeKHR");
  table->GetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) gpa(instance, "vkGetDisplayPlaneCapabilitiesKHR");
  table->CreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) gpa(instance, "vkCreateDisplayPlaneSurfaceKHR");
#ifdef VK_USE_PLATFORM_XLIB_KHR
  table->CreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) gpa(instance, "vkCreateXlibSurfaceKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
  table->GetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) gpa(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
  table->CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) gpa(instance, "vkCreateXcbSurfaceKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
  table->GetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) gpa(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
  table->CreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) gpa(instance, "vkCreateWaylandSurfaceKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
  table->GetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) gpa(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_MIR_KHR
  table->CreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR) gpa(instance, "vkCreateMirSurfaceKHR");
#endif // VK_USE_PLATFORM_MIR_KHR
#ifdef VK_USE_PLATFORM_MIR_KHR
  table->GetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR) gpa(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif // VK_USE_PLATFORM_MIR_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
  table->CreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) gpa(instance, "vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
  table->CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) gpa(instance, "vkCreateWin32SurfaceKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
  table->GetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) gpa(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
  table->GetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) gpa(instance, "vkGetPhysicalDeviceFeatures");
  table->GetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) gpa(instance, "vkGetPhysicalDeviceProperties");
  table->GetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) gpa(instance, "vkGetPhysicalDeviceFormatProperties");
  table->GetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) gpa(instance, "vkGetPhysicalDeviceImageFormatProperties");
  table->GetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) gpa(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
  table->GetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) gpa(instance, "vkGetPhysicalDeviceMemoryProperties");
  table->GetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) gpa(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
  table->CreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) gpa(instance, "vkCreateDebugReportCallbackEXT");
  table->DestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) gpa(instance, "vkDestroyDebugReportCallbackEXT");
  table->DebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) gpa(instance, "vkDebugReportMessageEXT");
  table->GetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV) gpa(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  // table->GetPhysicalDevicePresentRectanglesKHX = (PFN_vkGetPhysicalDevicePresentRectanglesKHX) gpa(instance, "vkGetPhysicalDevicePresentRectanglesKHX");
#ifdef VK_USE_PLATFORM_VI_NN
  table->CreateViSurfaceNN = (PFN_vkCreateViSurfaceNN) gpa(instance, "vkCreateViSurfaceNN");
#endif // VK_USE_PLATFORM_VI_NN
  // table->EnumeratePhysicalDeviceGroupsKHX = (PFN_vkEnumeratePhysicalDeviceGroupsKHX) gpa(instance, "vkEnumeratePhysicalDeviceGroupsKHX");
  // table->GetPhysicalDeviceExternalBufferPropertiesKHX = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHX) gpa(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHX");
  // table->GetPhysicalDeviceExternalSemaphorePropertiesKHX = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHX) gpa(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHX");
  // table->GetPhysicalDeviceGeneratedCommandsPropertiesNVX = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX) gpa(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  // table->ReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT) gpa(instance, "vkReleaseDisplayEXT");
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
  table->AcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT) gpa(instance, "vkAcquireXlibDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
  table->GetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT) gpa(instance, "vkGetRandROutputDisplayEXT");
#endif // VK_USE_PLATFORM_XLIB_XRANDR_EXT
  table->GetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) gpa(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
#ifdef VK_USE_PLATFORM_IOS_MVK
  table->CreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK) gpa(instance, "vkCreateIOSSurfaceMVK");
#endif // VK_USE_PLATFORM_IOS_MVK
#ifdef VK_USE_PLATFORM_MACOS_MVK
  table->CreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK) gpa(instance, "vkCreateMacOSSurfaceMVK");
#endif // VK_USE_PLATFORM_MACOS_MVK
}

void LayerInitDeviceDispatchTable(VkDevice device, VkLayerDispatchTable *table, PFN_vkGetDeviceProcAddr gpa)
{
  std::memset(table, 0, sizeof(*table));
  // Device function pointers
  table->GetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) gpa(device, "vkGetDeviceProcAddr");
  table->DestroyDevice = (PFN_vkDestroyDevice) gpa(device, "vkDestroyDevice");
  table->GetDeviceQueue = (PFN_vkGetDeviceQueue) gpa(device, "vkGetDeviceQueue");
  table->QueueSubmit = (PFN_vkQueueSubmit) gpa(device, "vkQueueSubmit");
  table->QueueWaitIdle = (PFN_vkQueueWaitIdle) gpa(device, "vkQueueWaitIdle");
  table->DeviceWaitIdle = (PFN_vkDeviceWaitIdle) gpa(device, "vkDeviceWaitIdle");
  table->AllocateMemory = (PFN_vkAllocateMemory) gpa(device, "vkAllocateMemory");
  table->FreeMemory = (PFN_vkFreeMemory) gpa(device, "vkFreeMemory");
  table->MapMemory = (PFN_vkMapMemory) gpa(device, "vkMapMemory");
  table->UnmapMemory = (PFN_vkUnmapMemory) gpa(device, "vkUnmapMemory");
  table->FlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) gpa(device, "vkFlushMappedMemoryRanges");
  table->InvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) gpa(device, "vkInvalidateMappedMemoryRanges");
  table->GetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) gpa(device, "vkGetDeviceMemoryCommitment");
  table->BindBufferMemory = (PFN_vkBindBufferMemory) gpa(device, "vkBindBufferMemory");
  table->BindImageMemory = (PFN_vkBindImageMemory) gpa(device, "vkBindImageMemory");
  table->GetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) gpa(device, "vkGetBufferMemoryRequirements");
  table->GetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) gpa(device, "vkGetImageMemoryRequirements");
  table->GetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) gpa(device, "vkGetImageSparseMemoryRequirements");
  table->QueueBindSparse = (PFN_vkQueueBindSparse) gpa(device, "vkQueueBindSparse");
  table->CreateFence = (PFN_vkCreateFence) gpa(device, "vkCreateFence");
  table->DestroyFence = (PFN_vkDestroyFence) gpa(device, "vkDestroyFence");
  table->ResetFences = (PFN_vkResetFences) gpa(device, "vkResetFences");
  table->GetFenceStatus = (PFN_vkGetFenceStatus) gpa(device, "vkGetFenceStatus");
  table->WaitForFences = (PFN_vkWaitForFences) gpa(device, "vkWaitForFences");
  table->CreateSemaphore = (PFN_vkCreateSemaphore) gpa(device, "vkCreateSemaphore");
  table->DestroySemaphore = (PFN_vkDestroySemaphore) gpa(device, "vkDestroySemaphore");
  table->CreateEvent = (PFN_vkCreateEvent) gpa(device, "vkCreateEvent");
  table->DestroyEvent = (PFN_vkDestroyEvent) gpa(device, "vkDestroyEvent");
  table->GetEventStatus = (PFN_vkGetEventStatus) gpa(device, "vkGetEventStatus");
  table->SetEvent = (PFN_vkSetEvent) gpa(device, "vkSetEvent");
  table->ResetEvent = (PFN_vkResetEvent) gpa(device, "vkResetEvent");
  table->CreateQueryPool = (PFN_vkCreateQueryPool) gpa(device, "vkCreateQueryPool");
  table->DestroyQueryPool = (PFN_vkDestroyQueryPool) gpa(device, "vkDestroyQueryPool");
  table->GetQueryPoolResults = (PFN_vkGetQueryPoolResults) gpa(device, "vkGetQueryPoolResults");
  table->CreateBuffer = (PFN_vkCreateBuffer) gpa(device, "vkCreateBuffer");
  table->DestroyBuffer = (PFN_vkDestroyBuffer) gpa(device, "vkDestroyBuffer");
  table->CreateBufferView = (PFN_vkCreateBufferView) gpa(device, "vkCreateBufferView");
  table->DestroyBufferView = (PFN_vkDestroyBufferView) gpa(device, "vkDestroyBufferView");
  table->CreateImage = (PFN_vkCreateImage) gpa(device, "vkCreateImage");
  table->DestroyImage = (PFN_vkDestroyImage) gpa(device, "vkDestroyImage");
  table->GetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) gpa(device, "vkGetImageSubresourceLayout");
  table->CreateImageView = (PFN_vkCreateImageView) gpa(device, "vkCreateImageView");
  table->DestroyImageView = (PFN_vkDestroyImageView) gpa(device, "vkDestroyImageView");
  table->CreateShaderModule = (PFN_vkCreateShaderModule) gpa(device, "vkCreateShaderModule");
  table->DestroyShaderModule = (PFN_vkDestroyShaderModule) gpa(device, "vkDestroyShaderModule");
  table->CreatePipelineCache = (PFN_vkCreatePipelineCache) gpa(device, "vkCreatePipelineCache");
  table->DestroyPipelineCache = (PFN_vkDestroyPipelineCache) gpa(device, "vkDestroyPipelineCache");
  table->GetPipelineCacheData = (PFN_vkGetPipelineCacheData) gpa(device, "vkGetPipelineCacheData");
  table->MergePipelineCaches = (PFN_vkMergePipelineCaches) gpa(device, "vkMergePipelineCaches");
  table->CreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) gpa(device, "vkCreateGraphicsPipelines");
  table->CreateComputePipelines = (PFN_vkCreateComputePipelines) gpa(device, "vkCreateComputePipelines");
  table->DestroyPipeline = (PFN_vkDestroyPipeline) gpa(device, "vkDestroyPipeline");
  table->CreatePipelineLayout = (PFN_vkCreatePipelineLayout) gpa(device, "vkCreatePipelineLayout");
  table->DestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) gpa(device, "vkDestroyPipelineLayout");
  table->CreateSampler = (PFN_vkCreateSampler) gpa(device, "vkCreateSampler");
  table->DestroySampler = (PFN_vkDestroySampler) gpa(device, "vkDestroySampler");
  table->CreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) gpa(device, "vkCreateDescriptorSetLayout");
  table->DestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) gpa(device, "vkDestroyDescriptorSetLayout");
  table->CreateDescriptorPool = (PFN_vkCreateDescriptorPool) gpa(device, "vkCreateDescriptorPool");
  table->DestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) gpa(device, "vkDestroyDescriptorPool");
  table->ResetDescriptorPool = (PFN_vkResetDescriptorPool) gpa(device, "vkResetDescriptorPool");
  table->AllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) gpa(device, "vkAllocateDescriptorSets");
  table->FreeDescriptorSets = (PFN_vkFreeDescriptorSets) gpa(device, "vkFreeDescriptorSets");
  table->UpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) gpa(device, "vkUpdateDescriptorSets");
  table->CreateFramebuffer = (PFN_vkCreateFramebuffer) gpa(device, "vkCreateFramebuffer");
  table->DestroyFramebuffer = (PFN_vkDestroyFramebuffer) gpa(device, "vkDestroyFramebuffer");
  table->CreateRenderPass = (PFN_vkCreateRenderPass) gpa(device, "vkCreateRenderPass");
  table->DestroyRenderPass = (PFN_vkDestroyRenderPass) gpa(device, "vkDestroyRenderPass");
  table->GetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) gpa(device, "vkGetRenderAreaGranularity");
  table->CreateCommandPool = (PFN_vkCreateCommandPool) gpa(device, "vkCreateCommandPool");
  table->DestroyCommandPool = (PFN_vkDestroyCommandPool) gpa(device, "vkDestroyCommandPool");
  table->ResetCommandPool = (PFN_vkResetCommandPool) gpa(device, "vkResetCommandPool");
  table->AllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) gpa(device, "vkAllocateCommandBuffers");
  table->FreeCommandBuffers = (PFN_vkFreeCommandBuffers) gpa(device, "vkFreeCommandBuffers");
  table->BeginCommandBuffer = (PFN_vkBeginCommandBuffer) gpa(device, "vkBeginCommandBuffer");
  table->EndCommandBuffer = (PFN_vkEndCommandBuffer) gpa(device, "vkEndCommandBuffer");
  table->ResetCommandBuffer = (PFN_vkResetCommandBuffer) gpa(device, "vkResetCommandBuffer");
  table->CmdBindPipeline = (PFN_vkCmdBindPipeline) gpa(device, "vkCmdBindPipeline");
  table->CmdSetViewport = (PFN_vkCmdSetViewport) gpa(device, "vkCmdSetViewport");
  table->CmdSetScissor = (PFN_vkCmdSetScissor) gpa(device, "vkCmdSetScissor");
  table->CmdSetLineWidth = (PFN_vkCmdSetLineWidth) gpa(device, "vkCmdSetLineWidth");
  table->CmdSetDepthBias = (PFN_vkCmdSetDepthBias) gpa(device, "vkCmdSetDepthBias");
  table->CmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) gpa(device, "vkCmdSetBlendConstants");
  table->CmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) gpa(device, "vkCmdSetDepthBounds");
  table->CmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) gpa(device, "vkCmdSetStencilCompareMask");
  table->CmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) gpa(device, "vkCmdSetStencilWriteMask");
  table->CmdSetStencilReference = (PFN_vkCmdSetStencilReference) gpa(device, "vkCmdSetStencilReference");
  table->CmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) gpa(device, "vkCmdBindDescriptorSets");
  table->CmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) gpa(device, "vkCmdBindIndexBuffer");
  table->CmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) gpa(device, "vkCmdBindVertexBuffers");
  table->CmdDraw = (PFN_vkCmdDraw) gpa(device, "vkCmdDraw");
  table->CmdDrawIndexed = (PFN_vkCmdDrawIndexed) gpa(device, "vkCmdDrawIndexed");
  table->CmdDrawIndirect = (PFN_vkCmdDrawIndirect) gpa(device, "vkCmdDrawIndirect");
  table->CmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) gpa(device, "vkCmdDrawIndexedIndirect");
  table->CmdDispatch = (PFN_vkCmdDispatch) gpa(device, "vkCmdDispatch");
  table->CmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) gpa(device, "vkCmdDispatchIndirect");
  table->CmdCopyBuffer = (PFN_vkCmdCopyBuffer) gpa(device, "vkCmdCopyBuffer");
  table->CmdCopyImage = (PFN_vkCmdCopyImage) gpa(device, "vkCmdCopyImage");
  table->CmdBlitImage = (PFN_vkCmdBlitImage) gpa(device, "vkCmdBlitImage");
  table->CmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) gpa(device, "vkCmdCopyBufferToImage");
  table->CmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) gpa(device, "vkCmdCopyImageToBuffer");
  table->CmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) gpa(device, "vkCmdUpdateBuffer");
  table->CmdFillBuffer = (PFN_vkCmdFillBuffer) gpa(device, "vkCmdFillBuffer");
  table->CmdClearColorImage = (PFN_vkCmdClearColorImage) gpa(device, "vkCmdClearColorImage");
  table->CmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) gpa(device, "vkCmdClearDepthStencilImage");
  table->CmdClearAttachments = (PFN_vkCmdClearAttachments) gpa(device, "vkCmdClearAttachments");
  table->CmdResolveImage = (PFN_vkCmdResolveImage) gpa(device, "vkCmdResolveImage");
  table->CmdSetEvent = (PFN_vkCmdSetEvent) gpa(device, "vkCmdSetEvent");
  table->CmdResetEvent = (PFN_vkCmdResetEvent) gpa(device, "vkCmdResetEvent");
  table->CmdWaitEvents = (PFN_vkCmdWaitEvents) gpa(device, "vkCmdWaitEvents");
  table->CmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) gpa(device, "vkCmdPipelineBarrier");
  table->CmdBeginQuery = (PFN_vkCmdBeginQuery) gpa(device, "vkCmdBeginQuery");
  table->CmdEndQuery = (PFN_vkCmdEndQuery) gpa(device, "vkCmdEndQuery");
  table->CmdResetQueryPool = (PFN_vkCmdResetQueryPool) gpa(device, "vkCmdResetQueryPool");
  table->CmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) gpa(device, "vkCmdWriteTimestamp");
  table->CmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) gpa(device, "vkCmdCopyQueryPoolResults");
  table->CmdPushConstants = (PFN_vkCmdPushConstants) gpa(device, "vkCmdPushConstants");
  table->CmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) gpa(device, "vkCmdBeginRenderPass");
  table->CmdNextSubpass = (PFN_vkCmdNextSubpass) gpa(device, "vkCmdNextSubpass");
  table->CmdEndRenderPass = (PFN_vkCmdEndRenderPass) gpa(device, "vkCmdEndRenderPass");
  table->CmdExecuteCommands = (PFN_vkCmdExecuteCommands) gpa(device, "vkCmdExecuteCommands");
  table->CreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) gpa(device, "vkCreateSwapchainKHR");
  table->DestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) gpa(device, "vkDestroySwapchainKHR");
  table->GetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) gpa(device, "vkGetSwapchainImagesKHR");
  table->AcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) gpa(device, "vkAcquireNextImageKHR");
  table->QueuePresentKHR = (PFN_vkQueuePresentKHR) gpa(device, "vkQueuePresentKHR");
  table->CreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) gpa(device, "vkCreateSharedSwapchainsKHR");
  // table->TrimCommandPool = (PFN_vkTrimCommandPool) gpa(device, "vkTrimCommandPool");
  // table->CmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet) gpa(device, "vkCmdPushDescriptorSet");
  // table->CreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) gpa(device, "vkCreateDescriptorUpdateTemplate");
  // table->DestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) gpa(device, "vkDestroyDescriptorUpdateTemplate");
  // table->UpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) gpa(device, "vkUpdateDescriptorSetWithTemplate");
  // table->CmdPushDescriptorSetWithTemplate = (PFN_vkCmdPushDescriptorSetWithTemplate) gpa(device, "vkCmdPushDescriptorSetWithTemplate");
  table->DebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT) gpa(device, "vkDebugMarkerSetObjectTagEXT");
  table->DebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT) gpa(device, "vkDebugMarkerSetObjectNameEXT");
  table->CmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT) gpa(device, "vkCmdDebugMarkerBeginEXT");
  table->CmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT) gpa(device, "vkCmdDebugMarkerEndEXT");
  table->CmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT) gpa(device, "vkCmdDebugMarkerInsertEXT");
  table->CmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD) gpa(device, "vkCmdDrawIndirectCountAMD");
  table->CmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD) gpa(device, "vkCmdDrawIndexedIndirectCountAMD");
#ifdef VK_USE_PLATFORM_WIN32_KHR
  table->GetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV) gpa(device, "vkGetMemoryWin32HandleNV");
#endif // VK_USE_PLATFORM_WIN32_KHR
  // table->GetDeviceGroupPeerMemoryFeaturesKHX = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX) gpa(device, "vkGetDeviceGroupPeerMemoryFeaturesKHX");
  // table->BindBufferMemory2KHX = (PFN_vkBindBufferMemory2KHX) gpa(device, "vkBindBufferMemory2KHX");
  // table->BindImageMemory2KHX = (PFN_vkBindImageMemory2KHX) gpa(device, "vkBindImageMemory2KHX");
  // table->CmdSetDeviceMaskKHX = (PFN_vkCmdSetDeviceMaskKHX) gpa(device, "vkCmdSetDeviceMaskKHX");
  // table->GetDeviceGroupPresentCapabilitiesKHX = (PFN_vkGetDeviceGroupPresentCapabilitiesKHX) gpa(device, "vkGetDeviceGroupPresentCapabilitiesKHX");
  // table->GetDeviceGroupSurfacePresentModesKHX = (PFN_vkGetDeviceGroupSurfacePresentModesKHX) gpa(device, "vkGetDeviceGroupSurfacePresentModesKHX");
  // table->AcquireNextImage2KHX = (PFN_vkAcquireNextImage2KHX) gpa(device, "vkAcquireNextImage2KHX");
  // table->CmdDispatchBaseKHX = (PFN_vkCmdDispatchBaseKHX) gpa(device, "vkCmdDispatchBaseKHX");
#ifdef VK_USE_PLATFORM_WIN32_KHX
  table->GetMemoryWin32HandleKHX = (PFN_vkGetMemoryWin32HandleKHX) gpa(device, "vkGetMemoryWin32HandleKHX");
#endif // VK_USE_PLATFORM_WIN32_KHX
#ifdef VK_USE_PLATFORM_WIN32_KHX
  table->GetMemoryWin32HandlePropertiesKHX = (PFN_vkGetMemoryWin32HandlePropertiesKHX) gpa(device, "vkGetMemoryWin32HandlePropertiesKHX");
#endif // VK_USE_PLATFORM_WIN32_KHX
  // table->GetMemoryFdKHX = (PFN_vkGetMemoryFdKHX) gpa(device, "vkGetMemoryFdKHX");
  // table->GetMemoryFdPropertiesKHX = (PFN_vkGetMemoryFdPropertiesKHX) gpa(device, "vkGetMemoryFdPropertiesKHX");
#ifdef VK_USE_PLATFORM_WIN32_KHX
  table->ImportSemaphoreWin32HandleKHX = (PFN_vkImportSemaphoreWin32HandleKHX) gpa(device, "vkImportSemaphoreWin32HandleKHX");
#endif // VK_USE_PLATFORM_WIN32_KHX
#ifdef VK_USE_PLATFORM_WIN32_KHX
  table->GetSemaphoreWin32HandleKHX = (PFN_vkGetSemaphoreWin32HandleKHX) gpa(device, "vkGetSemaphoreWin32HandleKHX");
#endif // VK_USE_PLATFORM_WIN32_KHX
  // table->ImportSemaphoreFdKHX = (PFN_vkImportSemaphoreFdKHX) gpa(device, "vkImportSemaphoreFdKHX");
  // table->GetSemaphoreFdKHX = (PFN_vkGetSemaphoreFdKHX) gpa(device, "vkGetSemaphoreFdKHX");
  // table->CmdProcessCommandsNVX = (PFN_vkCmdProcessCommandsNVX) gpa(device, "vkCmdProcessCommandsNVX");
  // table->CmdReserveSpaceForCommandsNVX = (PFN_vkCmdReserveSpaceForCommandsNVX) gpa(device, "vkCmdReserveSpaceForCommandsNVX");
  // table->CreateIndirectCommandsLayoutNVX = (PFN_vkCreateIndirectCommandsLayoutNVX) gpa(device, "vkCreateIndirectCommandsLayoutNVX");
  // table->DestroyIndirectCommandsLayoutNVX = (PFN_vkDestroyIndirectCommandsLayoutNVX) gpa(device, "vkDestroyIndirectCommandsLayoutNVX");
  // table->CreateObjectTableNVX = (PFN_vkCreateObjectTableNVX) gpa(device, "vkCreateObjectTableNVX");
  // table->DestroyObjectTableNVX = (PFN_vkDestroyObjectTableNVX) gpa(device, "vkDestroyObjectTableNVX");
  // table->RegisterObjectsNVX = (PFN_vkRegisterObjectsNVX) gpa(device, "vkRegisterObjectsNVX");
  // table->UnregisterObjectsNVX = (PFN_vkUnregisterObjectsNVX) gpa(device, "vkUnregisterObjectsNVX");
  // table->CmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV) gpa(device, "vkCmdSetViewportWScalingNV");
  // table->DisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT) gpa(device, "vkDisplayPowerControlEXT");
  // table->RegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT) gpa(device, "vkRegisterDeviceEventEXT");
  // table->RegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT) gpa(device, "vkRegisterDisplayEventEXT");
  // table->GetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT) gpa(device, "vkGetSwapchainCounterEXT");
  // table->GetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE) gpa(device, "vkGetRefreshCycleDurationGOOGLE");
  // table->GetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE) gpa(device, "vkGetPastPresentationTimingGOOGLE");
  // table->CmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT) gpa(device, "vkCmdSetDiscardRectangleEXT");
  // table->SetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT) gpa(device, "vkSetHdrMetadataEXT");
}

}; // anoymous namespace

extern "C" {

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                              const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance)
{
  ShimBeginCommand(APIID_VULKAN, ID_vkCreateInstance);

  VkLayerInstanceCreateInfo* chain_info =
    FindLayerInstanceCreateInfo(pCreateInfo, VK_LAYER_LINK_INFO);

  if(NULL == chain_info)
  {
    Log(LogError, "Cannot find Instance chain info?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  if(NULL == chain_info->u.pLayerInfo)
  {
    Log(LogError, "Instance chain is missing LayerInfo?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  PFN_vkGetInstanceProcAddr pfnNextGetInstanceProcAddr =
    chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;

  PFN_vkCreateInstance pfnNextCreateInstance =
    (PFN_vkCreateInstance)pfnNextGetInstanceProcAddr(NULL, "vkCreateInstance");

  if(pfnNextCreateInstance == NULL)
  {
    Log(LogError, "Missing Command Called:%s", pChar_vkCreateInstance);
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  // Advance the link info for the next element of the chain
  chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

  VkResult result = pfnNextCreateInstance(pCreateInfo, pAllocator, pInstance);

  if (result != VK_SUCCESS)
  {
    return vulkanError(result);
  }

  auto table = GetDispatchTable(pInstance);

  if(!table)
  {
    Log(LogError, "Cannot create/find dispatch table?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  LayerInitInstanceDispatchTable(*pInstance, table, pfnNextGetInstanceProcAddr);

  EnableVulkanDebugCallback(*pInstance);

  ShimEndCommand(APIID_VULKAN, ID_vkCreateInstance);
  return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator)
{
  DestroyVulkanDebugCallback(instance);
  ShimBeginCommand(APIID_VULKAN, ID_vkDestroyInstance);

  auto table = GetDispatchTable(instance);

  if(table && table->DestroyInstance)
  {
    table->DestroyInstance(instance, pAllocator);
  }
  else
  {
    Log(LogError, "Missing Command Called:%s", pChar_vkDestroyInstance);
  }

  RemoveDispatchTable(instance);

  ShimEndCommand(APIID_VULKAN, ID_vkDestroyInstance);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
  ShimBeginCommand(APIID_VULKAN, ID_vkCreateDevice);
  VkLayerDeviceCreateInfo* chain_info = FindDeviceCreateInfo(pCreateInfo, VK_LAYER_LINK_INFO);

  if(NULL == chain_info)
  {
    Log(LogError, "Cannot find Instance chain info?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  if(NULL == chain_info->u.pLayerInfo)
  {
    Log(LogError, "Device chain is missing LayerInfo?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  auto instance = GetInstance(physicalDevice);

  PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
  PFN_vkCreateDevice pfnCreateDevice = (PFN_vkCreateDevice)pfnGetInstanceProcAddr(instance, "vkCreateDevice");

  if(NULL == pfnCreateDevice)
  {
    Log(LogError, "Missing Command Called:%s", pChar_vkCreateInstance);
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  // Advance the link info for the next element on the chain
  chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

  VkResult result = pfnCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
  if (result != VK_SUCCESS)
  {
    return vulkanError(result);
  }

  auto table = GetDispatchTable(pDevice);
  if(!table)
  {
    Log(LogError, "Cannot create/find dispatch table?");
    return vulkanError(VK_ERROR_INITIALIZATION_FAILED);
  }

  LayerInitDeviceDispatchTable(*pDevice, table, pfnGetDeviceProcAddr);

  ShimEndCommand(APIID_VULKAN, ID_vkCreateDevice);

  return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
  ShimBeginCommand(APIID_VULKAN, ID_vkDestroyDevice);

  auto table = GetDispatchTable(device);

  if(table && table->DestroyDevice)
  {
    table->DestroyDevice(device, pAllocator);
  }
  else
  {
    Log(LogError, "Missing Command Called:%s", pChar_vkDestroyDevice);
  }

  RemoveDispatchTable(device);

  ShimEndCommand(APIID_VULKAN, ID_vkDestroyDevice);
}


PFN_vkVoidFunction FindChainingFunction(const char *name)
{
  if(0 == strcmp("vkCreateInstance", name))  return reinterpret_cast<PFN_vkVoidFunction>(CreateInstance);
  if(0 == strcmp("vkDestroyInstance", name)) return reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance);
  if(0 == strcmp("vkCreateDevice", name))    return reinterpret_cast<PFN_vkVoidFunction>(CreateDevice);
  if(0 == strcmp("vkDestroyDevice", name))   return reinterpret_cast<PFN_vkVoidFunction>(DestroyDevice);

  return NULL;
}

VKAPI_ATTR PFN_vkVoidFunction DLL_EXPORT vkGetDeviceProcAddr(VkDevice device, const char *funcName)
{
  Log(LogVerbose, "%d\n", __LINE__);
  ShimBeginCommand(APIID_VULKAN, ID_vkGetDeviceProcAddr);

  PFN_vkVoidFunction address = FindChainingFunction(funcName);

  if(!address)
  {
    address = FindFunctionInThisLayer(funcName);
    if(address)
    {
      Log(LogVerbose, "Found device function in this layer '%s'\n", funcName);
    }
  }
  else
  {
    Log(LogVerbose, "Found device chaining function in this layer '%s'\n", funcName);
  }

  if(!address)
  {
    auto table = GetDispatchTable(device);

    if(table && table->GetDeviceProcAddr)
    {
      address = table->GetDeviceProcAddr(device, funcName);
    }
    else
    {
      address = NULL;
    }

    if(address)
    {
      Log(LogVerbose, "Found device function from chain '%s'\n", funcName);
    }
  }

  if(!address)
  {
    Log(LogError, "Unable to find function '%s'\n", funcName);
  }

  ShimEndCommand(APIID_VULKAN, ID_vkDestroyDevice);

  return address;
}

VKAPI_ATTR PFN_vkVoidFunction DLL_EXPORT vkGetInstanceProcAddr(VkInstance instance, const char *funcName)
{
  Log(LogVerbose, "%d\n", __LINE__);
  ShimBeginCommand(APIID_VULKAN, ID_vkGetInstanceProcAddr);

  PFN_vkVoidFunction address = FindChainingFunction(funcName);

  if(!address)
  {
    address = FindFunctionInThisLayer(funcName);
    if(address)
    {
      Log(LogVerbose, "Found function in this layer '%s'\n", funcName);
    }
  }
  else
  {
    Log(LogVerbose, "Found chaining function in this layer '%s'\n", funcName);
  }

  if(!address)
  {
    auto table = GetDispatchTable(instance);

    if(table && table->GetInstanceProcAddr)
    {
      address = table->GetInstanceProcAddr(instance, funcName);
    }
    else
    {
      address = NULL;
    }

    if(address)
    {
      Log(LogVerbose, "Found function from chain '%s'\n", funcName);
    }

  }

  if(!address)
  {
    Log(LogError, "Unable to find function '%s'\n", funcName);
  }

  ShimEndCommand(APIID_VULKAN, ID_vkGetInstanceProcAddr);

  return address;
}

// VKAPI_ATTR PFN_vkVoidFunction DLL_EXPORT GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName)
// {
//   Log(LogVerbose, "%d\n", __LINE__);
//     // assert(instance);
//     // auto pdev_data = GetLayerDataPtr(get_dispatch_key(instance), instance_layer_data_map);

//     // if (!pdev_data->dispatch_table.GetPhysicalDeviceProcAddr)
//     // {
//   return nullptr;
//     // }
//     // else
//     // {
//     //   return pdev_data->dispatch_table.GetPhysicalDeviceProcAddr(instance, funcName);
//     // }
// }

}; // extern "C"
