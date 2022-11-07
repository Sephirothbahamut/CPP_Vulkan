#pragma once

#include <tuple>
#include <atomic>
#include <future>
#include <utility>
#include <concepts>
#include <functional>
#include <filesystem>
#include <unordered_map>

#include <utils/containers/multihandled_default.h>
#include <utils/construct.h>
#include <utils/logger.h>
#include <utils/future.h>
#include <utils/thread_pool.h>

#include "one_time_tasks.h"

namespace iige::asset
	{
	template <typename T>
	class manager// manages only one type of asset at a time
		{
		public:
			using factory_t = std::function<T()>; 
			using unload_callback_t = std::function<void(T&)>;
			using handled_container_t = utils::containers::multihandled_default<T>;

			using handle_t = handled_container_t::handle_t;
			
			using value_type      = handled_container_t::value_type     ;
			using size_type       = handled_container_t::size_type      ;
			using reference       = handled_container_t::reference      ;
			using const_reference = handled_container_t::const_reference;
			using pointer         = handled_container_t::pointer        ;
			using const_pointer   = handled_container_t::const_pointer  ;

			class factories_manager_t
				{
				public:
					std::optional<factory_t> get(const std::string& name)
						{
						std::lock_guard lock{access_mutex};

						auto factories_it{factories.find(name)};
						if (factories_it == factories.end())
							{
							utils::globals::logger.err("Could not find factory for asset \"" + name + "\".");

							return std::nullopt;
							}
						return factories_it->second;
						}

					factory_t set(const std::string& name, factory_t factory)
						{
						std::lock_guard lock{access_mutex};

						auto factory_it{factories.find(name)};
						if (factory_it != factories.end())
							{
							utils::globals::logger.err("asset \"" + name + "\" already had a factory.");
							return factory_it->second;
							}
						else
							{
							factories[name] = factory;
							}
						return factory;
						}

				private:
					std::unordered_map<std::string, factory_t> factories;

					std::mutex access_mutex;
				};

			class multihandled_manager_t
				{
				public:
					using handle_t = utils::containers::multihandled_default<T>::handle_t;

					multihandled_manager_t(factory_t default_asset_factory) : container{std::move(default_asset_factory())} {}

					handle_t emplace(const std::string& name, factory_t factory) noexcept 
						{
						try
							{
							T value{factory()};

							std::lock_guard lock{access_mutex};
							return container.emplace(std::move(value));
							}
						catch (const std::exception& e)
							{
							utils::globals::logger.err("Failed to load asset \"" + name + "\"!\n" + e.what());

							std::lock_guard lock{access_mutex};
							return container.clone_default();
							}
						}

					inline void emplace_at(handle_t& handle, value_type&& asset) noexcept
						{
						std::lock_guard lock{access_mutex};
						container.emplace_at(handle, std::move(asset));
						}

					handle_t clone_default() noexcept 
						{
						std::lock_guard lock{access_mutex};
						return container.clone_default();
						}

					void remap_and_erase_target_handle(handle_t& handle_to_remap, const handle_t& handle_remap_target) noexcept
						{
						std::lock_guard lock{access_mutex};
						container.remap_and_erase_target_handle(handle_to_remap, handle_remap_target);
						}

					void erase(handle_t& handle) noexcept 
						{
						std::lock_guard lock{access_mutex};
						container.erase(handle); 
						}

				private:
					utils::containers::multihandled_default<T> container;

					std::mutex access_mutex;
				};

		public:
			/// <summary>
			/// 
			/// </summary>
			/// <param name="thread_pool">The thread pool where asynchronous load tasks are pushed.</param>
			/// <param name="one_time_tasks">A task will be added to this pool when a flush should be performed.</param>
			/// <typeparam name="T"></typeparam>
			manager(utils::thread_pool& thread_pool, iige::one_time_tasks& one_time_tasks, factory_t default_asset_factory) : thread_pool_ptr{&thread_pool}, one_time_tasks_ptr{&one_time_tasks}, assets { default_asset_factory } {}
			
			/// <summary>
			/// Loads a assets from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
			/// </summary>
			/// <param name="name">The unique name to associate to this asset.</param>
			/// <param name="factory">A callback that returns the asset to be created.</param>
			/// <returns>An handle to the loaded asset.</returns>
			handle_t load_sync(const std::string& name, factory_t factory)
				{
				return load_sync_inner(name, factories.set(name, factory));
				}

			/// <summary>
			/// Loads a assets from the given factory into the associated name. Requires a factory to have already been associated with that name.
			/// </summary>
			/// <param name="name">The unique name to associate to this asset.</param>
			/// <returns>An handle to the loaded asset.</returns>
			handle_t load_sync(const std::string& name)
				{
				if (auto factory_opt{factories.get(name)})
					{
					return load_sync_inner(name, factory_opt.value());
					}
				else 
					{
					utils::globals::logger.err("Could not find factory for asset \"" + name + "\".");
					return assets.clone_default();
					}
				}

			/// <summary>
			/// Unloads a asset. All the handles in the program will remain valid but they will now point to the default asset.
			/// </summary>
			/// <typeparam name="T"></typeparam>
			void unload_sync(const std::string& name)
				{
				auto eleme_it{ name_to_handle.find(name) };
				if (eleme_it != name_to_handle.end())
					{
					assets.erase(eleme_it->second);
					name_to_handle.erase(eleme_it);
					}
				else
					{
					utils::globals::logger.err("Failed to unload asset \"" + name + "\";the asset did not exist.\n");
					}
				}

			/// <summary>
			/// Asynchronously loads a assets from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
			/// The returned handle will point to the default asset until loading is completed and flush_loaded() is called.
			/// </summary>
			/// <param name="name">The unique name to associate to this asset.</param>
			/// <param name="factory">A callback that returns the asset to be created.</param>
			/// <returns>An handle to the loaded asset, or to the default asset if the factory failed. Will point to the default asset until loading is completed.</returns>
			handle_t load_async(const std::string& name, factory_t factory)
				{
				return load_async_inner(name, factories.set(name, factory));
				}

			/// <summary>
			/// Loads a assets from the given factory into the associated name. Requires a factory to have already been associated with that name.
			/// </summary>
			/// <param name="name">The unique name to associate to this asset.</param>
			/// <returns>An handle to the loaded asset, or to the default asset if the factory failed or was missing.</returns>
			handle_t load_async(const std::string& name)
				{
				if (auto factory_opt{factories.get(name)})
					{
					return load_async_inner(name, factory_opt.value());
					}
				else
					{
					utils::globals::logger.err("Could not find factory for asset \"" + name + "\".");
					return assets.clone_default();
					}
				}

			void unload_async(const std::string& name)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end())
					{
					auto handle{eleme_it->second};
					thread_pool_ptr->push_task([handle]() { assets.erase(handle); });
					name_to_handle.erase(eleme_it);
					}
				else
					{
					utils::globals::logger.err("Failed to unload asset \"" + name + "\";the asset did not exist.\n");
					}
				}

			factories_manager_t factories;

			// Dereferencing handles during this method is NOT safe.
			void flush()
				{
				should_flush = false;
				auto loaded_assets{loaded_buffer.swap_and_get()};
				for (auto& asset : loaded_assets)
					{
					assets.emplace_at(asset.handle_to_replace, std::move(asset.element));
					}
				}

		private:
			utils::observer_ptr<utils::thread_pool   > thread_pool_ptr;
			utils::observer_ptr<iige ::one_time_tasks> one_time_tasks_ptr;
			std::unordered_map<std::string, typename handled_container_t::handle_t> name_to_handle;
			multihandled_manager_t assets;

			struct loaded_buffer_value_type
				{
				handle_t handle_to_replace;
				std::string name;
				T element;
				};

			handle_t load_sync_inner(const std::string& name, factory_t factory)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end()) { return eleme_it->second; }
				else { return assets.emplace(name, factory); }
				}

			handle_t load_async_inner(const std::string& name, factory_t factory)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end()) { return eleme_it->second; }
				else
					{
					handle_t handle{assets.clone_default()};

					thread_pool_ptr->push_task([this, handle, name, factory]()
						{
						try
							{
							auto loaded_asset{factory()};

							loaded_buffer.emplace(handle, name, std::move(loaded_asset));

							//Ensure that flush isn't pushed multiple times for the same elements
							if (!should_flush)
								{
								should_flush = true;
								one_time_tasks_ptr->push([this]() { flush(); });
								}
							}
						catch (const std::exception& e) { utils::globals::logger.err("Failed to load asset \"" + name + "\"!\n" + e.what()); }
						});

					return handle;
					}
				}

			std::atomic_bool should_flush{false};
			utils::containers::multithreading::producer_consumer_queue<loaded_buffer_value_type> loaded_buffer;
		};
		
	namespace concepts
		{
		template<typename manager_t>
		concept manager_sync = requires(manager_t manager)
			{
				{ manager.load_sync   (std::string{}, typename manager_t::factory_t{}) } -> std::convertible_to<typename manager_t::handle_t>;
				{ manager.load_sync   (std::string{}                                 ) } -> std::convertible_to<typename manager_t::handle_t>;
				{ manager.unload_sync (std::string{}                                 ) } -> std::same_as<void>;
			};

		template<typename manager_t>
		concept manager_async = requires(manager_t manager)
			{
				{ manager.load_async  (std::string{}, typename manager_t::factory_t{}) } -> std::convertible_to<typename manager_t::handle_t>;
				{ manager.load_async  (std::string{}                                 ) } -> std::convertible_to<typename manager_t::handle_t>;
				{ manager.unload_async(std::string{}                                 ) } -> std::same_as<void>;
			};

		template<typename manager_t>
		concept manager = manager_sync<manager_t> && manager_async<manager_t>;

		}
	}