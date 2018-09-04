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

## GPUOpen Tools Project Description and instruction
The text below is repeated on their repository. It is for guidance only to the setup required. The project pre-requisitions are required.

## CodeXL

CodeXL is a comprehensive tool suite that enables developers to harness the benefits of CPUs, GPUs and APUs. It includes powerful GPU debugging, comprehensive GPU and CPU profiling, DirectX12® Frame Analysis, static OpenCL™, OpenGL®, Vulkan® and DirectX® kernel/shader analysis capabilities, and APU/CPU/GPU power profiling, enhancing accessibility for software developers to enter the era of heterogeneous computing. CodeXL is available both as a Visual Studio® extension and a standalone user interface application for Windows® and Linux®.

## Motivation

CodeXL, previously a tool developed as closed-source by Advanced Micro Devices, Inc., is now released as Open Source. AMD believes that adopting the open-source model and sharing the CodeXL source base with the world can help developers make better use of CodeXL and make CodeXL a better tool.

To encourage 3rd party contribution and adoption, CodeXL is no longer branded as an AMD product. AMD will still continue development of this tool and upload new versions and features to GPUOpen.


## Installation and Build

* Windows: To install CodeXL, use the [provided](https://github.com/GPUOpen-Tools/CodeXL/releases) executable file CodeXL_*.exe
* Linux: To install CodeXL, use the [provided](https://github.com/GPUOpen-Tools/CodeXL/releases) RPM file, Debian file, or simply extract the compressed archive onto your hard drive.
  * Note that future changes to the Linux kernel may cause the installation of the CodeXL Power Profiling driver (`amdtPwrProf`) to fail. If this happens installation of the package will report an error, but CodeXL will still be installed correctly and functional, without system-wide power profiling functionality.

Refer to [BUILD.md](BUILD.md) for information on building CodeXL from source.

## Contributors

CodeXL's GitHub repository (http://github.com/GPUOpen-Tools/CodeXL) is moderated by Advanced Micro Devices, Inc. as part of the GPUOpen initiative.

AMD encourages any and all contributors to submit changes, features, and bug fixes via Git pull requests to this repository.

Users are also encouraged to submit issues and feature requests via the repository's issue tracker.

## License

CodeXL is part of the [GPUOpen.com](http://gpuopen.com/) initiative. CodeXL source code and binaries are released under the following MIT license:

Copyright © 2016 Advanced Micro Devices, Inc. All rights reserved.

MIT LICENSE:
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Attribution and Copyrights

Component licenses can be found under the CodeXL GitHub repository source root, in the /Setup/Legal/ folder.

OpenCL is a trademark of Apple Inc. used by permission by Khronos. OpenGL is a registered trademark of Silicon Graphics, Inc. in the United States and/or other countries worldwide. Microsoft, Windows, DirectX and Visual Studio are registered trademarks of Microsoft Corporation in the United States and/or other jurisdictions. Vulkan is a registered trademark of Khronos Group Inc. in the United States and/or other jurisdictions. Linux is the registered trademark of Linus Torvalds in the United States and/or other jurisdictions.

LGPL (Copyright ©1991, 1999 Free Software Foundation, Inc.  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA). Use of the Qt library is governed by the GNU Lesser General Public License version 2.1 (LGPL v 2.1). CodeXL uses QT 5.5.1. Source code for QT is available here: http://qt-project.org/downloads. The QT source code has not been tempered with and the built binaries are identical to what any user that downloads the source code from the web and builds them will produce.

* Boost is Copyright © Beman Dawes, 2003.
* [CR]LunarG, Inc. is Copyright © 2015 LunarG, Inc.
* jqPlot is copyright © 2009-2011 Chris Leonello.
* glew - The OpenGL Extension Wrangler Library is Copyright © 2002-2007, Milan Ikits <milan ikits[]ieee org>, Copyright © 2002-2007, Marcelo E. Magallon <mmagallo[]debian org>, Copyright © 2002, Lev Povalahev, All rights reserved.
* lgplib is Copyright © 1994-1998, Thomas G. Lane., Copyright © 1991-2013, Thomas G. Lane, Guido Vollbeding.
* LibDwarf (BSD) is Copyright © 2007 John Birrell (jb@freebsd.org),  Copyright © 2010 Kai Wang,  All rights reserved.
* libpng is Copyright © 1998-2014 Glenn Randers-Pehrson, (Version 0.96 Copyright © 1996, 1997 Andreas Dilger) (Version 0.88 Copyright © 1995, 1996 Guy Eric Schalnat, Group 42, Inc.).
* QScintilla is Copyright © 2005 by Riverbank Computing Limited <info@riverbankcomputing.co.uk>.
* TinyXML is released under the zlib license © 2000-2007, Lee Thomason, © 2002-2004, Yves Berquin © 2005, Tyge Lovset.
* UTF8cpp is Copyright © 2006 Nemanja Trifunovic.
* zlib is Copyright © 1995-2010 Jean-loup Gailly and Mark Adler, Copyright © 2003 Chris Anderson <christop@charm.net>, Copyright © 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html ), Copyright © 2009-2010 Mathias Svensson ( http://result42.com ), Copyright © 2007-2008 Even Rouault.
* QCustomPlot, an easy to use, modern plotting widget for Qt, Copyright (C) 2011-2015 Emanuel Eichhammer
* LLVM is Copyright © 2007-2014 University of Illinois at Urbana-Champaign.
* ComputeCpp is Copyright © 2002-2018 Codeplay Software Ltd.

