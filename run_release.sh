cmake -DCMAKE_BUILD_TYPE=Release -B build -G "Unix Makefiles"
cd build
make
./Release/vulkan_window
