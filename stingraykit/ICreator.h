#ifndef STINGRAYKIT_ICREATOR_H
#define STINGRAYKIT_ICREATOR_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <stingraykit/function/FunctorInvoker.h>
#include <stingraykit/shared_ptr.h>


#define STINGRAYKIT_DECLARE_CREATOR(ClassName) \
		typedef stingray::ICreator<ClassName>				ClassName##Creator; \
		STINGRAYKIT_DECLARE_PTR(ClassName##Creator)

namespace stingray
{

	template < typename T >
	struct ICreator
	{
		virtual ~ICreator() { }

		virtual shared_ptr<T> Create() const = 0;
	};


	template < typename InterfaceType, typename ClassType, typename... Ts >
	class ConstructorCreator : public virtual ICreator<InterfaceType>
	{
		typedef TypeList<Ts...> ParamTypes;

	private:
		Tuple<ParamTypes>	_params;

	public:
		template < typename... Us >
		ConstructorCreator(Us&&... args) : _params(std::forward<Us>(args)...)
		{ CompileTimeAssert<sizeof...(Ts) == sizeof...(Us)> ERROR__invalid_number_of_parameters; }

		virtual shared_ptr<InterfaceType> Create() const
		{ return FunctorInvoker::Invoke(MakeShared<ClassType>(), _params); }
	};


	template < typename InterfaceType, typename ClassType, typename... Ts >
	shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts... > > MakeConstructorCreator(Ts&&... args)
	{ return make_shared_ptr<ConstructorCreator<InterfaceType, ClassType, Ts...> >(std::forward<Ts>(args)...); }


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
	class SingleInstanceCreator : public virtual ICreator<InterfaceType>, private Detail::SingleInstanceCreatorBase<ClassType>
	{
		typedef Detail::SingleInstanceCreatorBase<ClassType> base;

	public:
		virtual shared_ptr<InterfaceType> Create() const
		{ return base::GetInstance(); }
	};

}

#endif
