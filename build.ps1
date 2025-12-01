Param(
    [string]$Config = "Debug",
    [string]$BuildDir = "build",
    [switch]$Vulkan = $true,
    [string]$Target = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Configure (idempotent; re-runs CMake to pick up changes)
$configureArgs = @(
    "-S", ".",
    "-B", $BuildDir,
    "-DCMAKE_BUILD_TYPE=$Config"
)
if ($Vulkan) {
    $configureArgs += "-DFSO_BUILD_WITH_VULKAN=ON"
} else {
    $configureArgs += "-DFSO_BUILD_WITH_VULKAN=OFF"
}

cmake @configureArgs

# Build (optionally with an explicit target, e.g., -Target clean or -Target unittests)
$buildArgs = @(
    "--build", $BuildDir,
    "--config", $Config,
    "--parallel"
)
if ($Target -ne "") {
    $buildArgs += @("--target", $Target)
}

cmake @buildArgs
