#ifndef _BULLET_MANAGER_H_
#define _BULLET_MANAGER_H_

#include"manager.h"
#include"bullet.h"
#include"bullet_type.h"
#include"arrow_bullet.h"
#include"axe_bullet.h"
#include"shell_bullet.h"

class BulletManager :public Manager<BulletManager>
{
	friend class Manager<BulletManager>;

public:
	typedef std::vector<Bullet*>BulletList;

public:
	void on_update(double delta)
	{
		for (Bullet* bullet : bullet_list)
			bullet->on_update(delta);

		// 遍历结束后统一回收，避免多颗子弹同帧消失时迭代器失效。
		bullet_list.erase(std::remove_if(bullet_list.begin(), bullet_list.end(),
			[](Bullet* bullet)
			{
				if (!bullet->can_remove()) return false;
				delete bullet;
				return true;
			}), bullet_list.end());
	}
	
	void on_render(SDL_Renderer* renderer)
	{
		for (Bullet* bullet : bullet_list)
		{
			bullet->on_render (renderer);
		}
	}

	BulletList& get_bullet_list()
	{
		return bullet_list;
	}

	void fire_bullet(BulletType type,const Vector2&position,const Vector2&velocity,double damage)
	{
		Bullet* bullet = nullptr;

		switch (type)
		{
		case Arrow:
			bullet = new ArrowBullet();
			break;
		case Axe:
			bullet = new AxeBullet();
			break;
		case Shell:
			bullet = new ShellBullet();
			break;
		default:
			bullet = new ArrowBullet();
			break;
		}

		bullet->set_position(position);
		bullet->set_damage(damage);

		bullet->set_velocity(velocity);
		
		bullet_list.push_back(bullet);
	}

protected:
	BulletManager() = default;

	
	~BulletManager()
	{
		for (Bullet* bullet : bullet_list)
		{
			delete bullet;
		}
	}

private:
	BulletList bullet_list;
};


#endif // !_BULLET_MANAGER_H_
