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

	public:
		explicit InjectionSerializer(unique_ptr<ObjectIStream>&& stream)
			:	_stream(std::move(STINGRAYKIT_REQUIRE_NOT_NULL(stream)))
		{ }

		void SerializeAsValue(ObjectOStream& ar) const
		{ apply_visitor(Visitor(ar.GetRawObjectStream()), *_stream->Root()); }
	};

}

#endif
