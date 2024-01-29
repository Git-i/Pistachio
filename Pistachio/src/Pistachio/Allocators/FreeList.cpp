#include "ptpch.h"
#include "FreeList.h"

namespace Pistachio
{
	FreeList::FreeList(uint32_t size)
	{
		fullSize = size;
		firstBlock.size = size;
		firstBlock.offset = 0;
		firstBlock.next = nullptr;
	}
	void FreeList::Reset()
	{
		//first block is not allocated on the heap
		FreeBlock* block = firstBlock.next;
		while (block)
		{
			auto current = block;
			block = block->next;
			delete(current);
		}
		firstBlock.next = nullptr;
		firstBlock.offset = 0;
		firstBlock.size = fullSize;
	}
	void FreeList::Grow(uint32_t size)
	{
		//retrieve last item
		FreeBlock* block = &firstBlock;
		while (block->next)
		{
			block = block->next;
		}
		//if the last block is to the end of the list
		if (block->offset + block->size == fullSize)
		{
			block->size += size - fullSize;
			fullSize = size;
			return;
		}
		else
		{
			FreeBlock* next = new FreeBlock;
			next->next = nullptr;
			next->offset = fullSize;
			next->size = size - fullSize;
			fullSize = size;
			return;
		}
	}
	uint32_t FreeList::Allocate(uint32_t offset, uint32_t size)
	{
		//start at the beginning of the list
		FreeBlock* block = &firstBlock;
		while (block)
		{
			//check if said offset is within the block
			if (offset >= block->offset && offset <= block->offset + block->size)
			{
				//found the block
				PT_CORE_ASSERT(block->offset + block->size - offset >= size);
				if (offset + size == block->offset + block->size) // they end at the same byte
				{
					//resize the block
					block->size = block->size - size;
					return 0;
				}
				else 
				{
					//make a new block
					FreeBlock* newBlock = new FreeBlock;
					newBlock->offset = offset + size;
					newBlock->size = block->offset + block->size - newBlock->offset;
					newBlock->next = block->next;

					block->next = newBlock;
					block->size -= size + newBlock->size;
					return 0;
				}
				break;
			}
			block = block->next;
		}
		return 1;
	}
	void FreeList::DeAllocate(uint32_t offset, uint32_t size)
	{
		//expand free block by decreasing the offset of block after
		//probably extend block with bigger size to make more continuous free blocks
		FreeBlock* block = &firstBlock;
		while (block)
		{
			if (offset <= block->offset)
			{
				PT_CORE_ASSERT(offset + size < block->offset);
				block->offset -= offset;
				break;
			}
			block = block->next;
		}
	}
	uint32_t FreeList::Find(uint32_t size)
	{
		FreeBlock* block = &firstBlock;
		while (block)
		{
			if (block->size >= size)
			{
				return block->offset;
				break;
			}
			block = block->next;
		}
		return UINT32_MAX;
	}
}
