#ifndef STINGRAYKIT_DIAGNOSTICS_PROFILESESSION_H
#define STINGRAYKIT_DIAGNOSTICS_PROFILESESSION_H

// Copyright (c) 2011 - 2017, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/FunctionToken.h>
#include <stingraykit/optional.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/Token.h>
#include <stingraykit/log/Logger.h>
#include <stingraykit/time/ElapsedTime.h>
#include <stingraykit/string/lexical_cast.h>
#include <stingraykit/string/string_stream.h>

#include <map>

namespace stingray
{

	/**
	 * @addtogroup toolkit_profiling
	 * @{
	 */

	class ProfileSession;
	STINGRAYKIT_DECLARE_PTR(ProfileSession);


	class ProfileSession
	{
		struct SessionInfo
		{
			typedef std::map<std::string, SessionInfo>		ChildrenMap;

			std::string		Id;
			u64				Time;
			ChildrenMap		Children;

			SessionInfo() { }

			SessionInfo(const std::string& id) : Id(id), Time(0) { }

			void ToString(string_ostream & ss, size_t level, u64 parentTime) const
			{
				size_t nextLevel = level + 1;
				while (level--)
					ss << "\t";
				ss << Id << ", " << Time << " ms.";
				if (nextLevel > 1)
					ss << " (" << (Time ? (Time * 100) / parentTime : 0) << "%)";
				ss << '\n';
				for (ChildrenMap::const_iterator it = Children.begin(); it != Children.end(); ++it)
					it->second.ToString(ss, nextLevel, Time);
			}
		};

	private:
		SessionInfo				_info;

		optional<ElapsedTime>	_elapsedTime;
		Token					_reportToParentToken;

		u64						_threshold;
		bool					_enable;

	public:
		ProfileSession(const std::string& id, u64 threshold = 0, bool enable = true) : _info(id), _threshold(threshold), _enable(enable)
		{
			_elapsedTime = ElapsedTime();
		}

		ProfileSession(ProfileSession& parent, const std::string& id) : _info(id), _enable(parent._enable)
		{
			if (_enable)
			{
				_reportToParentToken = MakeToken<FunctionToken>(bind(&ProfileSession::ChildCallback, &parent, ref(*this)));
				_elapsedTime = ElapsedTime();
			}
		}

		~ProfileSession()
		{
			if (_enable)
			{
				_info.Time = _elapsedTime->ElapsedMilliseconds();

				if (!_reportToParentToken && _info.Time > _threshold)
				{
					string_ostream ss;
					_info.ToString(ss, 0, 0);
					Logger::Info() << ss.str();
				}
			}
		}

	private:
		void ChildCallback(const ProfileSession& child)
		{
			AddChildren(_info.Children[child._info.Id], child._info);
		}

		static void AddChildren(SessionInfo& to, const SessionInfo& from)
		{
			if (to.Id.empty())
				to = from;
			else
			{
				to.Time += from.Time;
				for (SessionInfo::ChildrenMap::const_iterator it = from.Children.begin(); it != from.Children.end(); ++it)
					AddChildren(to.Children[it->first], it->second);
			}
		}
	};

	/** @} */

}

#endif
