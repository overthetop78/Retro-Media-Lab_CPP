[CmdletBinding()]
param([string]$PackageRoot)
$ErrorActionPreference = 'Stop'
$project = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$manifestPath = Join-Path $project 'third_party/ffmpeg/package.json'
$package = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
$cache = Join-Path $project "build/dependencies/ffmpeg/$($package.version)"
$buildRoot = [IO.Path]::GetFullPath((Join-Path $project 'build')) + [IO.Path]::DirectorySeparatorChar
function Assert-CachePath([string]$Path) {
    if (-not [IO.Path]::GetFullPath($Path).StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) { throw 'FFmpeg cache path is outside build.' }
}
function Test-Executable([string]$Path) {
    return (Test-Path -LiteralPath $Path -PathType Leaf) -and ((Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash -eq $package.executableSha256)
}
Assert-CachePath $cache
$executable = Join-Path $cache 'ffmpeg.exe'
if ((Test-Executable $executable) -and (Test-Path -LiteralPath (Join-Path $cache 'LICENSE'))) { return $cache }
New-Item -ItemType Directory -Force -Path $cache | Out-Null
$staging = Join-Path $cache ('prepare-' + [guid]::NewGuid().ToString('N'))
Assert-CachePath $staging
try {
    if (-not $PackageRoot) {
        New-Item -ItemType Directory -Force -Path $staging | Out-Null
        $archive = Join-Path $staging 'package.zip'
        Invoke-WebRequest -Uri $package.url -OutFile $archive
        if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $package.archiveSha256) { throw 'FFmpeg archive checksum mismatch.' }
        Expand-Archive -LiteralPath $archive -DestinationPath $staging
        $PackageRoot = Join-Path $staging $package.directory
    }
    $sourceExecutable = Join-Path $PackageRoot 'bin/ffmpeg.exe'
    if (-not (Test-Executable $sourceExecutable)) { throw 'FFmpeg executable checksum mismatch.' }
    Copy-Item -LiteralPath $sourceExecutable -Destination $executable -Force
    Copy-Item -LiteralPath (Join-Path $PackageRoot 'LICENSE') -Destination (Join-Path $cache 'LICENSE') -Force
    Copy-Item -LiteralPath (Join-Path $PackageRoot 'README.txt') -Destination (Join-Path $cache 'README.txt') -Force
    Copy-Item -LiteralPath $manifestPath -Destination (Join-Path $cache 'package.json') -Force
    return $cache
}
finally {
    if (Test-Path -LiteralPath $staging) { Assert-CachePath $staging; Remove-Item -LiteralPath $staging -Recurse -Force }
}
