
write "$env:ReleaseBuild"
write "$env:ReleaseConfig"

if ([System.Convert]::ToBoolean($env:ReleaseBuild)) {
    # Release build
    if (! ([System.Convert]::ToBoolean($env:ReleaseConfig))) {
        Add-AppveyorMessage "This build will fail because we are doing a release build but this is not the right configuration."
        exit 1 # Fail the build
    }

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

    7z a builds-Win32.zip "$env:APPVEYOR_BUILD_FOLDER/$env:ProjectPath/Debug SSE2/*.exe" "$env:APPVEYOR_BUILD_FOLDER/$env:ProjectPath/Release SSE2/*.exe"
} else {
    # Standard CI build
    msbuild "$env:ProjectPath/Freespace2.vcxproj" /p:Configuration="Debug SSE2" /m /p:PlatformToolset="$env:PlatformToolset" /verbosity:minimal
}
