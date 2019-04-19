/*

	Template class that defines a pointer that sets all references to itself to null
	when its destructor is called.

	Also defines a template class that is used to refer to the master item.

*/

#pragma once
#include <list>
#include <algorithm>

template<typename T>
class lame_ptr
{
public:
	operator->()
	{
		return *obj;
	}

	operator=(lame_ptr<T> other)
	{
		this->ptr = other.ptr;
	}

	friend class god_ptr<T>;
private:
	god_ptr* ptr; // becomes NULL if obj is freed
};

template<typename T>
class god_ptr
{
public:
	god_ptr(T obj)
	{

	}

	~god_ptr()
	{
		std::for_each(refs.begin(), refs.end(), [](lame_ptr& p)
		{
			p.ptr = nullptr;
		});
	}

private:
	god_ptr(god_ptr<T>&) = delete;
	god_ptr(void) = delete;
	
	std::list<lame_ptr> refs;

	T* ptr;
};