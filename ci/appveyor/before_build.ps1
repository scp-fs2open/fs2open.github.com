New-Item build -type directory
Set-Location -Path build

$AdditionalFeatures="ON"
if ($Env:VS_VERSION -eq "10") {
    $AdditionalFeatures="OFF"
}

cmake -DFSO_BUILD_FRED2="${AdditionalFeatures}" -DFSO_USE_SPEECH="${AdditionalFeatures}" `
	-DCMAKE_INSTALL_PREFIX="$env:APPVEYOR_BUILD_FOLDER/../install" `
	-DFSO_USE_VOICEREC="${AdditionalFeatures}" -DMSVC_SIMD_INSTRUCTIONS=SSE2 -G "$Env:CMAKE_GENERATOR" -T "$Env:PlatformToolset" ..