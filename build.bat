
@ECHO OFF
SETLOCAL
    SET command_flag=false
    IF "%1"==""                     SET command_flag=true & CALL:funBuild Build
    IF "%1"=="build"                SET command_flag=true & CALL:funBuild Build
    IF "%1"=="rebuild"              SET command_flag=true & CALL:funBuild ReBuild

    IF %command_flag%==false ECHO "no such command!"
ENDLOCAL

pause

:: end
GOTO:EOF


:funBuild
SETLOCAL
    SET SolutionFile=dd.sln
    set Platforms=x64;x86
    set Configurations=Debug;Debug_Mdd;Release;Release_Md
    set BuildType=%~1

    :: 寻找最新的MSBuild.exe
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do set "VSPath=%%i"
    set MSBuildCommand="%VSPath%\MSBuild\Current\Bin\MSBuild.exe"
    for %%p in (%Platforms%) do (
        for %%c in (%Configurations%) do (
            echo Building %SolutionFile% for %%c - %%p...
            echo %MSBuildCommand% %SolutionFile% /t:%BuildType% /p:Configuration=%%c /p:Platform=%%p /m
            %MSBuildCommand% %SolutionFile% /t:%BuildType% /p:Configuration=%%c /p:Platform=%%p /m
            if %errorlevel% equ 0 (
                echo Build for %%c - %%p succeeded.
            ) else (
                echo Build for %%c - %%p failed.
            )
        )
    )
ENDLOCAL
GOTO:EOF
