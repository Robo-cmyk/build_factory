#pragma once
#include <cstdint>
#include <stdint.h>
#include <vector>
#include "../building/chest.h"


class world {
public:
  void load_world(const char *);
  void create_world(int, int, const char *);
  void save_world(const char *);
  uint8_t get_tileID(int, int);
  void load_buildings(const char*);
  void save_buildings(const char*);
  void set_tileID(int, int, uint8_t);
  void set_world_dimensions(int new_width, int new_height);
  int width;
  int height;
  std::vector<uint8_t> worldvector;
  std::vector<chest> chests;
};
