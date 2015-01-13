#ifndef STINGRAYKIT_STRING_STRINGREADER_H
#define STINGRAYKIT_STRING_STRINGREADER_H


#include <sstream>
#include <string>


namespace stingray
{

	class StringReader
	{

		class StreamPeeker
		{
		private:
			std::istringstream&	_stream;

		public:
			explicit StreamPeeker(std::istringstream& stream)
				: _stream(stream)
			{ }

			~StreamPeeker()
			{ _stream.peek(); }
		};

	private:
		std::istringstream		_stream;

	public:
		StringReader(const std::string& text)
			: _stream(text)
		{ }

		std::string ReadLine()
		{
			StreamPeeker peeker(_stream);

			std::string result;

			for (char ch; _stream.get(ch); )
			{
				if (ch == '\n')
					return result;

				if (ch == '\r')
				{
					if (_stream.peek() == '\n')
						_stream.get();

					return result;
				}

				result.push_back(ch);
			}

			return result;
		}

		bool IsEndOfString() const { return _stream.eof(); }
	};

}


#endif
