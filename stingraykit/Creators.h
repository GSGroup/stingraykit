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
		using Params = Tuple<ParamTypes>;

	private:
		Params	_params;

	public:
		explicit ConstructorCreator(const Params& params)
			:	_params(params)
		{ }

		explicit ConstructorCreator(Params&& params)
			:	_params(std::move(params))
		{ }

		shared_ptr<InterfaceType> Create() const override
		{ return FunctorInvoker::Invoke(MakeShared<ClassType>(), _params); }
	};


	namespace Detail
	{

		template < typename ClassType, typename... Ts >
		class ConstructorCreatorProxy
		{
			using ParamTypes = TypeList<typename Decay<Ts>::ValueT...>;
			using Params = Tuple<ParamTypes>;

		private:
			Params	_params;

		public:
			template < typename... Us >
			explicit ConstructorCreatorProxy(Us&&... params)
				:	_params(std::forward<Us>(params)...)
			{ }

			template < typename InterfaceType >
			operator shared_ptr<ICreator<InterfaceType>> () const &
			{ return make_shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts...>>(_params); }

			template < typename InterfaceType >
			operator shared_ptr<ICreator<InterfaceType>> () &&
			{ return make_shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts...>>(std::move(_params)); }
		};

	}


	template < typename InterfaceType, typename ClassType, typename... Ts >
	auto MakeConstructorCreator(Ts&&... params)
	{ return Detail::ConstructorCreatorProxy<ClassType, Ts...>(std::forward<Ts>(params)...); }


	template < typename ClassType, typename... Ts >
	auto MakeConstructorCreator(Ts&&... params)
	{ return Detail::ConstructorCreatorProxy<ClassType, Ts...>(std::forward<Ts>(params)...); }


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


	namespace Detail
	{

		template < typename ClassType >
		class SingleInstanceCreatorProxy
		{
		public:
			template < typename InterfaceType >
			operator shared_ptr<ICreator<InterfaceType>> () const
			{ return make_shared_ptr<SingleInstanceCreator<InterfaceType, ClassType>>(); }
		};

	}


	template < typename ClassType, typename... Ts >
	auto MakeSingleInstanceCreator(Ts&&... params)
	{ return Detail::SingleInstanceCreatorProxy<ClassType>(std::forward<Ts>(params)...); }


	template < typename To, typename From >
	class ConvertingCreator final : public virtual ICreator<To>
	{
	private:
		shared_ptr<ICreator<From>>	_creator;

	public:
		explicit ConvertingCreator(const shared_ptr<ICreator<From>>& creator)
			:	_creator(STINGRAYKIT_REQUIRE_NOT_NULL(creator))
		{ }

		explicit ConvertingCreator(shared_ptr<ICreator<From>>&& creator)
			:	_creator(STINGRAYKIT_REQUIRE_NOT_NULL(std::move(creator)))
		{ }

		shared_ptr<To> Create() const override
		{ return _creator->Create(); }
	};


	namespace Detail
	{

		template < typename From >
		class ConvertingCreatorProxy
		{
			using ICreatorPtr = shared_ptr<ICreator<From>>;

		private:
			ICreatorPtr		_creator;

		public:
			explicit ConvertingCreatorProxy(const ICreatorPtr& creator)
				:	_creator(creator)
			{ }

			explicit ConvertingCreatorProxy(ICreatorPtr&& creator)
				:	_creator(std::move(creator))
			{ }

			template < typename To >
			operator shared_ptr<ICreator<To>> () const &
			{ return make_shared_ptr<ConvertingCreator<To, From>>(_creator); }

			template < typename To >
			operator shared_ptr<ICreator<To>> () &&
			{ return make_shared_ptr<ConvertingCreator<To, From>>(std::move(_creator)); }
		};

	}


	template < typename From >
	auto MakeConvertingCreator(const shared_ptr<ICreator<From>>& creator)
	{ return Detail::ConvertingCreatorProxy<From>(creator); }


	template < typename From >
	auto MakeConvertingCreator(shared_ptr<ICreator<From>>&& creator)
	{ return Detail::ConvertingCreatorProxy<From>(std::move(creator)); }

}

#endif
