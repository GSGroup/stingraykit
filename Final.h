#ifndef STINGRAY_STINGRAYKIT_FINAL_H
#define STINGRAY_STINGRAYKIT_FINAL_H


namespace stingray
{

	// Usage:
	// class A : STINGRAYKIT_FINAL(A) { };

#define STINGRAYKIT_FINAL(ClassName_) private virtual stingray::Final<ClassName_>
#define STINGRAYKIT_NONPOLYMORPHIC(ClassName_) STINGRAYKIT_FINAL(ClassName_), public virtual stingray::NonPolymorphicMarker

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
