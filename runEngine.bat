if not exist build (
    echo "No build directory"
    exit
)
cd build
call LVEngine.exe
cd ..
