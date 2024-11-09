mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=~/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static ..
cmake --build . --config Release || (
    echo Build failed.
    pause
    exit /b
)

copy /Y Release\main.exe ..
copy /Y Release\glew32.dll ..
copy /Y Release\glfw3.dll ..

cd ..

ResourceHacker.exe -open main.exe -save main.exe -action addskip -res src/assets/icon.ico -mask ICONGROUP,MAINICON,

pause