
if ($env:ReleaseBuild) {
    # Release build
    if (! ($env:ReleaseConfig)) {
        Add-AppveyorMessage "This build will fail because we are doing a release build but this is not the right configuration."
        exit 1 # Fail the build
    }

    msbuild "$env:ProjectPath" /p:Configuration="Debug SSE2" /m /p:PlatformToolset="$env:PlatformToolset-xp" /verbosity:minimal

    if (! ($?)) {
        Add-AppveyorMessage "A release build failed!"
        exit 1
    }

    msbuild "$env:ProjectPath" /p:Configuration="Release SSE2" /m /p:PlatformToolset="$env:PlatformToolset-xp" /verbosity:minimal

    if (! ($?)) {
        Add-AppveyorMessage "A release build failed!"
        exit 1
    }
} else {
    # Standard CI build
    msbuild "$env:ProjectPath" /p:Configuration="Debug SSE2" /m /p:PlatformToolset="$env:PlatformToolset" /verbosity:minimal
}
