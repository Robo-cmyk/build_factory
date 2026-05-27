@echo off
g++ src/scripts/main/main.cpp src/scripts/renderer/renderer.cpp src/scripts/world/world.cpp -Isrc/scripts/main -Isrc/scripts/renderer -Isrc/scripts/world -Isrc/scripts/nlohmanjson -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -o build/game.exe
g++ -std=c++17 src/scripts/main/main.cpp src/scripts/building/*.cpp src/scripts/player/*.cpp src/scripts/renderer/*.c
pp src/scripts/world/*.cpp -o build/game.exe -I src/scripts/building -I src/scripts/main -I src/scripts/player -I src/scripts/renderer
 -I src/scripts/world -I src/scripts/nlohmannjson -lraylib -lopengl32 -lgdi32 -lwinmm

pause

