Param(
    [string]$Dump,

    [string]$SymbolPath = "build\bin\Debug",

    [string]$CdbPath = "C:\Users\danie\AppData\Local\Microsoft\WindowsApps\cdbx64.exe",

    [switch]$Help
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path

if ($Help) {
    Write-Host "Analyzes the newest fs2_26_0_0_*.mdmp dump under build\bin\Debug by default."
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File scripts\analyze_minidump.ps1"
    Write-Host "  powershell -ExecutionPolicy Bypass -File scripts\analyze_minidump.ps1 -Dump path\to\dump.mdmp"
    Write-Host ""
    Write-Host "Optional parameters:"
    Write-Host "  -Dump        Specific dump file (otherwise newest fs2_26_0_0_*.mdmp is used)"
    Write-Host "  -SymbolPath  Directory containing symbols (default: build\bin\Debug)"
    Write-Host "  -CdbPath     Path to cdbx64.exe (default: $CdbPath)"
    exit 0
}

function Get-LatestDump {
    param(
        [string]$SearchRoot
    )

    $pattern = "fs2_26_0_0_*.mdmp"
    $dumpDir = Join-Path $SearchRoot "build\bin\Debug"
    if (-not (Test-Path -LiteralPath $dumpDir)) {
        return $null
    }

    $latest = Get-ChildItem -Path $dumpDir -Filter $pattern -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    return $latest
}

if (-not $Dump) {
    $candidate = Get-LatestDump -SearchRoot $RepoRoot
    if (-not $candidate) {
        Write-Error "No dump files matching fs2_26_0_0_*.mdmp found under $RepoRoot\build\bin\Debug. Provide -Dump explicitly."
    }
    $Dump = $candidate.FullName
    Write-Host "Using latest dump: $Dump"
}

Set-Location $RepoRoot

if (-not (Test-Path -LiteralPath $Dump)) {
    Write-Error "Dump file not found: $Dump"
}

if (-not (Test-Path -LiteralPath $SymbolPath)) {
    Write-Error "Symbol path not found: $SymbolPath"
}

if (-not (Test-Path -LiteralPath $CdbPath)) {
    Write-Error "cdbx64.exe not found at: $CdbPath"
}

$DumpFull = (Resolve-Path -LiteralPath $Dump).Path
$SymFull = (Resolve-Path -LiteralPath $SymbolPath).Path

$escapedSym = $SymFull.Replace('"', '\"')
$Commands = ".symfix; .sympath+ ""$escapedSym""; .reload; !analyze -v; .ecxr; kv; q"

& $CdbPath -z $DumpFull -c $Commands
