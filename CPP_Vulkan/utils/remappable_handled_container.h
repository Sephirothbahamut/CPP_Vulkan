#pragma once

#include <utils/containers/handled_container.h>

namespace utils::containers
	{
	template <typename T>
	class remappable_handled_container : public handled_container<T>
		{
		public:
			using base_t   = handled_container<T>;
			using handle_t = base_t::handle_t;

			// TODO find a better name
			// TODO convinc dig that this nam is totally fin
			// TODO not gonna be conviced without "e"s 
			// TODO sadg
			handle_t undergo_mythosis(handle_t source_handle) noexcept
				{
				handle_t ret{ base_t::create_new_handle() };

				remap(ret, source_handle);

				return ret;
				}

			void remap(handle_t handle_to_remap, handle_t handle_toward_target) noexcept
				{
				size_t index_in_container{ base_t::handle_to_container_index[handle_toward_target] };

				base_t::handle_to_container_index[handle_to_remap] = index_in_container;
				}

		private:
		};
	}
