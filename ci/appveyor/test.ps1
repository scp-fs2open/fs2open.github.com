
if ([System.Convert]::ToBoolean($env:ReleaseBuild) -Or [System.Convert]::ToBoolean($env:NightlyBuild) -Or [System.Convert]::ToBoolean($env:TestBuild)) {
    # Skip tests for deployment
    exit 0
} else {    
    write "Running unit tests..."
    # Run unit tests
    &"$pwd\bin\$Env:CONFIGURATION\unittests.exe" --gtest_shuffle 2>&1

    if (! ($?)) {
        exit 1
    }
}
