# Installs ATL and MFC for the v143 toolset, which the Visual Studio 2026 runner images
# only ship for the latest toolset.  ATL is needed by SAPI speech (sphelper.h) and MFC by FRED2.

$ErrorActionPreference = 'Stop'

$msvcVersion = '14.44'
$component = 'Microsoft.VisualStudio.Component.VC.14.44.17.14'
$suffix = if ($env:RUNNER_ARCH -eq 'ARM64') { '.ARM64' } else { '' }

$installerDir = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
$installPath = & "$installerDir\vswhere.exe" -version '[18.0,19.0)' -latest -property installationPath
if (-not $installPath) {
    Write-Error 'Visual Studio 2026 was not found'
}

$arguments = @(
    '/c', 'vs_installer.exe', 'modify',
    '--installPath', "`"$installPath`"",
    '--add', "$component.ATL$suffix",
    '--add', "$component.MFC$suffix",
    '--quiet', '--norestart', '--nocache'
)
$process = Start-Process -FilePath cmd.exe -ArgumentList $arguments -WorkingDirectory $installerDir -Wait -PassThru -NoNewWindow
# 3010 = success, reboot required
if ($process.ExitCode -notin 0, 3010) {
    Write-Error "vs_installer exited with code $($process.ExitCode)"
}

# fail here rather than with a confusing compile error if the components didn't install
foreach ($header in 'atlbase.h', 'afxwin.h') {
    if (-not (Test-Path "$installPath\VC\Tools\MSVC\$msvcVersion.*\atlmfc\include\$header")) {
        Write-Error "$header for MSVC $msvcVersion is missing after install"
    }
}
