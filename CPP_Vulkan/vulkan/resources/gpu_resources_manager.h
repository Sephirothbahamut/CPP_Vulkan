#pragma once

#include <utils/containers/multithreading/multiqueue_consumer.h>
#include <utils/containers/multithreading/consumption_delegating_queue.h>

#include <utils/memory.h>
#include "../core/manager.h"
#include "../core/memory_operations_command_buffer.h"
#include "../../iige/resources_manager.h"

namespace utils::graphics::vulkan
	{
	template<iige::concepts::resource_manager ...managers_Ts>
	class gpu_resources_manager;

	namespace resource
		{
		template <typename T>
		class gpu_manager_async // manages only one type of resource at a time
			{
			template<iige::concepts::resource_manager ...managers_Ts>
			friend class gpu_resources_manager;
			public:
				using factory_t = std::function<T(vk::CommandBuffer&)>;
				using unload_callback_t = std::function<void(T&)>;
				using handled_container_t = utils::containers::multihandled_default<T>;
				struct handle_t : public handled_container_t::handle_t
					{
					friend class gpu_manager_async<T>;
					public:
						using value_type      = T;
						using reference       = value_type&;
						using const_reference = const value_type&;
						using pointer         = value_type*;
						using const_pointer   = const value_type* const;
						using inner_handle_t  = handled_container_t::handle_t;

						handle_t           (const handle_t& copy) : inner_handle_t{ copy }, outer_container_ptr{ copy.outer_container_ptr } {}
						handle_t& operator=(const handle_t& copy)
							{
							this->inner_handle_t::operator=(copy);
							outer_container_ptr = copy.outer_container_ptr;
							return *this;
							}
						handle_t           (handle_t&& move) noexcept : inner_handle_t{ std::move(move) }, outer_container_ptr{ move.outer_container_ptr } {};
						handle_t& operator=(handle_t&& move) noexcept 
							{
							this->inner_handle_t::operator=(std::move(move)); 
							outer_container_ptr = move.outer_container_ptr;
							return *this; 
							}

						constexpr       reference operator* (             )       noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::operator* (); }
						constexpr const_reference operator* (             ) const noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::operator* (); }
						constexpr       pointer   operator->(             )       noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::operator->(); }
						constexpr const_pointer   operator->(             ) const noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::operator->(); }
						constexpr       reference value     (             )       noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::value     (); }
						constexpr const_reference value     (             ) const noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::value     (); }
						constexpr       pointer   get       (             )       noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::get       (); }
						constexpr const_pointer   get       (             ) const noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; return inner_handle_t::get       (); }
						void                      remap(const handle_t& to)       noexcept { std::unique_lock lock{outer_container_ptr->handled_container_mutex}; inner_handle_t::remap(to); }

					private:
						handle_t(gpu_manager_async<T>& container, inner_handle_t inner_handle) : inner_handle_t{ inner_handle }, outer_container_ptr{ &container }
							{};

						utils::observer_ptr<gpu_manager_async<T>> outer_container_ptr;
					};
			
				using value_type      = handled_container_t::value_type;
				using size_type       = handled_container_t::size_type;
				using reference       = handled_container_t::reference;
				using const_reference = handled_container_t::const_reference;
				using pointer         = handled_container_t::pointer;
				using const_pointer   = handled_container_t::const_pointer;

				gpu_manager_async(core::manager& vulkan_manager_ptr, T&& default_resource, unload_callback_t unload_callback = [](T&) {}) : 
					vulkan_manager_ptr{ &vulkan_manager_ptr }, handled_container { std::move(default_resource) }, unload_callback{ unload_callback } {}

				handle_t load_sync(const std::string& name, factory_t factory)
					{
					auto factory_it{ factories.find(name) };
					if (factory_it != factories.end())
						{
						utils::globals::logger.err("Resource \"" + name + "\" already had a factory. The parameter factory will be ignored.");
						}
					else
						{
						factories[name] = factory;
						}

					return load_sync(name);
					}

				handle_t load_sync(const std::string& name)
					{
					auto eleme_it{ name_to_handle.find(name) };
					if (eleme_it != name_to_handle.end())
						{
						if (eleme_it->second.unloaded)
							{
							eleme_it->second.unloaded = false;
							auto& previous_handle = eleme_it->second.handle;

							// Reload
							auto factories_it{ factories.find(name) };
							if (factories_it == factories.end())
								{
								utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
								//After unload previous_handle is already set to default
								}

							try
								{
								auto handle{ handled_container.emplace(factories_it->second()) };
								previous_handle.remap(handle);
								}
							catch (const std::exception& e)
								{
								utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());
								//After unload previous_handle is already set to default
								}
							}
						return eleme_it->second.handle;
						}

					// First load
					auto factories_it{ factories.find(name) };
					if (factories_it == factories.end())
						{
						utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");

						return handled_container.get_default();
						}

					try
						{
						core::memory_operations_command_buffer mocb{ *vulkan_manager_ptr };
						mocb.begin();
						auto handle{ handled_container.emplace(factories_it->second(mocb.get())) };
						name_to_handle.emplace(name, handle);
						mocb.submit();
						return handle;
						}
					catch (const std::exception& e)
						{
						utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());

						return handled_container.get_default();
						}
					}

				void unload_sync(const std::string& name)
					{
					auto eleme_it{ name_to_handle.find(name) };
					if (eleme_it != name_to_handle.end())
						{
						eleme_it->second.unloaded = true;
						handled_container.reset_handle(eleme_it->second.handle);
						}
					else
						{
						utils::globals::logger.err("Failed to unload resource \"" + name + "\"; the resource did not exist.\n");
						}
					}


				handle_t load_async(const std::string& name, factory_t factory)
					{
					auto factory_it{ factories.find(name) };
					if (factory_it != factories.end())
						{
						utils::globals::logger.err("Resource \"" + name + "\" already had a factory. The parameter factory will be ignored.");
						}
					else
						{
						factories[name] = factory;
						}

					return load_async(name);
					}

				handle_t load_async(const std::string& name)
					{
					auto eleme_it{ name_to_handle.find(name) };
					if (eleme_it != name_to_handle.end())
						{
						if (eleme_it->second.unloaded)
							{
							eleme_it->second.unloaded = false;
							auto& previous_handle = eleme_it->second.handle;
						
							// Reload
							auto factories_it{ factories.find(name) };
							if (factories_it == factories.end())
								{
								utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");
								//After unload previous_handle is already set to default
								}

							std::unique_lock lock{ handled_container_mutex };
							loading_queue.emplace(name, previous_handle, factories_it->second);
							}
						return { *this, eleme_it->second.handle };
						}

					// First load
					auto factories_it{ factories.find(name) };
					if (factories_it == factories.end())
						{
						utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");

						return { *this, handled_container.get_default() };
						}

					std::unique_lock lock{ handled_container_mutex };
					auto handle{ handled_container.splice(handled_container.get_default()) };
					name_to_handle.emplace(name, handle);
					loading_queue.emplace(name, handle, factories_it->second);

					return { *this, handle };
					}

				void unload_async(const std::string& name)
					{
					auto eleme_it{ name_to_handle.find(name) };
					if (eleme_it != name_to_handle.end())
						{
						eleme_it->second.unloaded = true; 
					
						unloading_queue.emplace(name, eleme_it->second.handle);
						}
					else
						{
						utils::globals::logger.err("Failed to unload resource \"" + name + "\"; the resource did not exist.\n");
						}
					}

				std::unordered_map<std::string, factory_t> factories;

			private:
				const T& get(handled_container_t::handle_t handle) const noexcept
					{
					std::unique_lock lock{ handled_container_mutex };
					return handled_container[handle];
					}
					  T& get(handled_container_t::handle_t handle)       noexcept
					{
					std::unique_lock lock{ handled_container_mutex };
					return handled_container[handle];
					}
				
				utils::observer_ptr<core::memory_operations_command_buffer> memory_operations_command_buffer_ptr;
				utils::observer_ptr<core::manager> vulkan_manager_ptr;
				handled_container_t handled_container;

				struct handle_and_status_t
					{
					handled_container_t::handle_t handle;
					bool unloaded;
					};

				std::unordered_map<std::string, handle_and_status_t> name_to_handle;

				std::mutex handled_container_mutex;

				struct loading_queue_value_type
					{
					std::string name; //only for debugging, second thread should never interact with the maps
					typename handled_container_t::handle_t handle;
					factory_t factory;
					};

				utils::containers::multithreading::consumption_delegating_queue<loading_queue_value_type> loading_queue
					{
					[this](loading_queue_value_type& element) -> void
						{
						using namespace std::string_literals;
						try
							{
							std::unique_lock lock{ handled_container_mutex };

							auto loaded_resource_handle{ handled_container.emplace(element.factory(memory_operations_command_buffer_ptr->get())) };
							handled_container.remap(element.handle, loaded_resource_handle);
							}
						catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
						}
					};
				
				struct unloading_queue_value_type
					{
					std::string name; //only for debugging, second thread should never interact with the maps
					typename handled_container_t::handle_t handle;
					};

				unload_callback_t unload_callback;

				utils::containers::multithreading::self_consuming_queue<unloading_queue_value_type> unloading_queue
					{
					[this](unloading_queue_value_type& element) -> void
						{
						using namespace std::string_literals;
						try
							{
							std::unique_lock lock{ handled_container_mutex };
							unload_callback(*element.handle);
							handled_container.reset_handle(element.handle);
							}
						catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
						}
					};
			};
		}

	template<iige::concepts::resource_manager ...managers_Ts>
	class gpu_resources_manager : public iige::resources_manager<managers_Ts...>
		{
		public:
			gpu_resources_manager(core::manager& vulkan_manager, managers_Ts& ...managers) :
				iige::resources_manager<managers_Ts...>{(managers)...},
				memory_operations_command_buffer {vulkan_manager}
				{
				((managers.memory_operations_command_buffer_ptr = &memory_operations_command_buffer), ...);
				((consumer.bind(managers.loading_queue)), ...);
				}

		private:
			utils::containers::multithreading::multiqueue_consumer consumer{[this]() { queue_get(); }, [this]() { queue_submit(); }};

			utils::observer_ptr<vulkan::core::manager> manager_ptr;

			core::memory_operations_command_buffer memory_operations_command_buffer;

			void queue_get()
				{
				memory_operations_command_buffer.begin();
				}
			void queue_submit()
				{
				memory_operations_command_buffer.submit();
				}
		};
	}