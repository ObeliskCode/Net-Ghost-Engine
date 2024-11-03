#!/usr/bin/bash

python3 build.py ../Resources/test-shader-wireframe.blend
python3 build.py ../Resources/test-script-rotate.blend

python3 build.py ../Resources/test-shader-wireframe.blend  --wasm
python3 build.py ../Resources/test-script-rotate.blend  --wasm
