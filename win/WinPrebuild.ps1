param([string]$targetdir)
if ([string]::IsNullOrEmpty($targetdir))
{
    $targetdir = "C:\3rd_party"
}

Add-Type -AssemblyName System.IO.Compression.FileSystem

function Unzip {
    param([string]$zipfile, [string]$outpath)

    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
    write-host "Unpacked: $zipfile" -ForegroundColor Green
}

function Download-File {
    param([string]$fileurl, [string]$outpath)

    $filename = $fileurl.Substring($fileurl.LastIndexOf("/") + 1)
    write-host "Download: $filename" -ForegroundColor Yellow

    $WebClient = New-Object System.Net.WebClient
    $filepath = Join-Path $outpath $filename
    $WebClient.DownloadFile($fileurl, $filepath)
    write-host "Downloaded: $fileurl" -ForegroundColor Green
    return $filepath
}

function New-TemporaryDirectory {
    $parent = [System.IO.Path]::GetTempPath()
    [string] $name = [System.Guid]::NewGuid()
    $tmp_dir = Join-Path $parent $name
    New-Item -ItemType Directory -Path "$tmp_dir"
}

$WebClient = New-Object System.Net.WebClient

$temp_path = New-TemporaryDirectory

$FFMPEG_VERSION = "4.0"
$FFMPEGDIR = "$targetdir\ffmpeg-$FFMPEG_VERSION"
if (!(Test-Path -Path $FFMPEGDIR)) {
    $arch = Download-File "https://ffmpeg.zeranoe.com/builds/win32/dev/ffmpeg-$FFMPEG_VERSION-win32-dev.zip" $temp_path
    Unzip $arch $targetdir
    New-Item -ItemType Directory -Path $FFMPEGDIR  | Out-Null
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev\include" -Recurse -Destination $FFMPEGDIR
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev\examples" -Recurse -Destination $FFMPEGDIR
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev\LICENSE.txt" -Destination $FFMPEGDIR
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev\README.txt" -Destination $FFMPEGDIR
    New-Item -ItemType Directory -Path "$FFMPEGDIR\lib"  | Out-Null
    New-Item -ItemType Directory -Path "$FFMPEGDIR\lib\Win32"  | Out-Null
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev\lib\*.*" -Destination "$FFMPEGDIR\lib\Win32"
    Remove-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-dev" -Recurse

    $arch = Download-File "https://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-$FFMPEG_VERSION-win64-dev.zip" $temp_path
    Unzip $arch $targetdir
    New-Item -ItemType Directory -Path "$FFMPEGDIR\lib\Win64"  | Out-Null
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win64-dev\lib\*.*" -Destination "$FFMPEGDIR\lib\Win64"
    Remove-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win64-dev" -Recurse
    
    $arch = Download-File "https://ffmpeg.zeranoe.com/builds/win32/shared/ffmpeg-$FFMPEG_VERSION-win32-shared.zip" $temp_path
    Unzip $arch $targetdir
    New-Item -ItemType Directory -Path "$FFMPEGDIR\bin"  | Out-Null
    New-Item -ItemType Directory -Path "$FFMPEGDIR\bin\Win32"  | Out-Null
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-shared\bin\*.*" -Destination "$FFMPEGDIR\bin\Win32"
    Remove-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win32-shared" -Recurse

    $arch = Download-File "https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-$FFMPEG_VERSION-win64-shared.zip" $temp_path
    Unzip $arch $targetdir
    New-Item -ItemType Directory -Path "$FFMPEGDIR\bin\Win64"  | Out-Null
    Copy-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win64-shared\bin\*.*" -Destination "$FFMPEGDIR\bin\Win64"
    Remove-Item -Path "$targetdir\ffmpeg-$FFMPEG_VERSION-win64-shared" -Recurse
}
[Environment]::SetEnvironmentVariable("FFMPEGDIR", "$FFMPEGDIR", "User")

$OPENAL_VERSION = "1.18.2"
$OPENALDIR = "$targetdir\openal-$OPENAL_VERSION"
if (!(Test-Path -Path $OPENALDIR)) {
    $arch = Download-File "http://kcat.strangesoft.net/openal-binaries/openal-soft-$OPENAL_VERSION-bin.zip" $temp_path
    Unzip $arch $targetdir
    Rename-Item -Path "$targetdir\openal-soft-$OPENAL_VERSION-bin" -NewName $OPENALDIR
}
[Environment]::SetEnvironmentVariable("OPENALDIR", "$OPENALDIR", "User")

function Set-VS-Variables {
    param([string]$platform)

    # Set environment variables for Visual Studio Command Prompt
    pushd 'c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build'
    cmd /c "vcvars32.bat $platform&set" |
    foreach {
        if ($_ -match "=") {
            $v = $_.split("=")
            Set-Item -force -path "ENV:\$($v[0])" -value "$($v[1])"
        }
    }
    popd
    write-host "Visual Studio 2017 Command Prompt variables set" -ForegroundColor Green
}

$ZLIB_VERSION = "1.2.11"
$ZLIBDIR = "$targetdir\zlib-$ZLIB_VERSION"
if (!(Test-Path -Path $ZLIBDIR)) {
    $LINK_VER = $ZLIB_VERSION -Replace '[.]',''
    $arch = Download-File "http://www.zlib.net/zlib$LINK_VER.zip" $temp_path
    Unzip $arch $targetdir
    pushd $ZLIBDIR
    Set-VS-Variables "x86"
    nmake /f win32\Makefile.msc
    popd
}
[Environment]::SetEnvironmentVariable("ZLIBDIR", "$ZLIBDIR", "User")

$SDL_VERSION = "2.0.8"
$SDL2DIR = "$targetdir\SDL2-$SDL_VERSION"
if (!(Test-Path -Path $SDL2DIR)) {
    $arch = Download-File "https://libsdl.org/release/SDL2-devel-$SDL_VERSION-VC.zip" $temp_path
    Unzip $arch $targetdir
}
[Environment]::SetEnvironmentVariable("SDL2DIR", "$SDL2DIR", "User")
