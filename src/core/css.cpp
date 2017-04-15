// css.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>
#include <string>
#include <neolib/lexer.hpp>
#include <neogfx/core/css.hpp>

using namespace std::string_literals;

namespace neogfx
{
	css::selector::selector(type_e aType, const arguments_type& aArguments) :
		iType(aType), iArguments(aArguments)
	{
	}

	css::selector::type_e css::selector::type() const
	{
		return iType;
	}

	css::declaration::declaration()
	{
	}

	namespace
	{
		enum class token
		{
			Comment,
			Whitespace,
			Escape,
			Comma,
			Period,
			Colon,
			DoubleColon, 
			DoubleQuote,
			SingleQuote,
			Backslash,
			Multiply,
			Divide,
			Integer,
			Float,
			String,
			Symbol,
			PropertyColor,
			PropertyBackground,
		};

		typedef neolib::lexer_atom<token> lexer_atom;
		typedef neolib::lexer_rule<lexer_atom> lexer_rule;
		const lexer_rule sLexerRules[] =
		{
			{ token::Comma, {{ ',' }} },
			{ token::Period, {{ '.' }} },
			{ token::Divide, {{ '/' }} },
			{ token::Multiply, {{ '*' }} },
			{ token::Whitespace, {{ ' ' }} },
			{ token::Whitespace, {{ '\t' }} },
			{ token::Whitespace, {{ '\r' }} },
			{ token::Whitespace, {{ '\n' }} },
			{ token::Colon, {{ ':' }} },
			{ token::DoubleColon, {{ token::Colon, token::Colon }} },
			{ token::Backslash, {{ '\\' }} },
			{ neolib::token_eat(token::Escape), {{ token::Backslash, 't' }} },
			{ neolib::token_eat(token::Escape), {{ token::Backslash, 'r' }} },
			{ neolib::token_eat(token::Escape), {{ token::Backslash, 'n' }} },
			{ neolib::token_push(token::Comment), {{ token::Divide, token::Multiply }} },
			{ neolib::token_pop(token::Comment), {{ token::Multiply, token::Divide }} },
			{ token::Integer, {{ neolib::token_range('0', '9') }} },
			{ token::Integer, {{ token::Integer, token::Integer }} },
			{ token::Float, {{ token::Integer, token::Period, token::Integer }} },
			{ token::Float, {{ token::Float, token::Integer }} },
			{ token::Symbol, {{ neolib::token_range('A', 'Z') }} },
			{ token::Symbol, {{ neolib::token_range('a', 'z') }} },
			{ token::Symbol, {{ token::Symbol, token::Symbol }} },
			{ token::Symbol, {{ token::Symbol, token::Integer }} },
			{ token::DoubleQuote, {{ '"' }} },
			{ token::SingleQuote, {{ '\'' }} },
			{ neolib::token_eat(neolib::token_eat(token::String)), {{ token::DoubleQuote, token::DoubleQuote }} }, // empty string
			{ neolib::token_eat(neolib::token_push(token::String)), {{ token::DoubleQuote, neolib::token_not(token::DoubleQuote) }} },
			{ token::String, {{ token::String, neolib::token_not(token::DoubleQuote) }} },
			{ neolib::token_pop(token::String), {{ token::String, token::DoubleQuote }} },
			{ token::PropertyColor, {{ "color"s }} },
			{ token::PropertyBackground, {{ "background"s }} }
		};
	}

	css::css(const std::string& aStyleSheet)
	{
		std::istringstream iss(aStyleSheet);
		parse(iss);
	}

	css::css(std::istream& aStyleSheet)
	{
		parse(aStyleSheet);
	}

	void css::accept(i_visitor& aVisitor) const
	{
	}

	const css::rule_list& css::rules() const
	{
		return iRules;
	}

	std::string css::to_string() const
	{
		/* todo */
		return "";
	}

	void css::parse(std::istream& aStyleSheet)
	{
		static neolib::lexer<lexer_atom> sLexer{ aStyleSheet, std::cbegin(sLexerRules), std::cend(sLexerRules) };
	}
}
