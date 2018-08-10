# LPGPU2 CodeXL

![LPGPU2 Logo](http://lpgpu.org/wp/wp-content/uploads/2016/01/logo_web.png)

## About LPGPU2

Low-Power Parallel Computing on GPUs 2 (LPGPU<sup>2</sup>) is a EU-funded research project into low powered graphics devices. It is the work of a specially formed consortium of four companies and one university, all from across the EU, who are collaborating to deliver advances in tools and applications for energy efficient use of mobile GPUs.

## About this repository

This repository is the home for the open source release of the work done by the [LPGPU<sup>2</sup> consortium](http://lpgpu.org/wp/) on top of CodeXL.

> [CodeXL Repository](https://github.com/GPUOpen-Tools/CodeXL)

> **Note**: User Manual will be posted soon

## Build Instructions

For instructions on how to build LPGPU<sup>2</sup> from source see [BUILD.md](BUILD.md).

## Demo

The following video demonstrates some of the features added to CodeXL in order to enable profiling of low-power GPU applications running on an Android device.

[![LPGPU2 CodeXL workflow and Feedback Engine demo](https://i.ytimg.com/vi/zB9WeHpiUhY/hqdefault.jpg?sqp=-oaymwEZCPYBEIoBSFXyq4qpAwsIARUAAIhCGAFwAQ==&rs=AOn4CLACtRxPqsZLiK9iiH3tYeEX6Wvupw)](https://www.youtube.com/watch?v=zB9WeHpiUhY)

## Features and Extensions
* Ability to profile, in great detail, Android applications leveraging the **Data Collection (DC) API** capabilities.
    * Does not require root access to Android devices
    * Uses ADB transparently to control the execution of the Remote Agent.
    * Built-in Feedback Engine capable of giving advice on how to improve your code.
        * Feedback scripts are written in Lua and can be modified at any time by the user while CodeXL is running.
    * Several visualisation widgets to inspect the collected data and find hotspots and points of improvement.
    * Detailed call information of several graphics APIs: **OpenGL ES, OpenCL, Vulkan** on Android.
* Experimental support for **Agnostic OpenCL profiling**.
    * Removes the restriction of only being able to profile OpenCL in AMD specific devices.
* Experimental support for profiling **desktop SYCL applications** using **ComputeCpp**.
    * Display ComputeCpp runtime information on top of OpenCL traces.
    * Works with ComputeCpp Community Edition.
* Display **Code Coverage** results generated with `llvm-cov` with ability to refresh the interface when new results are available.

## More information

- [LPGPU2 Website](http://lpgpu.org/wp/)
- [LPGPU2 YouTube Channel](https://www.youtube.com/channel/UCDVYThdvn8gwNj0FdzpH1hw)
