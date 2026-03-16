@echo off
REM Launch OpenEnroth from the personal branch with a separate userdata folder.
REM Saves, config (openenroth.ini), and screenshots go to OpenEnroth_Personal
REM instead of the default OpenEnroth folder in Saved Games.
set EXE=%~dp0build\src\Bin\OpenEnroth\Debug\OpenEnroth.exe
set USERPATH=C:\Users\danjo\Saved Games\OpenEnroth_Personal
"%EXE%" --user-path "%USERPATH%"
