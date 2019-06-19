#ifndef STINGRAYKIT_CMDLINE_H
#define STINGRAYKIT_CMDLINE_H

#include <stingraykit/function/function.h>
#include <stingraykit/string/StringUtils.h>
#include <stingraykit/reference.h>
#include <stingraykit/toolkit.h>

#include <set>
#include <stdlib.h>
#include <string>
#include <vector>

namespace stingray
{

	namespace Detail
	{

		typedef function<std::set<std::string>()>						CustomCompleteFunc;
		typedef shared_ptr<CustomCompleteFunc>							CustomCompleteFuncPtr;
		typedef std::map<size_t, CustomCompleteFuncPtr>					CustomCompleteFuncsMap;

		template < typename T, bool THasFromString = HasMethod_FromString<T>::Value, bool TIsIntType = IsInt<T>::Value >
		struct CmdArgReader;


		template < typename T >
		struct IntCmdArgReader
		{
			static size_t Read(const char* str, size_t len, T& val)
			{
				char* end_p = null;
				s64 tmp = strtoll(str, &end_p, 10);
				T tmp2 = (T)tmp;
				if ((s64)tmp2 != tmp)
					return 0;
				val = tmp2;
				return end_p - str;
			}
		};

		template < typename T >
		struct CmdArgReader<T, false, true> : public IntCmdArgReader<T> { };

		template < >
		struct CmdArgReader<std::string, false, false>
		{
			static size_t Read(const char* str, size_t len, std::string& val)
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
		};

		template < >
		struct CmdArgReader<bool, false, true>
		{
			static size_t Read(const char* str, size_t len, bool& val)
			{
				std::string str_val;
				size_t result = CmdArgReader<std::string, false>::Read(str, len, str_val);
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
		};

		template < typename T >
		struct CmdArgReader<optional<T>, false, false>
		{
			static size_t Read(const char* str, size_t len, optional<T>& val)
			{
				std::string str_val;
				size_t result = CmdArgReader<std::string, false>::Read(str, len, str_val);
				if (str_val == "none")
				{
					val = null;
					return result;
				}
				T t_val = T();
				size_t t_result = CmdArgReader<T>::Read(str_val.c_str(), str_val.size(), t_val);
				if (t_result != str_val.size())
					return 0;
				val = t_val;
				return result;
			}
		};

		template < typename T >
		struct CmdArgReader<T, true, false>
		{
			static size_t Read(const char* str, size_t len, T& val)
			{
				std::string str_val;
				size_t result = CmdArgReader<std::string, false>::Read(str, len, str_val);
				try
				{ val = T::FromString(str_val); }
				catch(const std::exception&)
				{ return 0; }
				return result;
			}
		};

		template < typename T >
		struct CmdArgReader<std::vector<T>, false, false>
		{
			static size_t Read(const char* str, size_t len, std::vector<T>& val)
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
					size_t tmp_result = CmdArgReader<T>::Read(str + result, len - result, tmp);
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

		template < typename T >
		size_t ReadCmdArg(const char* str, size_t len, T& val)
		{ return CmdArgReader<T>::Read(str, len, val); }

		template < typename T, size_t N, bool TIsUnknownType = !IsEnumClass<T>::Value && !IsSame<bool, T>::Value >
		struct CmdArgCompleter
		{
			static bool Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete)
			{
				if (customComplete.empty())
					return false;

				CustomCompleteFuncsMap::const_iterator it = customComplete.find(N);
				if (it == customComplete.end())
					return false;

				const CustomCompleteFunc& complete_func = *(it->second);
				std::set<std::string> pre_complete_list = complete_func();

				std::string str_val;
				size_t read_size = CmdArgReader<std::string, false>::Read(input.c_str(), input.size(), str_val);
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
		};


		template < typename T, size_t N >
		struct CmdArgCompleter<T, N, false >
		{
			static bool Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete)
			{
				if (customComplete.find(N) != customComplete.end())
					return CmdArgCompleter<T, N, true>::Complete(input, results, customComplete);

				std::string str_val;
				size_t read_size = CmdArgReader<std::string, false>::Read(input.c_str(), input.size(), str_val);
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
		};

		template < size_t N >
		struct CmdArgCompleter<bool, N, false>
		{
			static bool Complete(std::string& input, std::set<std::string>& results, const CustomCompleteFuncsMap& customComplete)
			{
				if (customComplete.find(N) != customComplete.end())
					return CmdArgCompleter<bool, N, true>::Complete(input, results, customComplete);

				std::string value;
				const size_t size = CmdArgReader<std::string, false>::Read(input.c_str(), input.size(), value);

				bool got_exact_match = false;
				const std::vector<std::string> variants = VectorBuilder<std::string>() % "on" % "off" % "true" % "false";
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
		typedef std::set<std::string> CompletionResults;
	private:

		struct ICommandHandler
		{
			virtual ~ICommandHandler() { }
			virtual bool Execute(const std::string& cmd) = 0;
			virtual void Complete(const std::string& cmd, CompletionResults& results) = 0;
			virtual void AddCustomComplete(const CustomComplete& customComplete) = 0;
		};
		STINGRAYKIT_DECLARE_PTR(ICommandHandler);


		template < typename StringsTuple, typename ParamsList >
		class CmdHandler : public virtual ICommandHandler
		{
			typedef typename FunctionConstructor<void, ParamsList>::ValueT	HandlerFunc;

			template < int N >
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

			template < int N >
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

			template < int N >
			struct ArgsParser
			{
				static bool Call(Tuple<ParamsList>& args, const char*& cmdPtr, size_t& tailLen)
				{
					typedef typename GetTypeListItem<ParamsList, N>::ValueT	ParamType;

					ParamType p = ParamType();
					size_t bytes_read = Detail::ReadCmdArg(cmdPtr, tailLen, p);
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

					args.template Set<N>(p);
					return true;
				}
			};

			template < int N >
			struct ArgsCompleter
			{
				static bool Call(std::string& input, CompletionResults& results, const Detail::CustomCompleteFuncsMap& customComplete)
				{
					typedef typename GetTypeListItem<ParamsList, N>::ValueT	ParamType;
					return Detail::CmdArgCompleter<ParamType, N>::Complete(input, results, customComplete);
				}
			};


		private:
			StringsTuple					_strings;
			Detail::CustomCompleteFuncsMap	_customComplete;
			HandlerFunc						_handler;

		public:
			CmdHandler(const StringsTuple& strings, const HandlerFunc& handler)
				: _strings(strings), _handler(handler)
			{ }

			virtual void AddCustomComplete(const CustomComplete& customComplete)
			{
				Detail::CustomCompleteFuncsMap::const_iterator it = _customComplete.find(customComplete.N);
				if (it != _customComplete.end())
					STINGRAYKIT_THROW("Custom complete func for argument #" + stingray::ToString(customComplete.N) + " already registered!");
				_customComplete[customComplete.N] = make_shared_ptr<Detail::CustomCompleteFunc>(customComplete.Func);
			}

			virtual bool Execute(const std::string& cmd)
			{
				const char* cmd_str = cmd.c_str();
				size_t tail_len = cmd.size();

				Tuple<ParamsList> args;
				if (!ForIf<GetTypeListLength<typename StringsTuple::TypeList>::Value, StringsParser>::Do(wrap_ref(_strings), wrap_ref(cmd_str), wrap_ref(tail_len)))
					return false;
				if (!ForIf<GetTypeListLength<ParamsList>::Value, ArgsParser>::Do(wrap_ref(args), wrap_ref(cmd_str), wrap_ref(tail_len)))
					return false;

				if (tail_len != 0)
					return false;

				FunctorInvoker::Invoke(_handler, args);

				return true;
			}

			virtual void Complete(const std::string& cmd, CompletionResults& results)
			{
				std::string local(cmd);
				if (!ForIf<GetTypeListLength<typename StringsTuple::TypeList>::Value, StringsCompleter>::Do(wrap_ref(_strings), wrap_ref(local), wrap_ref(results)))
					return;
				ForIf<GetTypeListLength<ParamsList>::Value, ArgsCompleter>::Do(wrap_ref(local), wrap_ref(results), wrap_const_ref(_customComplete));
			}
		};

		typedef std::vector<ICommandHandlerPtr> 	Commands;

	public:
		class CustomCompleteFuncSetter
		{
		private:
			ICommandHandlerPtr		_cmdHandler;

		public:
			CustomCompleteFuncSetter(const ICommandHandlerPtr& cmdHandler)
				: _cmdHandler(STINGRAYKIT_REQUIRE_NOT_NULL(cmdHandler))
			{ }

			CustomCompleteFuncSetter& operator % (const CustomComplete& customComplete)
			{
				_cmdHandler->AddCustomComplete(customComplete);
				return *this;
			}
		};

		template < typename StringsTuple >
		class HandlerInserter
		{
			friend class CmdLine;

			CmdLine*			_inst;
			StringsTuple	_strings;

			HandlerInserter(CmdLine* inst, const StringsTuple& strings) : _inst(inst), _strings(strings) { }

		public:
			template < typename HandlerFunc >
			CustomCompleteFuncSetter operator = (const HandlerFunc& handlerFunc)
			{
				typedef typename TypeListTransform<typename function_info<HandlerFunc>::ParamTypes, Decay>::ValueT	DecayedParams;
				ICommandHandlerPtr ch = make_shared_ptr<CmdHandler<StringsTuple, DecayedParams> >(_strings, handlerFunc);
				_inst->_commands.push_back(ch);
				return CustomCompleteFuncSetter(ch);
			}
		};

	private:
		Commands		_commands;

	public:
		bool Execute(const std::string& cmd)
		{
			if (!cmd.empty() && cmd[0] == '#')
				return true;

			for (Commands::const_iterator it = _commands.begin(); it != _commands.end(); ++it)
				if ((*it)->Execute(cmd))
					return true;

			return false;
		}

		void Complete(const std::string& cmd, CompletionResults& results) const
		{
			for (Commands::const_iterator it = _commands.begin(); it != _commands.end(); ++it)
				(*it)->Complete(cmd, results);
		}

		typedef std::string Str;

		HandlerInserter<Tuple<TypeList<Str>::type> > Handler(const Str& s1)
		{ return HandlerImpl(MakeTuple(s1)); }

		HandlerInserter<Tuple<TypeList<Str, Str>::type> > Handler(const Str& s1, const Str& s2)
		{ return HandlerImpl(MakeTuple(s1, s2)); }

		HandlerInserter<Tuple<TypeList<Str, Str, Str>::type> > Handler(const Str& s1, const Str& s2, const Str& s3)
		{ return HandlerImpl(MakeTuple(s1, s2, s3)); }

		HandlerInserter<Tuple<TypeList<Str, Str, Str, Str>::type> > Handler(const Str& s1, const Str& s2, const Str& s3, const Str& s4)
		{ return HandlerImpl(MakeTuple(s1, s2, s3, s4)); }

		HandlerInserter<Tuple<TypeList<Str, Str, Str, Str, Str>::type> > Handler(const Str& s1, const Str& s2, const Str& s3, const Str& s4, const Str& s5)
		{ return HandlerImpl(MakeTuple(s1, s2, s3, s4, s5)); }

	private:
		template < typename StringsTuple >
		HandlerInserter<StringsTuple> HandlerImpl(const StringsTuple& s)
		{ return HandlerInserter<StringsTuple>(this, s); }
	};


}


#endif
