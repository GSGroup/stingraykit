#ifndef STINGRAYKIT_CREATORS_H
#define STINGRAYKIT_CREATORS_H

// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/ICreator.h>

namespace stingray
{

	template < typename InterfaceType, typename ClassType, typename... Ts >
	class ConstructorCreator final : public virtual ICreator<InterfaceType>
	{
		using ParamTypes = TypeList<typename Decay<Ts>::ValueT...>;

	private:
		Tuple<ParamTypes>	_params;

	public:
		template < typename... Us >
		explicit ConstructorCreator(Us&&... args)
			:	_params(std::forward<Us>(args)...)
		{ static_assert(sizeof...(Ts) == sizeof...(Us), "Invalid number of parameters"); }

		shared_ptr<InterfaceType> Create() const override
		{ return FunctorInvoker::Invoke(MakeShared<ClassType>(), _params); }
	};


	template < typename InterfaceType, typename ClassType, typename... Ts >
	shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts... >> MakeConstructorCreator(Ts&&... args)
	{ return make_shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts...>>(std::forward<Ts>(args)...); }


	namespace Detail
	{
		template <typename ClassType >
		class SingleInstanceCreatorBase
		{
		protected:
			shared_ptr<ClassType> GetInstance() const
			{
				static shared_ptr<ClassType> instance(make_shared_ptr<ClassType>());
				return instance;
			}
		};
	}


	template < typename InterfaceType, typename ClassType >
	class SingleInstanceCreator final : public virtual ICreator<InterfaceType>, private Detail::SingleInstanceCreatorBase<ClassType>
	{
		using base = Detail::SingleInstanceCreatorBase<ClassType>;

	public:
		shared_ptr<InterfaceType> Create() const override
		{ return base::GetInstance(); }
	};


	template < typename To, typename From >
	class ConvertingCreator final : public virtual ICreator<To>
	{
	private:
		shared_ptr<ICreator<From>>	_creator;

	public:
		explicit ConvertingCreator(const shared_ptr<ICreator<From>>& creator)
			:	_creator(STINGRAYKIT_REQUIRE_NOT_NULL(creator))
		{ }

		shared_ptr<To> Create() const override
		{ return _creator->Create(); }
	};


	template < typename To, typename From >
	shared_ptr<ICreator<To>> MakeConvertingCreator(const shared_ptr<ICreator<From>>& creator)
	{ return make_shared_ptr<ConvertingCreator<To, From>>(creator); }

}

#endif
