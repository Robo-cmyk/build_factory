#include <string>

class Building {
  public:
    int x, y;
    int direction;
    std::string type;
    virtual ~Building() = default;
};

class Stone_Furnace : public Building {
  public:
    int inputItemID;
    int outputItemID;
    float progress;
};

class Chest : public Building {
  public:
    std::vector<std::string> inventory;
    int inventory[10];
};
