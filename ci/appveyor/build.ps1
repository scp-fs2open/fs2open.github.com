
class BuildConfig {
	[string]$Generator
	[string]$PackageType
	[string]$Toolset
	[string]$SimdType
	[string]$QtDir
}

$NightlyConfigurations = @(
	[BuildConfig]@{ 
		Generator="Visual Studio 14 2015 Win64";
		PackageType="Win64";
		Toolset="v140_xp";
		SimdType="SSE2";
		QtDir="C:\Qt\5.7\msvc2015_64";
	},
	[BuildConfig]@{ 
		Generator="Visual Studio 14 2015";
		PackageType="Win32";
		Toolset="v140_xp";
		SimdType="SSE2";
		QtDir="C:\Qt\5.7\msvc2015";
	}
)
$ReleaseConfigurations = @(
	[BuildConfig]@{
		Generator="Visual Studio 14 2015";
		PackageType="Win32";
		Toolset="v140_xp";
		SimdType="SSE2";
		QtDir="C:\Qt\5.7\msvc2015";
	}
	[BuildConfig]@{
		Generator="Visual Studio 14 2015";
		PackageType="Win32-AVX";
		Toolset="v140_xp";
		SimdType="AVX";
		QtDir="C:\Qt\5.7\msvc2015";
	}
	[BuildConfig]@{
		Generator="Visual Studio 14 2015 Win64";
		PackageType="Win64";
		Toolset="v140_xp";
		SimdType="SSE2";
		QtDir="C:\Qt\5.7\msvc2015_64";
	}
	[BuildConfig]@{
		Generator="Visual Studio 14 2015 Win64";
		PackageType="Win64-AVX";
		Toolset="v140_xp";
		SimdType="AVX";
		QtDir="C:\Qt\5.7\msvc2015_64";
	}
)

$BuildConfigurations = $null

# Default values
$ReleaseBuild = $false
$NightlyBuild = $false
$DeployBuild = $false

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release_(.*)")) {
    # Tag matches
    $ReleaseBuild = $true
    $PackageName = "fs2_open_$($matches[1])"
	$BuildConfigurations = $ReleaseConfigurations
}

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^nightly_(.*)")) {
    # Tag matches
    $NightlyBuild = $true
    $PackageName = "nightly_$($matches[1])"
	Set-AppveyorBuildVariable 'VersionName' "$($matches[1])"
	$BuildConfigurations = $NightlyConfigurations
}

# Multiply by 2 so that we can add 0 or 1 for debug or release Configs
$buildID = [convert]::ToInt32($env:BuildID) * 2
if ($Env:Configuration -eq "Release") {
    $buildID = $buildID + 1
}
Write-Host "$buildID"

if ($ReleaseBuild -Or $NightlyBuild) {
    $DeployBuild = $true
}

Set-AppveyorBuildVariable 'ReleaseBuild' "$ReleaseBuild"
Set-AppveyorBuildVariable 'NightlyBuild' "$NightlyBuild"

New-Item build -type directory
Set-Location -Path build

if ($DeployBuild) {
	# Release build
	if ($buildID -ge $BuildConfigurations.Length) {
		# Not needed
        exit 0
	}

	$buildConfig = $BuildConfigurations[$buildID]
	
	cmake -DCMAKE_INSTALL_PREFIX="$env:APPVEYOR_BUILD_FOLDER/../install" -DFSO_USE_SPEECH="ON" `
		-DFSO_USE_VOICEREC="ON" -DMSVC_SIMD_INSTRUCTIONS="$($buildConfig.SimdType)" -DFSO_BUILD_FRED2="OFF" `
		-DFSO_BUILD_QTFRED=ON -DQT5_INSTALL_ROOT="$($buildConfig.QtDir)" -DMSVC_USE_RUNTIME_DLL="ON" `
		-G "$($buildConfig.Generator)" -T "$($buildConfig.Toolset)" ..

	$Configs = @("Release", "FastDebug")
	foreach ($config in $Configs) {
		cmake --build . --config "$config" --target INSTALL -- /verbosity:minimal
		if (! ($?)) {
			Add-AppveyorMessage "A release build failed!"
			exit 1
    	}
	}

    7z a "$($PackageName)-builds-$($buildConfig.PackageType).zip" "$env:APPVEYOR_BUILD_FOLDER/../install/*"
    Push-AppveyorArtifact "$($PackageName)-builds-$($buildConfig.PackageType).zip"
} else {
	cmake -DFSO_USE_SPEECH="ON" -DFSO_FATAL_WARNINGS="ON" -DFSO_USE_VOICEREC="ON" -DFSO_BUILD_TESTS="ON" -DMSVC_SIMD_INSTRUCTIONS=SSE2 `
	-DFSO_BUILD_QTFRED=ON -DQT5_INSTALL_ROOT="$env:QT_DIR" -DMSVC_USE_RUNTIME_DLL="ON" -DFSO_BUILD_FRED2="OFF" `
	-G "$Env:CMAKE_GENERATOR" -T "$Env:PlatformToolset" ..

    cmake --build . --config "$Env:CONFIGURATION" -- /verbosity:minimal

    if (! ($?)) {
        exit 1
    }
}
