// html.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/html.hpp>

namespace neogfx
{
    namespace
    {
        enum class token
        {
            OpenTag,
            CloseTag,
            Comment,
            Whitespace
        };

        enum class scope
        {
            Document,
            ElementTag,
            ElementContents
        };

        typedef neolib::lexer_token<token> lexer_token;
        typedef neolib::lexer_atom<token, scope> lexer_atom;
        typedef neolib::lexer_rule<lexer_atom> lexer_rule;
        const lexer_rule sLexerRules[] =
        {
            { token::OpenTag, {{ '<' }} },
            { token::CloseTag, {{ '>' }} },
            { lexer_rule::enter_scope(scope::ElementTag), {{ scope::Document, token::OpenTag }} },
            { lexer_rule::leave_scope(scope::ElementTag), {{ scope::Document, token::CloseTag }} },
        };
    }

    html::html(const std::string& aStyle) :
        iDocument{std::make_shared<std::istringstream>(aStyle)}
    {
        parse();
    }

    html::html(std::istream& aDocument) :
        iDocument{ std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{}, &aDocument } }
    {
        parse();
    }

    void html::parse()
    {
        static const neolib::lexer<lexer_atom> sLexer{ scope::Document, std::cbegin(sLexerRules), std::cend(sLexerRules) };
        neolib::lexer<lexer_atom>::context lexerContext = sLexer.use(*iDocument);
        if (!lexerContext)
            throw failed_to_open_html();
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
