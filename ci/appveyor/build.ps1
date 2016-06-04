
if ([System.Convert]::ToBoolean($env:DeployBuild)) {
    # Release build
    if (! ([System.Convert]::ToBoolean($env:DeployConfig))) {
        exit 0 # End the build
    }
    
    "Test" > "$($env:PackageName)-builds-Win32.zip"
    Push-AppveyorArtifact "$($env:PackageName)-builds-Win32.zip"
    exit 0

    msbuild "$env:ProjectPath/Freespace2.sln" /p:Configuration="Debug SSE2" /m /p:PlatformToolset="$($env:PlatformToolset)_xp" /verbosity:minimal
    if (! ($?)) {
        Add-AppveyorMessage "A release build failed!"
        exit 1
    }

    msbuild "$env:ProjectPath/Freespace2.sln" /p:Configuration="Release SSE2" /m /p:PlatformToolset="$($env:PlatformToolset)_xp" /verbosity:minimal
    if (! ($?)) {
        Add-AppveyorMessage "A release build failed!"
        exit 1
    }

    7z a "$($env:PackageName)-builds-Win32.zip" "$env:APPVEYOR_BUILD_FOLDER/$env:ProjectPath/Debug SSE2/*.exe" "$env:APPVEYOR_BUILD_FOLDER/$env:ProjectPath/Release SSE2/*.exe"
    Push-AppveyorArtifact "$($env:PackageName)-builds-Win32.zip"
} else {
    # Standard CI build
    msbuild "$env:ProjectPath/Freespace2.vcxproj" /p:Configuration="Debug SSE2" /m /p:PlatformToolset="$env:PlatformToolset" /verbosity:minimal
}
