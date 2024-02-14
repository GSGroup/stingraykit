// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_CMDLINE_H
#define STINGRAYKIT_CMDLINE_H

#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/function/function.h>
#include <stingraykit/string/StringUtils.h>

#include <map>
#include <set>

namespace stingray
{

	namespace Detail
	{

		using CustomCompleteFunc = function<std::set<std::string>()>;
		using CustomCompleteFuncPtr = shared_ptr<CustomCompleteFunc>;
		using CustomCompleteFuncsMap = std::map<size_t, CustomCompleteFuncPtr>;


		struct CmdArgReader
		{
			template < typename T >
			static auto Read(const char* str, size_t len, T& val, int)
					-> decltype(val = FromString<T>(std::declval<std::string>()), size_t())
			{
				std::string str_val;
				size_t result = Read(str, len, str_val, 0);
				try
				{ val = FromString<T>(str_val); }
				catch(const std::exception&)
				{ return 0; }
				return result;
			}

			static size_t Read(const char* str, size_t len, std::string& val, int)
			{
				val.clear();
				bool escaping = false;
				size_t i = 0;
				for (; i < len; ++i)
				{
					if ((str[i] == ' ' || str[i] == ',' || str[i] == ']') && !escaping)
						break;

					if (str[i] == '\\' && !escaping)
						escaping = true;
					else
					{
						if (escaping)
							escaping = false;
						val.push_back(str[i]);
					}
				}
				return i;
			}

			static size_t Read(const char* str, size_t len, bool& val, int)
			{
				std::string str_val;
				size_t result = Read(str, len, str_val, 0);
				if (str_val == "on" || str_val == "true")
				{
					val = true;
					return result;
				}
				else if (str_val == "off" || str_val == "false")
				{
					val = false;
					return result;
				}
				return 0;
			}

			template < typename T >
			static size_t Read(const char* str, size_t len, optional<T>& val, long)
			{
				std::string str_val;
				size_t result = Read(str, len, str_val, 0);
				if (str_val == "none")
				{
					val = null;
					return result;
				}
				T t_val = T();
				size_t t_result = Read(str, result, t_val, 0);
				if (t_result != result)
					return 0;
				val = t_val;
				return result;
			}

			template < typename T >
			static size_t Read(const char* str, size_t len, std::vector<T>& val, long)
			{
				size_t result = 0;
				if (len - result == 0 || str[result] != '[')
					return 0;
				++result;
				val.clear();
				while (len - result != 0 && str[result] != ']')
				{
					T tmp;
					while (len - result != 0 && str[result] == ' ')
						++result;
					size_t tmp_result = Read(str + result, len - result, tmp, 0);
					if (tmp_result == 0)
						return 0;
					val.push_back(tmp);
					result += tmp_result;
					while (len - result != 0 && str[result] == ' ')
						++result;
					if (len - result == 0 || str[result] == ']')
						break;
					if (len - result == 0 || str[result] != ',')
						return 0;
					++result;
					while (len - result != 0 && str[result] == ' ')
						++result;
				}
				if (len - result == 0)
					return 0;
				++result;
				return result;
			}
		};


		template < size_t N >
		struct CmdArgCompleter
		{
			template < typename T >
			static bool Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete, long)
			{
				CustomCompleteFuncsMap::const_iterator it = customComplete.find(N);
				if (it == customComplete.end())
				{
					std::string str_val;
					size_t read_size = CmdArgReader::Read(input.c_str(), input.size(), str_val, 0);
					if (input.size() > read_size)
					{
						input.erase(0, read_size + 1);
						return true;
					}
					return false;
				}

				const CustomCompleteFunc& complete_func = *(it->second);
				std::set<std::string> pre_complete_list = complete_func();

				std::string str_val;
				size_t read_size = CmdArgReader::Read(input.c_str(), input.size(), str_val, 0);
				bool got_exact_match = false;
				std::set<std::string>::const_iterator iter = pre_complete_list.begin(), iend = pre_complete_list.end();
				for(; iter != iend; ++iter)
				{
					std::string str(*iter);
					if (str_val.size() > str.size())
						continue;
					if (str_val == str)
						got_exact_match = true;
					if (str.substr(0, str_val.size()) == str_val)
					{
						std::string res(str.substr(str_val.size()));
						ReplaceAll(res, " ", "\\ ");
						ReplaceAll(res, "]", "\\]");
						ReplaceAll(res, ",", "\\,");
						results.insert(res + " ");
					}
				}
				input.erase(0, read_size);
				bool ret = got_exact_match && (results.size() == 1 || !input.empty());
				if (!input.empty())
					input.erase(0, 1);
				if (ret)
					results.clear();
				return ret;
			}

			template < typename T >
			static auto Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete, int)
					-> decltype(std::declval<typename T::Enum>(), T::begin(), T::end(), bool())
			{
				if (customComplete.find(N) != customComplete.end())
					return CmdArgCompleter<N>::Complete<T>(input, results, customComplete, 0L);

				std::string str_val;
				size_t read_size = CmdArgReader::Read(input.c_str(), input.size(), str_val, 0);
				bool got_exact_match = false;
				typename T::const_iterator iter = T::begin(), iend = T::end();
				for(; iter != iend; ++iter)
				{
					std::string str(iter->ToString());
					if (str_val.size() > str.size())
						continue;
					if (str_val == str)
						got_exact_match = true;
					if (str.substr(0, str_val.size()) == str_val)
						results.insert(str.substr(str_val.size()) + " ");
				}
				input.erase(0, read_size);
				bool ret = got_exact_match && (results.size() == 1 || !input.empty());
				if (!input.empty())
					input.erase(0, 1);
				if (ret)
					results.clear();
				return ret;
			}

			template < typename T >
			static bool Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete, typename EnableIf<IsSame<T, bool>::Value, int>::ValueT)
			{
				if (customComplete.find(N) != customComplete.end())
					return CmdArgCompleter<N>::Complete<T>(input, results, customComplete, 0L);

				std::string value;
				const size_t size = CmdArgReader::Read(input.c_str(), input.size(), value, 0);

				bool got_exact_match = false;
				const std::vector<std::string> variants = { "on", "off", "true", "false" };
				for(std::vector<std::string>::const_iterator it = variants.begin(); it != variants.end(); ++it)
				{
					if (value.size() > (*it).length())
						continue;
					if (value == (*it))
						got_exact_match = true;
					if ((*it).substr(0, value.size()) == value)
						results.insert((*it).substr(value.size()) + " ");
				}
				input.erase(0, size);
				bool ret = got_exact_match && (results.size() == 1 || !input.empty());
				if (!input.empty())
					input.erase(0, 1);
				if (ret)
					results.clear();
				return ret;
			}
		};
	}


	struct CustomComplete
	{
		size_t							N;
		Detail::CustomCompleteFunc		Func;

		CustomComplete(size_t n, const Detail::CustomCompleteFunc& func)
			: N(n), Func(func)
		{ }
	};


	class CmdLine
	{
	public:
		using CompletionResults = std::set<std::string>;

	private:
		struct ICommandHandler
		{
			virtual ~ICommandHandler() { }

			virtual bool Execute(const std::string& cmd) = 0;
			virtual void Complete(const std::string& cmd, CompletionResults& results) const = 0;
			virtual void AddCustomComplete(const CustomComplete& customComplete) = 0;

			virtual void Release() = 0;
		};
		STINGRAYKIT_DECLARE_PTR(ICommandHandler);


		template < typename StringsTuple, typename ParamsList >
		class CmdHandler : public virtual ICommandHandler
		{
			using HandlerFunc = function<typename SignatureBuilder<void, ParamsList>::ValueT>;

			template < size_t N >
			struct StringsParser
			{
				static bool Call(const StringsTuple& strings, const char*& cmdPtr, size_t& tailLen)
				{
					const std::string& s = strings.template Get<N>();
					if (s.size() > tailLen)
						return false;
					const std::string cmd_s(cmdPtr, s.size());

					if (s != cmd_s)
						return false;

					tailLen -= cmd_s.size();
					cmdPtr += cmd_s.size();

					if (tailLen > 0)
					{
						if (*cmdPtr != ' ')
							return false;
						--tailLen;
						++cmdPtr;
					}
					return true;
				}
			};

			template < size_t N >
			struct StringsCompleter
			{
				static bool Call(const StringsTuple& strings, std::string& input, CompletionResults& results)
				{
					const std::string& str = strings.template Get<N>();
					size_t s = std::min(str.size(), input.size());

					if (str.substr(0, s) != input.substr(0, s))
						return false;

					if (str.size() >= input.size())
					{
						results.insert(str.substr(s) + " ");
						return false;
					}
					else
					{
						if (input[str.size()] != ' ')
							return false;
						input.erase(0, str.size() + 1);
					}
					return true;
				}
			};

			template < size_t N >
			struct ArgsParser
			{
				static bool Call(Tuple<ParamsList>& args, const char*& cmdPtr, size_t& tailLen)
				{
					using ParamType = typename GetTypeListItem<ParamsList, N>::ValueT;

					ParamType p = ParamType();
					size_t bytes_read = Detail::CmdArgReader::Read(cmdPtr, tailLen, p, 0);
					if (bytes_read == 0)
						return false;

					tailLen -= bytes_read;
					cmdPtr += bytes_read;

					if (tailLen > 0)
					{
						if (*cmdPtr != ' ')
							return false;
						--tailLen;
						++cmdPtr;
					}

					args.template Get<N>() = p;
					return true;
				}
			};

			template < size_t N >
			struct ArgsCompleter
			{
				static bool Call(std::string& input, CompletionResults& results, const Detail::CustomCompleteFuncsMap& customComplete)
				{
					using ParamType = typename GetTypeListItem<ParamsList, N>::ValueT;
					return Detail::CmdArgCompleter<N>::template Complete<ParamType>(input, results, customComplete, 0);
				}
			};

		private:
			StringsTuple					_strings;
			Detail::CustomCompleteFuncsMap	_customComplete;
			optional<HandlerFunc>			_handler;
			TaskLifeToken					_taskLife;

		public:
			CmdHandler(const StringsTuple& strings, const HandlerFunc& handler)
				: _strings(strings), _handler(handler)
			{ }

			void AddCustomComplete(const CustomComplete& customComplete) override
			{
				LocalExecutionGuard guard(_taskLife.GetExecutionTester());
				STINGRAYKIT_CHECK(guard, "Handler already released");

				STINGRAYKIT_CHECK(_customComplete.find(customComplete.N) == _customComplete.end(),
						StringBuilder() % "Custom complete func for argument #" % customComplete.N % " already registered!");

				_customComplete[customComplete.N] = make_shared_ptr<Detail::CustomCompleteFunc>(customComplete.Func);
			}

			bool Execute(const std::string& cmd) override
			{
				const char* cmd_str = cmd.c_str();
				size_t tail_len = cmd.size();

				Tuple<ParamsList> args;
				if (!ForIf<GetTypeListLength<typename StringsTuple::Types>::Value, StringsParser>::Do(_strings, wrap_ref(cmd_str), wrap_ref(tail_len)))
					return false;
				if (!ForIf<GetTypeListLength<ParamsList>::Value, ArgsParser>::Do(wrap_ref(args), wrap_ref(cmd_str), wrap_ref(tail_len)))
					return false;

				if (tail_len != 0)
					return false;

				LocalExecutionGuard guard(_taskLife.GetExecutionTester());
				if (!guard)
					return false;

				FunctorInvoker::Invoke(*_handler, args);
				return true;
			}

			void Complete(const std::string& cmd, CompletionResults& results) const override
			{
				std::string local(cmd);
				if (!ForIf<GetTypeListLength<typename StringsTuple::Types>::Value, StringsCompleter>::Do(_strings, wrap_ref(local), wrap_ref(results)))
					return;

				LocalExecutionGuard guard(_taskLife.GetExecutionTester());
				if (guard)
					ForIf<GetTypeListLength<ParamsList>::Value, ArgsCompleter>::Do(wrap_ref(local), wrap_ref(results), _customComplete);
			}

			void Release() override
			{
				_taskLife.Release();
				_handler.reset();
				_customComplete.clear();
			}
		};

		template < size_t Size >
		struct StringsTupleCreator
		{
			template < size_t > struct Functor { using ValueT = std::string; };

			using ValueT = Tuple<typename GenerateTypeList<Size, Functor>::ValueT>;
		};

		using Commands = IObservableDictionary<size_t, ICommandHandlerPtr>;
		STINGRAYKIT_DECLARE_PTR(Commands);

	public:
		class CustomCompleteFuncSetter
		{
		private:
			ICommandHandlerPtr		_handler;
			Token					_token;

		public:
			CustomCompleteFuncSetter(const ICommandHandlerPtr& handler, const Token& token)
				:	_handler(STINGRAYKIT_REQUIRE_NOT_NULL(handler)),
					_token(token)
			{ }

			CustomCompleteFuncSetter& operator % (const CustomComplete& customComplete)
			{
				_handler->AddCustomComplete(customComplete);
				return *this;
			}

			operator Token () const
			{ return _token; }
		};

		template < typename StringsTuple >
		class HandlerInserter
		{
		private:
			CommandsPtr			_commands;
			StringsTuple		_strings;

		public:
			HandlerInserter(const CommandsPtr& commands, const StringsTuple& strings)
				:	_commands(STINGRAYKIT_REQUIRE_NOT_NULL(commands)),
					_strings(strings)
			{ }

			template < typename HandlerFunc >
			CustomCompleteFuncSetter operator = (const HandlerFunc& handlerFunc)
			{
				using DecayedParams = typename TypeListTransform<typename function_info<HandlerFunc>::ParamTypes, Decay>::ValueT;

				const ICommandHandlerPtr handler = make_shared_ptr<CmdHandler<StringsTuple, DecayedParams>>(_strings, handlerFunc);
				const Token token = Register(_commands, handler);

				return CustomCompleteFuncSetter(handler, token);
			}
		};

	private:
		CommandsPtr			_commands;

	public:
		CmdLine();

		bool Execute(const std::string& cmd);
		void Complete(const std::string& cmd, CompletionResults& results) const;

		template < typename T0, typename... Ts >
		HandlerInserter<typename StringsTupleCreator<sizeof...(Ts) + 1>::ValueT> Handler(const T0& p0, const Ts&... args)
		{ return HandlerImpl(typename StringsTupleCreator<sizeof...(Ts) + 1>::ValueT(p0, args...)); }

	private:
		template < typename StringsTuple >
		HandlerInserter<StringsTuple> HandlerImpl(const StringsTuple& s)
		{ return HandlerInserter<StringsTuple>(_commands, s); }

		static Token Register(const CommandsPtr& commands, const ICommandHandlerPtr& handler);
		static void Unregister(const CommandsPtr& commands, size_t key);
	};

}

#endif
