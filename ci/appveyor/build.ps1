
write "$env:DeployBuild"
write "$env:DeployConfig"

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

    7z a "$($env:PackageName)-builds-Win32.zip" "$env:APPVEYOR_BUILD_FOLDER/../install/*"
    Push-AppveyorArtifact "$($env:PackageName)-builds-Win32.zip"
} else {
    cmake --build . --config "$Env:CONFIGURATION" -- /verbosity:minimal

    if (! ($?)) {
        exit 1
    }
}
