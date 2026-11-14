[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$QtRoot = $env:RETROMEDIALAB_QT_DIR
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$preset = $Configuration.ToLowerInvariant()
if (-not $QtRoot) {
    $QtRoot = Get-ChildItem -Path 'C:\Qt\*\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake' -ErrorAction SilentlyContinue |
        Sort-Object FullName -Descending | Select-Object -First 1 |
        ForEach-Object { Split-Path (Split-Path (Split-Path (Split-Path $_.FullName -Parent) -Parent) -Parent) -Parent }
}
if (-not $QtRoot -or -not (Test-Path -LiteralPath (Join-Path $QtRoot 'lib/cmake/Qt6/Qt6Config.cmake'))) {
    throw 'Specify -QtRoot with a Qt MSVC x64 kit directory.'
}
$vsRoot = $null
if ($IsWindows -or $env:OS -eq 'Windows_NT') {
    $locator = Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (-not (Test-Path -LiteralPath $locator)) { throw 'Visual Studio C++ tools are required.' }
    $vsRoot = & $locator -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsRoot) { throw 'Visual Studio MSVC x64 tools are required.' }
    $vcvars = Join-Path $vsRoot 'VC/Auxiliary/Build/vcvarsall.bat'
    $environmentLines = & $env:ComSpec /d /c "call `"$vcvars`" x64 -vcvars_ver=14.44 >nul && set"
    if ($LASTEXITCODE -ne 0) { throw 'MSVC environment initialization failed.' }
    foreach ($line in $environmentLines) {
        if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
    }
}
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if (-not $cmake -and $vsRoot) { $cmake = Join-Path $vsRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' }
$ninja = (Get-Command ninja -ErrorAction SilentlyContinue).Source
if (-not $ninja -and $vsRoot) { $ninja = Join-Path $vsRoot 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe' }
if (-not $cmake -or -not (Test-Path -LiteralPath $cmake)) { throw 'CMake is required.' }
if (-not $ninja -or -not (Test-Path -LiteralPath $ninja)) { throw 'Ninja is required.' }
Push-Location $projectRoot
try {
    & python (Join-Path $PSScriptRoot 'update-solution.py')
    if ($LASTEXITCODE -ne 0) { throw 'Visual C++ solution refresh failed.' }
    $includePrefixArgument = @()
    if ($vsRoot) {
        # Read raw compiler output: CMake's console decoding can corrupt French non-breaking spaces.
        $probeRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'build/compiler-probe'))
        $expectedBuild = [IO.Path]::GetFullPath((Join-Path $projectRoot 'build')) + [IO.Path]::DirectorySeparatorChar
        if (-not $probeRoot.StartsWith($expectedBuild, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid compiler probe path' }
        New-Item -ItemType Directory -Path $probeRoot -Force | Out-Null
        try {
            $header = Join-Path $probeRoot 'header.h'
            $source = Join-Path $probeRoot 'probe.cpp'
            $object = Join-Path $probeRoot 'probe.obj'
            $output = Join-Path $probeRoot 'output.log'
            [IO.File]::WriteAllText($header, '// Include dependency probe', [Text.UTF8Encoding]::new($false))
            [IO.File]::WriteAllText($source, "#include `"$($header.Replace('\','/'))`"", [Text.UTF8Encoding]::new($false))
            $compiler = (Get-Command cl -ErrorAction Stop).Source
            & $env:ComSpec /d /c "`"$compiler`" /nologo /showIncludes /c `"$source`" /Fo`"$object`" >`"$output`" 2>&1"
            if ($LASTEXITCODE -ne 0) { throw 'MSVC include dependency probe failed' }
            $bytes = [IO.File]::ReadAllBytes($output)
            try { $raw = [Text.UTF8Encoding]::new($false, $true).GetString($bytes) }
            catch { $raw = [Text.Encoding]::GetEncoding([Globalization.CultureInfo]::CurrentCulture.TextInfo.OEMCodePage).GetString($bytes) }
            $match = [regex]::Match($raw, '(?m)^(.+?)(?=[A-Za-z]:[\\/].*header\.h)')
            if (-not $match.Success) { throw 'Cannot identify MSVC include dependency prefix' }
            $includePrefixArgument = @("-DRETROMEDIALAB_MSVC_INCLUDE_PREFIX:STRING=$($match.Groups[1].Value)")
        }
        finally { Remove-Item -LiteralPath $probeRoot -Recurse -Force }
    }
    & $cmake --preset $preset "-DCMAKE_PREFIX_PATH=$QtRoot" "-DCMAKE_MAKE_PROGRAM=$ninja" @includePrefixArgument
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    & $cmake --build --preset $preset
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
    $destination = Join-Path $projectRoot "build/$Configuration/Retro Media Lab"
    & $cmake --install "build/$preset" --prefix $destination
    if ($LASTEXITCODE -ne 0) { throw 'Installation into the run directory failed.' }
    $buildRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'build')) + [IO.Path]::DirectorySeparatorChar
    function Assert-BuildPath([string]$Path) {
        $absolute = [IO.Path]::GetFullPath($Path)
        if (-not $absolute.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) { throw "Path outside build directory: $absolute" }
    }
    $staging = Join-Path $projectRoot "build/qt-staging/$preset"
    Assert-BuildPath $staging
    New-Item -ItemType Directory -Path $staging -Force | Out-Null
    & (Join-Path $QtRoot 'bin/windeployqt.exe') --no-translations --no-compiler-runtime --dir $staging (Join-Path $destination 'lib/App.dll')
    if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed.' }
    function Move-Dependency([IO.FileInfo]$File) {
        $category = switch -Regex ($File.Name) {
            '^Qt6.*\.dll$' { 'Qt6'; break }
            '^(d3d.*|dxcompiler|dxil)\.dll$' { 'DirectX'; break }
            '^(opengl32sw|libEGL|libGLESv2)\.dll$' { 'OpenGL'; break }
            '^(RetroMediaLab.*|App|Translations)\.dll$' { ''; break }
            default { throw "Unclassified DLL: $($File.FullName)" }
        }
        $targetDir = Join-Path $destination "lib/$category"
        Assert-BuildPath $File.FullName
        Assert-BuildPath $targetDir
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        Move-Item -LiteralPath $File.FullName -Destination (Join-Path $targetDir $File.Name) -Force
    }
    foreach ($file in Get-ChildItem -LiteralPath $staging -File -Filter '*.dll') { Move-Dependency $file }
    # Migrate DLLs from the earlier flat build layout as well.
    foreach ($file in Get-ChildItem -LiteralPath $destination -File -Filter '*.dll') {
        if ($file.Name -match '^(RetroMediaLab.*|App|Translations)\.dll$' -and (Test-Path -LiteralPath (Join-Path $destination "lib/$($file.Name)"))) {
            Assert-BuildPath $file.FullName
            Remove-Item -LiteralPath $file.FullName
        } else { Move-Dependency $file }
    }
    function Move-PluginFolder([IO.DirectoryInfo]$Folder) {
        Assert-BuildPath $Folder.FullName
        $targetDir = Join-Path $destination "lib/Qt6/plugins/$($Folder.Name)"
        Assert-BuildPath $targetDir
        foreach ($file in Get-ChildItem -LiteralPath $Folder.FullName -File -Recurse) {
            $relative = $file.FullName.Substring($Folder.FullName.Length).TrimStart([char[]]'\/')
            $target = Join-Path $targetDir $relative
            New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
            Copy-Item -LiteralPath $file.FullName -Destination $target -Force
        }
        Remove-Item -LiteralPath $Folder.FullName -Recurse -Force
    }
    foreach ($folder in Get-ChildItem -LiteralPath $staging -Directory) { Move-PluginFolder $folder }
    foreach ($name in @('generic','iconengines','imageformats','networkinformation','platforms','styles','tls')) {
        $legacy = Join-Path $destination $name
        if (Test-Path -LiteralPath $legacy -PathType Container) { Move-PluginFolder (Get-Item -LiteralPath $legacy) }
    }
    Remove-Item -LiteralPath $staging -Recurse -Force
    $legacyCatalogs = Join-Path $destination 'Translations/Catalogs'
    if (Test-Path -LiteralPath $legacyCatalogs) {
        Assert-BuildPath $legacyCatalogs
        Remove-Item -LiteralPath $legacyCatalogs -Recurse -Force
        $legacyTranslations = Split-Path -Parent $legacyCatalogs
        if (-not (Get-ChildItem -LiteralPath $legacyTranslations -Force)) { Remove-Item -LiteralPath $legacyTranslations }
    }
    $oldCatalogLibraries = Join-Path $destination 'lib/Translations'
    if (Test-Path -LiteralPath $oldCatalogLibraries) {
        Assert-BuildPath $oldCatalogLibraries
        Remove-Item -LiteralPath $oldCatalogLibraries -Recurse -Force
    }
    foreach ($name in @('RetroMediaLab.App.dll', 'RetroMediaLab.Translations.dll')) {
        $legacy = Join-Path $destination "lib/$name"
        if (Test-Path -LiteralPath $legacy) {
            Assert-BuildPath $legacy
            Remove-Item -LiteralPath $legacy
        }
    }
    $oldThemeData = Join-Path $destination 'Themes/RetroMediaLab'
    if (Test-Path -LiteralPath $oldThemeData) {
        Assert-BuildPath $oldThemeData
        Remove-Item -LiteralPath $oldThemeData -Recurse -Force
        $oldThemes = Split-Path -Parent $oldThemeData
        if (-not (Get-ChildItem -LiteralPath $oldThemes -Force)) { Remove-Item -LiteralPath $oldThemes }
    }
    if (Get-ChildItem -LiteralPath $destination -File -Filter '*.dll') { throw 'Unexpected DLL at application root' }
    Write-Output (Join-Path $destination 'retromedialab.exe')
}
finally { Pop-Location }
