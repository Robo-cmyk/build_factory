#include <cstdint>
#include <iostream>
#include <stdint.h>
#include <vector>

class world {
public:
  void load_world(const char *);
  void create_world(int, int, const char *);
  void save_world(const char *);
  uint8_t get_tileID(int, int);
  void load_buildings();
  void set_tileID(int, int, uint8_t);
  int width;
  int height;
  std::vector<uint8_t> worldvector;
};
