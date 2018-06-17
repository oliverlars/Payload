@set "LLVMPath=C:\Program Files\LLVM"
@set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community"
@set "VSVersion=14.10.25017"
@set "WinSDKVersion=10.0.14393.0"
@set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
@set "OUT=..\build\out\Payload.exe"
@set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
@set "PATH=%PATH%;%LLVMPath%\bin;%VSBasePath%\bin\HostX64\x64"

@set CompilerFlags= ^
	-std=c++14 ^
	-Wno-everything ^
	-I "..\lib"

@set LinkFlags= ^
	-machine:x64 ^
	-subsystem:console

@set DLLLibs= ^
	/libpath:"%VSBasePath%\lib\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
	/libpath:"C:\Program Files\LLVM\lib" ^
	/libpath:"..\lib\embree3\lib" ^
	libcmt.lib User32.lib embree3.lib


@set ExeLibs= ^
	/libpath:"%VSBasePath%\lib\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
	/libpath:"C:\Program Files\LLVM\lib" ^
	libcmt.lib User32.lib

@ctime -begin payload.ctm
@echo Compiling...
@clang++ win32_payload.cc -o ..\build\payload.o -c %CompilerFlags%

@clang++ payload.cc -o ..\build\payloaddll.o -c %CompilerFlags%
@link /DLL "..\build\payloaddll.o" -out:"..\build\out\Payload.dll" %LinkFlags% %DLLLibs%


@link "..\build\payload.o" -out:"%OUT%" %LinkFlags% %ExeLibs%
@echo Done
@ctime -end payload.ctm