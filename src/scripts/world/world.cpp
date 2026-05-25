#include "world.h"
#include <fstream>
#include <iostream>
#include <ostream>
#include <cstdint>
#include <vector>
#include <random>

uint8_t zero = 0;

std::random_device rd;

std::mt19937 gen(rd());

std::uniform_int_distribution<> dis(0,9);

void world::create_world(int w, int h, const char *world_file_path) {
  std::ofstream outFile(world_file_path, std::ios::binary);
  outFile.write(reinterpret_cast<const char *>(&w), sizeof(w));
  outFile.write(reinterpret_cast<const char *>(&h), sizeof(h));
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      zero = dis(gen);
      outFile.write(reinterpret_cast<const char *>(&zero), 1);
    }
  }
}

void world::load_world(const char *world_file_path) {
  std::ifstream inFile(world_file_path, std::ios::binary);

  inFile.read(reinterpret_cast<char *>(&width), sizeof(width));
  inFile.read(reinterpret_cast<char *>(&height), sizeof(height));

  worldvector.resize(width * height);

  inFile.read(reinterpret_cast<char *>(worldvector.data()),
              worldvector.size() * sizeof(uint8_t));
}

uint8_t world::get_tileID(int x ,int y){
  return worldvector[x + y * width];
}

void world::set_tileID(int x , int y , uint8_t tileID){
  if(x >= 0 && x < width && y >=0 && y < height){
    int index = y * width + x;
    if(index >= 0 && index < (int)worldvector.size()){
      worldvector[index] = tileID;
    }
  }
}

void world::save_world(const char *filename){
  std::ofstream worldfile(filename ,std::ios::binary);
  worldfile.write(reinterpret_cast<char *>(&width) , sizeof(width));
  worldfile.write(reinterpret_cast<char *>(&height) , sizeof(height));
  if(worldfile.is_open()){
    for(const auto& tile : worldvector){
      worldfile.write(reinterpret_cast<const char*>(&tile) , sizeof(tile));
    }
  }
}
