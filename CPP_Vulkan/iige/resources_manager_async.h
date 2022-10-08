#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <functional>
#include <filesystem>
#include <unordered_map>

#include <utils/containers/multihandled_default.h>
#include <utils/construct.h>
#include <utils/containers/self_consuming_queue.h>
#include <utils/logger.h>

namespace iige::resource
	{

	// TODO add size to template
	template <typename T>
	class manager_async // manages only one type of resource at a time
		{
		public:
			using factory_t = std::function<T()>; 
			using unload_callback_t = std::function<void(T&)>;
			using handled_container_t = utils::containers::multihandled_default<T>;
			struct handle_t : public handled_container_t::handle_t
				{
				friend class manager_async<T>;
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
					handle_t(manager_async<T>& container, inner_handle_t inner_handle) : inner_handle_t{ inner_handle }, outer_container_ptr{ &container }
						{};

					utils::observer_ptr<manager_async<T>> outer_container_ptr;
				};
			
			using value_type      = handled_container_t::value_type;
			using size_type       = handled_container_t::size_type;
			using reference       = handled_container_t::reference;
			using const_reference = handled_container_t::const_reference;
			using pointer         = handled_container_t::pointer;
			using const_pointer   = handled_container_t::const_pointer;

			manager_async(factory_t factory, unload_callback_t unload_callback = [](T&){}) : handled_container{ factory() }, unload_callback{unload_callback} {}

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
							std::unique_lock lock{ handled_container_mutex };
							auto handle{ handled_container.emplace(factories_it->second()) };
							previous_handle.remap(handle);
							}
						catch (const std::exception& e)
							{
							utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());
							//After unload previous_handle is already set to default
							}
						}
					return { *this, eleme_it->second.handle };
					}


				auto factories_it{ factories.find(name) };
				if (factories_it == factories.end())
					{
					utils::globals::logger.err("Could not find factory for resource \"" + name + "\".");

					return { *this, handled_container.get_default() };
					}

				try
					{
					std::unique_lock lock{ handled_container_mutex };
					auto handle{ handled_container.emplace(factories_it->second()) };
					name_to_handle.emplace(name, handle);
					return { *this, handle };
					}
				catch (const std::exception& e)
					{
					utils::globals::logger.err("Failed to load resource \"" + name + "\"!\n" + e.what());

					return { *this, handled_container.get_default() };
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
					utils::globals::logger.err("Failed to unload resource \"" + name + "\";the resource did not exist.\n");
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

			utils::containers::self_consuming_queue<loading_queue_value_type> loading_queue
				{
				[this](std::vector<loading_queue_value_type>& to_load) -> void
					{
					using namespace std::string_literals;
					for (auto& element : to_load)
						{
						try
							{
							std::unique_lock lock{ handled_container_mutex };

							auto loaded_resource_handle{ handled_container.emplace(element.factory()) };
							handled_container.remap(element.handle, loaded_resource_handle);
							}
						catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
						}
					}
				};
				
			struct unloading_queue_value_type
				{
				std::string name; //only for debugging, second thread should never interact with the maps
				typename handled_container_t::handle_t handle;
				};

			unload_callback_t unload_callback;

			utils::containers::self_consuming_queue<unloading_queue_value_type> unloading_queue
				{
				[this](std::vector<unloading_queue_value_type>& to_load) -> void
					{
					using namespace std::string_literals;
					for (auto& element : to_load)
						{
						try
							{
							std::unique_lock lock{ handled_container_mutex };
							unload_callback(*element.handle);
							handled_container.reset_handle(element.handle);
							}
						catch (const std::exception& e) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!\n" + e.what()); }
						}
					}
				};
		};
		
	namespace concepts
		{
		template<typename T>
		concept manager_async = std::same_as<T, iige::resource::manager_async<typename T::value_type>>;
		}
	}