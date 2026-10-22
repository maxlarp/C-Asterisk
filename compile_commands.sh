# 1. bmpconverter
make -C bmpconverter

# 2. viewer
cmake -B build/viewer -S viewer && cmake --build build/viewer
