
@echo off

SETLOCAL

SET OUT_DIR=build
SET FLAGS=/EHsc /O2 /Ferush_park.exe
SET INCLUDE_DIRS=/I"..\libs\DX\Include" /I"..\libs\imgui"
SET LIBS=user32.lib gdi32.lib shell32.lib Ws2_32.lib ..\libs\DX\Lib\x86\dxgi.lib ..\libs\DX\Lib\x86\d3d11.lib ..\libs\DX\Lib\x86\d3dx11.lib ..\libs\DX\Lib\x86\d3dx10.lib
REM SET SOURCE=..\source\platform_win32\compiler_translation_unit.cpp ..\libs\imgui\imgui.cpp ..\libs\imgui\imgui_demo.cpp ..\libs\imgui\imgui_draw.cpp ..\libs\imgui\imgui_impl_dx11.cpp ..\libs\imgui\imgui_impl_win32.cpp ..\libs\imgui\imgui_widgets.cpp
SET SOURCE=..\source\platform_win32\compiler_translation_unit.cpp

pushd .
cd build
REM cl %FlAGS% %INCLUDE_DIRS% %SOURCE% %LIBS% > build_log.txt
REM color_output.exe build_log.txt
cl %FlAGS% %INCLUDE_DIRS% %SOURCE% %LIBS%
popd

xcopy /y build\rush_park.exe .

