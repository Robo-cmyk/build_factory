@echo off
g++ src/scripts/main/main.cpp src/scripts/renderer/renderer.cpp src/scripts/world/world.cpp -Isrc/scripts/main -Isrc/scripts/renderer -Isrc/scripts/world -Isrc/scripts/nlohmanjson -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -o build/game.exe
pause

