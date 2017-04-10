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
#include <neolib/vecarray.hpp>
#include <neogfx/core/css.hpp>

using namespace std::string_literals;

#if 0

namespace neogfx
{
	css::selector::selector(type_e aType, const arguments_type& aArguments = arguments_type()) :
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

	css::css(const std::string& aStyleSheet) : 
		css(std::istringstream(aStyleSheet))
	{
	}

	namespace
	{
		enum class token
		{
			Comment,
			Whitespace,
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
		typedef neolib::lexer_rule<token> lexer_rule;
		lexer_rule sLexerRules[] =
		{
			{{ ',', token::Comma }},
			{{ '.', token::Period }},
			{{ '/', token::Divide }},
			{{ '*', token::Multiply }},
			{{ ' ', token::Whitespace }}, {{ '\t', token::Whitespace }}, {{ '\r', token::Whitespace }}, {{ '\n', token::Whitespace }},
			{{ ':', token::Colon }},
			{{ token::Colon, token::Colon, token::DoubleColon }},
			{{ '\\', token::Backslash }},
			{{ token::Divide, token::Multiply, neolib::token_push(token::Comment) }},
			{{ token::Comment, token::Multiply, token::Divide, neolib::token_pop(token::Comment) }},
			{{ neolib::token_range<'0', '9'>, token::Integer }},
			{{ token::Integer, token::Integer, token::Integer }},
			{{ token::Integer, token::Period, token::Integer, token::Float }},
			{{ token::Float, token::Integer, token::Float }},
			{{ neolib::token_range<'A', 'Z'>, token::Symbol }},
			{{ neolib::token_range<'a', 'z'>, token::Symbol }},
			{{ token::Symbol, token::Symbol, token::Symbol }},
			{{ token::Symbol, token::Integer, token::Symbol }},
			{{ '"', token::DoubleQuote }},
			{{ '\'', token::SingleQuote }},
			{{ token::DoubleQuote, token::DoubleQuote, neolib::token_eat_left(neolib::token_eat_right(token::String)) }}, // empty string
			{{ token::DoubleQuote, neolib::token_not(token::DoubleQuote), neolib::token_eat_left(neolib::token_push(token::String)) }},
			{{ token::String, token::Backslash, neolib::token_any(), neolib::token_eat_middle(token::String) }},
			{{ token::String, token::token_not(token::DoubleQuote), token::String }},
			{{ token::String, token::DoubleQuote, neolib::token_pop(token::String) }},
			{{ "color"s, token::PropertyColor }},
			{{ "background"s, token::PropertyBackground }}
		};
	}

	css::css(const std::istream& aStyleSheet)
	{
		neolib::lexer<token> lexer{ aStyleSheet };
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
	}
}

#endif