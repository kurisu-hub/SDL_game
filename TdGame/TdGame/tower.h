#ifndef _TOWER_H_
#define _TOWER_H_

#include"vector2.h"
#include"animation.h"
#include"tower_type.h"
#include"bullet_manager.h"
#include"facing.h"
#include"enemy_manager.h"
#include"coin_manager.h"

class Tower
{
public:
	Tower()
	{	
		timer_fire.set_one_shot(true);
		timer_fire.set_on_timeout(
			[&]()
			{
				can_fire = true;
			}

		);
		
		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.2);
		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.2);
		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.2);
		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.2);

		anim_fire_up.set_loop(false);
		anim_fire_up.set_interval(0.2);
		anim_fire_up.set_on_finished(
			[&]()
			{
				update_idle_animation();
			}
		);
		anim_fire_down.set_loop(false);
		anim_fire_down.set_interval(0.2);
		anim_fire_down.set_on_finished(
			[&]()
			{
				update_idle_animation();
			}
		);
		anim_fire_left.set_loop(false);
		anim_fire_left.set_interval(0.2);
		anim_fire_left.set_on_finished(
			[&]()
			{
				update_idle_animation();
			}
		);
		anim_fire_right.set_loop(false);
		anim_fire_right.set_interval(0.2);
		anim_fire_right.set_on_finished(
			[&]()
			{
				update_idle_animation();
			}
		);
	}

	virtual ~Tower() = default;

	int get_level() const { return level; }

	double upgrade_price() const
	{
		return level == 10 ? -1 : ConfigManager::instance()->tower_stats(tower_type).upgrade_cost[level - 1];
	}

	// 只修改本塔等级；下次索敌和发射时使用新一级的属性。
	bool buy_upgrade()
	{
		const double price = upgrade_price();
		auto* coins = CoinManager::instance();
		if (price < 0 || coins->get_current_coin_num() < price) return false;
		coins->decrease_coin(price);
		++level;
		auto& sounds = ResourcesManager::instance()->get_sound_pool();
		Mix_PlayChannel(-1, sounds.at(ResID::Sound_TowerLevelUp), 0);
		return true;
	}

	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	const Vector2& get_size()const
	{
		return size;
	}

	const Vector2& get_position()const
	{
		return position;
	}
	
	void on_update(double delta)
	{
		timer_fire.on_update(delta);
		anim_current->on_update(delta);

		if (can_fire)
			on_fire();

	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point;
		
		point.x = (int)(position.x -size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		
		anim_current->on_render(renderer, point);
	}
protected:
	Vector2 size;
	
	//默认动画
	Animation anim_idle_up;
	Animation anim_idle_down;
	Animation anim_idle_left;
	Animation anim_idle_right;

	//发射动画
	Animation anim_fire_up;
	Animation anim_fire_down;
	Animation anim_fire_left;
	Animation anim_fire_right;
	
	TowerType tower_type = TowerType::Archer;
	
	double fire_speed = 0;
	BulletType bullet_type = BulletType::Arrow;
private:
	int level = 1;
	Timer timer_fire;
	Vector2 position;
	bool can_fire = true;
	Facing facing = Facing::Right;
	Animation* anim_current = &anim_idle_right;

private:
	//切换使用哪个动画
	void update_idle_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_idle_left;
			break;
		case Right:
			anim_current = &anim_idle_right;
			break;
		case Up:
			anim_current = &anim_idle_up;
			break;
		case Down:
			anim_current = &anim_idle_down;
			break;
		}
	}
	void update_fire_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_fire_left;
			break;
		case Right:
			anim_current = &anim_fire_right;
			break;
		case Up:
			anim_current = &anim_fire_up;
			break;
		case Down:
			anim_current = &anim_fire_down;
			break;
		default:
			break;
		}
	}
	
	Enemy* find_target_enemy()
	{	
		//逻辑：谁接近家，优先攻击谁
		double process = -1;
		const double view_range = ConfigManager::instance()->tower_stats(tower_type).view_range[level - 1];
		Enemy* enemy_target = nullptr;
		EnemyManager::EnemyList& enemy_list = EnemyManager::instance()->get_enemy_list();
		
		for (Enemy* enemy : enemy_list)
		{
			if ((enemy->get_position() - position).length() <= view_range * SIZE_TILE)
			{
				double new_process = enemy->get_route_process();
				if (new_process > process)
				{
					enemy_target = enemy;
					process = new_process;
				}
			}
		}
		return enemy_target;
		
	}
	
	//�����߼�
	void on_fire()
	{
		Enemy* target_enemy = find_target_enemy();

		if (!target_enemy)return;
		
		can_fire = false;
		static ConfigManager* instance = ConfigManager::instance();
		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		
		const auto& stats = instance->tower_stats(tower_type);
		const double interval = stats.interval[level - 1];
		const double damage = stats.damage[level - 1];

		switch (tower_type)
		{
		case Archer:
			switch (rand()%3)
			{
			case 0:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_1)->second, 0);
				break;
			case 1:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_2)->second, 0);
				break;
			default:
				break;
			}
			break;
		case Axeman:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
			break;
		case Gunner:
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
			break;
		}
		timer_fire.set_wait_time(interval);
		timer_fire.restart();
		
		Vector2 direction = target_enemy->get_position() - position;
		BulletManager::instance()->fire_bullet(bullet_type, position, direction.normalize() * fire_speed * SIZE_TILE, damage);
			
		bool is_show_x_anim = abs(direction.x) >= abs(direction.y);
		if (is_show_x_anim)
			facing = direction.x > 0 ? Facing::Right : Facing::Left;
		else
			facing = direction.y > 0 ? Facing::Down : Facing::Up;

		update_fire_animation();

		//��ͷ��ʼ����
		anim_current->reset();
	}
};


#endif // !_TOWER_H_
