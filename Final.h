#ifndef __GS_DVRLIB_TOOLKIT_FINAL_H__
#define __GS_DVRLIB_TOOLKIT_FINAL_H__


namespace dvrlib
{

	// Usage: 
	// class A : TOOLKIT_FINAL(A) { };

#define TOOLKIT_FINAL(ClassName_) private virtual dvrlib::Final<ClassName_>

	template < typename >
	class Final
	{
	protected:
		Final() { }
		Final(const Final&) { }
		~Final() { }

	private:
		Final& operator = (const Final&);
	};

}


#endif
