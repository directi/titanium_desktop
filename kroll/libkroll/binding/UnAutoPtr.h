#ifndef BINDING_UN_AUTO_PTR_H
#define BINDING_UN_AUTO_PTR_H

namespace kroll {


template <class C>
class UnAutoPtr
{
public:
	UnAutoPtr(): _ptr(0)
	{
	}

	UnAutoPtr(C* ptr): _ptr(ptr)
	{
	}
	
	C* operator -> ()
	{
		return _ptr;
	}

	const C* operator -> () const
	{
		return _ptr;
	}

	C& operator * ()
	{
		return *_ptr;
	}

	const C& operator * () const
	{
		return *_ptr;
	}

	C* get()
	{
		return _ptr;
	}

	const C* get() const
	{
		return _ptr;
	}

	operator C* ()
	{
		return _ptr;
	}
	
	operator const C* () const
	{
		return _ptr;
	}
	
	bool operator ! () const
	{
		return _ptr == 0;
	}

	bool isNull() const
	{
		return _ptr == 0;
	}
	
	template <class Other>
	Other* cast() const
	{
		return dynamic_cast<Other*>(_ptr);
	}

	bool operator == (const C* ptr) const
	{
		return _ptr == ptr;
	}

	bool operator == (C* ptr) const
	{
		return _ptr == ptr;
	}

	bool operator != (const C* ptr) const
	{
		return _ptr != ptr;
	}

	bool operator != (C* ptr) const
	{
		return _ptr != ptr;
	}

	bool operator < (const C* ptr) const
	{
		return _ptr < ptr;
	}

	bool operator < (C* ptr) const
	{
		return _ptr < ptr;
	}

	bool operator <= (const C* ptr) const
	{
		return _ptr <= ptr;
	}

	bool operator <= (C* ptr) const
	{
		return _ptr <= ptr;
	}

	bool operator > (const C* ptr) const
	{
		return _ptr > ptr;
	}

	bool operator > (C* ptr) const
	{
		return _ptr > ptr;
	}

	bool operator >= (const C* ptr) const
	{
		return _ptr >= ptr;
	}

	bool operator >= (C* ptr) const
	{
		return _ptr >= ptr;
	}

private:
	C* _ptr;
};

} 


#endif // BINDING_UN_AUTO_PTR_H
