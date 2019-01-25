#ifndef STINGRAYKIT_DIAGNOSTICS_EXTERNALAPIGUARD_H
#define STINGRAYKIT_DIAGNOSTICS_EXTERNALAPIGUARD_H

// Copyright (c) 2011 - 2019, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/toolkit.h>

namespace stingray
{

	class ExternalAPIGuardStack
	{
		friend class EnterExternalAPIGuardBase;
		friend class LeaveExternalAPIGuardBase;

	public:
		struct StackEntry
		{
			enum Type { EnterExternalAPI, LeaveExternalAPI };
			
			Type			type;
			const char*		entry;

			StackEntry(Type type, const char* entry) : type(type), entry(entry) { }
		};

		typedef std::vector<StackEntry>	Stack;

	private:
		Stack	_stack;

	public:
		std::string Get() const
		{
			if (_stack.empty())
				return "<external API guard stack empty>";

			std::string result;

			if (_stack.rbegin()->type == StackEntry::EnterExternalAPI)
				result += "in external API: ";				
			else
				result += "in stingray: ";				

			for (Stack::const_iterator it = _stack.begin(); it != _stack.end(); ++it)
				result += std::string(it != _stack.begin() ? " -> " : "") + it->entry;

			return result;
		}

	private:
		void Push(StackEntry entry)	{ _stack.push_back(entry); }
		void Pop()					{ _stack.pop_back(); }
	};


	class EnterExternalAPIGuardBase
	{
		STINGRAYKIT_NONCOPYABLE(EnterExternalAPIGuardBase);

	private:
		ExternalAPIGuardStack*	_stack;
		const char*				_externalApiEntry;

	public:
		EnterExternalAPIGuardBase(ExternalAPIGuardStack* stack, const char* externalApiEntry)
			: _stack(stack), _externalApiEntry(externalApiEntry)
		{ if (_stack) _stack->Push(ExternalAPIGuardStack::StackEntry(ExternalAPIGuardStack::StackEntry::EnterExternalAPI, externalApiEntry)); }

		~EnterExternalAPIGuardBase()
		{ if (_stack) _stack->Pop(); }
	};


	class LeaveExternalAPIGuardBase
	{
		STINGRAYKIT_NONCOPYABLE(LeaveExternalAPIGuardBase);

	private:
		ExternalAPIGuardStack*	_stack;
		const char*				_internalApiEntry;

	public:
		LeaveExternalAPIGuardBase(ExternalAPIGuardStack* stack, const char* internalApiEntry)
			: _stack(stack), _internalApiEntry(internalApiEntry)
		{ if (_stack) _stack->Push(ExternalAPIGuardStack::StackEntry(ExternalAPIGuardStack::StackEntry::LeaveExternalAPI, internalApiEntry)); }

		~LeaveExternalAPIGuardBase()
		{ if (_stack) _stack->Pop(); }
	};

}

#endif
