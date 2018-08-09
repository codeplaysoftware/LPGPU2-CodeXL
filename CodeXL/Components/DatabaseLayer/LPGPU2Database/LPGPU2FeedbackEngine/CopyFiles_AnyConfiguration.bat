@echo off

rem CONFIG_NAME = output bin Debug or Release directory filename
set CONFIG_NAME=%1

set OUTPUT_FOLDER=%~dp0..\..\..\..\Output

if not exist %OUTPUT_FOLDER%\%CONFIG_NAME%\bin\LPGPU2_scripts mkdir %OUTPUT_FOLDER%\%CONFIG_NAME%\bin\LPGPU2_scripts 

echo "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\LPGPU2_scripts"
XCopy /r /d /y "*.lua" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\LPGPU2_scripts"
