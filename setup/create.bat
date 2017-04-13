@echo off

call fetch_files.bat

set SETUPPATH= .
set VERSION=0.8
set BINPATH=C:\Qt\Tools\QtInstallerFramework\2.0\bin\binarycreator.exe

%BINPATH% -c config.xml -p packages MultiDownload%VERSION%-Setup.exe

rmdir bintemp /S /Q
pause