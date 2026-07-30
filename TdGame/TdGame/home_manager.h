#ifndef _HOME_MANAGER_H_
#define _HOME_MANAGER_H_

#include"manager.h"
#include"config_manager.h"
#include"resources_manager.h"

class HomeManager :public Manager<HomeManager>	
{
	friend class Manager<HomeManager>;
public:

	double get_current_hp()
	{
		return num_hp;
	}
	
	void decrease_hp(double val)
	{
		num_hp -= val;
		if (num_hp < 0)
			num_hp = 0;
		
		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();
		
		//混合音道，-1是随便找个音道，0是指不循环播放
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_HomeHurt)->second, 0);
	}

protected:
	HomeManager()
	{
		num_hp = ConfigManager::instance()->num_initial_hp;
	}
	

	~HomeManager() = default;

private:
	double num_hp = 0;
};


#endif // !_HOME_MANAGER_H_
