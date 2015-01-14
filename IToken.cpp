#include <stingray/toolkit/IToken.h>

#include <stingray/toolkit/diagnostics/Backtrace.h>
#include <stingray/toolkit/log/Logger.h>


namespace stingray
{

	class TokenReturnProxy::Impl
	{
	private:
		static NamedLogger	s_logger;
		Token				_token;
		bool				_tokenWasObtained;

	public:
		Impl(const Token& token) : _token(token), _tokenWasObtained(false)
		{ }

		~Impl()
		{
			if (!_tokenWasObtained)
				s_logger.Error() << "Releasing the Token object that was not obtained. This will unregister/disconnect whatever you tried to register or connect. " << Backtrace();
		}

		Token GetToken()
		{
			_tokenWasObtained = true;
			return _token;
		}
	};
	STINGRAYKIT_DEFINE_NAMED_LOGGER(TokenReturnProxy::Impl);


	TokenReturnProxy::TokenReturnProxy(const Token& token) : _impl(new Impl(token))
	{ }

	TokenReturnProxy::TokenReturnProxy(const TokenReturnProxy& other) : _impl(other._impl)
	{ }

	TokenReturnProxy::~TokenReturnProxy()
	{ }

	TokenReturnProxy::operator Token() const
	{ return _impl->GetToken(); }

}
