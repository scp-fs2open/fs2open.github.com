
write "$env:APPVEYOR_REPO_TAG"
write "$env:APPVEYOR_REPO_TAG_NAME"

if (! ($env:APPVEYOR_REPO_TAG)) {
    # Nothing to do here
    Set-AppveyorBuildVariable 'ReleaseBuild' 'False'
    exit 0
}

if (! ("$env:APPVEYOR_REPO_TAG_NAME" -match "^release.*")) {
    # Tag is not a release tag
    Set-AppveyorBuildVariable 'ReleaseBuild' 'False'
    exit 0
}

# Tag matches
Set-AppveyorBuildVariable 'ReleaseBuild' 'True'
