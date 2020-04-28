if "%APPVEYOR_REPO_TAG_NAME%"=="continuous" (

    exit 1

)

git submodule init
git submodule update

set QTDIR=C:\Qt\5.12\msvc2017_64
set PATH=%PATH%;%QTDIR%\bin
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

git clone https://github.com/vicr123/the-libs.git
cd the-libs
git checkout blueprint
qmake the-libs.pro "CONFIG+=release"
nmake release
nmake install
cd ..

git clone https://github.com/vicr123/contemporary-theme.git
cd contemporary-theme
git checkout blueprint
qmake Contemporary.pro "CONFIG+=release"
nmake release
cd ..

mkdir build
cd build
qmake ../theFrame.pro "CONFIG+=release"
nmake release
cd ..

mkdir deploy
mkdir deploy\styles
mkdir deploy\translations
mkdir deploy\audio
copy "contemporary-theme\release\Contemporary.dll" deploy\styles
copy build\frame\release\theFrame.exe deploy
copy translations\*.qm deploy\translations
copy audio\* deploy\audio
copy build\libtheframe\release\libtheframe.dll deploy
copy build\theframe-render\release\theframe-render.exe deploy
copy "C:\Program Files\thelibs\lib\the-libs.dll" deploy
cd deploy
windeployqt theFrame.exe -svg -multimedia -concurrent
