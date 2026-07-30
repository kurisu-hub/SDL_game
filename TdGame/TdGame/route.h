#ifndef _ROUTE_H_
#define _ROUTE_H_

#include"tile.h"

#include<vector>

#include<SDL.h>


class Route
{
public:
	typedef std::vector<SDL_Point> IdxList;
public:
	Route() = default;
	Route(const TileMap& map, const SDL_Point& idx_origin) 
	{
		size_t width_max = map[0].size();
		size_t height_max = map.size();
		SDL_Point idx_next = idx_origin;
		while (true) 
		{
			if (idx_next.x >= width_max || idx_next.y >= height_max)
				break;
			if (check_duplicate_idx(idx_next))
				break;
			else
				idx_list.push_back(idx_next);
			bool is_next_dir_exist = true;
			const Tile& tile = map[idx_next.y][idx_next.x];
			if (tile.special_flag == 0) 
				break;
			switch (tile.direction)
			{
			case Tile::Direction::up:
				idx_next.y--;
				break;
			case Tile::Direction::Down:
				idx_next.y++;
				break;
			case Tile::Direction::Left:
				idx_next.x--;
				break;
			case Tile::Direction::Right:
				idx_next.x++;
				break;
			default:
				is_next_dir_exist = false;
				break;
			}
			if (!is_next_dir_exist)
				break;
			
			
		}
		
	}
	~Route() = default;
	const IdxList& get_idx_list() const
	{
		return idx_list;
	}
private:
	IdxList idx_list;

private:
	bool check_duplicate_idx(const SDL_Point& target_idx) 
	{
		for (const SDL_Point& idx : idx_list) 
		{
			if (target_idx.x == idx.x && target_idx.y == idx.y)
				return true;
		}
		return false;
	}
};

#endif // !_ROUTE_H_
