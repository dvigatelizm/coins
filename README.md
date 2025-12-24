## Build on Ubuntu

```bash
sudo apt update
sudo apt install -y cmake build-essential libopencv-dev libwxgtk3.2-dev

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/tools/label_editor_wx/label_editor_wx
