// Copyright (c) 2011 - 2024, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef STINGRAYKIT_SERIALIZATION_INJECTIONSERIALIZER_H
#define STINGRAYKIT_SERIALIZATION_INJECTIONSERIALIZER_H

#include <stingraykit/serialization/Serialization.h>
#include <stingraykit/serialization/SettingsValue.h>

namespace stingray
{

	class InjectionSerializer
	{
		class Visitor : public static_visitor<void>
		{
		private:
			IObjectOStreamPrivate*		_stream;

		public:
			explicit Visitor(IObjectOStreamPrivate* stream)
				:	_stream(stream)
			{ }

			template < typename  T >
			void operator () (const T& value) const
			{ _stream->Write(value); }

			void operator () (const EmptyType&) const
			{ _stream->WriteNull(); }

			void operator () (const SettingsValueList& list) const
			{
				_stream->BeginList();

				for (const auto& item : list)
					apply_visitor(Visitor(_stream), *item);

				_stream->EndList();
			}

			void operator () (const SettingsValueMap& map) const
			{
				_stream->BeginObject();

				for (const auto& item : map)
				{
					_stream->WritePropertyName(item.first);
					apply_visitor(Visitor(_stream), *item.second);
				}

				_stream->EndObject();
			}
		};

	private:
		unique_ptr<ObjectIStream>		_stream;
		SettingsValueSelfCountPtr		_value;

	public:
		explicit InjectionSerializer(unique_ptr<ObjectIStream>&& stream)
			:	_stream(std::move(STINGRAYKIT_REQUIRE_NOT_NULL(stream)))
		{ }

		explicit InjectionSerializer(const SettingsValueSelfCountPtr& value)
			:	_value(STINGRAYKIT_REQUIRE_NOT_NULL(value))
		{ }

		void SerializeAsValue(ObjectOStream& ar) const
		{ apply_visitor(Visitor(ar.GetRawObjectStream()), _stream ? *_stream->Root() : *_value); }
	};

}

#endif
