#ifndef _TILE_H_
#define _TILE_H_
#include <vector>

//瓦片大小48像素
#define SIZE_TILE 48
struct Tile
{
	enum class Direction
	{
		None = 0,
		up,
		Down,
		Left,
		Right
	};
	//地形
	int terrain = 0;
	//装饰
	int decoration = -1;
	//特殊标志
	int special_flag = -1; 
	bool has_tower = false;
	Direction direction = Direction::None;
	

};

typedef std::vector<std::vector<Tile>> TileMap;

#endif // !_TILE_H_
