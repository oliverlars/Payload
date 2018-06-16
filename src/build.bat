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

@set LinkLibs= ^
	/libpath:"%VSBasePath%\lib\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
	/libpath:"C:\Program Files\LLVM\lib" ^
	/libpath:"..\lib\embree3\lib" ^
	/libpath:"C:\Python35\libs" ^
	libcmt.lib User32.lib embree3.lib

@ctime -begin payload.ctm
@echo Compiling...
@clang++ payload.cc -o ..\build\payload.o -c %CompilerFlags%

@lld-link "..\build\payload.o" -out:"%OUT%" %LinkFlags% %LinkLibs%
@echo Done
@ctime -end payload.ctm