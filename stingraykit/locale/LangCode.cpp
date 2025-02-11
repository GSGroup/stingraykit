// Copyright (c) 2011 - 2025, GS Group, https://github.com/GSGroup
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <stingraykit/locale/LangCode.h>

#include <stingraykit/string/ToString.h>

namespace stingray
{

	namespace
	{

		const array<char, 4> KnownCodes[] =
		{
				// NOTE: keep it sorted
				"AAR", "ABK", "ACE", "ACH", "ADA", "ADY", "AFA", "AFH", "AFR", "AIN", "AKA", "AKK", "ALB", "ALE", "ALG", "ALT", "AMH", "ANG", "ANP", "APA", "ARA", "ARC", "ARG", "ARM", "ARN", "ARP", "ART", "ARW", "ASM", "AST", "ATH", "AUS", "AVA", "AVE", "AWA", "AYM", "AZE",
				"BAD", "BAI", "BAK", "BAL", "BAM", "BAN", "BAQ", "BAS", "BAT", "BEJ", "BEL", "BEM", "BEN", "BER", "BHO", "BIH", "BIK", "BIN", "BIS", "BLA", "BNT", "BOD", "BOS", "BRA", "BRE", "BTK", "BUA", "BUG", "BUL", "BUR", "BYN",
				"CAD", "CAI", "CAR", "CAT", "CAU", "CEB", "CEL", "CES", "CHA", "CHB", "CHE", "CHG", "CHI", "CHK", "CHM", "CHN", "CHO", "CHP", "CHR", "CHU", "CHV", "CHY", "CMC", "COP", "COR", "COS", "CPE", "CPF", "CPP", "CRE", "CRH", "CRP", "CSB", "CUS", "CYM", "CZE",
				"DAK", "DAN", "DAR", "DAY", "DEL", "DEN", "DEU", "DGR", "DIN", "DIV", "DOI", "DRA", "DSB", "DUA", "DUM", "DUT", "DYU", "DZO",
				"EFI", "EGY", "EKA", "ELL", "ELX", "ENG", "ENM", "EPO", "EST", "EUS", "EWE", "EWO",
				"FAN", "FAO", "FAS", "FAT", "FIJ", "FIL", "FIN", "FIU", "FON", "FRA", "FRE", "FRM", "FRO", "FRR", "FRS", "FRY", "FUL", "FUR",
				"GAA", "GAY", "GBA", "GEM", "GEO", "GER", "GEZ", "GIL", "GLA", "GLE", "GLG", "GLV", "GMH", "GOH", "GON", "GOR", "GOT", "GRB", "GRC", "GRE", "GRN", "GSW", "GUJ", "GWI",
				"HAI", "HAT", "HAU", "HAW", "HEB", "HER", "HIL", "HIM", "HIN", "HIT", "HMN", "HMO", "HRV", "HSB", "HUN", "HUP", "HYE",
				"IBA", "IBO", "ICE", "IDO", "III", "IJO", "IKU", "ILE", "ILO", "INA", "INC", "IND", "INE", "INH", "IPK", "IRA", "IRO", "ISL", "ITA",
				"JAV", "JBO", "JPN", "JPR", "JRB",
				"KAA", "KAB", "KAC", "KAL", "KAM", "KAN", "KAR", "KAS", "KAT", "KAU", "KAW", "KAZ", "KBD", "KHA", "KHI", "KHM", "KHO", "KIK", "KIN", "KIR", "KMB", "KOK", "KOM", "KON", "KOR", "KOS", "KPE", "KRC", "KRL", "KRO", "KRU", "KUA", "KUM", "KUR", "KUT",
				"LAD", "LAH", "LAM", "LAO", "LAT", "LAV", "LEZ", "LIM", "LIN", "LIT", "LOL", "LOZ", "LTZ", "LUA", "LUB", "LUG", "LUI", "LUN", "LUO", "LUS",
				"MAC", "MAD", "MAG", "MAH", "MAI", "MAK", "MAL", "MAN", "MAO", "MAP", "MAR", "MAS", "MAY", "MDF", "MDR", "MEN", "MGA", "MIC", "MIN", "MIS", "MKD", "MKH", "MLG", "MLT", "MNC", "MNI", "MNO", "MOH", "MON", "MOS", "MRI", "MSA", "MUL", "MUN", "MUS", "MWL", "MWR", "MYA", "MYN", "MYV",
				"NAH", "NAI", "NAP", "NAU", "NAV", "NBL", "NDE", "NDL", "NDO", "NDS", "NEP", "NEW", "NIA", "NIC", "NIU", "NLD", "NNO", "NOB", "NOG", "NON", "NOR", "NQO", "NSO", "NUB", "NWC", "NYA", "NYM", "NYN", "NYO", "NZI",
				"OCI", "OJI", "ORI", "ORM", "OSA", "OSS", "OTA", "OTO",
				"PAA", "PAG", "PAL", "PAM", "PAN", "PAP", "PAU", "PEO", "PER", "PHI", "PHN", "PLI", "POL", "PON", "POR", "PRA", "PRO", "PUS",
				"QUE",
				"RAJ", "RAP", "RAR", "ROA", "ROH", "ROM", "RON", "RUM", "RUN", "RUP", "RUS",
				"SAD", "SAG", "SAH", "SAI", "SAL", "SAM", "SAN", "SAS", "SAT", "SCC", "SCN", "SCO", "SCR", "SEL", "SEM", "SGA", "SGN", "SHN", "SID", "SIN", "SIO", "SIT", "SLA", "SLK", "SLO", "SLV", "SMA", "SME", "SMI", "SMJ", "SMN", "SMO", "SMS", "SNA", "SND", "SNK", "SOG", "SOM", "SON", "SOT", "SPA", "SQI", "SRD", "SRN", "SRP", "SRR", "SSA", "SSW", "SUK", "SUN", "SUS", "SUX", "SVE", "SWA", "SWE", "SYC", "SYR",
				"TAH", "TAI", "TAM", "TAT", "TEL", "TEM", "TER", "TET", "TGK", "TGL", "THA", "TIB", "TIG", "TIR", "TIV", "TKL", "TLH", "TLI", "TMH", "TOG", "TON", "TPI", "TSI", "TSN", "TSO", "TUK", "TUM", "TUP", "TUR", "TUT", "TVL", "TWI", "TYV",
				"UDM", "UGA", "UIG", "UKR", "UMB", "UND", "URD", "UZB",
				"VAI", "VEN", "VIE", "VOL", "VOT",
				"WAK", "WAL", "WAR", "WAS", "WEL", "WEN", "WLN", "WOL",
				"XAL", "XHO",
				"YAO", "YAP", "YID", "YOR", "YPK",
				"ZAP", "ZBL", "ZEN", "ZGH", "ZHA", "ZHO", "ZND", "ZUL", "ZUN", "ZXX", "ZZA"
		};

		char DoToUpper(char c)
		{ return c >= 'a' && c <= 'z'? c - 'a' + 'A' : c; }

		char DoToLower(char c)
		{ return c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c; }

		u32 ToUpper(u32 code)
		{
			if (code == 0)
				return code;

			array<char, 3> c;
			c[0] = (code >> 16) & 0xff;
			c[1] = (code >> 8) & 0xff;
			c[2] = (code >> 0) & 0xff;

			array<char, 3> r;
			std::transform(c.begin(), c.end(), r.begin(), &DoToUpper);

			return (u32)(r[0] << 16) + (r[1] << 8) + r[2];
		}

		array<char, 4> ToArray(u32 code)
		{ return array<char, 4>({ (char)((code >> 16) & 0xff), (char)((code >> 8) & 0xff), (char)((code >> 0) & 0xff), 0 }); }

	}


	const LangCode::AnyType LangCode::Any = { };

	LangCode LangCode::Eng() { return LangCode("ENG"); }
	LangCode LangCode::Rus() { return LangCode("RUS"); }


	LangCode::LangCode(u32 code) : _code(0)
	{
		STINGRAYKIT_CHECK(!(code & 0xff000000u), StringBuilder() % "Invalid language code: " % code);
		_code = ToUpper(code);
	}


	LangCode::LangCode(string_view code)
	{
		STINGRAYKIT_CHECK(code.size() == 3, StringBuilder() % "Invalid language code: " % code);

		const char a = DoToUpper(code[0]);
		const char b = DoToUpper(code[1]);
		const char c = DoToUpper(code[2]);

		//STINGRAYKIT_CHECK(a >= 'A' && a <= 'Z' && b >= 'A' && b <= 'Z' && c >= 'A' && c <= 'Z', StringBuilder() % "Invalid language code: " % code);

		_code = (((u8)a) << 16) | (((u8)b) << 8) | (((u8)c) << 0);
	}


	bool LangCode::IsKnown() const
	{ return std::binary_search(std::begin(KnownCodes), std::end(KnownCodes), ToArray(_code)); }


	std::string LangCode::ToString() const
	{
		if (_code == 0)
			return std::string();

		return std::string(ToArray(_code).data());
	}


	std::string LangCode::To2LetterString() const
	{
		const char r[3] = { DoToLower((char)((_code >> 16) & 0xff)), DoToLower((char)((_code >> 8) & 0xff)), 0 };
		return r;
	}


	LangCode LangCode::From2Letter(string_view code)
	{
		STINGRAYKIT_CHECK(code.size() == 2, StringBuilder() % "Invalid language code: " % code);

		std::string subcode = code.substr(0, 2).copy();
		std::transform(subcode.begin(), subcode.end(), subcode.begin(), &DoToUpper);

		if (subcode == "RU")
			return LangCode("RUS");

		if (subcode == "EN")
			return LangCode("ENG");

		return LangCode();
	}


	LangCode LangCode::From3Letter(string_view code)
	{ return LangCode(code); }

}
