$ErrorActionPreference = 'Stop'
$bundle = Join-Path $PSScriptRoot 'Binaries\EQP-1A.vst3'
$vstRoot = Join-Path $env:CommonProgramFiles 'VST3'
try {
    New-Item -ItemType Directory -Force -Path $vstRoot | Out-Null
    Copy-Item -LiteralPath $bundle -Destination $vstRoot -Recurse -Force
} catch {
    $vstRoot = Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
    New-Item -ItemType Directory -Force -Path $vstRoot | Out-Null
    Copy-Item -LiteralPath $bundle -Destination $vstRoot -Recurse -Force
}
$programDir = Join-Path $env:LOCALAPPDATA 'Programs\Papielguapo\EQP-1A'
New-Item -ItemType Directory -Force -Path $programDir | Out-Null
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'Binaries\EQP-1A.exe') -Destination $programDir -Force
$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut((Join-Path ([Environment]::GetFolderPath('Desktop')) 'EQP-1A.lnk'))
$shortcut.TargetPath = Join-Path $programDir 'EQP-1A.exe'
$shortcut.WorkingDirectory = $programDir
$shortcut.Save()
$startMenu = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Papielguapo'
New-Item -ItemType Directory -Force -Path $startMenu | Out-Null
Copy-Item -LiteralPath (Join-Path ([Environment]::GetFolderPath('Desktop')) 'EQP-1A.lnk') -Destination $startMenu -Force
Write-Output "VST3: $vstRoot\EQP-1A.vst3"
Write-Output "Standalone: $programDir\EQP-1A.exe"
