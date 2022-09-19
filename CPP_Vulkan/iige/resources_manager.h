#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <filesystem>
#include <unordered_map>

#include <utils/containers/multitype_container.h>
#include <utils/construct.h>

#include "../utils/remappable_handled_container.h"
#include "../utils/self_consuming_queue.h"

namespace iige::resource
	{
	template <typename T, typename ...Args>
		requires std::constructible_from<T, Args...>
	class manager;

	template <typename T, typename ...Args>
	class handle
		{
		friend class manager<T, Args...>;
		using handled_container_t = utils::containers::remappable_handled_container<T>;

		public:
			using value_type      = T;
			using reference       =       value_type&;
			using const_reference = const value_type&;
			using pointer         =       value_type*;
			using const_pointer   = const value_type* const;
				
			      reference operator* ()       noexcept;
			const_reference operator* () const noexcept;

			      pointer   operator->()       noexcept;
			const_pointer   operator->() const noexcept;

			      reference value     ()       noexcept;
			const_reference value     () const noexcept;

			      pointer   get       ()       noexcept;
			const_pointer   get       () const noexcept;

		private:
			handle(manager<T, Args...>& resman, handled_container_t::handle_t inner_handle) : resman{ resman }, inner_handle{ inner_handle } {}

			manager<T, Args...>& resman;
			handled_container_t::handle_t inner_handle;
		};



	template <typename T, typename ...Args>
		requires std::constructible_from<T, Args...>
	class manager // manages only one type of resource at a time
		{
		enum class construction_type_t { single_parameter, tuple };
		inline static constexpr construction_type_t construction_type = sizeof...(Args) > 1 ? construction_type_t::tuple : construction_type_t::single_parameter;

		using create_info_t = std::conditional_t<construction_type == construction_type_t::tuple, std::tuple<Args...>, std::tuple_element_t<0, std::tuple<Args...>> >;


		public:
			using handled_container_t = utils::containers::remappable_handled_container<T>;

			using value_type      = handled_container_t::value_type;
			using size_type       = handled_container_t::size_type;
			using reference       = handled_container_t::reference;
			using const_reference = handled_container_t::const_reference;
			using pointer         = handled_container_t::pointer;
			using const_pointer   = handled_container_t::const_pointer;

			using handle_t = handle<T, Args...>;


			handle_t load_sync(std::string name, Args&& ...args)
				{
				auto cinfo_it{ map_create_infos.find(name) };
				if (cinfo_it != map_create_infos.end())
					{
					throw std::runtime_error{ "Resource \"" + name + "\" already had create infos." };
					}

				map_create_infos[name] = { std::forward<Args>(args)... };

				return load_sync(name);
				}

			handle_t load_async(std::string name, Args&& ...args)
				{
				auto cinfo_it{ map_create_infos.find(name) };
				if (cinfo_it != map_create_infos.end())
					{
					throw std::runtime_error{ "Resource \"" + name + "\" already had create infos." };
					}

				map_create_infos[name] = { std::forward<Args>(args)... };

				return load_async(name);
				}

			handle_t load_sync(std::string name)
				{
				auto cinfo_it{ map_create_infos.find(name) };
				if (cinfo_it == map_create_infos.end())
					{
					throw std::runtime_error{"Could not find create info for resource \"" + name + "\"."};
					}

				auto eleme_it{ map.find(name) };
				if (eleme_it != map.end()) { return { *this, eleme_it->second }; }

				std::unique_lock lock{ handled_container_mutex };
				auto handle{ handled_container.emplace(cinfo_it->second) };
				map[name] = handle;

				return { *this, handle };
				}

			handle_t load_async(std::string name)
				{
				auto eleme_it{ map.find(name) };
				if (eleme_it != map.end()) { return { *this, eleme_it->second }; }

				auto cinfo_it{ map_create_infos.find(name) };
				if (cinfo_it == map_create_infos.end())
					{
					throw std::runtime_error{ "Could not find create info for resource \"" + name + "\"." };
					}

				std::unique_lock lock{ handled_container_mutex };
				auto handle{ handled_container.undergo_mythosis(0) };
				map[name] = handle;

				loading_queue.emplace(name, handle, cinfo_it->second);

				return { *this, handle };
				}

			std::unordered_map<std::string, create_info_t> map_create_infos;

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

			std::mutex handled_container_mutex;
			handled_container_t handled_container;
			std::unordered_map<std::string, typename handled_container_t::handle_t> map;

			struct queue_value_type
				{
				std::string name;
				typename handled_container_t::handle_t handle;
				create_info_t create_info;
				};

			utils::multithread::self_consuming_queue<queue_value_type> loading_queue
				{
				[this](std::vector<queue_value_type>& to_load) -> void
					{
					using namespace std::string_literals;
					for (const auto& element : to_load)
						{
						try
							{
							auto store{ [&](T&& asset) -> void
								{
								std::unique_lock lock{ handled_container_mutex };

								auto loaded_resource_handle{ handled_container.emplace(std::move(asset)) };
								handled_container.remap(element.handle, loaded_resource_handle);
								} };

							if constexpr(construction_type == construction_type_t::single_parameter)
								{
								store({ element.create_info });
								}
							else if constexpr (construction_type == construction_type_t::tuple)
								{
								store(utils::construct::from_tuple<T>(element.create_info));
								}
							}
						catch (const std::exception&) { utils::globals::logger.err("Failed to load resource \"" + element.name + "\"!"); }
						}
					} 
				};
		};
	template <typename T, typename ...Args> handle<T, Args...>::      reference handle<T, Args...>::operator* ()       noexcept { return                resman.get(inner_handle) ; }
	template <typename T, typename ...Args> handle<T, Args...>::const_reference handle<T, Args...>::operator* () const noexcept { return                resman.get(inner_handle) ; }

	template <typename T, typename ...Args> handle<T, Args...>::      pointer   handle<T, Args...>::operator->()       noexcept { return std::addressof(resman.get(inner_handle)); }
	template <typename T, typename ...Args> handle<T, Args...>::const_pointer   handle<T, Args...>::operator->() const noexcept { return std::addressof(resman.get(inner_handle)); }

	template <typename T, typename ...Args> handle<T, Args...>::      reference handle<T, Args...>::value     ()       noexcept { return                resman.get(inner_handle) ; }
	template <typename T, typename ...Args> handle<T, Args...>::const_reference handle<T, Args...>::value     () const noexcept { return                resman.get(inner_handle) ; }

	template <typename T, typename ...Args> handle<T, Args...>::      pointer   handle<T, Args...>::get       ()       noexcept { return std::addressof(resman.get(inner_handle)); }
	template <typename T, typename ...Args> handle<T, Args...>::const_pointer   handle<T, Args...>::get       () const noexcept { return std::addressof(resman.get(inner_handle)); }

	namespace concepts
		{
		template<typename T>
		concept manager = std::same_as<T, iige::resource::manager<typename T::create_info_t>>;
		}
	}
namespace iige
	{
	template<resource::concepts::manager ...managers_Ts>
	class resources_manager // manages resource_managers
		{
		public:
			template <typename T, typename ...Args>
			resource::handle<T, Args...> load_sync(std::string name, Args&&... args)
				{
				auto& container{ get_containing_type<T>() };

				return container.load_sync(name, args);
				}

			template <typename T, typename ...Args>
			resource::handle<T, Args...> load_async(std::string name, Args&&... args)
				{
				auto& container{ get_containing_type<T>() };

				return container.load_async(name, args);

				}

			template <typename T>
			auto load_sync(std::string name) -> typename decltype(get_containing_type<T>())::handle_t
				{
				auto& container{ get_containing_type<T>() };

				return container.load_sync(name);
				}

			template <typename T>
			auto load_async(std::string name) -> typename decltype(get_containing_type<T>())::handle_t
				{
				auto& container{ get_containing_type<T>() };

				return container.load_async(name);
				}

		private:
			using tuple_t = std::tuple<managers_Ts...>;
			tuple_t managers;

			template <typename type>
			constexpr auto& get_containing_type()
				{
				return std::get<get_index_containing_type<type>()>(managers);
				}

			template<typename T, std::size_t index = 0>
			static constexpr std::size_t get_index_containing_type()
				{
				if constexpr (std::same_as<T, typename std::tuple_element<index, tuple_t>::type::value_type>) { return index; }
				else { return get_index_containing_type<T, index + 1>(); }
				}
		};
	}