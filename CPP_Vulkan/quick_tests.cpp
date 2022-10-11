#include <thread>
#include <utils/logger.h>
#include <utils/containers/multithreading/self_consuming_queue.h>

void pluto(int&)
	{
	std::cout << "pippo";
	}

void pippo()
	{
	while (true)
		utils::globals::logger.log("pippo");
	}

int mainf()
	{
	utils::containers::multithreading::self_consuming_queue<int> q(pluto);
	return 0;
	}

int maing()
	{
	utils::logger<utils::message> new_logger{"main.txt"};
	std::thread t_pippo{ &pippo };
	while (true) { new_logger.log("main"); }
	}
