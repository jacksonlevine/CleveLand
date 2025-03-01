mkdir build
cd build
cmake -A x64 -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=~/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static ..
cmake --build . --config Release || (
    echo Build failed.
    pause
    exit /b
)

copy /Y Release\main.exe ..

cd ..

ResourceHacker.exe -open main.exe -save main.exe -action addskip -res assets/icon.ico -mask ICONGROUP,MAINICON,

del "HungaryServer.exe"

ren main.exe "HungaryServer.exe"

pause