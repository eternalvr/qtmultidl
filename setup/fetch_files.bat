set BINPATH=C:\Qt\5.8\msvc2015\bin\windeployqt.exe
set BINARY=..\..\build-MultiDownload-Desktop_Qt_5_8_0_MSVC2015_32bit-Release\release\MultiDownload.exe
SET VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
rmdir bintemp /Q /S
mkdir bintemp
copy %BINARY% bintemp

%BINPATH% bintemp --release --no-translations --no-quick-import --no-system-d3d-compiler --compiler-runtime --no-webkit2 --no-angle --no-opengl-sw


copy bintemp\vcredist_x86.exe packages\com.microsoft.vcruntime\data\
del bintemp\vcredist_x86.exe

xcopy /s /e /Y bintemp\* packages\com.etvr.qtmultidl\data\
copy openssl\ packages\com.etvr.qtmultidl\data\
