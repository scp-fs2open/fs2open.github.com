
class BuildConfig {
	[string]$Generator
	[string]$PackageType
	[string]$Toolset
	[string]$SimdType
	[string]$QtDir
	[Bool]$SourcePackage
}

$NightlyConfigurations = @(
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015 Win64";
#		PackageType="Win64";
#		Toolset="v140";
#		SimdType="SSE2";
#		QtDir="C:\Qt\5.9\msvc2015_64";
#		SourcePackage=$false;
#	},
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015";
#		PackageType="Win32";
#		Toolset="v140";
#		SimdType="SSE2";
#		QtDir="C:\Qt\5.9\msvc2015";
#		SourcePackage=$false;
#	}
)
$ReleaseConfigurations = @(
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015";
#		PackageType="Win32";
#		Toolset="v140";
#		SimdType="SSE2";
#		QtDir="C:\Qt\5.9\msvc2015";
#		SourcePackage=$true;
#	}
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015";
#		PackageType="Win32-AVX";
#		Toolset="v140";
#		SimdType="AVX";
#		QtDir="C:\Qt\5.9\msvc2015";
#		SourcePackage=$false;
#	}
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015 Win64";
#		PackageType="Win64";
#		Toolset="v140";
#		SimdType="SSE2";
#		QtDir="C:\Qt\5.9\msvc2015_64";
#		SourcePackage=$false;
#	}
#	[BuildConfig]@{
#		Generator="Visual Studio 14 2015 Win64";
#		PackageType="Win64-AVX";
#		Toolset="v140";
#		SimdType="AVX";
#		QtDir="C:\Qt\5.9\msvc2015_64";
#		SourcePackage=$false;
#	}
)

$BuildConfigurations = $null

# Default values
$ReleaseBuild = $false
$NightlyBuild = $false
$TestBuild = $false

# Set to true to test either the release or the nightly part of this script
$ReleaseTest = $false
$NightlyTest = $false
$TestBuildTest = $false

if ($ReleaseTest -Or ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release_(.*)"))) {
    # Tag matches
    $ReleaseBuild = $true
    $PackageName = "fs2_open_$($matches[1])"
	$BuildConfigurations = $ReleaseConfigurations
}

if ($NightlyTest -Or ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^nightly_(.*)"))) {
    # Tag matches
    $NightlyBuild = $true
    $PackageName = "nightly_$($matches[1])"
	Set-AppveyorBuildVariable 'VersionName' "$($matches[1])"
	$BuildConfigurations = $NightlyConfigurations
}

if ($TestBuildTest -Or ([System.Convert]::ToInt32($env:APPVEYOR_PULL_REQUEST_NUMBER) -le 0 -And ("$env:APPVEYOR_REPO_BRANCH" -match "^test\/(.*)"))) {
    # Tag matches
    $TestBuild = $true
    $PackageName = "test_$($matches[1])"
	Set-AppveyorBuildVariable 'VersionName' "$($matches[1])"
	$BuildConfigurations = $NightlyConfigurations

    # Override the revision string so that the builds are named correctly
	[System.IO.File]::WriteAllLines("$env:APPVEYOR_BUILD_FOLDER/version_override.cmake", "set(FSO_VERSION_REVISION_STR $env:VersionName)")
}

# Multiply by 2 so that we can add 0 or 1 for debug or release Configs
$buildID = [convert]::ToInt32($env:BuildID) * 2
if ($Env:Configuration -eq "Release") {
    $buildID = $buildID + 1
}

$DeployBuild = $false
if ($ReleaseBuild -Or $NightlyBuild -Or $TestBuild) {
    $DeployBuild = $true
}

Set-AppveyorBuildVariable 'ReleaseBuild' "$ReleaseBuild"
Set-AppveyorBuildVariable 'NightlyBuild' "$NightlyBuild"
Set-AppveyorBuildVariable 'TestBuild' "$TestBuild"

New-Item build -type directory
Set-Location -Path build

if ($DeployBuild) {
	# Release build
	if ($buildID -ge $BuildConfigurations.Length) {
		# Not needed
        exit 0
	}

	$buildConfig = $BuildConfigurations[$buildID]

	if ($buildConfig.SourcePackage -eq $True) {
		7z a -xr'!.git' "$($PackageName)-source-Win.zip" "$env:APPVEYOR_BUILD_FOLDER"
		Push-AppveyorArtifact "$($PackageName)-source-Win.zip"
	}
	
	cmake -DCMAKE_INSTALL_PREFIX="$env:APPVEYOR_BUILD_FOLDER/../install" -DFSO_USE_SPEECH="ON" `
		-DFSO_USE_VOICEREC="ON" -DMSVC_SIMD_INSTRUCTIONS="$($buildConfig.SimdType)" `
		-DQT5_INSTALL_ROOT="$($buildConfig.QtDir)" -DFSO_BUILD_QTFRED=OFF `
		-DFSO_INSTALL_DEBUG_FILES="ON" `
		-G "$($buildConfig.Generator)" -T "$($buildConfig.Toolset)" ..

	$Configs = @("Release", "FastDebug")
	foreach ($config in $Configs) {
		cmake --build . --config "$config" --target INSTALL -- /verbosity:minimal
		if (! ($?)) {
			Add-AppveyorMessage "A release build failed!"
			exit 1
    	}
	}

    7z a -xr'!*.pdb' "$($PackageName)-builds-$($buildConfig.PackageType).zip" "$env:APPVEYOR_BUILD_FOLDER/../install/*"
    Push-AppveyorArtifact "$($PackageName)-builds-$($buildConfig.PackageType).zip"

	7z a "$($PackageName)-debug-$($buildConfig.PackageType).7z" "$env:APPVEYOR_BUILD_FOLDER/../install/*.pdb"
	Push-AppveyorArtifact "$($PackageName)-debug-$($buildConfig.PackageType).7z"
} else {
	cmake -DFSO_USE_SPEECH="ON" -DFSO_FATAL_WARNINGS="ON" -DFSO_USE_VOICEREC="ON" -DFSO_BUILD_TESTS="ON" -DMSVC_SIMD_INSTRUCTIONS=SSE2 `
	-DFSO_BUILD_QTFRED=OFF -DQT5_INSTALL_ROOT="$env:QT_DIR" -DFSO_BUILD_FRED2="ON" `
	-G "$Env:CMAKE_GENERATOR" -T "$Env:PlatformToolset" ..

    cmake --build . --config "$Env:CONFIGURATION" -- /verbosity:minimal

    if (! ($?)) {
        exit 1
    }
}
