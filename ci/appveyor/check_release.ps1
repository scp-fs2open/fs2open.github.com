
write "$env:APPVEYOR_REPO_TAG"
write "$env:APPVEYOR_REPO_TAG_NAME"

# Default values
Set-AppveyorBuildVariable 'ReleaseBuild' 'false'
Set-AppveyorBuildVariable 'NightlyBuild' 'false'
Set-AppveyorBuildVariable 'DeployBuild' 'false'

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release.*")) {
    # Tag matches
    Set-AppveyorBuildVariable 'ReleaseBuild' 'true'
}

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^nightly.*")) {
    # Tag matches
    Set-AppveyorBuildVariable 'NightlyBuild' 'true'
}
Set-AppveyorBuildVariable 'NightlyBuild' 'true'

if ([System.Convert]::ToBoolean($env:ReleaseBuild) -Or [System.Convert]::ToBoolean($env:NightlyBuild)) {
    Set-AppveyorBuildVariable 'DeployBuild' 'true'
}
