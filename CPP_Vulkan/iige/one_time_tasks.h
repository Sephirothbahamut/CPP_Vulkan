#pragma once

#include <functional>

#include <utils/containers/multithreading/concurrent_queue.h>

namespace iige
	{
	class one_time_tasks
		{
		using inner_container_t = utils::containers::multithreading::concurrent_queue<std::function<void()>>;

		public:
			void push(const std::function<void()>& callable) { inner_container.enqueue(callable); }
			void execute()
				{
				std::function<void()> callable;
				while (inner_container.try_dequeue(callable)) { callable(); }
				}

		private:
			inner_container_t inner_container;
		};
	}