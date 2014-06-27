#ifndef STINGRAY_TOOLKIT_CLONEBUILDERBASE_H
#define STINGRAY_TOOLKIT_CLONEBUILDERBASE_H


#include <stingray/toolkit/shared_ptr.h>


namespace stingray
{

	template < typename T >
	class CloneBuilderBase
	{
		typedef shared_ptr<T>	TPtr;

	private:
		TPtr	_instPtr;

	public:
		T Make() const			{ return *_instPtr; }
		TPtr MakeShared() const	{ return make_shared<T>(*_instPtr); }

		operator T() const		{ return Make(); }
		operator TPtr() const	{ return MakeShared(); }

	protected:
		CloneBuilderBase(const T& inst) : _instPtr(new T(inst)) { }
		~CloneBuilderBase() { }

		T& GetInst() { return *_instPtr; }
	};

}

#endif
