Param(
    [string]$FsoExe = "build/bin/Debug/fs2_26_0_0.exe",
    [string]$GameDir,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

if ($Help) {
    Write-Host "Runs the Vulkan build and collects logs into out/vulkan_debug/<timestamp>/"
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File scripts/vulkan_debug_session.ps1 [-FsoExe <path>] [-GameDir <path>] [-- <extra FSO args>]"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  ./scripts/vulkan_debug_session.ps1"
    Write-Host "  ./scripts/vulkan_debug_session.ps1 -FsoExe build/bin/Debug/fs2_custom.exe -GameDir 'C:\Games\Freespace2' -- -mod MediaVPs"
    exit 0
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path

$FsoExeCandidate = $FsoExe
if (-not [System.IO.Path]::IsPathRooted($FsoExeCandidate)) {
    $FsoExeCandidate = Join-Path $RepoRoot $FsoExeCandidate
}

try {
    $FsoExePath = (Resolve-Path -LiteralPath $FsoExeCandidate).Path
} catch {
    Write-Error "Unable to resolve FSO executable path: $FsoExe"
    exit 1
}

if (-not (Test-Path -LiteralPath $FsoExePath -PathType Leaf)) {
    Write-Error "FSO executable not found: $FsoExePath"
    Write-Host "Build the Debug Vulkan target first or pass -FsoExe with the correct path."
    exit 1
}

$FsoExeDir = Split-Path -Parent $FsoExePath
$FsoExeName = Split-Path -Leaf $FsoExePath

$GameDirCandidate = $GameDir
if ([string]::IsNullOrWhiteSpace($GameDirCandidate)) {
    # Default to exe directory first
    $GameDirCandidate = Split-Path -Parent $FsoExePath

    # Check if exe directory has game data (look for a key file like strings.tbl or root_fs2.vp)
    $HasGameData = (Test-Path (Join-Path $GameDirCandidate "strings.tbl")) -or
                   (Test-Path (Join-Path $GameDirCandidate "root_fs2.vp"))

    # If no game data found, try to auto-detect Steam installation
    if (-not $HasGameData) {
        $SteamPaths = @(
            "C:\Program Files (x86)\Steam\steamapps\common\Freespace 2",
            "C:\Program Files\Steam\steamapps\common\Freespace 2"
        )

        foreach ($SteamPath in $SteamPaths) {
            if (Test-Path $SteamPath) {
                $HasSteamData = (Test-Path (Join-Path $SteamPath "strings.tbl")) -or
                               (Test-Path (Join-Path $SteamPath "root_fs2.vp"))
                if ($HasSteamData) {
                    Write-Host "Auto-detected Steam FreeSpace 2 installation at: $SteamPath"
                    $GameDirCandidate = $SteamPath
                    break
                }
            }
        }
    }
}
if (-not [System.IO.Path]::IsPathRooted($GameDirCandidate)) {
    $GameDirCandidate = Join-Path $RepoRoot $GameDirCandidate
}

try {
    $ResolvedGameDir = (Resolve-Path -LiteralPath $GameDirCandidate).Path
} catch {
    Write-Error "Unable to resolve game directory path: $GameDirCandidate"
    exit 1
}

if (-not (Test-Path -LiteralPath $ResolvedGameDir -PathType Container)) {
    Write-Error "Game directory not found or not a directory: $ResolvedGameDir"
    exit 1
}

$SessionRoot = Join-Path $RepoRoot "out/vulkan_debug"
New-Item -ItemType Directory -Force -Path $SessionRoot | Out-Null

$SessionId = Get-Date -Format "yyyyMMdd_HHmmss"
$SessionDir = Join-Path $SessionRoot $SessionId
New-Item -ItemType Directory -Force -Path $SessionDir | Out-Null

Write-Host "Vulkan debug session: $SessionId"
Write-Host "Session directory: $SessionDir"
Write-Host ""


function Initialize-LogFile {
    param(
        [string]$Label,
        [string]$Path
    )

    if (Test-Path -LiteralPath $Path) {
        Write-Host "  Found existing $Label at $Path, snapshotting and truncating..."
        Copy-Item -LiteralPath $Path -Destination (Join-Path $SessionDir "${Label}_pre.log") -Force
        Clear-Content -LiteralPath $Path
    }
}

$Fs2DataDir = Join-Path $env:APPDATA "HardLightProductions/FreeSpaceOpen/data"
if (-not (Test-Path -LiteralPath $Fs2DataDir -PathType Container)) {
    $Fs2DataDir = Join-Path $env:USERPROFILE ".local/share/HardLightProductions/FreeSpaceOpen/data"
}

$PassThroughArgs = @()
if ($args.Count -gt 0) {
    $SentinelIndex = [Array]::IndexOf($args, "--")
    if ($SentinelIndex -ge 0) {
        if ($SentinelIndex -lt ($args.Count - 1)) {
            $PassThroughArgs = $args[($SentinelIndex + 1)..($args.Count - 1)]
        }
    } else {
        $PassThroughArgs = $args
    }
}

$Fs2LogPath = Join-Path $Fs2DataDir "fs2_open.log"
$VkLogPath = Join-Path $ResolvedGameDir "vulkan_debug.log"
$VkHdrLogPath = Join-Path $ResolvedGameDir "vulkan_hdr_debug.txt"

Write-Host "Preparing log files..."
Initialize-LogFile -Label "fs2_open" -Path $Fs2LogPath
Initialize-LogFile -Label "vulkan_debug" -Path $VkLogPath
Initialize-LogFile -Label "vulkan_hdr_debug" -Path $VkHdrLogPath

$Arguments = @("-vulkan", "-window") + $PassThroughArgs
$StdoutPath = Join-Path $SessionDir "stdout_stderr.txt"

@(
    "# Vulkan debug session $SessionId"
    "repo: $RepoRoot"
    "game_dir: $ResolvedGameDir"
    "exe: $FsoExePath"
    "args: $($Arguments -join ' ')"
    ""
) | Set-Content -Path (Join-Path $SessionDir "command.txt")

Write-Host ""
Write-Host "Running FSO Vulkan build once. Reproduce your issue, then quit the game."
Write-Host "Executable: $FsoExePath"
Write-Host "Arguments : $($Arguments -join ' ')"
Write-Host "Game data : $ResolvedGameDir"
Write-Host ""

Push-Location -LiteralPath $ResolvedGameDir
try {
    # Run exe with & operator - Push-Location ensures correct working directory
    & $FsoExePath @Arguments
    $ExitCode = $LASTEXITCODE
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "FSO exited with code $ExitCode"
Write-Host "Collecting logs into session directory..."

if (Test-Path -LiteralPath $Fs2LogPath) {
    Copy-Item -LiteralPath $Fs2LogPath -Destination (Join-Path $SessionDir "fs2_open.log") -Force
} else {
    Write-Warning "fs2_open.log not found at $Fs2LogPath"
}

if (Test-Path -LiteralPath $VkLogPath) {
    Copy-Item -LiteralPath $VkLogPath -Destination (Join-Path $SessionDir "vulkan_debug.log") -Force
} else {
    Write-Warning "vulkan_debug.log not found at $VkLogPath"
}

if (Test-Path -LiteralPath $VkHdrLogPath) {
    Copy-Item -LiteralPath $VkHdrLogPath -Destination (Join-Path $SessionDir "vulkan_hdr_debug.txt") -Force
}

Write-Host ""
Write-Host "Done."
Write-Host "For LLM debugging, point to this directory:"
Write-Host "  $SessionDir"
Write-Host "and especially share:"
Write-Host "  - stdout_stderr.txt"
Write-Host "  - fs2_open.log"
Write-Host "  - vulkan_debug.log"
Write-Host ""
