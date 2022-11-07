#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <functional>
#include <filesystem>
#include <unordered_map>

#include <utils/containers/multihandled_default.h>
#include <utils/containers/hive/next.h>
#include <utils/construct.h>
#include <utils/logger.h>
#include <utils/containers/multithreading/concurrent_queue.h>
#include <utils/thread_pool.h>

#include "resources_manager_sync.h"

namespace iige::resource
	{
	template <typename T>
	class manager_async // manages only one type of resource at a time
		{
		public:
			using factory_t = std::function<T()>;
			using unload_callback_t = std::function<void(T&)>;
			using handled_container_t = utils::containers::multihandled_default<T>;

			using handle_t = handled_container_t::handle_t;

			using value_type = handled_container_t::value_type;
			using size_type = handled_container_t::size_type;
			using reference = handled_container_t::reference;
			using const_reference = handled_container_t::const_reference;
			using pointer = handled_container_t::pointer;
			using const_pointer = handled_container_t::const_pointer;

			struct handle_and_status_t : public handle_t
				{
				//implicit cast
				handle_and_status_t(const handle_t& src, bool unloaded = false, bool loading = false) : handle_t{src}, unloaded{unloaded}, loading{loading} {}
				operator handle_t() const noexcept { return *this; }

				bool unloaded{false};
				bool loading{false};
				};

			class factories_manager
				{
				public:
					std::optional<factory_t> get(const std::string& name)
						{
						std::lock_guard lock{access_mutex};

						auto factories_it{factories.find(name)};
						if (factories_it == factories.end())
							{
							utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");

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
							utils::globals::logger.err("Resource \"" + name + "\" already had a factory.");
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

		public:
			manager_async(utils::thread_pool& thread_pool, factory_t factory/*, unload_callback_t unload_callback = [](T&){}*/) : thread_pool_ptr{&thread_pool}, handled_container{factory()} {}//, unload_callback{unload_callback} {}

			/// <summary>
			/// Loads a resources from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
			/// </summary>
			/// <param name="name">The unique name to associate to this resource.</param>
			/// <param name="factory">A callback that returns the resource to be created.</param>
			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed.</returns>
			handle_and_status_t load_sync(const std::string& name, factory_t factory)
				{
				return load_sync_inner(name, factories.set(name, factory));
				}

			/// <summary>
			/// Loads a resources from the given factory into the associated name. Requires a factory to have already been associated with that name.
			/// </summary>
			/// <param name="name">The unique name to associate to this resource.</param>
			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed or was missing.</returns>
			handle_and_status_t load_sync(const std::string& name)
				{
				if (auto factory_opt{factories.get(name)})
					{
					return load_sync_inner(name, factory_opt.value());
					}
				else
					{
					utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
					return handled_container.get_default();
					}
				}

			/// <summary>
			/// Unloads a resource. All the handles in the program will remain valid but they will now point to the default resource.
			/// </summary>
			/// <typeparam name="T"></typeparam>
			void unload_sync(const std::string& name)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end())
					{
					eleme_it->second.unloaded = true;
					handled_container.reset_handle(eleme_it->second);
					}
				else
					{
					utils::globals::logger.err("Failed to unload resource \"" + name + "\";the resource did not exist.\n");
					}
				}

			/// <summary>
			/// Asynchronously loads a resources from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
			/// The returned handle will point to the default resource until loading is completed and flush_loaded() is called.
			/// </summary>
			/// <param name="name">The unique name to associate to this resource.</param>
			/// <param name="factory">A callback that returns the resource to be created.</param>
			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed. Will point to the default resource until loading is completed.</returns>
			handle_and_status_t load_async(const std::string& name, factory_t factory)
				{
				return load_async_inner(name, factories.set(name, factory));
				}

			/// <summary>
			/// Loads a resources from the given factory into the associated name. Requires a factory to have already been associated with that name.
			/// </summary>
			/// <param name="name">The unique name to associate to this resource.</param>
			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed or was missing.</returns>
			handle_and_status_t load_async(const std::string& name)
				{
				if (auto factory_opt{factories.get(name)})
					{
					return load_async_inner(name, factory_opt.value());
					}
				else
					{
					utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
					return handled_container.get_default();
					}
				}

			//TODO check, consider removing
			//void unload_async(const std::string& name)
			//	{
			//	auto eleme_it{ name_to_handle.find(name) };
			//	if (eleme_it != name_to_handle.end())
			//		{
			//		eleme_it->second.unloaded = true; 
			//		
			//		unloading_queue.emplace(name, eleme_it->second);
			//		}
			//	else
			//		{
			//		utils::globals::logger.err("Failed to unload resource \"" + name + "\"; the resource did not exist.\n");
			//		}
			//	}

			handle_t get_default() noexcept
				{
				return handled_container.get_default();
				}

			/// <summary>
			/// Moves resources loaded by the second thread in the active resources pool.
			/// </summary>
			/// <typeparam name="T"></typeparam>
			void flush_loaded()
				{
				std::unique_lock lock_load_calls{mutex_load_calls};

				auto& elements{loaded_buffer.swap_and_get()};
				for (auto& element : elements)
					{
					auto eleme_it{name_to_handle.find(element.name)};
					if (eleme_it != name_to_handle.end())
						{
						if (eleme_it->second.loading)
							{
							eleme_it->second.unloaded = false;
							auto& previous_handle = eleme_it->second;

							auto handle{handled_container.emplace(std::move(element.element))};
							previous_handle.remap(handle);
							}
						//else was loaded multiple times and someone came first, ignore
						// ^ akshually shouldn't happen because we don't push stuff to be loaded and we skip load sync if "loading" is true
						else if constexpr (utils::compilation::debug) { throw "NO"; }
						}
					else
						{
						utils::globals::logger.err(""); //TODO error message
						}
					}
				}

			/// <summary>
			/// Flushes all the pending loads in the second thread.
			/// </summary>
			void flush_loading()
				{
				//TODO
				}

			factories_manager factories;

		private:
			utils::thread_pool* thread_pool_ptr;

			handle_and_status_t load_sync_inner(const std::string& name, factory_t factory)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end())
					{
					if (eleme_it->second.unloaded && !eleme_it->second.loading)
						{
						eleme_it->second.unloaded = false;
						auto& previous_handle = eleme_it->second;

						// Reload
						try
							{
							auto handle{handled_container.emplace(factory())};
							previous_handle.remap(handle);
							}
						catch (const std::exception& e)
							{
							utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());
							//After unload previous_handle is already set to default
							}
						}
					return eleme_it->second;
					}
				else
					{
					try
						{
						auto handle{handled_container.emplace(factory())};

						name_to_handle.emplace(name, handle);
						return handle;
						}
					catch (const std::exception& e)
						{
						utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());

						auto handle{handled_container.get_default()};

						name_to_handle.emplace(name, handle);
						return handle;
						}
					}
				}

			handle_and_status_t load_async_inner(const std::string& name, factory_t factory)
				{
				auto eleme_it{name_to_handle.find(name)};
				if (eleme_it != name_to_handle.end())
					{
					if (eleme_it->second.unloaded && !eleme_it->second.loading)
						{
						eleme_it->second.loading = true;
						loading_queue.emplace(name, factory);
						}
					return eleme_it->second;
					}
				else
					{
					eleme_it->second.loading = true;
					loading_queue.emplace(name, factory);

					auto handle{handled_container.get_default()};
					name_to_handle.emplace(name, handle);
					return handle;
					}
				}

			handled_container_t handled_container;

			std::unordered_map<std::string, handle_and_status_t> name_to_handle;

			struct loading_queue_value_type
				{
				std::string name;
				factory_t factory;
				};
			struct loaded_buffer_value_type
				{
				std::string name;
				T element;
				};

			// No need for map; all names must be unique because elements are added to the loading_queue only if their name doesn't exist or they are unloaded and not loading.
			// Producer is loading_queue's thread, consumer is main thread on calling flush_loaded
			utils::containers::multithreading::concurrent_queue<loaded_buffer_value_type> loaded_buffer;

			utils::containers::multithreading::self_consuming_queue<loading_queue_value_type> loading_queue
				{
				[this](loading_queue_value_type& element) -> void
					{
					using namespace std::string_literals;
					try
						{
						auto loaded_resource{element.factory()};

						loaded_buffer.emplace(element.name, std::move(loaded_resource));
						}
					catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
					}
				};

			//struct unloading_queue_value_type
			//	{
			//	std::string name; //only for debugging, second thread should never interact with the maps
			//	typename handled_container_t::handle_t handle;
			//	};
			//
			//unload_callback_t unload_callback;
			//
			//utils::containers::multithreading::self_consuming_queue<unloading_queue_value_type> unloading_queue
			//	{
			//	[this](unloading_queue_value_type& element) -> void
			//		{
			//		using namespace std::string_literals;
			//		try
			//			{
			//			std::unique_lock lock{ handled_container_mutex };
			//			unload_callback(*element.handle);
			//			handled_container.reset_handle(element.handle);
			//			}
			//		catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
			//		}
			//	};
		};
	//	// TODO add size to template
	//	template <typename T>
	//	class manager_async // manages only one type of resource at a time
	//		{
	//
	//		public:
	//			using factory_t = std::function<T()>; 
	//			using unload_callback_t = std::function<void(T&)>;
	//			using handled_container_t = utils::containers::multihandled_default<T>;
	//
	//			using handle_t = handled_container_t::handle_t;
	//			
	//			using value_type      = handled_container_t::value_type;
	//			using size_type       = handled_container_t::size_type;
	//			using reference       = handled_container_t::reference;
	//			using const_reference = handled_container_t::const_reference;
	//			using pointer         = handled_container_t::pointer;
	//			using const_pointer   = handled_container_t::const_pointer;
	//
	//			struct handle_and_status_t : public handle_t
	//				{
	//				//implicit cast
	//				handle_and_status_t(const handle_t& src, bool unloaded = false, bool loading = false) : handle_t{src}, unloaded{unloaded}, loading{loading} {}
	//				operator handle_t() const noexcept { return *this; }
	//
	//				bool unloaded{false};
	//				bool loading{false};
	//				};
	//
	//		public:
	//			manager_async(factory_t factory/*, unload_callback_t unload_callback = [](T&){}*/) : handled_container{factory()} {}//, unload_callback{unload_callback} {}
	//			
	//			/// <summary>
	//			/// Loads a resources from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
	//			/// </summary>
	//			/// <param name="name">The unique name to associate to this resource.</param>
	//			/// <param name="factory">A callback that returns the resource to be created.</param>
	//			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed.</returns>
	//			handle_and_status_t load_sync(const std::string& name, factory_t factory)
	//				{
	//				std::unique_lock lock{mutex_load_calls};
	//
	//				auto factory_it{ factories.find(name) };
	//				if (factory_it != factories.end())
	//					{
	//					utils::globals::logger.err("Resource \"" + name + "\" already had a factory.");
	//					}
	//				else
	//					{
	//					factories[name] = factory;
	//					}
	//
	//				return load_sync_inner(name, factory);
	//				}
	//
	//			/// <summary>
	//			/// Loads a resources from the given factory into the associated name. Requires a factory to have already been associated with that name.
	//			/// </summary>
	//			/// <param name="name">The unique name to associate to this resource.</param>
	//			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed or was missing.</returns>
	//			handle_and_status_t load_sync(const std::string& name)
	//				{
	//				std::unique_lock lock{mutex_load_calls};
	//
	//				auto factories_it{ factories.find(name) };
	//				if (factories_it == factories.end())
	//					{
	//					utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
	//
	//					return handled_container.get_default();
	//					}
	//
	//				return load_sync_inner(name, factories_it->second);
	//				}
	//
	//			/// <summary>
	//			/// Unloads a resource. All the handles in the program will remain valid but they will now point to the default resource.
	//			/// </summary>
	//			/// <typeparam name="T"></typeparam>
	//			void unload_sync(const std::string& name)
	//				{
	//				std::unique_lock lock{mutex_load_calls};
	//
	//				auto eleme_it{ name_to_handle.find(name) };
	//				if (eleme_it != name_to_handle.end())
	//					{
	//					eleme_it->second.unloaded = true;
	//					handled_container.reset_handle(eleme_it->second);
	//					}
	//				else
	//					{
	//					utils::globals::logger.err("Failed to unload resource \"" + name + "\";the resource did not exist.\n");
	//					}
	//				}
	//
	//			/// <summary>
	//			/// Asynchronously loads a resources from the given factory into the associated name. If multiple factories are provided to the same name the last one will apply.
	//			/// The returned handle will point to the default resource until loading is completed and flush_loaded() is called.
	//			/// </summary>
	//			/// <param name="name">The unique name to associate to this resource.</param>
	//			/// <param name="factory">A callback that returns the resource to be created.</param>
	//			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed. Will point to the default resource until loading is completed.</returns>
	//			handle_and_status_t load_async(const std::string& name, factory_t factory)
	//				{
	//				std::unique_lock lock{mutex_load_calls};
	//
	//				auto factory_it{factories.find(name)};
	//				if (factory_it != factories.end())
	//					{
	//					utils::globals::logger.err("Resource \"" + name + "\" already had a factory.");
	//					}
	//				else
	//					{
	//					factories[name] = factory;
	//					}
	//
	//				return load_async_inner(name, factory);
	//				}
	//
	//			/// <summary>
	//			/// Loads a resources from the given factory into the associated name. Requires a factory to have already been associated with that name.
	//			/// </summary>
	//			/// <param name="name">The unique name to associate to this resource.</param>
	//			/// <returns>An handle to the loaded resource, or to the default resource if the factory failed or was missing.</returns>
	//			handle_and_status_t load_async(const std::string& name)
	//				{
	//				std::unique_lock lock{mutex_load_calls};
	//
	//				auto factories_it{factories.find(name)};
	//				if (factories_it == factories.end())
	//					{
	//					utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
	//
	//					return handled_container.get_default();
	//					}
	//
	//				return load_async_inner(name, factories_it->second);
	//				}
	//
	//			//TODO check, consider removing
	//			//void unload_async(const std::string& name)
	//			//	{
	//			//	auto eleme_it{ name_to_handle.find(name) };
	//			//	if (eleme_it != name_to_handle.end())
	//			//		{
	//			//		eleme_it->second.unloaded = true; 
	//			//		
	//			//		unloading_queue.emplace(name, eleme_it->second);
	//			//		}
	//			//	else
	//			//		{
	//			//		utils::globals::logger.err("Failed to unload resource \"" + name + "\"; the resource did not exist.\n");
	//			//		}
	//			//	}
	//
	//			handle_t get_default() noexcept
	//				{
	//				return handled_container.get_default();
	//				}
	//
	//			std::unordered_map<std::string, factory_t> factories;
	//
	//			/// <summary>
	//			/// Moves resources loaded by the second thread in the active resources pool.
	//			/// </summary>
	//			/// <typeparam name="T"></typeparam>
	//			void flush_loaded()
	//				{
	//				std::unique_lock lock_load_calls{mutex_load_calls};
	//
	//				auto& elements{loaded_buffer.swap_and_get()};
	//				for (auto& element : elements)
	//					{
	//					auto eleme_it{name_to_handle.find(element.name)};
	//					if (eleme_it != name_to_handle.end())
	//						{
	//						if (eleme_it->second.loading)
	//							{
	//							eleme_it->second.unloaded = false;
	//							auto& previous_handle = eleme_it->second;
	//
	//							auto handle{handled_container.emplace(std::move(element.element))};
	//							previous_handle.remap(handle);
	//							}
	//						//else was loaded multiple times and someone came first, ignore
	//						// ^ akshually shouldn't happen because we don't push stuff to be loaded and we skip load sync if "loading" is true
	//						else if constexpr (utils::compilation::debug) { throw "NO"; }
	//						}
	//					else
	//						{
	//						utils::globals::logger.err(""); //TODO error message
	//						}
	//					}
	//				}
	//
	//			/// <summary>
	//			/// Flushes all the pending loads in the second thread.
	//			/// </summary>
	//			void flush_loading() 
	//				{
	//				//TODO
	//				}
	//
	//		private:
	//			std::mutex mutex_load_calls;
	//
	//			handle_and_status_t load_sync_inner(const std::string& name, factory_t factory)
	//				{
	//				auto eleme_it{name_to_handle.find(name)};
	//				if (eleme_it != name_to_handle.end())
	//					{
	//					if (eleme_it->second.unloaded && !eleme_it->second.loading)
	//						{
	//						eleme_it->second.unloaded = false;
	//						auto& previous_handle = eleme_it->second;
	//
	//						// Reload
	//						try
	//							{
	//							auto handle{handled_container.emplace(factory())};
	//							previous_handle.remap(handle);
	//							}
	//						catch (const std::exception& e)
	//							{
	//							utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());
	//							//After unload previous_handle is already set to default
	//							}
	//						}
	//					return eleme_it->second;
	//					}
	//				else
	//					{
	//					try
	//						{
	//						auto handle{handled_container.emplace(factory())};
	//
	//						name_to_handle.emplace(name, handle);
	//						return handle;
	//						}
	//					catch (const std::exception& e)
	//						{
	//						utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());
	//
	//						auto handle{handled_container.get_default()};
	//
	//						name_to_handle.emplace(name, handle);
	//						return handle;
	//						}
	//					}
	//				}
	//
	//			handle_and_status_t load_async_inner(const std::string& name, factory_t factory)
	//				{
	//				auto eleme_it{name_to_handle.find(name)};
	//				if (eleme_it != name_to_handle.end())
	//					{
	//					if (eleme_it->second.unloaded && !eleme_it->second.loading)
	//						{
	//						eleme_it->second.loading = true;
	//						loading_queue.emplace(name, factory);
	//						}
	//					return eleme_it->second;
	//					}
	//				else
	//					{
	//					eleme_it->second.loading = true;
	//					loading_queue.emplace(name, factory);
	//
	//					auto handle{handled_container.get_default()};
	//					name_to_handle.emplace(name, handle);
	//					return handle;
	//					}
	//				}
	//
	//			handled_container_t handled_container;
	//			
	//			std::unordered_map<std::string, handle_and_status_t> name_to_handle;
	//
	//			struct loading_queue_value_type
	//				{
	//				std::string name; 
	//				factory_t factory;
	//				};
	//			struct loaded_buffer_value_type
	//				{
	//				std::string name;
	//				T element;
	//				};
	//
	//			// No need for map; all names must be unique because elements are added to the loading_queue only if their name doesn't exist or they are unloaded and not loading.
	//			// Producer is loading_queue's thread, consumer is main thread on calling flush_loaded
	//			utils::containers::multithreading::producer_consumer_queue<loaded_buffer_value_type> loaded_buffer;
	//
	//			utils::containers::multithreading::self_consuming_queue<loading_queue_value_type> loading_queue
	//				{
	//				[this](loading_queue_value_type& element) -> void
	//					{
	//					using namespace std::string_literals;
	//					try
	//						{
	//						auto loaded_resource{element.factory()};
	//						
	//						loaded_buffer.emplace(element.name, std::move(loaded_resource));
	//						}
	//					catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
	//					}
	//				};
	//				
	//			//struct unloading_queue_value_type
	//			//	{
	//			//	std::string name; //only for debugging, second thread should never interact with the maps
	//			//	typename handled_container_t::handle_t handle;
	//			//	};
	//			//
	//			//unload_callback_t unload_callback;
	//			//
	//			//utils::containers::multithreading::self_consuming_queue<unloading_queue_value_type> unloading_queue
	//			//	{
	//			//	[this](unloading_queue_value_type& element) -> void
	//			//		{
	//			//		using namespace std::string_literals;
	//			//		try
	//			//			{
	//			//			std::unique_lock lock{ handled_container_mutex };
	//			//			unload_callback(*element.handle);
	//			//			handled_container.reset_handle(element.handle);
	//			//			}
	//			//		catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
	//			//		}
	//			//	};
	//		};

	namespace concepts
		{
		template<typename manager_t>
		concept manager_async = requires(manager_t manager)
			{
					{ manager.load_async(std::string{}, typename manager_t::factory_t{}) } -> std::convertible_to<typename manager_t::handle_t>;
					{ manager.load_async(std::string{})                                  } -> std::convertible_to<typename manager_t::handle_t>;
					{ manager.unload_async(std::string{})                                } -> std::same_as<void>;

			}&& iige::resource::concepts::manager_sync<manager_t>;

		}
	}