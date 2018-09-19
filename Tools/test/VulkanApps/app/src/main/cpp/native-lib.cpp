/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "shim2ify.h"

/*
 * Vulkan Samples
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
VULKAN_SAMPLE_SHORT_DESCRIPTION
Draw Textured Cube
*/

/* This is part of the draw cube progression */

#include <util_init.hpp>
#include <assert.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>

#include "apps/Triangle.hpp"
#include "apps/DrawTexCube.hpp"
#include "apps/Raymarching.hpp"
#include "apps/Overdraw.hpp"
#include "apps/OverdrawTex.hpp"
#include "apps/Hypercube.hpp"
#include "apps/Menger.hpp"
#include "apps/Globe.hpp"
#include "apps/Carousel.hpp"
#include "apps/Correctness.hpp"



// LPGPU2: Choose your app here by assigning LPGPU2_APP to one of the following values:
//
//  Raymarching
//  Overdraw
//  OverdrawTex
//  Hypercube
//  Menger
//  Globe
//  Uber

#define LPGPU2_APP Raymarching



typedef LPGPU2_APP TheApp;



void pipeline_issue_triggers(VkCommandBuffer& cmd) {

 // Although it won't break anything, there are a number of issues with this command...
 vkCmdPipelineBarrier(
  cmd,
  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // This is meaningless as a src because all useful stages occur after it
  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // This is meaningless as a dst because all useful stages occur before it
  0,         // Not framebuffer local
  0,nullptr, // Memory barriers         //                     memory-,
  0,nullptr, // Memory buffer barriers  // If there are no     buffer-,    barriers, then there is no barrier!
  0,nullptr  // Image memory barriers   //                    or image-
 );


 // Too many barriers in a renderpass is a cause for concern...
 for(int i=20;i;i--)
  vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,0,nullptr);
}



/* For this sample, we'll start with GLSL so the shader function is plain */
/* and then use the glslang GLSLtoSPV utility to convert it to SPIR-V for */
/* the driver.  We do this for clarity rather than using pre-compiled     */
/* SPIR-V                                                                 */

int sample_main(int argc, char *argv[]) {
    sleep(2); // Give the RAgent a chance!!

    VkResult U_ASSERT_ONLY res;
    struct sample_info info = {};
    char sample_title[] = "LPGPU2 Vulkan Apps"; // suspect not used for android
    const bool depthPresent = true;

    process_command_line_args(info, argc, argv);

    Instance instance(info,depthPresent);



    TheApp app(instance); // This is the app



    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.2f;
    clear_values[0].color.float32[1] = 0.2f;
    clear_values[0].color.float32[2] = 0.2f;
    clear_values[0].color.float32[3] = 0.2f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    while(1) { // Render loop
        VkSemaphore imageAcquiredSemaphore;
        VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
        imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        imageAcquiredSemaphoreCreateInfo.pNext = NULL;
        imageAcquiredSemaphoreCreateInfo.flags = 0;

        res = vkCreateSemaphore(info.device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
        assert(res == VK_SUCCESS);

        // Get the index of the next available swapchain image:
        res = vkAcquireNextImageKHR(info.device, info.swap_chain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
                                &info.current_buffer);
        // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
        // return codes
        assert(res == VK_SUCCESS);

        recreate_command_buffers(info);

        VkRenderPassBeginInfo rp_begin;
        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin.pNext = NULL;
        rp_begin.renderPass = info.render_pass;
        rp_begin.framebuffer = info.framebuffers[info.current_buffer];
        rp_begin.renderArea.offset.x = 0;
        rp_begin.renderArea.offset.y = 0;
        rp_begin.renderArea.extent.width = info.width;
        rp_begin.renderArea.extent.height = info.height;
        rp_begin.clearValueCount = 2;
        rp_begin.pClearValues = clear_values;

        vkCmdBeginRenderPass(info.cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        init_viewports(info);
        init_scissors (info);



        app.renderall(); // Only explicit reference to the app after construction



        pipeline_issue_triggers(info.cmd);



        vkCmdEndRenderPass(info.cmd);

        res = vkEndCommandBuffer(info.cmd);
        assert(res == VK_SUCCESS);

        const VkCommandBuffer cmd_bufs[] = {info.cmd};
        VkFenceCreateInfo fenceInfo;
        VkFence drawFence;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = NULL;
        fenceInfo.flags = 0;
        vkCreateFence(info.device, &fenceInfo, NULL, &drawFence);

        VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit_info[1] = {};
        submit_info[0].pNext = NULL;
        submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info[0].waitSemaphoreCount = 1;
        submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
        submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
        submit_info[0].commandBufferCount = 1;
        submit_info[0].pCommandBuffers = cmd_bufs;
        submit_info[0].signalSemaphoreCount = 0;
        submit_info[0].pSignalSemaphores = NULL;

        /* Queue the command buffer for execution */
        res = vkQueueSubmit(info.graphics_queue, 1, submit_info, drawFence);
        assert(res == VK_SUCCESS);

        /* Now present the image in the window */

        VkPresentInfoKHR present;
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.pNext = NULL;
        present.swapchainCount = 1;
        present.pSwapchains = &info.swap_chain;
        present.pImageIndices = &info.current_buffer;
        present.pWaitSemaphores = NULL;
        present.waitSemaphoreCount = 0;
        present.pResults = NULL;

        /* Make sure command buffer is finished before presenting */
        do {
            res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
        } while (res == VK_TIMEOUT);
        assert(res == VK_SUCCESS);
        res = vkQueuePresentKHR(info.present_queue, &present);
        assert(res == VK_SUCCESS);

        vkDestroyFence(info.device, drawFence, NULL);
        vkDestroySemaphore(info.device, imageAcquiredSemaphore, NULL);

        if (app.escape()) break;
    } // end render loop

    return 0;
}
