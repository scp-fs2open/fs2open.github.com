
if ([System.Convert]::ToBoolean($env:ReleaseBuild) -Or [System.Convert]::ToBoolean($env:NightlyBuild)) {
    # Skip tests for deployment
    exit 0
} else {
    # Appveyor doesn't have OpenAL so copy our DLLs to the output FOLDER
    $arch=""
    if ($Env:CMAKE_GENERATOR -Match "Win64") {
        $arch="win64"
    } else {
        $arch="win32"
    }
    $dllPath = "$Env:APPVEYOR_BUILD_FOLDER\lib\openal\libs\$arch\OpenAL32.dll"
    $copyDestination = "$pwd\bin\$Env:CONFIGURATION\"
    
    write "Copying OpenAL DLL..."
    Copy-Item "$dllPath" "$copyDestination"
    
    write "Running unit tests..."
    # Run unit tests
    &"$pwd\bin\$Env:CONFIGURATION\unittests.exe" --gtest_shuffle 2>&1

    if (! ($?)) {
        exit 1
    }
}
