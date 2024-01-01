mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=~/Documents/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake ..
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