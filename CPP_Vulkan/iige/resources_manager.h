#pragma once

#include <tuple>
#include <utility>
#include <concepts>
#include <filesystem>
#include <unordered_map>
#include <utils/containers/multitype_container.h>
#include "../utils/remappable_handled_container.h"
#include "../utils/self_consuming_queue.h"

namespace iige::resources
	{

	template <typename T>
	class inner_manager;

	template <typename T>
	class handle
		{
		friend class inner_manager<T>;
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
			handle(inner_manager<T>& resman, handled_container_t::handle_t inner_handle) : resman{ resman }, inner_handle{ inner_handle } {}

			inner_manager<T>& resman;
			handled_container_t::handle_t inner_handle;
		};

	template <typename T>
	class inner_manager // manages only one type of resource at a time
		{
		public:
			using handled_container_t = utils::containers::remappable_handled_container<T>;

			using value_type      = handled_container_t::value_type;
			using size_type       = handled_container_t::size_type;
			using reference       = handled_container_t::reference;
			using const_reference = handled_container_t::const_reference;
			using pointer         = handled_container_t::pointer;
			using const_pointer   = handled_container_t::const_pointer;

			friend class handle_t;

			handle<T> load_sync(const std::filesystem::path& path)
				{
				auto pathstring{ path.string() };
				auto it{ map.find(pathstring) };
				if (it != map.end()) { return { *this, it->second }; }

				std::unique_lock lock{ handled_container_mutex };
				auto handle{ handled_container.emplace(pathstring) };
				map[pathstring] = handle;

				return { *this, handle };
				}

			handle<T> load_async(const std::filesystem::path& path)
				{
				auto pathstring{ path.string() };
				auto it{ map.find(pathstring) };
				if (it != map.end()) { return { *this, it->second }; }

				std::unique_lock lock{ handled_container_mutex };
				auto handle{ handled_container.undergo_mythosis(0) };
				map[pathstring] = handle;

				loading_queue.emplace(handle, pathstring);

				return { *this, handle };
				}

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
		
			void consume(std::vector<std::pair<typename handled_container_t::handle_t, const std::filesystem::path>>& to_load)
				{
				for (const auto& element : to_load)
					{
					try
						{
						T asset{ element.second };

						std::unique_lock lock{ handled_container_mutex };

						auto loaded_resource_handle{ handled_container.emplace(std::move(asset)) };
						handled_container.remap(element.first, loaded_resource_handle);
						}
					catch (const std::exception&) {  }//TODO log
					}
				}

			std::mutex handled_container_mutex;
			handled_container_t handled_container;
			std::unordered_map<std::string, typename handled_container_t::handle_t> map;

			using queue_value_type = std::pair<typename handled_container_t::handle_t, const std::filesystem::path>;
			utils::multithread::self_consuming_queue<queue_value_type> loading_queue{ std::bind(&resource_manager<T>::consume, this) };
		};


	template <typename T> handle<T>::      reference handle<T>::operator* ()       noexcept { return                resman.get(handle) ; }
	template <typename T> handle<T>::const_reference handle<T>::operator* () const noexcept { return                resman.get(handle) ; }

	template <typename T> handle<T>::      pointer   handle<T>::operator->()       noexcept { return std::addressof(resman.get(handle)); }
	template <typename T> handle<T>::const_pointer   handle<T>::operator->() const noexcept { return std::addressof(resman.get(handle)); }

	template <typename T> handle<T>::      reference handle<T>::value     ()       noexcept { return                resman.get(handle) ; }
	template <typename T> handle<T>::const_reference handle<T>::value     () const noexcept { return                resman.get(handle) ; }

	template <typename T> handle<T>::      pointer   handle<T>::get       ()       noexcept { return std::addressof(resman.get(handle)); }
	template <typename T> handle<T>::const_pointer   handle<T>::get       () const noexcept { return std::addressof(resman.get(handle)); }

	template<std::constructible_from<std::filesystem::path> ...Ts>
	class manager // manages resource_managers
		{
		public:
			template <typename T>
			using container_t = inner_manager<T>;
		
			template <typename T>
			handle<T> load_sync(const std::filesystem::path& path)
				{
				auto& container{ resource_managers_container.get_containing_type<T>() };

				return container.load_sync(path);
				}

			template <typename T>
			handle<T> load_async(const std::filesystem::path& path)
				{
				auto& container{ resource_managers_container.get_containing_type<T>() };

				return container.load_async(path);

				}


		private:
			utils::containers::multitype_container<inner_manager, Ts...> resource_managers_container;//if this stays, mythosis stays as well
		};
	}