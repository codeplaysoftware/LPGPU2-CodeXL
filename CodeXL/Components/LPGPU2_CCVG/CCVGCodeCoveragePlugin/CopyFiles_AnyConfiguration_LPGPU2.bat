@echo off

rem ccvg_files 0 = do not copy, 1 = do copy files
set ccvg_files=%1

rem CONFIG_NAME = output bin debug or release directory filename
set CONFIG_NAME=%2

set OUTPUT_FOLDER=%~dp0..\..\..\Output

if not exist %OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML mkdir %OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML
 
rem Copy welcome page resources
if %ccvg_files%==1 (
   rem echo Copying LPGPU2 CCVG execution mode files...   
   XCopy /r /d /y "res\welcome\LPGPU2_CCVG_new.png" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\StartupPage_CodeXLLogo_LPGPU2.png" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\WelcomeAnalyzeNALPGPU2CCVG.html" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\WelcomeLPGPU2CCVG.html" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\WelcomeLPGPU2CCVGOnly.html" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\welcomepage_LPGPU2_CCVG.png" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\welcomepage_LPGPU2_CCVG_large.png" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
   XCopy /r /d /y "res\welcome\welcomepage_LPGPU2_CCVG_selected.png" "%OUTPUT_FOLDER%\%CONFIG_NAME%\bin\HTML\"
)
rem echo Done copying LPGPU2 CCVG execution mode files
