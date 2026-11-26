#ifndef _TOWER_MANAGER_H_
#define _TOWER_MANAGER_H_

#include"manager.h"
#include"tower.h"
#include"tower_type.h"
#include"archer_tower.h"
#include"gunner_tower.h"
#include"axeman_tower.h"
#include"config_manager.h"
#include"resources_manager.h"

#include"vector"

class TowerManager:public Manager<TowerManager>

{
	friend class Manager<TowerManager>;

public:
	void on_update(double delta)
	{
		for (const auto& entry : tower_list)
		{
			entry.tower->on_update(delta);
		}
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (const auto& entry : tower_list)
		{
			entry.tower->on_render(renderer);
		}
	}

	const Tower* find_tower(SDL_Point idx) const
	{
		for (const auto& entry : tower_list)
			if (entry.idx.x == idx.x && entry.idx.y == idx.y) return entry.tower;
		return nullptr;
	}

	void place_tower(TowerType type, const SDL_Point& idx)
	{
		auto* config = ConfigManager::instance();
		const double price = config->tower_stats(type).cost[0];
		auto* coins = CoinManager::instance();
		if (coins->get_current_coin_num() < price || find_tower(idx)) return;

		Tower* tower = nullptr;
		
		switch (type)
		{
		case Archer:
			tower = new ArcherTower();
			break;
		case Axeman:
			tower = new AxemanTower();
			break;
		case Gunner:
			tower = new GunnerTower();
			break;
		default:
			tower = new ArcherTower();
			break;
		}

		static Vector2 position;
		static const SDL_Rect& rect = ConfigManager::instance()->rect_tile_map;

		position.x = rect.x +idx.x * SIZE_TILE +SIZE_TILE / 2;
		position.y = rect.y+idx.y * SIZE_TILE + SIZE_TILE / 2;
		tower->set_position(position);
		tower_list.push_back({idx, tower, price});
		coins->decrease_coin(price);
		config->map.set_tower_occupied(idx, true);
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
		
	}

	double refund_at(SDL_Point idx) const
	{
		for (const auto& entry : tower_list)
			if (entry.idx.x == idx.x && entry.idx.y == idx.y) return entry.build_price;
		return 0;
	}

	void upgrade_at(SDL_Point idx)
	{
		for (const auto& entry : tower_list)
			if (entry.idx.x == idx.x && entry.idx.y == idx.y)
			{
				entry.tower->buy_upgrade();
				return;
			}
	}

	// 按建造时实付金额退款；移除记录后，同一格不能重复领取。
	void withdraw_at(SDL_Point idx)
	{
		for (auto it = tower_list.begin(); it != tower_list.end(); ++it)
		{
			if (it->idx.x != idx.x || it->idx.y != idx.y) continue;
			CoinManager::instance()->increase_coin(it->build_price);
			ConfigManager::instance()->map.set_tower_occupied(idx, false);
			delete it->tower;
			tower_list.erase(it);
			return;
		}
	}

protected:
	TowerManager() = default;
	~TowerManager()
	{
		for (const auto& entry : tower_list)
			delete entry.tower;
	}
private:
	struct Entry
	{
		SDL_Point idx;
		Tower* tower;
		double build_price;
	};
	std::vector<Entry> tower_list;
};


#endif // !_TOWER_MANAGER_H_
