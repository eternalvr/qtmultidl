@echo off

call fetch_files.bat

set SETUPPATH= .
set VERSION=0.9
set BINPATH=C:\Qt\Tools\QtInstallerFramework\2.0\bin\


%BINPATH%\binarycreator -c config.xml -p packages MultiDownload-%VERSION%-OfflineSetup.exe
%BINPATH%\binarycreator -c config_online.xml -n -p packages MultiDownload-%VERSION%-Setup.exe
%BINPATH%\repogen -p packages -i com.etvr.qtmultidl,com.microsoft.vcruntime upload

rmdir bintemp /S /Q
pause