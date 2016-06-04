
write "$env:APPVEYOR_REPO_TAG"
write "$env:APPVEYOR_REPO_TAG_NAME"

if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release.*")) {
    # Tag matches
    Set-AppveyorBuildVariable 'ReleaseBuild' 'true'
} else {
    # Standard CI build
    Set-AppveyorBuildVariable 'ReleaseBuild' 'false'
}
if ([System.Convert]::ToBoolean($env:APPVEYOR_REPO_TAG) -And ("$env:APPVEYOR_REPO_TAG_NAME" -match "^nightly.*")) {
    # Tag matches
    Set-AppveyorBuildVariable 'NightlyBuild' 'true'
} else {
    # Standard CI build
    Set-AppveyorBuildVariable 'NightlyBuild' 'false'
}

if ([System.Convert]::ToBoolean($env:ReleaseBuild) -Or [System.Convert]::ToBoolean($env:NightlyBuild)) {
    Set-AppveyorBuildVariable 'DeployBuild' 'true'
} else {
    Set-AppveyorBuildVariable 'DeployBuild' 'false'
}
Set-AppveyorBuildVariable 'TestVariable' 'test'
