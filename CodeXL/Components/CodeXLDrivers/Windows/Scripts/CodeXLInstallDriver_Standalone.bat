::
:: Script used to install CodeXL power profiling services and drivers
::
:: If CodeXL is not installed, this script can be executed to installed
:: the required driver libraries for Cpu and Power profiling.    
::
@echo off
setlocal enableextensions

set interactive=0
set install_debug=0

if "%1"=="--help" (
    goto :print_help    
)

if "%1"=="--interactive" (
    set interactive=1
)

if "%1"=="--install-debug" (
    set install_debug=1
)

:: to execute this script admin rights are required
goto verify_admin_privledge

:print_help
    echo.
    echo Install CodeXL Power Proling drivers from the build folders    
    echo.
    echo Usage CodeXLInstallDriver_Standalone.bat [--help] [--interactive]
    echo.
    echo    --help           Display this help message
    echo    --interactive    Stops the execution in strategic points useful for debugging the script
    echo    --install-debug  Uses the debug version of the executable CodeXLDriversLoadService. 
                             Useful for debugging the service with Visual Studio "Attach to process" capability.
    goto :EOF

:verify_admin_privledge
    echo.
    echo Script needs to be executed in admnin mode, detecting adminstrative permissions...    

    net session >nul 2>&1
    if %errorLevel% == 0 (
        echo Adminstrative permissions confirmed.        
        if %interactive%==1 pause
        goto stop_services
    ) else (
        echo Exiting, please open the command prompt with Administrative permissions and rerun this script.        
        if %interactive%==1 pause else pause >nul
        goto :wait_and_exit_with_error
    )

:stop_services
	echo.
	echo.
	echo Stopping CodeXLDriversLoadService service...
    echo.

	sc stop CodeXLDriversLoadService >nul 2>&1
	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService service stoppped.
	) else 	(
		echo CodeXLDriversLoadService has not been started.
	)

	echo Deleting CodeXLDriversLoadService service...
	sc delete CodeXLDriversLoadService >nul 2>&1
	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService service deleted.
	) else 	(
		echo CodeXLDriversLoadService does not exists as an installed service
	)

	echo Stopping CpuProf service...
	sc stop cpuprof >nul 2>&1
	if %errorLevel% == 0 (
		echo CpuProf service stoppped.
	) else 	(
		echo CpuProf services has not been started.
	)

	echo Stopping AmdtPwrProf service...
	sc stop amdtpwrprof >nul 2>&1
	if %errorLevel% == 0 (
		echo AmdtPwrProf service stoppped.
	) else 	(
		echo AmdtPwrProf services has not been started.
	)

	echo Stopping pcore service...
	sc stop pcore >nul 2>&1
	if %errorLevel% == 0 (
		echo pcore service stoppped.
	) else 	(
		echo pcore services has not been started.
	)

    if %interactive%==1 pause

	goto delete_services

:delete_services
    echo.
    echo.
    echo Deleting CpuProf service...
    sc delete cpuprof >nul 2>&1
    if %errorLevel% == 0 (
        echo CpuProf service deleted.
    ) else 	(
        echo Cpuprof does not exists as an installed service.
    )

    echo Deleting AmdtPwrProf service...
    sc delete amdtpwrprof >nul 2>&1
    if %errorLevel% == 0 (
        echo AmdtPwrProf service deleted.
    ) else 	(
        echo AmdtPwrProf does not exists as an installed service
    )

    echo Deleting Pcore service...
    sc delete pcore >nul 2>&1
    if %errorLevel% == 0 (
        echo Pcore service deleted.
    ) else 	(
        echo Pcore does not exists as an installed service
    )

    if %interactive%==1 pause

    goto delete_files

: delete_files 
   	echo.
   	echo.
   	echo Deleting installed service files from C:\Windows\System32\drivers directory
	
    set folder=""%Systemroot%\System32\drivers"
	pushd %folder% 
	del /s CpuProf.*
	del /s AMDTPwrProf.*
	del /s pcore.*
	popd

    if %interactive%==1 pause

	goto copy_libs

:copy_libs
   	echo.
   	echo.
   	echo Copying requireds system file to C:\Windows\System32\drivers

    echo Detecting OS version ...
	reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86"  && set OS=32BIT || set OS=64BIT > nul 2>&1
	set path=..\lib
	set folder="%Systemroot%\System32\drivers"
	
	if %OS%==32BIT (
		set release=x86
	) 
	if %OS%==64BIT (
		set release=x64
	)

    echo OS version detected: %release%    
   
   	set complete_path=%path%\%release%   	

   	pushd %complete_path%
	copy CpuProf.* >nul %folder%\.
	copy AMDTPwrProf.* %folder%\.
	copy pcore.* %folder%\.
   	popd

    if %interactive%==1 pause

   	goto create_service 

:create_service
    echo.
    echo.
    echo Creating and starting CodeXLDriversLoadService which will creates and start pcore, amdtpwrprof and cpuprof services.

	set system32=""%Systemroot%\System32"
	
	pushd %system32%
	set script_path=%~dp0
    
    set build_config=Release

    if %install_debug%==1 set build_config=Debug
    
    set output_folder_relative=%script_path%..\..\..\..\Output\%build_config%\bin

    :: Finds out the absolute path of the output folder
    set output_folder=

    :: 1. Save current directory and change to target directory
    pushd %output_folder_relative%

    :: 2. Save the value of CD variable (current directory)
    set output_folder=%cd%

    :: 3. Restore original directory
    popd    

    :: Derive the name of the executable based on the OS version
    if %release%==x64 (
        set service_executable=%output_folder%\CodeXLDriversLoadService-x64.exe
    ) else (
        set service_executable=%output_folder%\CodeXLDriversLoadService.exe
    )

    if not exist %service_executable% (
        echo.
        echo ERROR: The file "%service_executable%" was not found. Did you build the whole solution?
        goto :wait_and_exit_with_error
    )

	sc create CodeXLDriversLoadService type= own binPath= %service_executable% displayName= "CodeXL Drivers Loader Service [%build_config%]"
    sc description CodeXLDriversLoadService "This is part of the CodeXL suite of tools to perform Power Profiling in local and remote applications. This services is used by CodeXL to load the actual drivers that perform the profiling. Currently there are the following drivers available: CpuProf, AMDTPwrProf and pcore."
	sc start CodeXLDriversLoadService
	popd

    if %interactive%==1 pause

	goto verify_service

:verify_service
	echo.
	echo.
	echo Verifing CodeXLDriversLoadService, pcore, amdtpwrprof and cpuprof services....
	set ret=0
	pushd %system32%

    :: Give time for the services to start running
	timeout /t 5
	sc query CodeXLDriversLoadService | findstr "STATE" | findstr "RUNNING"  >nul 2>&1

	if %errorLevel% == 0 (
		echo CodeXLDriversLoadService is running.
	) else 	(
		call :service_failed CodeXLDriversLoadService
	)  

	sc query pcore | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo pcore is running.
	) else 	(
		call :service_failed pcore
	)  

	sc query amdtpwrprof | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo amdtpwrprof is running.
	) else 	(
		call :service_failed amdtpwrprof 
	)  

	sc query cpuprof | findstr "STATE" | findstr "RUNNING" >nul 2>&1
	if %errorLevel% == 0 (
		echo cpuprof is running.
	) else 	(
		call :service_failed cpuprof
	)  

    goto :wait_and_exit_with_success

:service_failed
    echo.
    echo %* is not running. Rebooting your machine may help.
    popd
    goto :wait_and_exit_with_error

:wait_and_exit_with_success
    echo.
    echo SUCCESS: CodeXLDriversLoadService and drivers were installed successfully
    goto :wait_and_exit

:wait_and_exit_with_error
    echo.
    echo ERROR: Something went wrong during the installation
    goto :wait_and_exit

:wait_and_exit
    echo.
    echo This script will be closed shortly
    timeout /t 7
    exit /B 0
	