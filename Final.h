#ifndef STINGRAY_TOOLKIT_FINAL_H
#define STINGRAY_TOOLKIT_FINAL_H


namespace stingray
{

	// Usage:
	// class A : TOOLKIT_FINAL(A) { };

#define TOOLKIT_FINAL(ClassName_) private virtual stingray::Final<ClassName_>
#define TOOLKIT_NONPOLYMORPHIC(ClassName_) TOOLKIT_FINAL(ClassName_), public virtual stingray::NonPolymorphicMarker

	class NonPolymorphicMarker { };

	template < typename >
	class Final
	{
	protected:
		Final() { }
		Final(const Final&) { }
		~Final() { }

	protected:
		Final& operator = (const Final&) { return *this; }
	};

}


#endif
