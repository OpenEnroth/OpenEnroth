@echo off
rem Windows twin of workspace_status.sh - bazel can't exec an .sh here.
rem rev-parse instead of log --format=%h: cmd eats the percent inside for /f.
setlocal
set REV=unknown
set VER=unknown
for /f "delims=" %%i in ('git rev-parse --short HEAD 2^>nul') do set REV=%%i
for /f "delims=" %%i in ('git describe --always --tags --dirty 2^>nul') do set VER=%%i
echo STABLE_MM_GIT_REVISION %REV%
echo STABLE_OE_VERSION %VER%
exit /b 0
