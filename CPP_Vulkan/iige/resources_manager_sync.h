#pragma once

#include <tuple>
#include <tuple >
#include <utility>
#include <concepts>
#include <concepts >
#include <functional>
#include <filesystem >
#include <filesystem  >
#include <unordered_map>

#include <utils/containers/multihandled_default.h>
#include <utils/construct.h>
#include <utils/logger.h>

namespace iige::resource
	{

	// TODO add size to template
	template <typename T>
	class manager_sync // manages only one type of resource at a time
		{

		public:
			using factory_t = std::function<T()>; 
			using handled_container_t = utils::containers::multihandled_default<T>;
			using handle_t = handled_container_t::handle_t;

			using value_type      = handled_container_t::value_type;
			using size_type       = handled_container_t::size_type;
			using reference       = handled_container_t::reference;
			using const_reference = handled_container_t::const_reference;
			using pointer         = handled_container_t::pointer;
			using const_pointer   = handled_container_t::const_pointer;

			manager_sync(factory_t factory) : handled_container{ factory() } {}

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
					auto handle{ handled_container.emplace(factories_it->second()) };
					name_to_handle.emplace(name, handle);
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
					handled_container.erase_and_remap(eleme_it->second.handle, handled_container.get_default());
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
				return handled_container[handle];
				}
				  T& get(handled_container_t::handle_t handle)       noexcept
				{
				return handled_container[handle];
				}

			handled_container_t handled_container;

			struct handle_and_status_t
				{
				handled_container_t::handle_t handle;
				bool unloaded;
				};

			std::unordered_map<std::string, handle_and_status_t> name_to_handle;
			};
	}