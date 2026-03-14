param(
  [ValidateSet("nmake", "ninja", "auto")]
  [string]$Generator = "nmake"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

function Get-RequiredCommand {
  param(
    [Parameter(Mandatory = $true)]
    [string]$Name
  )

  $command = Get-Command $Name -ErrorAction SilentlyContinue
  if ($null -eq $command) {
    throw "Missing required command in PATH: $Name"
  }

  return $command.Source
}

function Get-VcVars32Path {
  $vsWhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
  if (!(Test-Path $vsWhere)) {
    throw "Missing Visual Studio locator: $vsWhere"
  }

  $installationPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($installationPath)) {
    throw "Unable to locate a Visual Studio installation with C++ build tools."
  }

  $vcVars = Join-Path $installationPath.Trim() "VC\Auxiliary\Build\vcvars32.bat"
  if (!(Test-Path $vcVars)) {
    throw "Missing Visual Studio environment script: $vcVars"
  }

  return $vcVars
}

function Invoke-VsCommand {
  param(
    [Parameter(Mandatory = $true)]
    [string]$VcVars,
    [Parameter(Mandatory = $true)]
    [string]$Command
  )

  $fullCommand = "call `"$VcVars`" >nul && $Command"
  cmd /c $fullCommand
  if ($LASTEXITCODE -ne 0) {
    throw "Command failed: $Command"
  }
}

$cmakePath = Get-RequiredCommand -Name "cmake"
$vcVarsPath = Get-VcVars32Path
$selectedGenerator = switch ($Generator) {
  "ninja" { "Ninja" }
  "auto" {
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
      "Ninja"
    } else {
      "NMake Makefiles"
    }
  }
  default { "NMake Makefiles" }
}
$generatorKey = if ($selectedGenerator -eq "Ninja") { "ninja" } else { "nmake" }
$buildDir = Join-Path $repoRoot "build\smoke\$generatorKey"

if (!(Test-Path $buildDir)) {
  New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host "Using generator: $selectedGenerator"
Write-Host "Configuring CMake"
Invoke-VsCommand -VcVars $vcVarsPath -Command "`"$cmakePath`" -S `"$repoRoot`" -B `"$buildDir`" -G `"$selectedGenerator`" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DAERA_ENABLE_PROTOBUF=OFF"

Write-Host "Building"
Invoke-VsCommand -VcVars $vcVarsPath -Command "`"$cmakePath`" --build `"$buildDir`""

Write-Host "Running CTest smoke suite"
ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE -ne 0) {
  throw "CTest failed"
}

Write-Host ""
Write-Host "Smoke test passed through CMake."
