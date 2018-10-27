// css.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
			Semicolon,
			Colon,
			OpenBrace,
			CloseBrace,
			DoubleColon,
			DoubleQuote,
			SingleQuote,
			Hash,
			Backslash,
			Plus,
			Minus,
			Multiply,
			Divide,
			Integer,
			Float,
			String,
			Symbol,
			Hex,
			Px,
			Pt,
			Id,
			PropertyColor,
			PropertyBackground,
			PropertyBackgroundAttachment,
			PropertyBackgroundClip,
			PropertyBackgroundColor,
			PropertyBackgroundImage,
			PropertyBackgroundOrigin,
			PropertyBackgroundPosition,
			PropertyBackgroundRepeat,
			PropertyBackgroundSize,
			PropertyBorder
		};

		enum class scope
		{
			ScopeSelector,
			ScopeDeclarationBlock
		};

		typedef neolib::lexer_token<token> lexer_token;
		typedef neolib::lexer_atom<token, scope> lexer_atom;
		typedef neolib::lexer_rule<lexer_atom> lexer_rule;
		const lexer_rule sLexerRules[] =
		{
			{ token::Comma, {{ ',' }} },
			{ token::Period, {{ '.' }} },
			{ token::OpenBrace, {{ '{' }} },
			{ token::CloseBrace, {{ '}' }} },
			{ lexer_rule::enter_scope(scope::ScopeDeclarationBlock), {{ scope::ScopeSelector, token::OpenBrace }} },
			{ lexer_rule::leave_scope(scope::ScopeDeclarationBlock), {{ scope::ScopeSelector, token::CloseBrace }} },
			{ token::Plus, {{ '+' }} },
			{ token::Minus, {{ '-' }} },
			{ token::Divide, {{ '/' }} },
			{ token::Divide, {{ token::Divide }} },
			{ token::Multiply, {{ '*' }} },
			{ token::Multiply, {{ token::Multiply }} },
			{ token::Whitespace, {{ ' ' }} },
			{ token::Whitespace, {{ '\t' }} },
			{ token::Whitespace, {{ '\r' }} },
			{ token::Whitespace, {{ '\n' }} },
			{ token::Whitespace, {{ token::Whitespace }} },
			{ token::Whitespace, {{ token::Whitespace, token::Whitespace }} },
			{ token::Semicolon, {{ ';' }} },
			{ token::Colon, {{ ':' }} },
			{ token::Colon, {{ token::Colon }} },
			{ token::DoubleColon, {{ token::Colon, token::Colon }} },
			{ token::Backslash, {{ '\\' }} },
			{ token::Hash, {{ '#' }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hash, lexer_rule::token_range('0', '9') }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hash, lexer_rule::token_range('A', 'F') }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hash, lexer_rule::token_range('a', 'f') }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hex, lexer_rule::token_range('0', '9') }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hex, lexer_rule::token_range('A', 'F') }} },
			{ token::Hex, {{ scope::ScopeDeclarationBlock, token::Hex, lexer_rule::token_range('a', 'f') }} },
			{ token::Id, {{ token::Hash, token::Symbol }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_make(token::Escape, '\t'))), {{ token::Backslash, 't' }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_make(token::Escape, '\r'))), {{ token::Backslash, 'r' }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_make(token::Escape, '\n'))), {{ token::Backslash, 'n' }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_make(token::Escape, '\''))), {{ token::Backslash, '\'' }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_make(token::Escape, '\"'))), {{ token::Backslash, '\"' }} },
			{ token::Comment, {{ token::Comment }} },
			{ token::Comment, {{ token::Divide, token::Multiply }} },
			{ token::Comment, {{ token::Comment, token::Comment }} },
			{ token::Comment, {{ token::Comment, lexer_rule::token_not(token::Multiply) }} },
			{ token::Comment, {{ token::Comment, token::Multiply, lexer_rule::token_not(token::Divide) }} },
			{ lexer_rule::token_end(token::Comment), {{ token::Comment, token::Multiply, token::Divide }} },
			{ token::Integer, {{ lexer_rule::token_range('0', '9') }} },
			{ token::Integer, {{ token::Integer }} },
			{ token::Integer, {{ token::Integer, token::Integer }} },
			{ token::Integer, {{ token::Minus, token::Integer }} },
			{ token::Float, {{ token::Integer, token::Period, token::Integer }} },
			{ token::Float, {{ token::Float, token::Integer }} },
			{ token::Float, {{ token::Float }} },
			{ token::Float, {{ token::Minus, token::Float }} },
			{ token::Symbol, {{ lexer_rule::token_range('A', 'Z') }} },
			{ token::Symbol, {{ lexer_rule::token_range('a', 'z') }} },
			{ token::Symbol, {{ '_'}} },
			{ token::Symbol, {{ token::Symbol }} },
			{ token::Symbol, {{ token::Symbol, token::Symbol }} },
			{ token::Symbol, {{ token::Symbol, token::Integer }} },
			{ token::Symbol, {{ token::Symbol, token::Minus }} },
			{ token::DoubleQuote, {{ '"' }} },
			{ token::SingleQuote, {{ '\'' }} },
			{ lexer_rule::token_eat(lexer_rule::token_eat(lexer_rule::token_end(token::String))), {{ token::DoubleQuote, token::DoubleQuote }} }, // empty string
			{ lexer_rule::token_eat(token::String), {{ token::DoubleQuote }} },
			{ token::String, {{ token::String, token::Escape }} },
			{ token::String, {{ token::String, ' ' }} },
			{ token::String, {{ token::String, '\t' }} },
			{ token::String, {{ token::String, lexer_rule::token_not(token::Whitespace) }} },
			{ lexer_rule::token_end(lexer_rule::token_eat(lexer_rule::token_keep(token::String))), {{ token::String, token::DoubleQuote }} },
			{ token::Px, {{ "px"s }} },
			{ token::Pt, {{ "pt"s }} },
			{ token::PropertyColor, {{ "color"s }} },
			{ token::PropertyBackground, {{ "background"s }} },
			{ token::PropertyBackgroundAttachment, {{ "background-attachment"s }} },
			{ token::PropertyBackgroundClip, {{ "background-clip"s }} },
			{ token::PropertyBackgroundColor, {{ "background-color"s }} },
			{ token::PropertyBackgroundImage, {{ "background-image"s }} },
			{ token::PropertyBackgroundOrigin, {{ "background-origin"s }} },
			{ token::PropertyBackgroundPosition, {{ "background-position"s }} },
			{ token::PropertyBackgroundRepeat, {{ "background-repeat"s }} },
			{ token::PropertyBackgroundSize, {{ "background-size"s }} },
			{ token::PropertyBorder, {{ "border"s }} }
		};
	}

	css::css(const std::string& aStyle) : 
		iStyleSheet{std::make_shared<std::istringstream>(aStyle)}
	{
		parse();
	}

	css::css(std::istream& aStyleSheet) :
		iStyleSheet{ std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{}, &aStyleSheet } }
	{
		parse();
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

	void css::parse()
	{
		static const neolib::lexer<lexer_atom> sLexer{ scope::ScopeSelector, std::cbegin(sLexerRules), std::cend(sLexerRules) };
		neolib::lexer<lexer_atom>::context lexerContext = sLexer.use(*iStyleSheet);
		if (!lexerContext)
			throw failed_to_open_style_sheet();
		std::vector<lexer_token> tokens;
		lexer_token token;
		while (lexerContext >> token)
			tokens.push_back(token);
		tokens.erase(
			std::remove_if(tokens.begin(), tokens.end(), 
				[](const lexer_token& aToken) 
				{ 
					return aToken.first == token::Whitespace || aToken.first == token::Comment; 
				}), 
			tokens.end());
	}
}
