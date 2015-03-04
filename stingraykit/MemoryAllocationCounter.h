#ifndef STINGRAYKIT_MEMORYALLOCATIONCOUNTER_H
#define STINGRAYKIT_MEMORYALLOCATIONCOUNTER_H

// Copyright (c) 2011 - 2015, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/shared_ptr.h>

namespace stingray {
	struct IMemoryAllocationCountable
	{
		virtual ~IMemoryAllocationCountable() {}
		virtual size_t GetAllocatedMemory() const = 0;
	};

	struct MemoryAllocationCounter
	{
	private:
		size_t _allocatedMemory;
	public:
		MemoryAllocationCounter() : _allocatedMemory(0)
		{}
		void Add(size_t memory)								{ _allocatedMemory += memory; }
		void Remove(size_t memory)							{ _allocatedMemory -= memory; }
		template<typename T>
		void Add(const T &item)								{ _allocatedMemory += item.GetAllocatedMemory(); }
		template<typename T>
		void Remove(const T &item)							{ _allocatedMemory -= item.GetAllocatedMemory(); }
		template<typename T>
		void AddByPtr(const shared_ptr<T> &item)			{ _allocatedMemory += item->GetAllocatedMemory(); }
		template<typename T>
		void RemoveByPtr(const shared_ptr<T> &item)			{ _allocatedMemory -= item->GetAllocatedMemory(); }
		template<typename T>
		void Process(const T& item, CollectionOp event)
		{
			switch(event)
			{
			case CollectionOp::Added:		Add(item); break;
			case CollectionOp::Removed:		Remove(item); break;
			default: break;
			}
		}
		size_t GetAllocatedMemory() const					{ return _allocatedMemory; }
	};
}

#endif
