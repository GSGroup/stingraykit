#ifndef STINGRAYKIT_COMPRESSION_ICOMPRESSIONDICTIONARY_H
#define STINGRAYKIT_COMPRESSION_ICOMPRESSIONDICTIONARY_H


#include <stingraykit/compression/ICompressionEngine.h>

namespace stingray
{

	struct ICompressionDictionary : public virtual ICompressionEngine
	{ };
	STINGRAYKIT_DECLARE_PTR(ICompressionDictionary);


	struct ICompressionDictionaryBuilder
	{
		virtual ~ICompressionDictionaryBuilder() { }

		virtual void Process(ConstByteData data) = 0;
		virtual ICompressionDictionaryPtr Finish() = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ICompressionDictionaryBuilder);


	struct ICompressionDictionaryFactory
	{
		virtual ~ICompressionDictionaryFactory() { }

		virtual ICompressionDictionaryBuilderPtr CreateBuilder() const = 0;
	};
	STINGRAYKIT_DECLARE_PTR(ICompressionDictionaryFactory);

}

#endif
