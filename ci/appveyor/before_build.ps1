New-Item build -type directory
Set-Location -Path build

cmake -DCMAKE_INSTALL_PREFIX="$env:APPVEYOR_BUILD_FOLDER/../install" -DFSO_USE_SPEECH="ON" -DFSO_FATAL_WARNINGS="ON" `
	-DFSO_USE_VOICEREC="ON" -DMSVC_SIMD_INSTRUCTIONS=SSE2 -G "$Env:CMAKE_GENERATOR" -T "$Env:PlatformToolset" ..