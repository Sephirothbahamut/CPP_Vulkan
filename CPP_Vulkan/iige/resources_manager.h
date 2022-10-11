#pragma once

#include <tuple >
#include <utility>
#include <typeinfo>
#include <concepts >
#include <filesystem>
#include <unordered_map>

#include <utils/containers/multihandled_default.h>
#include <utils/construct.h>
#include <utils/memory.h>

#include "resources_manager_async.h"
#include "resources_manager_sync.h"

namespace iige
	{
	namespace concepts
		{
		template<typename T>
		concept resource_manager = iige::resource::concepts::manager_sync<T> || iige::resource::concepts::manager_async<T>;
		//TODO in depth concept
		}

	template<concepts::resource_manager ...managers_Ts>
	class resources_manager // manages resource_managers
		{
		using tuple_t = std::tuple<utils::observer_ptr<managers_Ts>...>;
		struct containing_type_info_t { std::size_t index; bool direct; };

		//template <typename T>
		//struct qwe_get_type_containing_type
		//	{
		//	using type = typename get_type_containing_type<T, managers_Ts...>::type;
		//	};

		template <typename T, typename current, typename ...remaining>
		struct get_type_containing_type
			{
			using type = std::conditional_t<std::same_as<typename get_type_containing_type<T, current>::type, current>, current, get_type_containing_type<T, remaining...>>;
			};

		template <typename T, typename current>
		struct get_type_containing_type<T, current>
			{
			using type = std::conditional_t<std::same_as<T, typename std::remove_pointer_t<current>::value_type>, current, void>;
				////if
				//	std::conditional_t<std::same_as<T, typename std::remove_pointer_t<current>::value_type>, 
				//	//{
				//	current, 
				//	//}
				////else if
				//	std::conditional_t< std::same_as<T, typename std::remove_pointer_t<current>::qwe_get_type_containing_type<T>::type>,
				//	//{
				//	typename std::remove_pointer_t<current>::qwe_get_type_containing_type<T>::type,
				//	//}
				////else
				//void>
				//>;
			};

		template<typename T>
		//using container_t = std::remove_cvref_t<decltype(get_containing_type<T>())>;
		using container_t = std::remove_cvref_t<typename get_type_containing_type<T, managers_Ts...>::type>;

		public:
			resources_manager(managers_Ts& ...managers) :
				managers{ (&managers)... } {}

			template<typename T>
			using handle_t = typename container_t<T>::handle_t;

			template<typename T>
			using factory_t = typename container_t<T>::factory_t;

			template <typename type>
			constexpr auto& get_containing_type()
				{
				using namespace std::string_literals;

				constexpr containing_type_info_t containing_type_info{get_index_containing_type<type>()};

				static_assert(containing_type_info.index < std::tuple_size_v<tuple_t>, "This resource manager does not contain the resource type you're trying to use");
				
				if constexpr (containing_type_info.direct)
					{
					return *std::get<containing_type_info.index>(managers);
					}
				else if constexpr(!containing_type_info.direct)
					{
					return std::get<containing_type_info.index>(managers)->get_containing_type<type>();
					}
				}

			template <typename T>
			handle_t<T> load_sync(const std::string& name, factory_t<T> factory)
				{
				auto& container{ get_containing_type<T>() };

				return container.load_sync(name, factory);
				}
				
			template <typename T>
			handle_t<T> load_sync(const std::string& name)
				{
				auto& container{ get_containing_type<T>() };
				
				return get_containing_type<T>().load_sync(name);
				}

			template <typename T>
			void unload_sync(std::string name)
				{
				auto& container{ get_containing_type<T>() };

				container.unload_sync(name);
				}

			template <typename T>
			handle_t<T> load_async(std::string name, factory_t<T> factory)
			requires resource::concepts::manager_async<container_t<T>>
				{
				auto& container{ get_containing_type<T>() };

				return container.load_async(name, factory);
				}

			template <typename T>
			handle_t<T> load_async(std::string name)
			requires resource::concepts::manager_async<container_t<T>>
				{
				auto& container{ get_containing_type<T>() };

				return container.load_async(name);
				}
				
			template <typename T>
			void unload_async(std::string name)
			requires resource::concepts::manager_async<container_t<T>>
				{
				auto& container{ get_containing_type<T>() };

				container.unload_async(name);
				}

		private:
			tuple_t managers;

			template<typename T, std::size_t index = 0>
			static constexpr containing_type_info_t get_index_containing_type()
				{
				if constexpr (std::same_as<T, typename std::remove_pointer_t<typename std::tuple_element_t<index, tuple_t>>::value_type>) { return {index, true}; }
				//else if (/*TODO*/) { return {index, false}; }
				else { return get_index_containing_type<T, index + 1>(); }
				}
		};
	}