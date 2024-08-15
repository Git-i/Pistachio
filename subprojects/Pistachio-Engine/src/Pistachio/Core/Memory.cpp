//Global Allocation Routines For the profiler
#include "common/TracyAlloc.hpp"
#include "ptpch.h"
#include <cstddef>

#if PT_PROFILE_TRACY
	void* operator new(std::size_t sz)
	{
		if (sz == 0)
			++sz; 
		if (void *ptr = std::malloc(sz))
		{
			TracyAlloc(ptr, sz);
			return ptr;
		}
		abort();
	}
	void* operator new[](std::size_t sz)
	{
		if (sz == 0)
			++sz; 
		if (void *ptr = std::malloc(sz))
		{
			TracyAlloc(ptr, sz);
			return ptr;
		}
		abort();
	}
	void operator delete(void* ptr) noexcept
	{
	    free(ptr);
		TracyFree(ptr);
	}
	
	void operator delete(void* ptr, std::size_t size) noexcept
	{
	    free(ptr);
		TracyFree(ptr);
	}
	
	void operator delete[](void* ptr) noexcept
	{
	    free(ptr);
		TracyFree(ptr);
	}
	
	void operator delete[](void* ptr, std::size_t size) noexcept
	{
	    free(ptr);
		TracyFree(ptr);
	}
#endif