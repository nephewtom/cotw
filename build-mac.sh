rm -f ./cotw

RAYLIB_DIR=../rolling-cube/raylib/src

clang++ -g -std=c++20 -Wall -Wextra -Wno-missing-field-initializers cotw.cpp -o cotw -I$RAYLIB_DIR  -L$RAYLIB_DIR -lraylib -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit
./cotw
