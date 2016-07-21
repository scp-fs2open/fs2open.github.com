
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
