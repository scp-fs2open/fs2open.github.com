
# Default values
Set-AppveyorBuildVariable 'ReleaseBuild' 'false'
Set-AppveyorBuildVariable 'NightlyBuild' 'false'
Set-AppveyorBuildVariable 'DeployBuild' 'false'
Set-AppveyorBuildVariable 'PackagePrefix' ''
Set-AppveyorBuildVariable 'VersionName' ''

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release_(.*)")) {
    # Tag matches
    Set-AppveyorBuildVariable 'ReleaseBuild' 'true'
    Set-AppveyorBuildVariable 'PackageName' "fs2_open_$($matches[1])"
}

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^nightly_(.*)")) {
    # Tag matches
    Set-AppveyorBuildVariable 'NightlyBuild' 'true'
    Set-AppveyorBuildVariable 'PackageName' "nightly_$($matches[1])"
	Set-AppveyorBuildVariable 'VersionName' "$($matches[1])"
}

if (($Env:Configuration -eq "Release") -And ($Env:VS_VERSION -eq "14")) {
    Set-AppveyorBuildVariable 'DeployConfig' 'true'
} else {
    Set-AppveyorBuildVariable 'DeployConfig' 'false'
}

if ([System.Convert]::ToBoolean($env:ReleaseBuild) -Or [System.Convert]::ToBoolean($env:NightlyBuild)) {
    Set-AppveyorBuildVariable 'DeployBuild' 'true'
}

New-Item build -type directory
Set-Location -Path build

if ([System.Convert]::ToBoolean($env:DeployBuild)) {
	# Release build
	if (![System.Convert]::ToBoolean($env:DeployConfig)) {
        exit 0
	}
	
	cmake -DCMAKE_INSTALL_PREFIX="$env:APPVEYOR_BUILD_FOLDER/../install" -DFSO_USE_SPEECH="ON" `
		-DFSO_USE_VOICEREC="ON" -DMSVC_SIMD_INSTRUCTIONS=SSE2 -G "Visual Studio 14 2015 Win64" -T "$Env:PlatformToolset" ..
} else {
	cmake -DFSO_USE_SPEECH="ON" -DFSO_FATAL_WARNINGS="ON" -DFSO_USE_VOICEREC="ON" -DMSVC_SIMD_INSTRUCTIONS=SSE2 `
		-G "$Env:CMAKE_GENERATOR" -T "$Env:PlatformToolset" ..
}

if ([System.Convert]::ToBoolean($env:DeployBuild)) {
	# Release build
	if (![System.Convert]::ToBoolean($env:DeployConfig)) {
        exit 0 # End the build
	}

	$Configs = @("Release", "FastDebug")
	foreach ($config in $Configs) {
		cmake --build . --config "$config" --target INSTALL -- /verbosity:minimal
		if (! ($?)) {
			Add-AppveyorMessage "A release build failed!"
			exit 1
    	}
	}

    7z a "$($env:PackageName)-builds-Win64.zip" "$env:APPVEYOR_BUILD_FOLDER/../install/*"
    Push-AppveyorArtifact "$($env:PackageName)-builds-Win64.zip"
} else {
    cmake --build . --config "$Env:CONFIGURATION" -- /verbosity:minimal

    if (! ($?)) {
        exit 1
    }
}
