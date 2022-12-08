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
		namespace concepts
			{
			template <typename T>
			concept game_loop = requires(T t)
				{
				requires std::same_as<decltype(t.step), std::function<bool(float       )>>;
				requires std::same_as<decltype(t.draw), std::function<void(float, float)>>;

				{ t.run() } -> std::same_as<void>;

				{ t.update_delta_time   () } -> std::same_as<float>;
				{ t.update_interpolation() } -> std::same_as<float>;
				};
			}

		class fixed_game_speed_variable_framerate
			{
			private:
				const float steps_per_second = 1.f;
				const size_t max_frameskip = 5;

			public:
				std::function<bool(float       )> step; // step returns a bool which tells the loop to keep looping or not
				std::function<void(float, float)> draw;

				fixed_game_speed_variable_framerate(float steps_per_second = 1.f, size_t max_frameskip = 5) noexcept :
					steps_per_second{steps_per_second}, max_frameskip{max_frameskip}, step_delta_time{std::chrono::duration<float>{1.f / steps_per_second}}
					{}

				void run()
					{
					// https://dewitters.com/dewitters-gameloop/
					utils::clock<std::chrono::high_resolution_clock, float> fps_clock;
					           clock.restart();
					delta_time_clock.restart();

					next_step_time = clock.get_elapsed();
					interpolation  = 0.f;

					size_t   step_loops    {0};
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
							delta_time_clock.restart();
							}
						step_loops = 0;

						update_interpolation();

						frames_counter++;
						draw(update_delta_time(), interpolation);
						}
					}

				float update_interpolation() { return interpolation = (clock.get_elapsed() + step_delta_time - next_step_time) / step_delta_time; }
				float update_delta_time   () { return delta_time_clock.get_elapsed().count(); }

			private:
				std::chrono::duration<float> step_delta_time{};
				std::chrono::duration<float> next_step_time {};

				float interpolation {0.f};

				utils::clock<std::chrono::high_resolution_clock, float>            clock;
				utils::clock<std::chrono::high_resolution_clock, float> delta_time_clock;
			};
		static_assert(concepts::game_loop<fixed_game_speed_variable_framerate>);
		
		class fixed_fps_and_game_speed
			{
			private:
				const float steps_per_second = 1.f;

				std::chrono::duration<float> step_delta_time;

			public:
				std::function<bool(float       )> step; // step returns a bool which tells the loop to keep looping or not
				std::function<void(float, float)> draw;

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

				float update_interpolation() { return 0.f; }
				float update_delta_time()    { return 0.f; }

			private:
			};
		static_assert(concepts::game_loop<fixed_fps_and_game_speed           >);
		
		class variable_fps_and_game_speed
			{
			private:

			public:
				std::function<bool(float)> step; // step returns a bool which tells the loop to keep looping or not
				std::function<void(float, float)> draw;

				variable_fps_and_game_speed() noexcept {}

				void run()
					{
					// https://dewitters.com/dewitters-gameloop/
					clock.restart();
					curr_step_time = clock.get_elapsed();

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
						
						update_delta_time();

						if (!step(step_delta_time.count())) { break; };
						frames_counter++;
						draw(step_delta_time.count(), 0);
						}
					}

				float update_interpolation() { return 0; }
				float update_delta_time() 
					{
					curr_step_time = clock.get_elapsed();
					step_delta_time = curr_step_time - prev_step_time;
					return step_delta_time.count();
					}

			private:
				utils::clock<std::chrono::high_resolution_clock, float> clock;
				std::chrono::duration<float> prev_step_time;
				std::chrono::duration<float> curr_step_time;
				std::chrono::duration<float> step_delta_time;
			};
		static_assert(concepts::game_loop<variable_fps_and_game_speed        >);
		}
	}