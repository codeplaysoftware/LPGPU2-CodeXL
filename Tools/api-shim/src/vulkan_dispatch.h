/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#pragma once

#include "vulkan/vulkan.h"
#include "vulkan/vk_layer.h"

VkLayerInstanceDispatchTable* GetDispatchTable(VkInstance dispatchable);
VkLayerDispatchTable* GetDispatchTable(VkDevice dispatchable);
VkInstance GetInstance(VkPhysicalDevice dispatchable);

void RemoveDispatchTable(VkInstance dispatchable);
void RemoveDispatchTable(VkDevice dispatchable);

VkLayerDispatchTable* GetDispatchTable(VkCommandBuffer dispatchable);
VkLayerDispatchTable* GetDispatchTable(VkQueue dispatchable);
VkLayerInstanceDispatchTable* GetDispatchTable(VkPhysicalDevice dispatchable);
VkLayerInstanceDispatchTable* GetDispatchTable(VkInstance* dispatchable);
VkLayerDispatchTable* GetDispatchTable(VkDevice* dispatchable);

