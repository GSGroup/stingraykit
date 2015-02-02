#ifndef STINGRAYKIT_SERIALIZATION_SERIALIZATION_H
#define STINGRAYKIT_SERIALIZATION_SERIALIZATION_H


#include <string>
#include <vector>
#include <map>

#include <stingraykit/diagnostics/PrivateIncludeGuard.h>

#include <stingraykit/Factory.h>
#include <stingraykit/Final.h>
#include <stingraykit/MetaProgramming.h>
#include <stingraykit/TypeList.h>
#include <stingraykit/collection/ByteData.h>
#include <stingraykit/collection/ForEach.h>
#include <stingraykit/collection/IDictionary.h>
#include <stingraykit/collection/IList.h>
#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/IObservableList.h>
#include <stingraykit/collection/IObservableSet.h>
#include <stingraykit/collection/ISet.h>
#include <stingraykit/collection/ITransactionalDictionary.h>
#include <stingraykit/function/bind.h>
#include <stingraykit/reference.h>
#include <stingraykit/serialization/ISerializable.h>
#include <stingraykit/serialization/SettingsValueException.h>
#include <stingraykit/serialization/SettingsValueForward.h>
#include <stingraykit/shared_ptr.h>
#include <stingraykit/string/ToString.h>
#include <stingraykit/toolkit.h>
#include <stingraykit/variant.h>


namespace stingray
{

	/**
	 * @addtogroup core_settings
	 * @{
	 */

	struct SerializationUtils
	{
		STINGRAYKIT_DECLARE_METHOD_CHECK(Deserialize);
		STINGRAYKIT_DECLARE_METHOD_CHECK(Serialize);

		template<typename T>
		struct IsStringRepresentable
		{ static const bool Value = HasMethod_FromString<T>::Value && HasMethod_ToString<T>::Value && !(HasMethod_Serialize<T>::Value && HasMethod_Deserialize<T>::Value); };

		typedef TypeList<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, size_t, u8, u16, u32, u64>::type UnsignedTypes;
		typedef TypeList<char, short, int, long, long long, off_t, s8, s16, s32, s64>::type SignedTypes;

		struct TypeTraits
		{
			STINGRAYKIT_ENUM_VALUES( SignedInt, UnsignedInt, StringRepresentableClass, NullType, Other );
			STINGRAYKIT_DECLARE_ENUM_CLASS(TypeTraits);
		};
		template<typename Type, typename Stream, TypeTraits::Enum Traits =
			SameType<Type, NullPtrType>::Value? TypeTraits::NullType:
				TypeListContains<UnsignedTypes, Type>::Value? TypeTraits::UnsignedInt:
					(TypeListContains<SignedTypes, Type>::Value? TypeTraits::SignedInt:
						(IsStringRepresentable<Type>::Value ? TypeTraits::StringRepresentableClass : TypeTraits::Other))
		>
		struct TypeWriterImpl;

		template<typename Type, typename Stream>
		struct TypeWriterImpl<Type, Stream, TypeTraits::SignedInt>
		{
			static void Serialize(Stream &ar, Type t)		{ ar.Write((s64)t);}
			static void Deserialize(Stream &ar, Type &t)	{ t = ar.GetInt(); }
		};

		template<typename Type, typename Stream>
		struct TypeWriterImpl<Type, Stream, TypeTraits::UnsignedInt>
		{
			static void Serialize(Stream &ar, Type t)		{ ar.Write((s64)((u64)t));}
			static void Deserialize(Stream &ar, Type &t)	{ s64 v = ar.GetInt(); t = (u64)v; }
		};

		template<typename Type, typename Stream>
		struct TypeWriterImpl<Type, Stream, TypeTraits::StringRepresentableClass> {
			static void Serialize(Stream &ar, const Type& t)	{ ar.serialize(t.ToString());}
			static void Deserialize(Stream &ar, Type &t)		{ std::string tmp; ar.deserialize(tmp); t = (!tmp.empty())? Type(Type::FromString(tmp)): Type(); }
		};

		template<typename Type, typename Stream>
		struct TypeWriterImpl<Type, Stream, TypeTraits::NullType>
		{
			static void Serialize(Stream &ar, Type t)		{ ar.WriteNull();}
			static void Deserialize(Stream &ar, Type &t)	{ t = null; }
		};

		template<typename Type, typename Stream>
		struct TypeWriterImpl<Type, Stream, TypeTraits::Other>
		{
			static void Serialize(Stream &ar, const Type& t)	{ ar.serialize(t);}
			static void Deserialize(Stream &ar, Type &t)		{ ar.deserialize(t); }
		};


		template < typename StreamType_ >
		class VariantSerializeVisitor : public static_visitor<void>
		{
		private:
			StreamType_*	_oos;

		public:
			VariantSerializeVisitor(StreamType_& oos)
				: _oos(&oos)
			{ }

			template < typename T >
			void operator () (const T& val) const
			{ _oos->Serialize("obj", val); }

			void operator () (const EmptyType & val) const
			{ }
		};

		template < typename StreamType_ >
		class VariantDeserializeVisitor : public static_visitor<void>
		{
		private:
			StreamType_*	_oos;

		public:
			VariantDeserializeVisitor(StreamType_& oos)
				: _oos(&oos)
			{ }

			template < typename T >
			void operator () (T& val) const
			{ _oos->Deserialize("obj", val); }

			void operator () (EmptyType& val) const
			{ }
		};

		template < typename Types_ >
		struct VariantObjectCreator
		{
			template < int Index_ >
			struct CreatorFunc
			{
				static void Call(int deserializedIndex, variant<Types_>& v)
				{
					typedef typename GetTypeListItem<Types_, Index_>::ValueT		T;
					if (deserializedIndex == Index_)
						v = T();
				}
			};
		};
	};


	template < typename T, typename Stream >
	struct TypeWriter : public SerializationUtils::TypeWriterImpl<T, Stream>
	{ };


	struct IObjectOStreamPrivate
	{
		template < typename, typename > friend struct TypeWriter;
		template < typename, typename, SerializationUtils::TypeTraits::Enum > friend struct SerializationUtils::TypeWriterImpl;

		virtual ~IObjectOStreamPrivate() { }

		typedef std::vector<u8> byte_array;

		virtual void WriteNull() = 0;
		virtual void Write(bool value) = 0;
		virtual void Write(double value) = 0;
		virtual void Write(s64 value) = 0;
		virtual void Write(const std::string &value) = 0;
		virtual void Write(const byte_array &value) = 0;

		virtual void WritePropertyName(const std::string &name) = 0;

		virtual void BeginList() = 0;
		virtual void EndList() = 0;

		virtual void BeginObject() = 0;
		virtual void EndObject() = 0;
	};

	class ObjectOStream : protected IObjectOStreamPrivate
	{
		template < typename, typename > friend struct TypeWriter;
		template < typename, typename, SerializationUtils::TypeTraits::Enum > friend struct SerializationUtils::TypeWriterImpl;

		IObjectSerializator	*_collection;
		FactoryContextPtr	_context;
		bool				_topLevelMode;

	public:
		ObjectOStream(IObjectSerializator *collection, const FactoryContextPtr& context, bool topLevelMode = false): _collection(collection), _context(context), _topLevelMode(topLevelMode) {}
		virtual ~ObjectOStream() {}

		virtual ConstByteArray GetData() const = 0;

		template<typename T>
		ObjectOStream& Serialize(const T& value)
		{
			TypeWriter<T, ObjectOStream>::Serialize(*this, value);
			return *this;
		}

		template<typename T>
		ObjectOStream& Serialize(const optional<T>& value)
		{
			if (value)
				return Serialize(*value);
			return Serialize(null);
		}

		template<typename T>
		ObjectOStream& Serialize(const std::string& name, const T& value)
		{
			WritePropertyName(name);
			return Serialize(value);
		}

		template<typename T>
		ObjectOStream& Serialize(const std::string& name, const optional<T>& value)
		{
			if (value)
				return Serialize(name, *value);
			return *this;
		}

		IObjectOStreamPrivate *GetRawObjectStream() { return this; }

	private:
		//GENERIC COLLECTION SERIALIZATION
		STINGRAYKIT_DECLARE_METHOD_CHECK(Serialize);
		STINGRAYKIT_DECLARE_METHOD_CHECK(SerializeAsValue);
		STINGRAYKIT_DECLARE_METHOD_CHECK(begin);
		STINGRAYKIT_DECLARE_METHOD_CHECK(end);

		enum CollectionSerializationType
		{
			CollectionSerializeWithSerialize = 0,
			CollectionSerializeWithBeginEnd = 1,
			CollectionSerializeWithSerializeAsValue = 2
		};

		template <typename CollectionType, CollectionSerializationType ObjectType =
			!HasMethod_Serialize<CollectionType>::Value?
			(
				(HasMethod_begin<CollectionType>::Value && HasMethod_end<CollectionType>::Value)
					? CollectionSerializeWithBeginEnd :
					(
						HasMethod_SerializeAsValue<CollectionType>::Value? CollectionSerializeWithSerializeAsValue: CollectionSerializeWithSerialize
					)
			)
			: CollectionSerializeWithSerialize>
		struct CollectionSerializer;

		template< typename CollectionType>
		struct CollectionSerializer<CollectionType, CollectionSerializeWithBeginEnd>
		{
			static void Serialize(ObjectOStream& dst, const CollectionType &collection)
			{ dst.serialize(collection.begin(), collection.end()); }
		};

		template< typename CollectionType>
		struct CollectionSerializer<CollectionType, CollectionSerializeWithSerializeAsValue>
		{
			static void Serialize(ObjectOStream& dst, const CollectionType &collection)
			{ collection.SerializeAsValue(dst); }
		};

		template< typename CollectionType>
		struct CollectionSerializer<CollectionType, CollectionSerializeWithSerialize>
		{
			static void Serialize(ObjectOStream& dst, const CollectionType &collection)
			{
				dst.BeginObject();
				collection.Serialize(dst);
				dst.EndObject();
			}
		};

		template<typename T>
		ObjectOStream& serialize(const shared_ptr<T>& value, typename EnableIf<Inherits<T, NonPolymorphicMarker>::Value, int>::ValueT dummy = 0)
		{
			if (!value)
			{
				WriteNull();
				return *this;
			}

			BeginObject();
			bool top_level_mode = _topLevelMode;
			_topLevelMode = false;
			value->Serialize(*this);
			_topLevelMode = top_level_mode;
			EndObject();
			return *this;
		}


		template<typename T>
		ObjectOStream& serialize(const shared_ptr<T>& value, typename EnableIf<!Inherits<T, NonPolymorphicMarker>::Value, int>::ValueT dummy = 0)
		{
			if (!value)
			{
				WriteNull();
				return *this;
			}

			shared_ptr<IFactoryObject> factory_object = dynamic_caster(value);
			if (!factory_object)
				STINGRAYKIT_THROW(std::string("type ") + Demangle(typeid(*value).name()) + " cannot be serialized, no IFactoryObject interface found");

			std::string classname(_context->GetClassName(typeid(*value)));
			BeginObject();
			Serialize(".class", classname);
			bool ok = false;
			if (_collection && !_topLevelMode)
			{
				ISerializablePtr serializable = dynamic_caster(value);
				if (serializable)
				{
					Serialize(".pk", _collection->GetKey(classname, serializable));
					ok = true;
				}
			}
			if (!ok)
			{
				bool top_level_mode = _topLevelMode;
				_topLevelMode = false;
				value->Serialize(*this);
				_topLevelMode = top_level_mode;
			}
			EndObject();
			return *this;
		}

		template < typename Types_ >
		ObjectOStream& serialize(const variant<Types_>& v)
		{
			BeginObject();
			Serialize("index", v.which());
			apply_visitor(SerializationUtils::VariantSerializeVisitor<ObjectOStream>(*this), v);
			EndObject();
			return *this;
		}

		ObjectOStream& serialize(const std::vector<u8> & data)
		{
			Write(data);
			return *this;
		}

		ObjectOStream& serialize(const std::string & str)
		{
			Write(str);
			return *this;
		}

		ObjectOStream& serialize(bool value)
		{
			Write(value);
			return *this;
		}

		ObjectOStream& serialize(float value)
		{
			Write((double)value);
			return *this;
		}

		ObjectOStream& serialize(double value)
		{
			Write(value);
			return *this;
		}

		template<typename Iterator>
		inline void serialize(Iterator begin, const Iterator &end)
		{
			BeginList();
			for(; begin != end; ++begin)
				Serialize(implicit_cast<const typename Iterator::value_type&>(*begin));
			EndList();
		}

		template<typename K, typename V>
		ObjectOStream& serialize(const IReadonlyDictionary<K, V> & data)
		{
			typedef KeyValuePair<K, V>	Pair;
			std::vector<std::pair<K, V> > std_map;
			std_map.reserve(data.GetCount());
			FOR_EACH(Pair p IN data.GetEnumerator())
				std_map.push_back(std::make_pair(p.Key, p.Value));
			BeginObject();
			Serialize("data", std_map);
			EndObject();
			return *this;
		}

		template<typename T>
		ObjectOStream& serialize(const IList<T> & data)
		{
			std::vector<T> std_vec;
			std_vec.reserve(data.GetCount());
			FOR_EACH(T val IN data.GetEnumerator())
				std_vec.push_back(val);
			serialize(std_vec);
			return *this;
		}

		template<typename T>
		ObjectOStream& serialize(const IReadonlySet<T> & data)
		{
			std::vector<T> std_vec;
			std_vec.reserve(data.GetCount());
			FOR_EACH(T val IN data.GetEnumerator())
				std_vec.push_back(val);
			serialize(std_vec);
			return *this;
		}

		template<typename K, typename V>
		ObjectOStream& serialize(const std::pair<K, V> & pair)
		{
			BeginList();
			Serialize(pair.first);
			Serialize(pair.second);
			EndList();
			return *this;
		}

		template<typename T>
		typename EnableIf<!InheritsIList<T>::Value && !InheritsIReadonlySet<T>::Value && !InheritsIReadonlyDictionary<T>::Value, ObjectOStream&>::ValueT serialize(const T &obj)
		{
			CollectionSerializer<T>::Serialize(*this, obj);
			return *this;
		}

	};


	struct IObjectIStreamPrivate
	{
		template < typename, typename > friend struct TypeWriter;
		template < typename, typename, SerializationUtils::TypeTraits::Enum > friend struct SerializationUtils::TypeWriterImpl;

		virtual ~IObjectIStreamPrivate() { }

		typedef std::vector<u8> byte_array;

		virtual SettingsValue* GetRoot() = 0;
		virtual IObjectSerializator* GetCollection() = 0;
		virtual bool GetTopLevelMode() = 0;
	};

	class ObjectIStream: protected IObjectIStreamPrivate
	{
		template < typename, typename > friend struct TypeWriter;
		template < typename, typename, SerializationUtils::TypeTraits::Enum > friend struct SerializationUtils::TypeWriterImpl;

	protected:
		SettingsValue					*_root;
		IObjectSerializator				*_collection;
		FactoryContextPtr				_context;
		bool							_topLevelMode;

	private:
		SettingsValue* Get(const std::string &name);

	protected:
		virtual SettingsValue* GetRoot()				{ return _root; }
		virtual IObjectSerializator* GetCollection()	{ return _collection; }
		virtual bool GetTopLevelMode()					{ return _topLevelMode; }

		FactoryContextPtr GetContext() const			{ return _context; }

	public:
		ObjectIStream(SettingsValue* root, IObjectSerializator *collection, const FactoryContextPtr& context, bool topLevelMode = false):
			_root(root), _collection(collection), _context(context), _topLevelMode(topLevelMode) { }

		virtual ~ObjectIStream() {}

		inline SettingsValue* Root() const { return _root; }
		s64 GetInt();

		template<typename T>
		ObjectIStream& Deserialize(T& value)
		{
			try { TypeWriter<T, ObjectIStream>::Deserialize(*this, value); } catch(SettingsValueException &ex) { ex.Append("?"); throw; }
			return *this;
		}

		template<typename T>
		ObjectIStream& Deserialize(optional<T>& value)
		{
			if (is_null())
				value = null;
			else
			{
				T value_;
				Deserialize(value_);
				value = value_;
			}
			return *this;
		}

		template<typename T>
		ObjectIStream& Deserialize(const std::string& name, T& value)
		{
			SettingsValue *property = Get(name);
			if (!property)
				STINGRAYKIT_THROW(SettingsValueException(name + "(not found)"));

			try { ObjectIStream(property, _collection, _context).Deserialize(value); } catch(SettingsValueException &ex) { ex.Append(name); throw; }
			return *this;
		}

		template<typename T, typename U>
		ObjectIStream& Deserialize(const std::string& name, T& value, const U& defaultValue)
		{
			SettingsValue *property = Get(name);
			try
			{
				if (!property)
					value = defaultValue;
				else
				{
					try { ObjectIStream(property, _collection, _context).Deserialize(value); } catch(SettingsValueException &ex) { ex.Append(name); throw; }
				}
			}
			catch (const std::exception&)
			{ value = defaultValue; }
			return *this;
		}

		template<typename T>
		ObjectIStream& Deserialize(const std::string& name, reference<T> value)
		{ return Deserialize(name, implicit_cast<T&>(value)); }

		template<typename T>
		ObjectIStream& Deserialize(const std::string& name, optional<T>& value)
		{
			SettingsValue *property = Get(name);
			if (!property)
				value = null;
			else
			{
				try
				{
					T t;
					ObjectIStream(property, _collection, _context).Deserialize(t);
					value = t;
				}
				catch(SettingsValueException &ex)
				{ ex.Append(name); throw; }
			}
			return *this;
		}

	private:
		static void UnpackBinaryEncoding(byte_array &data, const std::string &str);

		template < typename Ar, typename T >
		static void call_Deserialize(Ar& ar, T& t)
		{ t.Deserialize(ar); }

		template < typename Ar, typename T >
		static void call_DeserializeAsValue(Ar& ar, T& t)
		{ t.DeserializeAsValue(ar); }

		//GENERIC COLLECTION DESERIALIZATION

		STINGRAYKIT_DECLARE_METHOD_CHECK(insert);
		STINGRAYKIT_DECLARE_METHOD_CHECK(push_back);
		STINGRAYKIT_DECLARE_METHOD_CHECK(Deserialize);
		STINGRAYKIT_DECLARE_METHOD_CHECK(DeserializeAsValue);

		template< typename CollectionType, int InsertType = HasMethod_Deserialize<CollectionType>::Value? 0:
			HasMethod_DeserializeAsValue<CollectionType>::Value? 1:
			((HasMethod_push_back<CollectionType>::Value? 2:
			(HasMethod_insert<CollectionType>::Value? 3: 0))) >
		struct CollectionDeserializer;

		template< typename CollectionType>
		struct CollectionDeserializer<CollectionType, 0>
		{
			static ObjectIStream& Deserialize(IObjectSerializator* owner, ObjectIStream& dst, CollectionType &collection)
			{ call_Deserialize(dst, collection); return dst; }
		};

		template< typename CollectionType>
		struct CollectionDeserializer<CollectionType, 1>
		{
			static ObjectIStream& Deserialize(IObjectSerializator* owner, ObjectIStream& dst, CollectionType &collection)
			{ call_DeserializeAsValue(dst, collection); return dst; }
		};

		template< typename CollectionType>
		struct CollectionDeserializer<CollectionType, 2 /*push_back*/>
		{
			static void DeserializeAndPushBack(const ObjectIStream& stream, IObjectSerializator *owner, CollectionType &collection, SettingsValue& src)
			{
				typename CollectionType::value_type value;
				ObjectIStream(&src, owner, stream.GetContext()).Deserialize(value);
				collection.push_back(value);
			}

			static ObjectIStream& Deserialize(IObjectSerializator *owner, ObjectIStream& dst, CollectionType &collection)
			{
				collection.clear();
				dst.for_each(bind(&DeserializeAndPushBack, dst, owner, ref(collection), _1));
				return dst;
			}
		};

		template<typename T>
		struct ValueTypeDeconstHelper
		{ typedef typename Deconst<T>::ValueT ValueT; };

		template<typename K, typename V>
		struct ValueTypeDeconstHelper<std::pair<K, V> >
		{ typedef std::pair<typename Deconst<K>::ValueT, typename Deconst<V>::ValueT> ValueT; };

		template< typename CollectionType>
		struct CollectionDeserializer<CollectionType, 3 /*insert*/>
		{
			static void DeserializeAndInsert(const ObjectIStream& stream, IObjectSerializator *owner, CollectionType &collection, SettingsValue& src)
			{
				typename ValueTypeDeconstHelper<typename CollectionType::value_type>::ValueT value;
				ObjectIStream(&src, owner, stream.GetContext()).Deserialize(value);
				collection.insert(value);
			}

			static ObjectIStream& Deserialize(IObjectSerializator *owner, ObjectIStream& dst, CollectionType &collection)
			{
				collection.clear();
				dst.for_each(bind(&DeserializeAndInsert, dst, owner, ref(collection), _1));
				return dst;
			}
		};

		void deserialize(std::string &str);
		void deserialize(bool &value);
		void deserialize(double &value);
		void deserialize(float &value) { double v = value; deserialize(v); value = v; }
		void deserialize(std::vector<u8> & data);

		void for_each(const function<void (SettingsValue&)>& func);
		void for_each_kv(const function<void (const std::string &, SettingsValue&)>& func); //string -> value maps

		bool is_null() const;
		bool is_array() const;

		template<typename T>
		void deserialize(shared_ptr<T>& value, typename EnableIf<Inherits<T, NonPolymorphicMarker>::Value, int>::ValueT dummy = 0)
		{
			value.reset();
			if (is_null())
				return;

			value.reset(new T);
			value->Deserialize(*this);
		}

		template<typename T>
		void deserialize(shared_ptr<T>& value, typename EnableIf<!Inherits<T, NonPolymorphicMarker>::Value, int>::ValueT dummy = 0)
		{
			value.reset();
			if (is_null())
				return;

			std::string classname;
			Deserialize(".class", classname);

			if (_collection && Get(".pk"))
			{
				int pk;
				Deserialize(".pk", pk);
				value = STINGRAYKIT_CHECKED_DYNAMIC_CASTER(_collection->GetObject(classname, pk));
			}
			else
			{	//default deserialize
				value.reset(_context->Create<T>(classname));
				value->Deserialize(*this);
			}
		}

		template < typename Types_ >
		void deserialize(variant<Types_>& v)
		{
			int index = 0;
			Deserialize("index", index);
			For<GetTypeListLength<Types_>::Value, SerializationUtils::VariantObjectCreator<Types_>::template CreatorFunc>::Do(index, ref(v));
			apply_visitor(SerializationUtils::VariantDeserializeVisitor<ObjectIStream>(*this), v);
		}

		template<typename K, typename V>
		struct PairDeserializer
		{
			typedef std::pair<K, V> value_type;

			unsigned			_index;
			value_type&			_value;
			FactoryContextPtr	_context;

			PairDeserializer(value_type &value, const FactoryContextPtr& context): _index(0), _value(value), _context(context) {}

			void Deserialize(IObjectSerializator *owner, SettingsValue &src)
			{
				int index = _index++;
				switch(index)
				{
				case 0: ObjectIStream(&src, owner, _context).Deserialize(_value.first); break;
				case 1: ObjectIStream(&src, owner, _context).Deserialize(_value.second); break;
				default: STINGRAYKIT_THROW(std::runtime_error("pair must contain two elements"));
				}
			}
		};

		template<typename K, typename V>
		ObjectIStream& deserialize(std::pair<K, V> & pair)
		{
			typedef PairDeserializer<K, V> helper_type;
			helper_type helper(pair, _context);
			for_each(bind(&helper_type::Deserialize, &helper, _collection, _1));
			return *this;
		}

		template<typename value_type>
		void deserialize_name_value(IObjectSerializator *owner, std::map<std::string, value_type> & collection, const std::string &name, SettingsValue& src)
		{
			typedef std::pair<std::string, value_type> collection_value_type;
			collection_value_type value;
			value.first = name;
			ObjectIStream(&src, owner, _context).Deserialize(value.second);
			collection.insert(value);
		}

		template<typename K, typename V>
		void deserialize_pair(IObjectSerializator *owner, std::map<K, V> & collection, SettingsValue& src)
		{
			typedef std::pair<typename Deconst<K>::ValueT, typename Deconst<V>::ValueT> value_type;
			value_type value;
			ObjectIStream(&src, owner, _context).Deserialize(value);
			collection.insert(value);
		}

		template<typename V>
		ObjectIStream& deserialize(std::map<std::string, V> & data)
		{
			//key-safe serialization used now
			if (is_array())
				return deserializeMap(data);

			//human readable "key": "value" maps
			data.clear();
			for_each_kv(bind(&ObjectIStream::deserialize_name_value<V>, this, _collection, ref(data), _1, _2));
			return *this;
		}

		template<typename K, typename V>
		ObjectIStream& deserializeMap(std::map<K, V> & data)
		{
			data.clear();
			for_each(bind(&ObjectIStream::deserialize_pair<K, V>, this, _collection, ref(data), _1));
			return *this;
		}

		template<typename K, typename V>
		ObjectIStream& deserialize(std::map<K, V> & data)
		{
			return deserializeMap(data);
		}

		template<typename K, typename V>
		ObjectIStream& deserialize(IDictionary<K, V> & data)
		{
			ObservableCollectionLockerPtr l;
			IObservableDictionary<K, V>* observable = dynamic_caster(&data);
			if (observable)
				l = observable->Lock();

			data.Clear();
			std::vector<std::pair<K, V> > std_map;
			Deserialize("data", std_map);
			for (typename std::vector<std::pair<K, V> >::const_iterator it = std_map.begin(); it != std_map.end(); ++it)
				data.Set(it->first, it->second);
			return *this;
		}

		template<typename T>
		ObjectIStream& deserialize(ISet<T> & data)
		{
			shared_ptr<MutexLock> l;
			IObservableSet<T>* observable = dynamic_caster(&data);
			if (observable)
				l = make_shared<MutexLock>(observable->GetSyncRoot());

			data.Clear();
			std::vector<T> std_vec;
			deserialize(std_vec);
			for (typename std::vector<T>::const_iterator it = std_vec.begin(); it != std_vec.end(); ++it)
				data.Add(*it);
			return *this;
		}

		template<typename K, typename V>
		ObjectIStream& deserialize(ITransactionalDictionary<K, V> & data)
		{
			std::vector<std::pair<K, V> > std_map;
			Deserialize("data", std_map);
			typename ITransactionalDictionary<K, V>::TransactionTypePtr trans = data.StartTransaction();
			trans->Clear();
			for (typename std::vector<std::pair<K, V> >::const_iterator it = std_map.begin(); it != std_map.end(); ++it)
				trans->Set(it->first, it->second);
			trans->Commit();
			return *this;
		}

		template<typename T>
		ObjectIStream& deserialize(IList<T> & data)
		{
			ObservableCollectionLockerPtr l;
			IObservableList<T>* observable = dynamic_caster(&data);
			if (observable)
				l = observable->Lock();

			data.Clear();
			std::vector<T> std_vec;
			deserialize(std_vec);
			for (typename std::vector<T>::const_iterator it = std_vec.begin(); it != std_vec.end(); ++it)
				data.Add(*it);
			return *this;
		}

		template<typename T>
		typename EnableIf<!InheritsIList<T>::Value && !InheritsIReadonlySet<T>::Value && !InheritsIReadonlyDictionary<T>::Value, ObjectIStream&>::ValueT deserialize(T& value)
		{ return CollectionDeserializer<T>::Deserialize(_collection, *this, value); }
	};

	/** @} */

}

#endif
