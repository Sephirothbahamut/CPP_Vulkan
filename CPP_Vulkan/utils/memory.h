#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>


#include <utils/memory.h>
#include <utils/wrapper.h>

namespace utils
	{
	template<typename T, typename subscriber_t>
	class destruction_notifier : public utils::wrapper<T>
		{
		public:
			void subscribe(subscriber_t& subscriber)
				{
				subscribers.emplace_back(&subscriber);
				}
			void unsubscribe(subscriber_t& subscriber)
				{
				subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), &subscriber), subscribers.end());
				}

			~destruction_notifier()
				{
				for (auto& subscriber : subscribers)
					{
						//TODO ensure that subscriber has the on_destruction method which takes (*this) as REFERENCE! (Important: do not perform copies)
						//TODO esurehat subscribe has then_detruction mehod which take(*this)s REFERENCE! (Important:do not perm copies)rfo  as ts or tn (live sharing version)
						//extra: template<typename T, typeamen subscriber_t>
					subscriber->on_destruction(*this);
					}
				}
		private:
			std::vector<utils::observer_ptr<subscriber_t>> subscribers;
		};


	template<typename T>
	class pippo_ptr //TODO rename lol
		{
		public:
			using value_type      = T;
			using reference       =       value_type&;
			using const_reference = const value_type&;
			using pointer         =       value_type*;
			using const_pointer   = const value_type* const;

			using notifier_t      =                     destruction_notifier<T, pippo_ptr<T>> ;
			using observer_t      = utils::observer_ptr<destruction_notifier<T, pippo_ptr<T>>>;

			observer_t ptr{ nullptr };

			pippo_ptr() = default;
			pippo_ptr(observer_t ptr) : ptr{ ptr }
				{
				ptr->subscribe(*this);
				};

			pippo_ptr(const pippo_ptr<T>& copy) : ptr{ copy.ptr }
				{
				ptr->subscribe(*this);
				}

			pippo_ptr(pippo_ptr<T>&& move) : ptr{ std::move(move.ptr) }
				{
				move.ptr = nullptr;

				ptr->unsubscribe(move);

				ptr->subscribe(*this);
				}

			pippo_ptr<T>& operator= (const pippo_ptr<T>& copy)
				{
				ptr = copy.ptr;
				ptr->subscribe(*this);
				return *this;
				}

			pippo_ptr<T>& operator= (pippo_ptr<T>&& move)
				{
				ptr = move.ptr;
				move.ptr = nullptr;

				ptr->unsubscribe(move);

				ptr->subscribe(*this);

				return *this;
				}

			~pippo_ptr()
				{
				if (ptr)
					{
					delete ptr;
					}
				}

			void on_destruction(notifier_t&)
				{
				ptr = nullptr;
				}

			//Compatible interface with pointers and optional
			      reference operator* ()       noexcept { return ptr->value(); }
			const_reference operator* () const noexcept { return ptr->value(); }

			      pointer   operator->()       noexcept { return ptr->get(); }
			const_pointer   operator->() const noexcept { return ptr->get(); }

			      reference value     ()       noexcept { return ptr->value(); }
			const_reference value     () const noexcept { return ptr->value(); }

			      pointer   get       ()       noexcept { return ptr->get(); }
			const_pointer   get       () const noexcept { return ptr->get(); }
		};

	template <typename T, typename ...Args>
	pippo_ptr<T> make_pippo(Args&& ...args)
		{
		auto unique_ptr{ std::make_unique<destruction_notifier<T, pippo_ptr<T>>>(std::forward<Args>(args)...) };
		return { unique_ptr.release() };
		}


	template<typename T>
	class pippo_vec
		{
		public:
			using value_type      = T;
			using reference       =       value_type&;
			using const_reference = const value_type&;
			using pointer         =       value_type*;
			using const_pointer   = const value_type* const;

			using notifier_t      =                     destruction_notifier<T, pippo_vec<T>> ;
			using observer_t      = utils::observer_ptr<destruction_notifier<T, pippo_vec<T>>>;

			std::vector<observer_t> stuff;

			template <typename T, typename ...Args>
			T& emplace(Args&& ...args)
				{
				auto unique_ptr{ std::make_unique<notifier_t>(std::forward<Args>(args)...) };
				auto ret_ptr{ stuff.emplace_back(unique_ptr.release()) };

				ret_ptr->subscribe();

				return ret_ptr->value();
				}

			~pippo_vec()
				{
				for (auto& stoff : stuff)
					{
					delete stoff;
					}
				}

			void on_destruction(notifier_t& notifier)
				{
				stuff.erase(std::remove(stuff.begin(), stuff.end(), &notifier), stuff.end());
				}

		};
		//TODO to test and rename properly
}

struct dummy
	{
	dummy()
		{
		std::cout << "dummy constructor" << std::endl;
		}

	~dummy()
		{
		std::cout << "dummy destructor" << std::endl;
		}
	};

struct person
	{
	//void copulate(person& partner) { ptr = utils::make_pippo<dummy>(); partner.ptr = ptr; }
	utils::pippo_ptr<dummy> ptr;
	};



inline constexpr struct _copulate
	{
	class _inner;

	inline friend _inner operator<(person& lhs, _copulate proxy) noexcept { return { lhs }; }

	class _inner
		{
		public:
			dummy& operator>(person& rhs) const noexcept 
				{
				lhs.ptr = utils::make_pippo<dummy>();
				rhs.ptr = lhs.ptr;

				return *lhs.ptr;
				}
			_inner(person& lhs) noexcept : lhs{ lhs } {}
		private:
			person& lhs;
		};

	} copulate;



