#pragma once
namespace Pistachio
{
	struct FreeBlock
	{
		uint32_t size;
		uint32_t offset;
		FreeBlock* next;
	};
	/*
	* Please note that this free list is quite unoptimal and fragile
	* It it not intended for general use, and is very unchecked
	* 
	*/
	class PISTACHIO_API FreeList
	{
	public:
		FreeList() = default;
		FreeList(uint32_t size);
		//Reset the free list, making all bytes free
		void Reset();
		//Adds @size more free bytes to end of the block
		void Grow(uint32_t size);
		/*If you're sure you have enough space at a given offset use this to
		* allocate memory at that given offset
		*/
		uint32_t Allocate(uint32_t offset, uint32_t size);
		//Adds a new free block at @offset that is @size bytes
		void DeAllocate(uint32_t offset, uint32_t size);
		//Finds a block of given size and returns the offset from start, else returns UINT32_MAX
		uint32_t Find(uint32_t size);
		inline FreeBlock* GetBlockPtr() { return &firstBlock; }
	private:
		uint32_t fullSize;
		FreeBlock firstBlock;
	};
}
