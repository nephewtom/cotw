call clean.bat

@echo off
set RAYLIB_DIR= ../raylib-quickstart/build/external/raylib-master/src/
set PDB_FILE=%OBJ_DIR%/vc142.pdb
set OUT_EXE=./cotw-vc.exe
:: set ILK_FILE=../raylib-quickstart/build-vc/build_files/obj/x64/Debug/raylib-quickstart/raylib-quickstart.ilk
set LIB_DIR=../raylib-quickstart/bin/Debug
set OUT_PDB=%LIB_DIR%/raylib-quickstart.pdb
set OUT_LIB=%LIB_DIR%/raylib-quickstart.lib
set OBJ_DIR=./
set MAIN_OBJ=%OBJ_DIR%/cotw.obj

@echo on

cl.exe /std:c++20 /TP ^
    /I%RAYLIB_DIR% ^
    /nologo /ZI /JMC /W3 /WX- /diagnostics:column /Od ^
    /D DEBUG /D PLATFORM_DESKTOP /D GRAPHICS_API_OPENGL_33 ^
    /D _WINSOCK_DEPRECATED_NO_WARNINGS /D _CRT_SECURE_NO_WARNINGS ^
    /D _WIN32 /D _UNICODE /D UNICODE ^
    /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t ^
    /Zc:forScope /Zc:inline ^
    /Fd.\cotw-vc.pdb ^
    cotw.cpp ^
    /link /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT ^
    /OUT:cotw-vc.exe /LIBPATH:../raylib-quickstart/bin/Debug ^
    raylib.lib winmm.lib gdi32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib ^
    winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
    
:: /external:W3 /Gd /TC /FC /errorReport:queue /Zc:__cplusplus ^

:: link.exe  /ERRORREPORT:QUEUE ^
::     /OUT:%OUT_EXE% ^
::     /NOLOGO ^
::     /LIBPATH:%LIB_DIR% ^
::     raylib.lib winmm.lib gdi32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ^
::     /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed ^
::     /DEBUG /PDB:%OUT_PDB% /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT ^
::     /IMPLIB:%OUT_LIB% /MACHINE:X64 %MAIN_OBJ%

    :: /INCREMENTAL /ILK:%ILK_FILE% ^

:: cl.exe /nologo /ZI /JMC /nologo /W3 /WX- /diagnostics:column /Od /D DEBUG /D PLATFORM_DESKTOP /D GRAPHICS_API_OPENGL_33 /D _WINSOCK_DEPRECATED_NO_WARNINGS /D _CRT_SECURE_NO_WARNINGS /D _WIN32 /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline cotw.cpp -I ..\raylib-quickstart\build\external\raylib-master\src /std:c++20 /TP /link /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT /OUT:cotw-vc.exe /LIBPATH:../raylib-quickstart/bin/Debug raylib.lib winmm.lib gdi32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib 
