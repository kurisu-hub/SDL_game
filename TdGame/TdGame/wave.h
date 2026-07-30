#ifndef _WAVE_H_
#define _WAVE_H_

#include"enemy_type.h"

#include<vector>

struct Wave 
{
	struct SpawnEvent
	{	
		//时间间隔
		double interval = 0;
		int spawn_point = 1;
		//默认生成敌人的类型
		EnemyType enemy_type = EnemyType::Slim;
	};

	//加载
	double rewards = 0;
	double interval = 0;
	std::vector<SpawnEvent>spawn_event_list;
	
};
#endif // !_WAVE_H_

