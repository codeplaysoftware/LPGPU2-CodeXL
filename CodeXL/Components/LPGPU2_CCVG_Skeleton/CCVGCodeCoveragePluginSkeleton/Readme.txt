CodeXL Skeleton Plugin v1.0
---------------------------

Introduction
------------
CCVG Code Coverage Project, part of the LPGPU2 European H2020 project is an additional plugin to CodeXl's existing set of 'execution mode' plugins (.dll/.so libraries).
To facilitate development of new plugins a fully operation skeleton plugin has been created to act as a template for new plugins. The plugin does not do anything useful but otherwise works as expected.

With the CCVG plugin there are a few additional build configuration possibilities. See the file CCVGProjectBuildDefinitions.h in the Visual Studio CCVG project. The CCVG plugin can be used in the following ways:
 
  1. CodeXL standard setup - 4 plugins/execution modes - CCVG plugin not loaded
  2. Like 1 but CodeXL now includes the additional CCVG plugin
  3. CodeXL only has one execution mode, the CCVG execution mode

Build configuration 3 is in anticipation of the requirement for CodeXL with CCVG to operate in a safety critical development project to minimise interference from other execution modes. The other modes have not been developed with safety critical quality management in mind.


Version compatibilty
--------------------

The files are for CodeXL 2.1 or higher. Visual Studio 2015.


CodeXL Welcome Page File Graphics for CCVG
------------------------------------------
For the CodeXL application to pick up a new plugin's details, icons and user instructional information the files in directory:

  CodeXL\AMDTApplicationFramework\Include\res\welcome 

need to be copied to the CodeXL's output build directory here:

  CodeXL\Output\Debug\bin\HTML

For a release build replace Debug with Release and copy files into there too.

Skeleton CCVG Plugin Source files
---------------------------------
The source and Visual Studio project files that make up the CCVG execution mode are found in the directory

  CodeXL\Components\LPGPU2_CCVG\CCVGCodeCoveragePluginSkeleton

Copy the directory into the directory 'Components'. Do not change the directory's name as this will break the include paths. 

In order for the CCVG plugin to integrate with the rest of CodeXl's framework some modifications have to been made to the CodeXl framework source files. The files that have been modified can be found in following directories:

  Common\Src\AMDTApplicationComponents
  CodeXL\AMDTApplicationFramework
  CodeXL\App\src

The code that needs to be changed can be found by searching for the following code tags:

  //++IOR: LPGPU2 Code Coverage Analysis
  //--IOR: LPGPU2 Code Coverage Analysis

For HTML files the same tags apply except using HTML comment tag instead.

The corresponding CodeXL framework files need to be either replaced or merged with the changes found in the same files here.

CodeXL Cover Coverage Execution mode icons
------------------------------------------
CodeXL icons are created from .png files and converted by Qt into .xpm resource files. There are several .pngs files each representing a different icon size. The icon files need to be edited to include or change the icons' graphical appearance. Once changes have been made re-compile the project. The relevant icon files can be found in directory:
  
  Common\Src\AMDTApplicationComponents\Include\res\icons

The CodeXL framework code base has built into it but not compiled normally the a modal dialog which can display all the icons used in CodeXL. To display the dialog go to file:

  Common\Src\AMDTApplicationComponents\src\acIcons.cpp

and uncomment the preprocessor definition AC_TEST_ICON_SHOW_ICONS_DLG. On startup CodeXL will display a modal dialog.

Any changes made to the icon files requires that CodeXL is rebuilt so that the project's Qt .xpm files are re-built.


Skeleton Plugin Visual Studio Project
-------------------------------------
An additional CCVG properties file needs to included in the solution. Copy LPGPU2_CCVGCodeCoverage.props found in:

  CodeXL\AMDTApplicationFramework

to the equivalent directory to sit along side the other propertie sheet definitions.
  
The project should be added to the CodeXL-AllProjects solution (.sln) and may be made a dependency of the startup project AMDCodeXL.

Note when building the project be sure to rebuild the 32 bit version even if working on the 64 bit version.
  