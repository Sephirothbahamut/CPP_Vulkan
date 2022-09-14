#pragma once

#include <vector>
#include <functional>
#include <chrono>

#include <utils/memory.h>
#include <utils/logger.h>

#include "../utils/clock.h"

namespace iige
	{
	namespace loop
		{
		namespace details
			{
			class base_loop
				{
				public:
					std::function<bool(float       )> step; // step returns a bool which tells the loop to keep looping or not
					std::function<void(float, float)> draw;

				public:
					virtual void run() = 0;
				};
			}

		class fixed_game_speed_variable_framerate : public details::base_loop
			{
			private:
				const float steps_per_second = 1.f;
				const size_t max_frameskip = 5;

				std::chrono::duration<float> step_delta_time;

			public:
				fixed_game_speed_variable_framerate(float steps_per_second = 1.f, size_t max_frameskip = 5) noexcept :
					steps_per_second{steps_per_second}, max_frameskip{max_frameskip}, step_delta_time{std::chrono::duration<float>(1.f / steps_per_second)}
					{}

				virtual void run() final override
					{
					// https://dewitters.com/dewitters-gameloop/

					utils::clock<std::chrono::high_resolution_clock, float> clock;
					std::chrono::duration<float> next_step_time{clock.get_elapsed()};
					size_t step_loops{0};
					float interpolation{0};

					utils::clock<std::chrono::high_resolution_clock, float> fps_clock;
					uint32_t frames_counter{0};

					while (true)
						{
						if (fps_clock.get_elapsed() > std::chrono::seconds(1))
							{
							utils::globals::logger.log("FPS: " + std::to_string(frames_counter / fps_clock.restart().count()));
							frames_counter = 0;
							}
						while (clock.get_elapsed() > next_step_time && step_loops < max_frameskip)
							{
							if (!step(step_delta_time.count())) { break; };
							step_loops++;
							next_step_time += step_delta_time;
							}
						step_loops = 0;

						interpolation = (clock.get_elapsed() + step_delta_time - next_step_time) / step_delta_time;

						frames_counter++;
						draw(step_delta_time.count(), interpolation);
						}
					}
			};
		
		class fixed_fps_and_game_speed : public details::base_loop
			{
			private:
				const float steps_per_second = 1.f;

				std::chrono::duration<float> step_delta_time;

			public:
				fixed_fps_and_game_speed(float steps_per_second = 1.f) noexcept :
					steps_per_second{steps_per_second}, step_delta_time{std::chrono::duration<float>(1.f / steps_per_second)}
					{}

				void run()
					{
					// https://dewitters.com/dewitters-gameloop/

					utils::clock<std::chrono::high_resolution_clock, float> clock;

					std::chrono::duration<float> next_step_time = clock.get_elapsed();

					std::chrono::duration<float> sleep_time{std::chrono::seconds(0)};

					utils::clock<std::chrono::high_resolution_clock, float> fps_clock;
					uint32_t frames_counter{0};

					while (true)
						{
						if (fps_clock.get_elapsed() > std::chrono::seconds(1))
							{
							utils::globals::logger.log("FPS: " + std::to_string(frames_counter / fps_clock.restart().count()));
							frames_counter = 0;
							}
						
						if (!step(step_delta_time.count())) { break; };

						frames_counter++;
						draw(step_delta_time.count(), 0);

						next_step_time += step_delta_time;
						sleep_time = next_step_time - clock.get_elapsed();
						if (sleep_time >= std::chrono::seconds(0)) { std::this_thread::sleep_for(sleep_time); }
						}
					}
			};
		
		class variable_fps_and_game_speed : public details::base_loop
			{
			private:

			public:
				variable_fps_and_game_speed() noexcept {}

				void run()
					{
					// https://dewitters.com/dewitters-gameloop/
					utils::clock<std::chrono::high_resolution_clock, float> clock;

					std::chrono::duration<float> prev_step_time;
					std::chrono::duration<float> curr_step_time{clock.get_elapsed()};

					std::chrono::duration<float> step_delta_time;

					utils::clock<std::chrono::high_resolution_clock, float> fps_clock;
					uint32_t frames_counter{0};

					while (true)
						{
						if (fps_clock.get_elapsed() > std::chrono::seconds(1))
							{
							utils::globals::logger.log("FPS: " + std::to_string(frames_counter / fps_clock.restart().count()));
							frames_counter = 0;
							}
						
						prev_step_time = curr_step_time;
						curr_step_time = clock.get_elapsed();

						step_delta_time = curr_step_time - prev_step_time;

						if (!step(step_delta_time.count())) { break; };
						frames_counter++;
						draw(step_delta_time.count(), 0);
						}
					}

			};
		
		enum class timing { fixed, variable };
		}
	}