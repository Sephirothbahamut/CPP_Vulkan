#include <thread>
#include <utils/logger.h>
#include <utils/containers/self_consuming_queue.h>

void pluto(std::vector<int>&)
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
	utils::containers::self_consuming_queue<int> q(pluto);
	return 0;
	}

int maing()
	{
	utils::logger<utils::message> new_logger{"main.txt"};
	std::thread t_pippo{ &pippo };
	while(true)
		new_logger.log("main");
	}