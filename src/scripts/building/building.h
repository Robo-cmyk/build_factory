#include <string>

class Building {
public:
  int x, y;
  int direction;
  std::string type;
  virtual ~Building() = default;
};
