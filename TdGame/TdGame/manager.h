#ifndef _MANAGER_H_
#define _MANAGER_H_
template<typename T>
class Manager
{
public:
	static T* instance()
	{
		if (!manager)
			manager = new T;
		return manager;

	}

	// 释放本关实例；下次访问时根据新关卡配置重新创建。
	static void release()
	{
		delete manager;
		manager = nullptr;
	}

protected:
	Manager() = default;
	~Manager() = default;
	Manager(const Manager&) = delete;
	Manager& operator=(const Manager&) = delete;
private:
	static T* manager;
};
template<typename T>
T* Manager<T>::manager = nullptr;
#endif // !_MANAGER_H_
