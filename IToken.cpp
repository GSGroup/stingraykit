#include <stingray/toolkit/IToken.h>

#include <stingray/toolkit/diagnostics/Backtrace.h>
#include <stingray/toolkit/log/Logger.h>


namespace stingray
{

	class TokenReturnProxy::Impl
	{
	private:
		static NamedLogger	s_logger;
		ITokenPtr			_token;
		bool				_tokenWasObtained;

	public:
		Impl(const ITokenPtr& token)
			: _token(token), _tokenWasObtained(false)
		{ }

		~Impl()
		{
			if (!_tokenWasObtained)
				s_logger.Error() << "Releasing the ITokenPtr object that was not obtained. This will unregister/disconnect whatever you tried to register or connect. " << Backtrace();
		}

		ITokenPtr GetToken()
		{
			_tokenWasObtained = true;
			return _token;
		}
	};
	STINGRAYKIT_DEFINE_NAMED_LOGGER(TokenReturnProxy::Impl);


	TokenReturnProxy::TokenReturnProxy(const ITokenPtr& token)
		: _impl(new Impl(token))
	{ }

	TokenReturnProxy::TokenReturnProxy(const TokenReturnProxy& other)
		: _impl(other._impl)
	{ }

	TokenReturnProxy::~TokenReturnProxy()
	{ }

	TokenReturnProxy::operator ITokenPtr() const
	{ return _impl->GetToken(); }

}
