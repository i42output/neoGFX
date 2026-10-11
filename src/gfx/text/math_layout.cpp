// math_layout.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ot.h>

#include "math_layout.hpp"

namespace neogfx::math_layout
{
    namespace
    {
        // ---------------------------------------------------------------------------------
        // Atom classes, styles and symbols
        // ---------------------------------------------------------------------------------

        enum class atom_class : std::uint8_t
        {
            Ord,
            Op,
            Bin,
            Rel,
            Open,
            Close,
            Punct,
            Inner
        };

        enum class font_variant : std::uint8_t
        {
            Default,
            Upright,        // \mathrm
            Italic,         // \mathit
            Bold,           // \mathbf
            BoldItalic,     // \boldsymbol
            DoubleStruck,   // \mathbb
            Script,         // \mathcal, \mathscr
            Fraktur,        // \mathfrak
            SansSerif,      // \mathsf
            Monospace       // \mathtt
        };

        struct symbol
        {
            char32_t cp;
            atom_class cls;
            bool largeOp = false;
            bool displayLimits = false;
        };

        std::unordered_map<std::string_view, symbol> const& symbols()
        {
            using enum atom_class;
            static std::unordered_map<std::string_view, symbol> const sSymbols = {
                // ordinary
                { "infty", { 0x221E, Ord } }, { "partial", { 0x2202, Ord } }, { "nabla", { 0x2207, Ord } },
                { "forall", { 0x2200, Ord } }, { "exists", { 0x2203, Ord } }, { "nexists", { 0x2204, Ord } },
                { "emptyset", { 0x2205, Ord } }, { "varnothing", { 0x2205, Ord } }, { "neg", { 0x00AC, Ord } },
                { "lnot", { 0x00AC, Ord } }, { "top", { 0x22A4, Ord } }, { "bot", { 0x22A5, Ord } },
                { "angle", { 0x2220, Ord } }, { "measuredangle", { 0x2221, Ord } }, { "triangle", { 0x25B3, Ord } },
                { "hbar", { 0x210F, Ord } }, { "hslash", { 0x210F, Ord } }, { "ell", { 0x2113, Ord } },
                { "wp", { 0x2118, Ord } }, { "Re", { 0x211C, Ord } }, { "Im", { 0x2111, Ord } },
                { "aleph", { 0x2135, Ord } }, { "beth", { 0x2136, Ord } }, { "gimel", { 0x2137, Ord } },
                { "prime", { 0x2032, Ord } }, { "backprime", { 0x2035, Ord } }, { "backslash", { 0x005C, Ord } },
                { "surd", { 0x221A, Ord } }, { "flat", { 0x266D, Ord } }, { "natural", { 0x266E, Ord } },
                { "sharp", { 0x266F, Ord } }, { "clubsuit", { 0x2663, Ord } }, { "diamondsuit", { 0x2662, Ord } },
                { "heartsuit", { 0x2661, Ord } }, { "spadesuit", { 0x2660, Ord } }, { "vdots", { 0x22EE, Ord } },
                { "degree", { 0x00B0, Ord } }, { "checkmark", { 0x2713, Ord } }, { "mho", { 0x2127, Ord } },
                { "complement", { 0x2201, Ord } }, { "imath", { 0x0131, Ord } }, { "jmath", { 0x0237, Ord } },
                { "eth", { 0x00F0, Ord } }, { "square", { 0x25A1, Ord } }, { "Box", { 0x25A1, Ord } },
                { "blacksquare", { 0x25A0, Ord } }, { "lozenge", { 0x25CA, Ord } }, { "Diamond", { 0x25C7, Ord } },
                { "vert", { 0x007C, Ord } }, { "|", { 0x2016, Ord } }, { "Vert", { 0x2016, Ord } },
                { "%", { U'%', Ord } }, { "#", { U'#', Ord } }, { "$", { U'$', Ord } }, { "&", { U'&', Ord } },
                { "_", { U'_', Ord } }, { "S", { 0x00A7, Ord } }, { "P", { 0x00B6, Ord } }, { "pounds", { 0x00A3, Ord } },
                { "copyright", { 0x00A9, Ord } }, { "dag", { 0x2020, Ord } }, { "ddag", { 0x2021, Ord } },
                // inner
                { "cdots", { 0x22EF, Inner } }, { "ldots", { 0x2026, Inner } }, { "dots", { 0x2026, Inner } },
                { "dotsc", { 0x2026, Inner } }, { "dotsb", { 0x22EF, Inner } }, { "dotsm", { 0x22EF, Inner } },
                { "ddots", { 0x22F1, Inner } },
                // binary operators
                { "pm", { 0x00B1, Bin } }, { "mp", { 0x2213, Bin } }, { "times", { 0x00D7, Bin } },
                { "div", { 0x00F7, Bin } }, { "cdot", { 0x22C5, Bin } }, { "ast", { 0x2217, Bin } },
                { "star", { 0x22C6, Bin } }, { "circ", { 0x2218, Bin } }, { "bullet", { 0x2219, Bin } },
                { "cap", { 0x2229, Bin } }, { "cup", { 0x222A, Bin } }, { "wedge", { 0x2227, Bin } },
                { "land", { 0x2227, Bin } }, { "vee", { 0x2228, Bin } }, { "lor", { 0x2228, Bin } },
                { "oplus", { 0x2295, Bin } }, { "ominus", { 0x2296, Bin } }, { "otimes", { 0x2297, Bin } },
                { "oslash", { 0x2298, Bin } }, { "odot", { 0x2299, Bin } }, { "setminus", { 0x2216, Bin } },
                { "smallsetminus", { 0x2216, Bin } }, { "sqcap", { 0x2293, Bin } }, { "sqcup", { 0x2294, Bin } },
                { "uplus", { 0x228E, Bin } }, { "amalg", { 0x2A3F, Bin } }, { "dagger", { 0x2020, Bin } },
                { "ddagger", { 0x2021, Bin } }, { "wr", { 0x2240, Bin } }, { "diamond", { 0x22C4, Bin } },
                { "bigtriangleup", { 0x25B3, Bin } }, { "bigtriangledown", { 0x25BD, Bin } },
                { "triangleleft", { 0x25C3, Bin } }, { "triangleright", { 0x25B9, Bin } }, { "lhd", { 0x22B2, Bin } },
                { "rhd", { 0x22B3, Bin } }, { "unlhd", { 0x22B4, Bin } }, { "unrhd", { 0x22B5, Bin } },
                { "bigcirc", { 0x25EF, Bin } }, { "centerdot", { 0x22C5, Bin } }, { "ltimes", { 0x22C9, Bin } },
                { "rtimes", { 0x22CA, Bin } }, { "dotplus", { 0x2214, Bin } }, { "intercal", { 0x22BA, Bin } },
                { "boxplus", { 0x229E, Bin } }, { "boxtimes", { 0x22A0, Bin } }, { "circledast", { 0x229B, Bin } },
                // relations
                { "leq", { 0x2264, Rel } }, { "le", { 0x2264, Rel } }, { "geq", { 0x2265, Rel } }, { "ge", { 0x2265, Rel } },
                { "neq", { 0x2260, Rel } }, { "ne", { 0x2260, Rel } }, { "equiv", { 0x2261, Rel } },
                { "approx", { 0x2248, Rel } }, { "sim", { 0x223C, Rel } }, { "simeq", { 0x2243, Rel } },
                { "cong", { 0x2245, Rel } }, { "propto", { 0x221D, Rel } }, { "subset", { 0x2282, Rel } },
                { "supset", { 0x2283, Rel } }, { "subseteq", { 0x2286, Rel } }, { "supseteq", { 0x2287, Rel } },
                { "subsetneq", { 0x228A, Rel } }, { "supsetneq", { 0x228B, Rel } }, { "in", { 0x2208, Rel } },
                { "notin", { 0x2209, Rel } }, { "ni", { 0x220B, Rel } }, { "owns", { 0x220B, Rel } },
                { "perp", { 0x22A5, Rel } }, { "parallel", { 0x2225, Rel } }, { "nparallel", { 0x2226, Rel } },
                { "mid", { 0x2223, Rel } }, { "nmid", { 0x2224, Rel } }, { "ll", { 0x226A, Rel } },
                { "gg", { 0x226B, Rel } }, { "lll", { 0x22D8, Rel } }, { "ggg", { 0x22D9, Rel } },
                { "prec", { 0x227A, Rel } }, { "succ", { 0x227B, Rel } }, { "preceq", { 0x2AAF, Rel } },
                { "succeq", { 0x2AB0, Rel } }, { "models", { 0x22A7, Rel } }, { "vdash", { 0x22A2, Rel } },
                { "dashv", { 0x22A3, Rel } }, { "vDash", { 0x22A8, Rel } }, { "asymp", { 0x224D, Rel } },
                { "doteq", { 0x2250, Rel } }, { "bowtie", { 0x22C8, Rel } }, { "smile", { 0x2323, Rel } },
                { "frown", { 0x2322, Rel } }, { "to", { 0x2192, Rel } }, { "rightarrow", { 0x2192, Rel } },
                { "leftarrow", { 0x2190, Rel } }, { "gets", { 0x2190, Rel } }, { "Rightarrow", { 0x21D2, Rel } },
                { "Leftarrow", { 0x21D0, Rel } }, { "leftrightarrow", { 0x2194, Rel } }, { "Leftrightarrow", { 0x21D4, Rel } },
                { "iff", { 0x27FA, Rel } }, { "implies", { 0x27F9, Rel } }, { "impliedby", { 0x27F8, Rel } },
                { "mapsto", { 0x21A6, Rel } }, { "longmapsto", { 0x27FC, Rel } }, { "longrightarrow", { 0x27F6, Rel } },
                { "longleftarrow", { 0x27F5, Rel } }, { "longleftrightarrow", { 0x27F7, Rel } },
                { "Longrightarrow", { 0x27F9, Rel } }, { "Longleftarrow", { 0x27F8, Rel } },
                { "Longleftrightarrow", { 0x27FA, Rel } }, { "uparrow", { 0x2191, Rel } }, { "downarrow", { 0x2193, Rel } },
                { "updownarrow", { 0x2195, Rel } }, { "Uparrow", { 0x21D1, Rel } }, { "Downarrow", { 0x21D3, Rel } },
                { "Updownarrow", { 0x21D5, Rel } }, { "nearrow", { 0x2197, Rel } }, { "searrow", { 0x2198, Rel } },
                { "swarrow", { 0x2199, Rel } }, { "nwarrow", { 0x2196, Rel } }, { "hookrightarrow", { 0x21AA, Rel } },
                { "hookleftarrow", { 0x21A9, Rel } }, { "rightharpoonup", { 0x21C0, Rel } },
                { "rightharpoondown", { 0x21C1, Rel } }, { "leftharpoonup", { 0x21BC, Rel } },
                { "leftharpoondown", { 0x21BD, Rel } }, { "rightleftharpoons", { 0x21CC, Rel } },
                { "leftrightharpoons", { 0x21CB, Rel } }, { "nleq", { 0x2270, Rel } }, { "ngeq", { 0x2271, Rel } },
                { "nless", { 0x226E, Rel } }, { "ngtr", { 0x226F, Rel } }, { "nsubseteq", { 0x2288, Rel } },
                { "nsupseteq", { 0x2289, Rel } }, { "lesssim", { 0x2272, Rel } }, { "gtrsim", { 0x2273, Rel } },
                { "sqsubseteq", { 0x2291, Rel } }, { "sqsupseteq", { 0x2292, Rel } }, { "sqsubset", { 0x228F, Rel } },
                { "sqsupset", { 0x2290, Rel } }, { "triangleq", { 0x225C, Rel } }, { "coloneqq", { 0x2254, Rel } },
                { "eqqcolon", { 0x2255, Rel } }, { "approxeq", { 0x224A, Rel } }, { "leqslant", { 0x2A7D, Rel } },
                { "geqslant", { 0x2A7E, Rel } }, { "ncong", { 0x2247, Rel } }, { "nsim", { 0x2241, Rel } },
                { "because", { 0x2235, Rel } }, { "therefore", { 0x2234, Rel } }, { "lessgtr", { 0x2276, Rel } },
                { "leadsto", { 0x21DD, Rel } }, { "Vdash", { 0x22A9, Rel } }, { "nvdash", { 0x22AC, Rel } },
                { "nRightarrow", { 0x21CF, Rel } }, { "nrightarrow", { 0x219B, Rel } }, { "nleftarrow", { 0x219A, Rel } },
                { "leftrightarrows", { 0x21C6, Rel } }, { "rightleftarrows", { 0x21C4, Rel } },
                { "longleftrightarrows", { 0x21C6, Rel } }, { "colon", { U':', Punct } },
                // large operators
                { "sum", { 0x2211, Op, true, true } }, { "prod", { 0x220F, Op, true, true } },
                { "coprod", { 0x2210, Op, true, true } }, { "int", { 0x222B, Op, true, false } },
                { "iint", { 0x222C, Op, true, false } }, { "iiint", { 0x222D, Op, true, false } },
                { "oint", { 0x222E, Op, true, false } }, { "oiint", { 0x222F, Op, true, false } },
                { "oiiint", { 0x2230, Op, true, false } }, { "bigcup", { 0x22C3, Op, true, true } },
                { "bigcap", { 0x22C2, Op, true, true } }, { "bigoplus", { 0x2A01, Op, true, true } },
                { "bigotimes", { 0x2A02, Op, true, true } }, { "bigodot", { 0x2A00, Op, true, true } },
                { "biguplus", { 0x2A04, Op, true, true } }, { "bigsqcup", { 0x2A06, Op, true, true } },
                { "bigvee", { 0x22C1, Op, true, true } }, { "bigwedge", { 0x22C0, Op, true, true } },
                // delimiters
                { "{", { U'{', Open } }, { "}", { U'}', Close } }, { "lbrace", { U'{', Open } }, { "rbrace", { U'}', Close } },
                { "lbrack", { U'[', Open } }, { "rbrack", { U']', Close } }, { "langle", { 0x27E8, Open } },
                { "rangle", { 0x27E9, Close } }, { "lfloor", { 0x230A, Open } }, { "rfloor", { 0x230B, Close } },
                { "lceil", { 0x2308, Open } }, { "rceil", { 0x2309, Close } }, { "lvert", { 0x007C, Open } },
                { "rvert", { 0x007C, Close } }, { "lVert", { 0x2016, Open } }, { "rVert", { 0x2016, Close } },
                { "lgroup", { 0x27EE, Open } }, { "rgroup", { 0x27EF, Close } }, { "llbracket", { 0x27E6, Open } },
                { "rrbracket", { 0x27E7, Close } }, { "lmoustache", { 0x23B0, Open } }, { "rmoustache", { 0x23B1, Close } }
            };
            return sSymbols;
        }

        std::unordered_map<std::string_view, char32_t> const& greek()
        {
            static std::unordered_map<std::string_view, char32_t> const sGreek = {
                { "alpha", 0x03B1 }, { "beta", 0x03B2 }, { "gamma", 0x03B3 }, { "delta", 0x03B4 }, { "epsilon", 0x03F5 },
                { "varepsilon", 0x03B5 }, { "zeta", 0x03B6 }, { "eta", 0x03B7 }, { "theta", 0x03B8 }, { "vartheta", 0x03D1 },
                { "iota", 0x03B9 }, { "kappa", 0x03BA }, { "varkappa", 0x03F0 }, { "lambda", 0x03BB }, { "mu", 0x03BC },
                { "nu", 0x03BD }, { "xi", 0x03BE }, { "omicron", 0x03BF }, { "pi", 0x03C0 }, { "varpi", 0x03D6 },
                { "rho", 0x03C1 }, { "varrho", 0x03F1 }, { "sigma", 0x03C3 }, { "varsigma", 0x03C2 }, { "tau", 0x03C4 },
                { "upsilon", 0x03C5 }, { "phi", 0x03D5 }, { "varphi", 0x03C6 }, { "chi", 0x03C7 }, { "psi", 0x03C8 },
                { "omega", 0x03C9 }, { "digamma", 0x03DD },
                { "Alpha", 0x0391 }, { "Beta", 0x0392 }, { "Gamma", 0x0393 }, { "Delta", 0x0394 }, { "Epsilon", 0x0395 },
                { "Zeta", 0x0396 }, { "Eta", 0x0397 }, { "Theta", 0x0398 }, { "Iota", 0x0399 }, { "Kappa", 0x039A },
                { "Lambda", 0x039B }, { "Mu", 0x039C }, { "Nu", 0x039D }, { "Xi", 0x039E }, { "Omicron", 0x039F },
                { "Pi", 0x03A0 }, { "Rho", 0x03A1 }, { "Sigma", 0x03A3 }, { "Tau", 0x03A4 }, { "Upsilon", 0x03A5 },
                { "Phi", 0x03A6 }, { "Chi", 0x03A7 }, { "Psi", 0x03A8 }, { "Omega", 0x03A9 }
            };
            return sGreek;
        }

        struct function_name
        {
            std::string_view text;
            bool displayLimits;
        };

        std::unordered_map<std::string_view, function_name> const& functions()
        {
            static std::unordered_map<std::string_view, function_name> const sFunctions = {
                { "sin", { "sin", false } }, { "cos", { "cos", false } }, { "tan", { "tan", false } }, { "cot", { "cot", false } },
                { "sec", { "sec", false } }, { "csc", { "csc", false } }, { "arcsin", { "arcsin", false } },
                { "arccos", { "arccos", false } }, { "arctan", { "arctan", false } }, { "sinh", { "sinh", false } },
                { "cosh", { "cosh", false } }, { "tanh", { "tanh", false } }, { "coth", { "coth", false } },
                { "log", { "log", false } }, { "ln", { "ln", false } }, { "lg", { "lg", false } }, { "exp", { "exp", false } },
                { "ker", { "ker", false } }, { "hom", { "hom", false } }, { "arg", { "arg", false } }, { "deg", { "deg", false } },
                { "dim", { "dim", false } }, { "lim", { "lim", true } }, { "liminf", { "lim inf", true } },
                { "limsup", { "lim sup", true } }, { "max", { "max", true } }, { "min", { "min", true } },
                { "sup", { "sup", true } }, { "inf", { "inf", true } }, { "det", { "det", true } }, { "gcd", { "gcd", true } },
                { "Pr", { "Pr", true } }, { "argmax", { "arg max", true } }, { "argmin", { "arg min", true } }
            };
            return sFunctions;
        }

        struct accent_info
        {
            char32_t cp;
            bool stretchy;
            bool under = false;
        };

        std::unordered_map<std::string_view, accent_info> const& accents()
        {
            static std::unordered_map<std::string_view, accent_info> const sAccents = {
                { "hat", { 0x0302, false } }, { "widehat", { 0x0302, true } }, { "check", { 0x030C, false } },
                { "widecheck", { 0x030C, true } }, { "tilde", { 0x0303, false } }, { "widetilde", { 0x0303, true } },
                { "acute", { 0x0301, false } }, { "grave", { 0x0300, false } }, { "dot", { 0x0307, false } },
                { "ddot", { 0x0308, false } }, { "dddot", { 0x20DB, false } }, { "breve", { 0x0306, false } },
                { "bar", { 0x0304, false } }, { "vec", { 0x20D7, false } }, { "mathring", { 0x030A, false } },
                { "overrightarrow", { 0x2192, true } }, { "overleftarrow", { 0x2190, true } },
                { "overleftrightarrow", { 0x2194, true } }, { "underrightarrow", { 0x2192, true, true } },
                { "underleftarrow", { 0x2190, true, true } }, { "underleftrightarrow", { 0x2194, true, true } },
                { "utilde", { 0x0330, true, true } }
            };
            return sAccents;
        }

        std::unordered_map<char32_t, char32_t> const& negations()
        {
            static std::unordered_map<char32_t, char32_t> const sNegations = {
                { U'=', 0x2260 }, { U'<', 0x226E }, { U'>', 0x226F }, { 0x2264, 0x2270 }, { 0x2265, 0x2271 },
                { 0x2208, 0x2209 }, { 0x220B, 0x220C }, { 0x2282, 0x2284 }, { 0x2283, 0x2285 }, { 0x2286, 0x2288 },
                { 0x2287, 0x2289 }, { 0x2261, 0x2262 }, { 0x223C, 0x2241 }, { 0x2243, 0x2244 }, { 0x2245, 0x2247 },
                { 0x2248, 0x2249 }, { 0x2223, 0x2224 }, { 0x2225, 0x2226 }, { 0x227A, 0x2280 }, { 0x227B, 0x2281 },
                { 0x22A2, 0x22AC }, { 0x22A8, 0x22AD }, { 0x2192, 0x219B }, { 0x2190, 0x219A }, { 0x21D2, 0x21CF },
                { 0x21D0, 0x21CD }, { 0x2194, 0x21AE }, { 0x21D4, 0x21CE }, { 0x2203, 0x2204 }
            };
            return sNegations;
        }

        char32_t map_variant(char32_t aChar, font_variant aVariant)
        {
            bool const upper = (aChar >= U'A' && aChar <= U'Z');
            bool const lower = (aChar >= U'a' && aChar <= U'z');
            bool const digit = (aChar >= U'0' && aChar <= U'9');
            bool const greekUpper = (aChar >= 0x0391 && aChar <= 0x03A9 && aChar != 0x03A2);
            bool const greekLower = (aChar >= 0x03B1 && aChar <= 0x03C9);
            auto latin = [&](char32_t aUpperBase, char32_t aLowerBase, std::initializer_list<std::pair<char32_t, char32_t>> aExceptions) -> char32_t
            {
                for (auto const& e : aExceptions)
                    if (e.first == aChar)
                        return e.second;
                return upper ? aUpperBase + (aChar - U'A') : aLowerBase + (aChar - U'a');
            };
            auto greekVariant = [&](char32_t aBase) -> char32_t
            {
                // the 25 capitals (with the theta symbol in the gap at U+03A2) then 25 small letters, then 7 variant forms
                if (greekUpper)
                    return aBase + (aChar - 0x0391);
                if (greekLower)
                    return aBase + 26 + (aChar - 0x03B1);
                switch (aChar)
                {
                case 0x2202: return aBase + 51; // partial
                case 0x03F5: return aBase + 52; // lunate epsilon
                case 0x03D1: return aBase + 53; // theta symbol
                case 0x03F0: return aBase + 54; // kappa symbol
                case 0x03D5: return aBase + 55; // phi symbol
                case 0x03F1: return aBase + 56; // rho symbol
                case 0x03D6: return aBase + 57; // pi symbol
                case 0x2207: return aBase + 25; // nabla
                default: return aChar;
                }
            };
            bool const greekOther = (aChar == 0x03F5 || aChar == 0x03D1 || aChar == 0x03F0 || aChar == 0x03D5 || aChar == 0x03F1 || aChar == 0x03D6);
            switch (aVariant)
            {
            case font_variant::Default:
                if (upper || lower)
                    return latin(0x1D434, 0x1D44E, { { U'h', 0x210E } });
                if (greekLower || greekOther)
                    return greekVariant(0x1D6E2);
                return aChar;
            case font_variant::Upright:
                return aChar;
            case font_variant::Italic:
                if (upper || lower)
                    return latin(0x1D434, 0x1D44E, { { U'h', 0x210E } });
                if (greekUpper || greekLower || greekOther)
                    return greekVariant(0x1D6E2);
                return aChar;
            case font_variant::Bold:
                if (upper || lower)
                    return latin(0x1D400, 0x1D41A, {});
                if (digit)
                    return 0x1D7CE + (aChar - U'0');
                if (greekUpper || greekLower || greekOther || aChar == 0x2207)
                    return greekVariant(0x1D6A8);
                return aChar;
            case font_variant::BoldItalic:
                if (upper || lower)
                    return latin(0x1D468, 0x1D482, {});
                if (digit)
                    return 0x1D7CE + (aChar - U'0');
                if (greekUpper)
                    return greekVariant(0x1D6A8);
                if (greekLower || greekOther)
                    return greekVariant(0x1D71C);
                return aChar;
            case font_variant::DoubleStruck:
                if (upper || lower)
                    return latin(0x1D538, 0x1D552, { { U'C', 0x2102 }, { U'H', 0x210D }, { U'N', 0x2115 }, { U'P', 0x2119 },
                        { U'Q', 0x211A }, { U'R', 0x211D }, { U'Z', 0x2124 } });
                if (digit)
                    return 0x1D7D8 + (aChar - U'0');
                return aChar;
            case font_variant::Script:
                if (upper || lower)
                    return latin(0x1D49C, 0x1D4B6, { { U'B', 0x212C }, { U'E', 0x2130 }, { U'F', 0x2131 }, { U'H', 0x210B },
                        { U'I', 0x2110 }, { U'L', 0x2112 }, { U'M', 0x2133 }, { U'R', 0x211B }, { U'e', 0x212F },
                        { U'g', 0x210A }, { U'o', 0x2134 } });
                return aChar;
            case font_variant::Fraktur:
                if (upper || lower)
                    return latin(0x1D504, 0x1D51E, { { U'C', 0x212D }, { U'H', 0x210C }, { U'I', 0x2111 }, { U'R', 0x211C },
                        { U'Z', 0x2128 } });
                return aChar;
            case font_variant::SansSerif:
                if (upper || lower)
                    return latin(0x1D5A0, 0x1D5BA, {});
                if (digit)
                    return 0x1D7E2 + (aChar - U'0');
                return aChar;
            case font_variant::Monospace:
                if (upper || lower)
                    return latin(0x1D670, 0x1D68A, {});
                if (digit)
                    return 0x1D7F6 + (aChar - U'0');
                return aChar;
            }
            return aChar;
        }

        // ---------------------------------------------------------------------------------
        // Parse tree
        // ---------------------------------------------------------------------------------

        enum class node_kind : std::uint8_t
        {
            Symbol,
            Text,
            Group,
            Fraction,
            Radical,
            Accent,
            Overline,
            Underline,
            Delimited,
            Array,
            Space,
            StyleChange,
            Phantom,
            Boxed,
            Stack,
            Negated,
            ExtensibleArrow
        };

        enum class limits_e : std::uint8_t
        {
            Auto,
            Limits,
            NoLimits
        };

        struct node;
        using node_list = std::vector<node>;

        struct array_row
        {
            std::vector<node_list> cells;
            bool hlineBefore = false;
        };

        struct node
        {
            node_kind kind = node_kind::Group;
            atom_class cls = atom_class::Ord;
            char32_t cp = 0;
            std::u32string text;
            font_variant textVariant = font_variant::Upright;
            bool largeOp = false;
            bool displayLimits = false;
            limits_e limits = limits_e::Auto;
            int delimiterSize = 0;          // \big = 1 ... \Bigg = 4
            bool middle = false;            // \middle delimiter inside a Delimited body
            node_list body;
            node_list body2;
            node_list sup;
            node_list sub;
            bool hasSup = false;
            bool hasSub = false;
            // fraction
            bool fractionRule = true;
            int fractionStyle = -1;         // forced style for \dfrac / \tfrac
            char32_t leftDelimiter = 0;     // also Delimited, Array
            char32_t rightDelimiter = 0;
            // accent / stack
            bool stretchy = false;
            bool under = false;
            // array
            std::vector<array_row> rows;
            std::string columns;            // 'l', 'c', 'r' and '|'
            double columnSpacing = 1.0;     // em between columns
            bool alignedPairs = false;      // aligned/align: r-l column pairs
            int arrayStyle = -1;            // forced cell style
            double rowGap = 0.0;            // em
            bool tightRows = false;         // \substack: no struts
            // space
            double space = 0.0;             // em
            double textSpace = -1.0;        // em in text style and smaller, if different
            // style change
            int style = 0;
            // phantom
            bool phantomWidth = true;
            bool phantomHeight = true;
        };

        // ---------------------------------------------------------------------------------
        // Parser
        // ---------------------------------------------------------------------------------

        std::u32string to_utf32(std::string_view aText)
        {
            std::u32string result;
            for (std::size_t i = 0; i < aText.size();)
            {
                auto const c = static_cast<unsigned char>(aText[i]);
                char32_t cp = 0xFFFD;
                std::size_t length = 1;
                if (c < 0x80)
                    cp = c;
                else if ((c & 0xE0) == 0xC0 && i + 1 < aText.size())
                {
                    cp = ((c & 0x1F) << 6) | (aText[i + 1] & 0x3F);
                    length = 2;
                }
                else if ((c & 0xF0) == 0xE0 && i + 2 < aText.size())
                {
                    cp = ((c & 0x0F) << 12) | ((aText[i + 1] & 0x3F) << 6) | (aText[i + 2] & 0x3F);
                    length = 3;
                }
                else if ((c & 0xF8) == 0xF0 && i + 3 < aText.size())
                {
                    cp = ((c & 0x07) << 18) | ((aText[i + 1] & 0x3F) << 12) | ((aText[i + 2] & 0x3F) << 6) | (aText[i + 3] & 0x3F);
                    length = 4;
                }
                result += cp;
                i += length;
            }
            return result;
        }

        bool is_letter(char32_t aChar)
        {
            return (aChar >= U'a' && aChar <= U'z') || (aChar >= U'A' && aChar <= U'Z');
        }

        class parser
        {
        public:
            explicit parser(std::u32string const& aSource) :
                iSource{ aSource }
            {
            }
        public:
            node_list parse(bool& aError)
            {
                std::vector<array_row> rows;
                auto list = parse_list(stop_none);
                while (iPosition < iSource.size())
                {
                    if (peek_command("\\"))
                    {
                        // top level line break: rows of a centred "gather"
                        iPosition += 2;
                        skip_optional_dimension();
                        rows.push_back(array_row{ { std::move(list) } });
                        list = parse_list(stop_none);
                        continue;
                    }
                    // stray closing brace, \right, \end, &: skip it
                    iError = true;
                    if (iSource[iPosition] == U'\\')
                        read_command();
                    else
                        ++iPosition;
                    auto more = parse_list(stop_none);
                    list.insert(list.end(), std::make_move_iterator(more.begin()), std::make_move_iterator(more.end()));
                }
                if (!rows.empty())
                {
                    rows.push_back(array_row{ { std::move(list) } });
                    node gather;
                    gather.kind = node_kind::Array;
                    gather.rows = std::move(rows);
                    gather.columns = "c";
                    gather.arrayStyle = 0;
                    gather.rowGap = 0.3;
                    list.clear();
                    list.push_back(std::move(gather));
                }
                aError = iError;
                return list;
            }
        private:
            enum stop_e : std::uint32_t
            {
                stop_none       = 0x00,
                stop_brace      = 0x01,
                stop_right      = 0x02,
                stop_array      = 0x04,
                stop_bracket    = 0x08
            };
            bool at_end() const
            {
                return iPosition >= iSource.size();
            }
            void skip_space()
            {
                while (!at_end() && (iSource[iPosition] == U' ' || iSource[iPosition] == U'\t' || iSource[iPosition] == U'\n' || iSource[iPosition] == U'\r'))
                    ++iPosition;
            }
            bool peek_command(std::string_view aName) const
            {
                if (iPosition >= iSource.size() || iSource[iPosition] != U'\\')
                    return false;
                if (iPosition + 1 + aName.size() > iSource.size())
                    return false;
                for (std::size_t i = 0; i < aName.size(); ++i)
                    if (iSource[iPosition + 1 + i] != static_cast<char32_t>(aName[i]))
                        return false;
                // a control word must not be followed by another letter
                if (is_letter(static_cast<char32_t>(aName.back())) && iPosition + 1 + aName.size() < iSource.size() && is_letter(iSource[iPosition + 1 + aName.size()]))
                    return false;
                return true;
            }
            std::string read_command()
            {
                // at '\'
                ++iPosition;
                std::string name;
                if (at_end())
                    return name;
                if (is_letter(iSource[iPosition]))
                {
                    while (!at_end() && is_letter(iSource[iPosition]))
                        name += static_cast<char>(iSource[iPosition++]);
                    if (!at_end() && iSource[iPosition] == U'*' && (name == "operatorname" || name == "align" || name == "gather"))
                    {
                        name += '*';
                        ++iPosition;
                    }
                }
                else
                {
                    auto const ch = iSource[iPosition++];
                    if (ch < 0x80)
                        name += static_cast<char>(ch);
                    else
                        name = "?";
                }
                return name;
            }
            std::u32string read_raw_group()
            {
                // a brace delimited argument returned verbatim, or a single character
                skip_space();
                std::u32string result;
                if (at_end())
                {
                    iError = true;
                    return result;
                }
                if (iSource[iPosition] != U'{')
                {
                    if (iSource[iPosition] == U'\\')
                    {
                        auto const start = iPosition;
                        read_command();
                        result.assign(iSource, start, iPosition - start);
                    }
                    else
                        result += iSource[iPosition++];
                    return result;
                }
                ++iPosition;
                int depth = 1;
                while (!at_end())
                {
                    auto const ch = iSource[iPosition];
                    if (ch == U'\\' && iPosition + 1 < iSource.size())
                    {
                        result += ch;
                        result += iSource[iPosition + 1];
                        iPosition += 2;
                        continue;
                    }
                    ++iPosition;
                    if (ch == U'{')
                        ++depth;
                    else if (ch == U'}' && --depth == 0)
                        return result;
                    result += ch;
                }
                iError = true;
                return result;
            }
            std::string read_raw_ascii_group()
            {
                std::string result;
                for (auto ch : read_raw_group())
                    result += (ch < 0x80 ? static_cast<char>(ch) : '?');
                return result;
            }
            std::optional<double> parse_dimension(std::string_view aText)
            {
                // returns em
                std::size_t i = 0;
                while (i < aText.size() && aText[i] == ' ')
                    ++i;
                std::string number;
                if (i < aText.size() && (aText[i] == '-' || aText[i] == '+'))
                    number += aText[i++];
                while (i < aText.size() && ((aText[i] >= '0' && aText[i] <= '9') || aText[i] == '.'))
                    number += aText[i++];
                if (number.empty() || number == "-" || number == "+")
                    return {};
                double value = 0.0;
                try
                {
                    value = std::stod(number);
                }
                catch (...)
                {
                    return {};
                }
                while (i < aText.size() && aText[i] == ' ')
                    ++i;
                auto const unit = aText.substr(i, 2);
                if (unit == "em")
                    return value;
                if (unit == "ex")
                    return value * 0.431;
                if (unit == "mu")
                    return value / 18.0;
                if (unit == "pt")
                    return value / 10.0;
                if (unit == "px")
                    return value / 13.33;
                if (unit == "cm")
                    return value * 2.845;
                if (unit == "mm")
                    return value * 0.2845;
                if (unit == "in")
                    return value * 7.227;
                return value / 10.0;
            }
            void skip_optional_dimension()
            {
                // \\[2pt]
                skip_space();
                if (!at_end() && iSource[iPosition] == U'[')
                {
                    auto const close = iSource.find(U']', iPosition);
                    iPosition = (close == std::u32string::npos ? iSource.size() : close + 1);
                }
            }
            node_list parse_argument(font_variant aVariant)
            {
                // a group, or a single token
                skip_space();
                if (at_end())
                {
                    iError = true;
                    return {};
                }
                if (iSource[iPosition] == U'{')
                {
                    ++iPosition;
                    auto saved = iVariant;
                    iVariant = aVariant;
                    auto list = parse_list(stop_brace);
                    iVariant = saved;
                    if (!at_end() && iSource[iPosition] == U'}')
                        ++iPosition;
                    else
                        iError = true;
                    return list;
                }
                auto saved = iVariant;
                iVariant = aVariant;
                node_list list;
                parse_atom(list, stop_none, true);
                iVariant = saved;
                return list;
            }
            node_list parse_argument()
            {
                return parse_argument(iVariant);
            }
            std::optional<node_list> parse_optional_argument()
            {
                skip_space();
                if (at_end() || iSource[iPosition] != U'[')
                    return {};
                ++iPosition;
                auto list = parse_list(stop_bracket);
                if (!at_end() && iSource[iPosition] == U']')
                    ++iPosition;
                else
                    iError = true;
                return list;
            }
            char32_t parse_delimiter()
            {
                skip_space();
                if (at_end())
                {
                    iError = true;
                    return 0;
                }
                auto const ch = iSource[iPosition];
                if (ch == U'\\')
                {
                    auto const name = read_command();
                    if (name == "{" || name == "lbrace")
                        return U'{';
                    if (name == "}" || name == "rbrace")
                        return U'}';
                    if (name == "|" || name == "Vert" || name == "lVert" || name == "rVert")
                        return 0x2016;
                    if (name == "vert" || name == "lvert" || name == "rvert")
                        return U'|';
                    if (name == "backslash")
                        return U'\\';
                    if (auto const s = symbols().find(name); s != symbols().end())
                        return s->second.cp;
                    iError = true;
                    return 0;
                }
                ++iPosition;
                if (ch == U'.')
                    return 0;
                if (ch == U'<')
                    return 0x27E8;
                if (ch == U'>')
                    return 0x27E9;
                return ch;
            }
            node symbol_node(char32_t aChar, atom_class aClass)
            {
                node result;
                result.kind = node_kind::Symbol;
                result.cp = aChar;
                result.cls = aClass;
                return result;
            }
            node text_node(std::u32string const& aText, atom_class aClass, font_variant aVariant = font_variant::Upright)
            {
                node result;
                result.kind = node_kind::Text;
                result.text = aText;
                result.cls = aClass;
                result.textVariant = aVariant;
                return result;
            }
            node space_node(double aEm)
            {
                node result;
                result.kind = node_kind::Space;
                result.space = aEm;
                return result;
            }
            node_list parse_list(std::uint32_t aStop)
            {
                // infix \over, \atop, \choose, \brack, \brace: everything before is the numerator
                auto const depth = iInfixDepth + 1;
                auto list = parse_list_items(aStop);
                if (iInfix.empty() || iInfix.back().first != depth)
                    return list;
                auto infix = std::move(iInfix.back().second);
                iInfix.pop_back();
                infix.body2 = std::move(list);
                node_list result;
                result.push_back(std::move(infix));
                return result;
            }
            node_list parse_list_items(std::uint32_t aStop)
            {
                ++iInfixDepth;
                struct depth_guard { int& depth; ~depth_guard() { --depth; } } guard{ iInfixDepth };
                node_list list;
                while (!at_end())
                {
                    skip_space();
                    if (at_end())
                        break;
                    auto const ch = iSource[iPosition];
                    if (ch == U'}')
                    {
                        if ((aStop & stop_brace) == stop_brace)
                            break;
                        iError = true;
                        break;
                    }
                    if (ch == U']' && (aStop & stop_bracket) == stop_bracket)
                        break;
                    if (ch == U'&')
                    {
                        if ((aStop & stop_array) == stop_array)
                            break;
                        iError = true;
                        break;
                    }
                    if (ch == U'\\')
                    {
                        if (peek_command("\\") || peek_command("cr"))
                        {
                            if ((aStop & stop_array) == stop_array || aStop == stop_none)
                                break;
                            iPosition += 2;
                            continue;
                        }
                        if (peek_command("right") || peek_command("middle"))
                        {
                            if ((aStop & stop_right) == stop_right)
                                break;
                            iError = true;
                            break;
                        }
                        if (peek_command("end"))
                        {
                            if ((aStop & stop_array) == stop_array)
                                break;
                            iError = true;
                            break;
                        }
                        if (peek_command("hline"))
                        {
                            if ((aStop & stop_array) == stop_array)
                                break;
                            iPosition += 6;
                            continue;
                        }
                    }
                    if (ch == U'\\' && (peek_command("over") || peek_command("atop") || peek_command("choose") || peek_command("brack") || peek_command("brace")))
                    {
                        auto const name = read_command();
                        node infix;
                        infix.kind = node_kind::Fraction;
                        infix.fractionRule = (name == "over");
                        if (name == "choose") { infix.leftDelimiter = U'('; infix.rightDelimiter = U')'; }
                        else if (name == "brack") { infix.leftDelimiter = U'['; infix.rightDelimiter = U']'; }
                        else if (name == "brace") { infix.leftDelimiter = U'{'; infix.rightDelimiter = U'}'; }
                        infix.body = std::move(list);
                        list.clear();
                        if (!iInfix.empty() && iInfix.back().first == iInfixDepth)
                            iError = true; // ambiguous: two infix operators in one group
                        else
                            iInfix.emplace_back(iInfixDepth, std::move(infix));
                        continue;
                    }
                    if (ch == U'^' || ch == U'_' || ch == U'\'')
                    {
                        // script without a nucleus
                        list.push_back(node{});
                        parse_scripts(list.back());
                        continue;
                    }
                    parse_atom(list, aStop, false);
                }
                return list;
            }
            void parse_scripts(node& aNucleus)
            {
                for (;;)
                {
                    skip_space();
                    if (at_end())
                        return;
                    auto const ch = iSource[iPosition];
                    if (ch == U'\'')
                    {
                        while (!at_end() && iSource[iPosition] == U'\'')
                        {
                            ++iPosition;
                            aNucleus.sup.push_back(symbol_node(0x2032, atom_class::Ord));
                        }
                        aNucleus.hasSup = true;
                        continue;
                    }
                    if (ch == U'^' || ch == U'_')
                    {
                        ++iPosition;
                        auto argument = parse_argument();
                        auto& target = (ch == U'^' ? aNucleus.sup : aNucleus.sub);
                        auto& has = (ch == U'^' ? aNucleus.hasSup : aNucleus.hasSub);
                        if (has && ch == U'_')
                            iError = true;
                        target.insert(target.end(), std::make_move_iterator(argument.begin()), std::make_move_iterator(argument.end()));
                        has = true;
                        continue;
                    }
                    if (peek_command("limits"))
                    {
                        iPosition += 7;
                        aNucleus.limits = limits_e::Limits;
                        continue;
                    }
                    if (peek_command("nolimits"))
                    {
                        iPosition += 9;
                        aNucleus.limits = limits_e::NoLimits;
                        continue;
                    }
                    return;
                }
            }
            void push_atom(node_list& aList, node aNode, bool aSingle)
            {
                aList.push_back(std::move(aNode));
                if (!aSingle)
                    parse_scripts(aList.back());
            }
            void parse_atom(node_list& aList, std::uint32_t aStop, bool aSingle)
            {
                // bound the nesting depth so hostile input cannot exhaust the stack
                struct depth_guard { int& depth; ~depth_guard() { --depth; } } guard{ ++iDepth };
                if (iDepth > MaxNestingDepth)
                {
                    iError = true;
                    iPosition = iSource.size();
                    return;
                }
                skip_space();
                if (at_end())
                    return;
                auto const ch = iSource[iPosition];
                if (ch == U'{')
                {
                    ++iPosition;
                    node group;
                    group.kind = node_kind::Group;
                    group.body = parse_list(stop_brace);
                    if (!at_end() && iSource[iPosition] == U'}')
                        ++iPosition;
                    else
                        iError = true;
                    push_atom(aList, std::move(group), aSingle);
                    return;
                }
                if (ch == U'\\')
                {
                    parse_command(aList, aStop, aSingle);
                    return;
                }
                ++iPosition;
                if (is_letter(ch) || (ch >= 0x0391 && ch <= 0x03C9))
                {
                    push_atom(aList, symbol_node(map_variant(ch, iVariant), atom_class::Ord), aSingle);
                    return;
                }
                if (ch >= U'0' && ch <= U'9')
                {
                    push_atom(aList, symbol_node(map_variant(ch, iVariant), atom_class::Ord), aSingle);
                    return;
                }
                switch (ch)
                {
                case U'+': push_atom(aList, symbol_node(U'+', atom_class::Bin), aSingle); return;
                case U'-': push_atom(aList, symbol_node(0x2212, atom_class::Bin), aSingle); return;
                case U'*': push_atom(aList, symbol_node(0x2217, atom_class::Bin), aSingle); return;
                case U'=':
                case U'<':
                case U'>':
                case U':': push_atom(aList, symbol_node(ch, atom_class::Rel), aSingle); return;
                case U',':
                case U';': push_atom(aList, symbol_node(ch, atom_class::Punct), aSingle); return;
                case U'(':
                case U'[': push_atom(aList, symbol_node(ch, atom_class::Open), aSingle); return;
                case U')':
                case U']':
                case U'!':
                case U'?': push_atom(aList, symbol_node(ch, atom_class::Close), aSingle); return;
                case U'~': aList.push_back(space_node(0.333)); return;
                case U'$': return; // stray math shift
                default:
                    push_atom(aList, symbol_node(ch, (ch >= 0x2190 && ch <= 0x22FF) ? classify_unicode(ch) : atom_class::Ord), aSingle);
                    return;
                }
            }
            static atom_class classify_unicode(char32_t aChar)
            {
                for (auto const& [name, s] : symbols())
                    if (s.cp == aChar)
                        return s.cls;
                return atom_class::Ord;
            }
            void parse_command(node_list& aList, std::uint32_t aStop, bool aSingle)
            {
                auto const name = read_command();
                if (name.empty())
                {
                    iError = true;
                    return;
                }
                // spacing
                static std::unordered_map<std::string_view, double> const sSpaces = {
                    { ",", 3.0 / 18.0 }, { "thinspace", 3.0 / 18.0 }, { ":", 4.0 / 18.0 }, { ">", 4.0 / 18.0 },
                    { "medspace", 4.0 / 18.0 }, { ";", 5.0 / 18.0 }, { "thickspace", 5.0 / 18.0 }, { "!", -3.0 / 18.0 },
                    { "negthinspace", -3.0 / 18.0 }, { "negmedspace", -4.0 / 18.0 }, { "negthickspace", -5.0 / 18.0 },
                    { "quad", 1.0 }, { "qquad", 2.0 }, { " ", 0.333 }, { "enspace", 0.5 }, { "space", 0.333 } };
                if (auto const s = sSpaces.find(name); s != sSpaces.end())
                {
                    aList.push_back(space_node(s->second));
                    return;
                }
                if (name == "hspace" || name == "hskip" || name == "kern" || name == "mkern" || name == "mskip")
                {
                    std::string dimension;
                    skip_space();
                    if (!at_end() && iSource[iPosition] == U'{')
                        dimension = read_raw_ascii_group();
                    else
                    {
                        // an unbraced dimension: [sign] number unit
                        while (!at_end() && (iSource[iPosition] == U'-' || iSource[iPosition] == U'+' || iSource[iPosition] == U'.' ||
                            (iSource[iPosition] >= U'0' && iSource[iPosition] <= U'9') || iSource[iPosition] == U' '))
                            dimension += static_cast<char>(iSource[iPosition++]);
                        for (int i = 0; i < 2 && !at_end() && is_letter(iSource[iPosition]); ++i)
                            dimension += static_cast<char>(iSource[iPosition++]);
                    }
                    if (auto const em = parse_dimension(dimension))
                        aList.push_back(space_node(*em));
                    else
                        iError = true;
                    return;
                }
                if (name == "displaystyle" || name == "textstyle" || name == "scriptstyle" || name == "scriptscriptstyle")
                {
                    node change;
                    change.kind = node_kind::StyleChange;
                    change.style = (name == "displaystyle" ? 0 : name == "textstyle" ? 1 : name == "scriptstyle" ? 2 : 3);
                    aList.push_back(std::move(change));
                    return;
                }
                if (auto const s = symbols().find(name); s != symbols().end())
                {
                    node n = symbol_node(s->second.cp, s->second.cls);
                    n.largeOp = s->second.largeOp;
                    n.displayLimits = s->second.displayLimits;
                    if (s->second.cp == 0x2016 && (name == "|" || name == "Vert"))
                        n.cls = atom_class::Ord;
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (auto const g = greek().find(name); g != greek().end())
                {
                    push_atom(aList, symbol_node(map_variant(g->second, iVariant), atom_class::Ord), aSingle);
                    return;
                }
                if (auto const f = functions().find(name); f != functions().end())
                {
                    node n = text_node(to_utf32(f->second.text), atom_class::Op);
                    n.displayLimits = f->second.displayLimits;
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (auto const a = accents().find(name); a != accents().end())
                {
                    node n;
                    n.kind = node_kind::Accent;
                    n.cp = a->second.cp;
                    n.stretchy = a->second.stretchy;
                    n.under = a->second.under;
                    n.body = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "frac" || name == "dfrac" || name == "tfrac" || name == "cfrac" || name == "binom" || name == "dbinom" || name == "tbinom")
                {
                    node n;
                    n.kind = node_kind::Fraction;
                    n.body = parse_argument();
                    n.body2 = parse_argument();
                    if (name == "dfrac" || name == "cfrac" || name == "dbinom")
                        n.fractionStyle = 0;
                    else if (name == "tfrac" || name == "tbinom")
                        n.fractionStyle = 1;
                    if (name.find("binom") != std::string::npos)
                    {
                        n.fractionRule = false;
                        n.leftDelimiter = U'(';
                        n.rightDelimiter = U')';
                    }
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "sqrt")
                {
                    node n;
                    n.kind = node_kind::Radical;
                    if (auto degree = parse_optional_argument())
                        n.body2 = std::move(*degree);
                    n.body = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "left")
                {
                    node n;
                    n.kind = node_kind::Delimited;
                    n.cls = atom_class::Inner;
                    n.leftDelimiter = parse_delimiter();
                    for (;;)
                    {
                        auto part = parse_list(stop_right);
                        n.body.insert(n.body.end(), std::make_move_iterator(part.begin()), std::make_move_iterator(part.end()));
                        if (peek_command("middle"))
                        {
                            iPosition += 7;
                            node middle = symbol_node(parse_delimiter(), atom_class::Rel);
                            middle.middle = true;
                            n.body.push_back(std::move(middle));
                            continue;
                        }
                        if (peek_command("right"))
                        {
                            iPosition += 6;
                            n.rightDelimiter = parse_delimiter();
                        }
                        else
                            iError = true;
                        break;
                    }
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                static std::unordered_map<std::string_view, std::pair<int, atom_class>> const sBig = {
                    { "big", { 1, atom_class::Ord } }, { "Big", { 2, atom_class::Ord } }, { "bigg", { 3, atom_class::Ord } }, { "Bigg", { 4, atom_class::Ord } },
                    { "bigl", { 1, atom_class::Open } }, { "Bigl", { 2, atom_class::Open } }, { "biggl", { 3, atom_class::Open } }, { "Biggl", { 4, atom_class::Open } },
                    { "bigr", { 1, atom_class::Close } }, { "Bigr", { 2, atom_class::Close } }, { "biggr", { 3, atom_class::Close } }, { "Biggr", { 4, atom_class::Close } },
                    { "bigm", { 1, atom_class::Rel } }, { "Bigm", { 2, atom_class::Rel } }, { "biggm", { 3, atom_class::Rel } }, { "Biggm", { 4, atom_class::Rel } } };
                if (auto const b = sBig.find(name); b != sBig.end())
                {
                    node n = symbol_node(parse_delimiter(), b->second.second);
                    n.delimiterSize = b->second.first;
                    if (n.cp == 0)
                    {
                        aList.push_back(space_node(0.12));
                        return;
                    }
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "overline" || name == "underline")
                {
                    node n;
                    n.kind = (name == "overline" ? node_kind::Overline : node_kind::Underline);
                    n.body = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "overbrace" || name == "underbrace")
                {
                    node n;
                    n.kind = node_kind::Accent;
                    n.cp = (name == "overbrace" ? 0x23DE : 0x23DF);
                    n.stretchy = true;
                    n.under = (name == "underbrace");
                    n.body = parse_argument();
                    // scripts on the brace become limits
                    node wrapper;
                    wrapper.kind = node_kind::Group;
                    wrapper.cls = atom_class::Op;
                    wrapper.limits = limits_e::Limits;
                    wrapper.body.push_back(std::move(n));
                    push_atom(aList, std::move(wrapper), aSingle);
                    return;
                }
                if (name == "overset" || name == "underset" || name == "stackrel")
                {
                    node n;
                    n.kind = node_kind::Stack;
                    n.under = (name == "underset");
                    n.body2 = parse_argument();
                    n.body = parse_argument();
                    n.cls = (name == "stackrel" ? atom_class::Rel : atom_class::Ord);
                    if (name != "stackrel" && n.body.size() == 1 && n.body[0].kind == node_kind::Symbol)
                        n.cls = n.body[0].cls;
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "text" || name == "textrm" || name == "textnormal" || name == "mbox" || name == "textup" ||
                    name == "textbf" || name == "textit" || name == "textsf" || name == "texttt" || name == "hbox")
                {
                    auto raw = read_raw_group();
                    std::u32string text;
                    for (std::size_t i = 0; i < raw.size(); ++i)
                    {
                        if (raw[i] == U'\\' && i + 1 < raw.size() && !is_letter(raw[i + 1]))
                            text += raw[++i];
                        else if (raw[i] != U'{' && raw[i] != U'}' && raw[i] != U'$')
                            text += raw[i];
                    }
                    font_variant variant = font_variant::Upright;
                    if (name == "textbf")
                        variant = font_variant::Bold;
                    else if (name == "textit")
                        variant = font_variant::Italic;
                    else if (name == "textsf")
                        variant = font_variant::SansSerif;
                    else if (name == "texttt")
                        variant = font_variant::Monospace;
                    push_atom(aList, text_node(text, atom_class::Ord, variant), aSingle);
                    return;
                }
                if (name == "operatorname" || name == "operatorname*")
                {
                    auto raw = read_raw_group();
                    std::u32string text;
                    for (auto c : raw)
                        if (c != U'{' && c != U'}' && c != U'\\')
                            text += c;
                    node n = text_node(text, atom_class::Op);
                    n.displayLimits = (name == "operatorname*");
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                static std::unordered_map<std::string_view, font_variant> const sVariants = {
                    { "mathrm", font_variant::Upright }, { "mathup", font_variant::Upright }, { "mathit", font_variant::Italic },
                    { "mathnormal", font_variant::Default }, { "mathbf", font_variant::Bold }, { "boldsymbol", font_variant::BoldItalic },
                    { "bm", font_variant::BoldItalic }, { "mathbb", font_variant::DoubleStruck }, { "mathcal", font_variant::Script },
                    { "mathscr", font_variant::Script }, { "mathfrak", font_variant::Fraktur }, { "mathsf", font_variant::SansSerif },
                    { "mathtt", font_variant::Monospace } };
                if (auto const v = sVariants.find(name); v != sVariants.end())
                {
                    node group;
                    group.kind = node_kind::Group;
                    group.body = parse_argument(v->second);
                    push_atom(aList, std::move(group), aSingle);
                    return;
                }
                static std::unordered_map<std::string_view, std::pair<std::string_view, font_variant>> const sOldVariants = {
                    { "rm", { "", font_variant::Upright } }, { "bf", { "", font_variant::Bold } }, { "it", { "", font_variant::Italic } },
                    { "cal", { "", font_variant::Script } }, { "tt", { "", font_variant::Monospace } }, { "sf", { "", font_variant::SansSerif } } };
                if (auto const v = sOldVariants.find(name); v != sOldVariants.end())
                {
                    iVariant = v->second.second; // for the rest of the group
                    return;
                }
                static std::unordered_map<std::string_view, atom_class> const sClasses = {
                    { "mathord", atom_class::Ord }, { "mathop", atom_class::Op }, { "mathbin", atom_class::Bin },
                    { "mathrel", atom_class::Rel }, { "mathopen", atom_class::Open }, { "mathclose", atom_class::Close },
                    { "mathpunct", atom_class::Punct }, { "mathinner", atom_class::Inner } };
                if (auto const c = sClasses.find(name); c != sClasses.end())
                {
                    node group;
                    group.kind = node_kind::Group;
                    group.cls = c->second;
                    group.body = parse_argument();
                    if (c->second == atom_class::Op)
                        group.largeOp = false;
                    push_atom(aList, std::move(group), aSingle);
                    return;
                }
                if (name == "phantom" || name == "hphantom" || name == "vphantom")
                {
                    node n;
                    n.kind = node_kind::Phantom;
                    n.phantomWidth = (name != "vphantom");
                    n.phantomHeight = (name != "hphantom");
                    n.body = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "boxed" || name == "fbox")
                {
                    node n;
                    n.kind = node_kind::Boxed;
                    n.body = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "not")
                {
                    node_list target;
                    parse_atom(target, aStop, true);
                    if (target.empty())
                    {
                        iError = true;
                        return;
                    }
                    auto& negated = target.front();
                    if (negated.kind == node_kind::Symbol)
                        if (auto const n = negations().find(negated.cp); n != negations().end())
                        {
                            negated.cp = n->second;
                            push_atom(aList, std::move(negated), aSingle);
                            return;
                        }
                    node n;
                    n.kind = node_kind::Negated;
                    n.cls = (negated.kind == node_kind::Symbol ? negated.cls : atom_class::Rel);
                    n.body.push_back(std::move(negated));
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "color" || name == "textcolor" || name == "colorbox")
                {
                    // colour is not supported; the content is laid out normally
                    read_raw_group();
                    if (name != "color")
                    {
                        node group;
                        group.kind = node_kind::Group;
                        group.body = parse_argument();
                        push_atom(aList, std::move(group), aSingle);
                    }
                    return;
                }
                if (name == "mod" || name == "bmod")
                {
                    if (name == "mod")
                        aList.push_back(space_node(1.0));
                    push_atom(aList, text_node(U"mod", name == "bmod" ? atom_class::Bin : atom_class::Ord), aSingle);
                    if (name == "mod")
                        aList.push_back(space_node(6.0 / 18.0));
                    return;
                }
                if (name == "pmod")
                {
                    auto space = space_node(1.0);
                    space.textSpace = 8.0 / 18.0;
                    aList.push_back(std::move(space));
                    node group;
                    group.kind = node_kind::Group;
                    group.body.push_back(symbol_node(U'(', atom_class::Open));
                    group.body.push_back(text_node(U"mod", atom_class::Ord));
                    group.body.push_back(space_node(6.0 / 18.0));
                    auto argument = parse_argument();
                    group.body.insert(group.body.end(), std::make_move_iterator(argument.begin()), std::make_move_iterator(argument.end()));
                    group.body.push_back(symbol_node(U')', atom_class::Close));
                    push_atom(aList, std::move(group), aSingle);
                    return;
                }
                static std::unordered_map<std::string_view, char32_t> const sExtensibleArrows = {
                    { "xrightarrow", 0x2192 }, { "xleftarrow", 0x2190 }, { "xleftrightarrow", 0x2194 },
                    { "xRightarrow", 0x21D2 }, { "xLeftarrow", 0x21D0 }, { "xLeftrightarrow", 0x21D4 },
                    { "xmapsto", 0x21A6 }, { "xhookrightarrow", 0x21AA }, { "xhookleftarrow", 0x21A9 },
                    { "xrightharpoonup", 0x21C0 }, { "xrightleftharpoons", 0x21CC }, { "xlongequal", U'=' } };
                if (auto const a = sExtensibleArrows.find(name); a != sExtensibleArrows.end())
                {
                    node n;
                    n.kind = node_kind::ExtensibleArrow;
                    n.cls = atom_class::Rel;
                    n.cp = a->second;
                    if (auto under = parse_optional_argument())
                        n.body = std::move(*under);
                    n.body2 = parse_argument();
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "substack")
                {
                    skip_space();
                    node n;
                    n.kind = node_kind::Array;
                    n.columns = "c";
                    n.tightRows = true;
                    if (!at_end() && iSource[iPosition] == U'{')
                    {
                        ++iPosition;
                        n.rows.emplace_back();
                        for (;;)
                        {
                            auto cell = parse_list(stop_array | stop_brace);
                            n.rows.back().cells.push_back(std::move(cell));
                            if (at_end())
                            {
                                iError = true;
                                break;
                            }
                            if (peek_command("\\"))
                            {
                                iPosition += 2;
                                n.rows.emplace_back();
                                continue;
                            }
                            if (iSource[iPosition] == U'}')
                            {
                                ++iPosition;
                                break;
                            }
                            ++iPosition; // '&' is not meaningful here
                        }
                    }
                    else
                        iError = true;
                    push_atom(aList, std::move(n), aSingle);
                    return;
                }
                if (name == "begin")
                {
                    parse_environment(aList, aSingle);
                    return;
                }
                if (name == "nonumber" || name == "notag" || name == "label" || name == "tag")
                {
                    if (name == "label" || name == "tag")
                        read_raw_group();
                    return;
                }
                if (name == "limits" || name == "nolimits")
                {
                    if (!aList.empty())
                        aList.back().limits = (name == "limits" ? limits_e::Limits : limits_e::NoLimits);
                    return;
                }
                // unknown command: show it verbatim
                iError = true;
                push_atom(aList, text_node(to_utf32("\\" + name), atom_class::Ord), aSingle);
            }
            void parse_environment(node_list& aList, bool aSingle)
            {
                auto const environment = read_raw_ascii_group();
                node n;
                n.kind = node_kind::Array;
                n.columns = "c";
                n.columnSpacing = 1.0;
                std::string name = environment;
                if (name == "matrix" || name == "pmatrix" || name == "bmatrix" || name == "Bmatrix" || name == "vmatrix" ||
                    name == "Vmatrix" || name == "smallmatrix")
                {
                    if (name == "pmatrix") { n.leftDelimiter = U'('; n.rightDelimiter = U')'; }
                    else if (name == "bmatrix") { n.leftDelimiter = U'['; n.rightDelimiter = U']'; }
                    else if (name == "Bmatrix") { n.leftDelimiter = U'{'; n.rightDelimiter = U'}'; }
                    else if (name == "vmatrix") { n.leftDelimiter = U'|'; n.rightDelimiter = U'|'; }
                    else if (name == "Vmatrix") { n.leftDelimiter = 0x2016; n.rightDelimiter = 0x2016; }
                    n.arrayStyle = (name == "smallmatrix" ? 2 : 1);
                    if (name == "smallmatrix")
                        n.columnSpacing = 0.5;
                }
                else if (name == "cases" || name == "dcases" || name == "rcases")
                {
                    if (name == "rcases")
                        n.rightDelimiter = U'}';
                    else
                        n.leftDelimiter = U'{';
                    n.columns = "ll";
                    n.arrayStyle = (name == "dcases" ? 0 : 1);
                }
                else if (name == "aligned" || name == "align" || name == "align*" || name == "split" || name == "alignat" || name == "alignedat" || name == "eqnarray")
                {
                    if (name == "alignat" || name == "alignedat")
                        read_raw_group();
                    n.alignedPairs = true;
                    n.columns = "rl";
                    n.arrayStyle = 0;
                    n.rowGap = 0.3;
                    n.columnSpacing = 0.0;
                }
                else if (name == "gathered" || name == "gather" || name == "gather*" || name == "equation" || name == "equation*" || name == "multline")
                {
                    n.arrayStyle = 0;
                    n.rowGap = 0.3;
                }
                else if (name == "array" || name == "subarray")
                {
                    auto const spec = read_raw_ascii_group();
                    n.columns.clear();
                    for (auto c : spec)
                        if (c == 'l' || c == 'c' || c == 'r' || c == '|')
                            n.columns += c;
                    if (n.columns.empty())
                        n.columns = "c";
                    n.arrayStyle = (name == "subarray" ? 2 : 1);
                }
                else
                    iError = true;
                n.rows.emplace_back();
                for (;;)
                {
                    skip_space();
                    if (peek_command("hline"))
                    {
                        iPosition += 6;
                        n.rows.back().hlineBefore = true;
                        continue;
                    }
                    auto cell = parse_list(stop_array);
                    n.rows.back().cells.push_back(std::move(cell));
                    if (at_end())
                    {
                        iError = true;
                        break;
                    }
                    if (iSource[iPosition] == U'&')
                    {
                        ++iPosition;
                        continue;
                    }
                    if (peek_command("\\") || peek_command("cr"))
                    {
                        iPosition += (peek_command("\\") ? 2 : 3);
                        skip_optional_dimension();
                        n.rows.emplace_back();
                        continue;
                    }
                    if (peek_command("hline"))
                    {
                        iPosition += 6;
                        // \hline at the end of the last row
                        n.rows.emplace_back();
                        n.rows.back().hlineBefore = true;
                        continue;
                    }
                    if (peek_command("end"))
                    {
                        iPosition += 4;
                        read_raw_group();
                        break;
                    }
                    // anything else stops the cell (e.g. stray '}')
                    iError = true;
                    ++iPosition;
                }
                // drop a trailing empty row (from a final \\)
                if (n.rows.size() > 1)
                {
                    auto const& last = n.rows.back();
                    bool const empty = std::all_of(last.cells.begin(), last.cells.end(), [](node_list const& c) { return c.empty(); });
                    if (empty && !last.hlineBefore)
                        n.rows.pop_back();
                }
                if (n.alignedPairs)
                {
                    // spacing for a relation at the start of an "l" column, as amsmath's {} does
                    for (auto& row : n.rows)
                        for (std::size_t c = 1; c < row.cells.size(); c += 2)
                            row.cells[c].insert(row.cells[c].begin(), node{});
                }
                push_atom(aList, std::move(n), aSingle);
            }
        private:
            std::u32string const& iSource;
            std::size_t iPosition = 0;
            font_variant iVariant = font_variant::Default;
            bool iError = false;
            int iInfixDepth = 0;
            std::vector<std::pair<int, node>> iInfix;
            int iDepth = 0;
            static constexpr int MaxNestingDepth = 128;
        };

        // ---------------------------------------------------------------------------------
        // Font
        // ---------------------------------------------------------------------------------

        struct glyph_metrics
        {
            double advance;
            double top;       // ink extent above the baseline
            double bottom;    // ink extent below the baseline (positive downwards)
            double left;      // ink left edge relative to the origin
            double right;     // ink right edge relative to the origin
            double italic;
            double topAccent; // attachment point
        };

        class math_font
        {
        public:
            explicit math_font(hb_font_t* aFont) :
                iFont{ aFont },
                iHasMath{ hb_ot_math_has_data(hb_font_get_face(aFont)) != 0 }
            {
                int x = 0;
                int y = 0;
                hb_font_get_scale(aFont, &x, &y);
                iEm = std::max(1, y) / 64.0;
            }
        public:
            double em() const
            {
                return iEm;
            }
            double constant(hb_ot_math_constant_t aConstant) const
            {
                if (iHasMath)
                {
                    auto const value = hb_ot_math_get_constant(iFont, aConstant);
                    bool const percentage =
                        aConstant == HB_OT_MATH_CONSTANT_SCRIPT_PERCENT_SCALE_DOWN ||
                        aConstant == HB_OT_MATH_CONSTANT_SCRIPT_SCRIPT_PERCENT_SCALE_DOWN ||
                        aConstant == HB_OT_MATH_CONSTANT_RADICAL_DEGREE_BOTTOM_RAISE_PERCENT;
                    if (!percentage)
                        return value / 64.0;
                    if (value != 0)
                        return value;
                }
                return default_constant(aConstant);
            }
            double scale(int aLevel) const
            {
                if (aLevel <= 0)
                    return 1.0;
                if (aLevel == 1)
                    return constant(HB_OT_MATH_CONSTANT_SCRIPT_PERCENT_SCALE_DOWN) / 100.0;
                return constant(HB_OT_MATH_CONSTANT_SCRIPT_SCRIPT_PERCENT_SCALE_DOWN) / 100.0;
            }
            hb_codepoint_t glyph(char32_t aChar, int aLevel) const
            {
                auto const key = std::make_pair(aChar, std::min(aLevel, 2));
                if (auto const existing = iGlyphs.find(key); existing != iGlyphs.end())
                    return existing->second;
                hb_codepoint_t result = 0;
                if (!hb_font_get_nominal_glyph(iFont, aChar, &result))
                    result = 0;
                if (result != 0 && aLevel > 0 && iHasMath)
                {
                    // script style alternates
                    auto buffer = hb_buffer_create();
                    hb_buffer_add_utf32(buffer, reinterpret_cast<std::uint32_t const*>(&aChar), 1, 0, 1);
                    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
                    hb_buffer_set_script(buffer, HB_SCRIPT_COMMON);
                    hb_feature_t feature;
                    hb_feature_from_string(aLevel == 1 ? "ssty=1" : "ssty=2", -1, &feature);
                    hb_shape(iFont, buffer, &feature, 1);
                    unsigned int count = 0;
                    auto const info = hb_buffer_get_glyph_infos(buffer, &count);
                    if (count == 1 && info[0].codepoint != 0)
                        result = info[0].codepoint;
                    hb_buffer_destroy(buffer);
                }
                iGlyphs.emplace(key, result);
                return result;
            }
            bool has_glyph(char32_t aChar) const
            {
                hb_codepoint_t g = 0;
                return hb_font_get_nominal_glyph(iFont, aChar, &g) && g != 0;
            }
            glyph_metrics const& metrics(hb_codepoint_t aGlyph) const
            {
                if (auto const existing = iMetrics.find(aGlyph); existing != iMetrics.end())
                    return existing->second;
                hb_glyph_extents_t extents = {};
                hb_font_get_glyph_extents(iFont, aGlyph, &extents);
                glyph_metrics m;
                m.advance = hb_font_get_glyph_h_advance(iFont, aGlyph) / 64.0;
                m.top = extents.y_bearing / 64.0;
                m.bottom = -(extents.y_bearing + extents.height) / 64.0;
                m.left = extents.x_bearing / 64.0;
                m.right = (extents.x_bearing + extents.width) / 64.0;
                m.italic = iHasMath ? hb_ot_math_get_glyph_italics_correction(iFont, aGlyph) / 64.0 : 0.0;
                if (extents.width == 0 && extents.height == 0)
                    m.top = m.bottom = m.left = m.right = 0.0;
                // HarfBuzz reports half the advance when a glyph has no attachment point; the ink centre is a
                // better default (combining marks have no advance and their ink is left of the origin)
                auto const attachment = iHasMath ? hb_ot_math_get_glyph_top_accent_attachment(iFont, aGlyph) : hb_font_get_glyph_h_advance(iFont, aGlyph) / 2;
                m.topAccent = (attachment == hb_font_get_glyph_h_advance(iFont, aGlyph) / 2) ? (m.left + m.right) / 2.0 : attachment / 64.0;
                return iMetrics.emplace(aGlyph, m).first->second;
            }
            struct variant
            {
                hb_codepoint_t glyph;
                double advance;
            };
            std::vector<variant> variants(hb_codepoint_t aGlyph, hb_direction_t aDirection) const
            {
                std::vector<variant> result;
                if (!iHasMath)
                    return result;
                hb_ot_math_glyph_variant_t buffer[32];
                unsigned int offset = 0;
                for (;;)
                {
                    unsigned int count = 32;
                    auto const total = hb_ot_math_get_glyph_variants(iFont, aGlyph, aDirection, offset, &count, buffer);
                    for (unsigned int i = 0; i < count; ++i)
                        result.push_back(variant{ buffer[i].glyph, buffer[i].advance / 64.0 });
                    offset += count;
                    if (count == 0 || offset >= total)
                        break;
                }
                return result;
            }
            struct part
            {
                hb_codepoint_t glyph;
                double startConnector;
                double endConnector;
                double advance;
                bool extender;
            };
            std::vector<part> assembly(hb_codepoint_t aGlyph, hb_direction_t aDirection) const
            {
                std::vector<part> result;
                if (!iHasMath)
                    return result;
                hb_ot_math_glyph_part_t buffer[32];
                unsigned int offset = 0;
                for (;;)
                {
                    unsigned int count = 32;
                    hb_position_t italics = 0;
                    auto const total = hb_ot_math_get_glyph_assembly(iFont, aGlyph, aDirection, offset, &count, buffer, &italics);
                    for (unsigned int i = 0; i < count; ++i)
                        result.push_back(part{ buffer[i].glyph, buffer[i].start_connector_length / 64.0, buffer[i].end_connector_length / 64.0,
                            buffer[i].full_advance / 64.0, (buffer[i].flags & HB_OT_MATH_GLYPH_PART_FLAG_EXTENDER) != 0 });
                    offset += count;
                    if (count == 0 || offset >= total)
                        break;
                }
                return result;
            }
            double min_connector_overlap(hb_direction_t aDirection) const
            {
                return iHasMath ? hb_ot_math_get_min_connector_overlap(iFont, aDirection) / 64.0 : 0.0;
            }
            hb_font_t* handle() const
            {
                return iFont;
            }
        private:
            double default_constant(hb_ot_math_constant_t aConstant) const
            {
                // TeX (Computer Modern) values, in ems unless noted
                switch (aConstant)
                {
                case HB_OT_MATH_CONSTANT_SCRIPT_PERCENT_SCALE_DOWN: return 70.0;
                case HB_OT_MATH_CONSTANT_SCRIPT_SCRIPT_PERCENT_SCALE_DOWN: return 50.0;
                case HB_OT_MATH_CONSTANT_RADICAL_DEGREE_BOTTOM_RAISE_PERCENT: return 60.0;
                case HB_OT_MATH_CONSTANT_DELIMITED_SUB_FORMULA_MIN_HEIGHT: return 1.5 * iEm;
                case HB_OT_MATH_CONSTANT_DISPLAY_OPERATOR_MIN_HEIGHT: return 1.3 * iEm;
                case HB_OT_MATH_CONSTANT_AXIS_HEIGHT: return 0.25 * iEm;
                case HB_OT_MATH_CONSTANT_ACCENT_BASE_HEIGHT: return 0.431 * iEm;
                case HB_OT_MATH_CONSTANT_FLATTENED_ACCENT_BASE_HEIGHT: return 0.683 * iEm;
                case HB_OT_MATH_CONSTANT_SUBSCRIPT_SHIFT_DOWN: return 0.15 * iEm;
                case HB_OT_MATH_CONSTANT_SUBSCRIPT_TOP_MAX: return 0.344 * iEm;
                case HB_OT_MATH_CONSTANT_SUBSCRIPT_BASELINE_DROP_MIN: return 0.05 * iEm;
                case HB_OT_MATH_CONSTANT_SUPERSCRIPT_SHIFT_UP: return 0.363 * iEm;
                case HB_OT_MATH_CONSTANT_SUPERSCRIPT_SHIFT_UP_CRAMPED: return 0.289 * iEm;
                case HB_OT_MATH_CONSTANT_SUPERSCRIPT_BOTTOM_MIN: return 0.108 * iEm;
                case HB_OT_MATH_CONSTANT_SUPERSCRIPT_BASELINE_DROP_MAX: return 0.386 * iEm;
                case HB_OT_MATH_CONSTANT_SUB_SUPERSCRIPT_GAP_MIN: return 0.16 * iEm;
                case HB_OT_MATH_CONSTANT_SUPERSCRIPT_BOTTOM_MAX_WITH_SUBSCRIPT: return 0.344 * iEm;
                case HB_OT_MATH_CONSTANT_SPACE_AFTER_SCRIPT: return 0.056 * iEm;
                case HB_OT_MATH_CONSTANT_UPPER_LIMIT_GAP_MIN: return 0.111 * iEm;
                case HB_OT_MATH_CONSTANT_UPPER_LIMIT_BASELINE_RISE_MIN: return 0.2 * iEm;
                case HB_OT_MATH_CONSTANT_LOWER_LIMIT_GAP_MIN: return 0.167 * iEm;
                case HB_OT_MATH_CONSTANT_LOWER_LIMIT_BASELINE_DROP_MIN: return 0.6 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_TOP_SHIFT_UP: return 0.444 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_TOP_DISPLAY_STYLE_SHIFT_UP: return 0.677 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_BOTTOM_SHIFT_DOWN: return 0.345 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_BOTTOM_DISPLAY_STYLE_SHIFT_DOWN: return 0.686 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_GAP_MIN: return 0.12 * iEm;
                case HB_OT_MATH_CONSTANT_STACK_DISPLAY_STYLE_GAP_MIN: return 0.28 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_SHIFT_UP: return 0.394 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_DISPLAY_STYLE_SHIFT_UP: return 0.677 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_SHIFT_DOWN: return 0.345 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_DISPLAY_STYLE_SHIFT_DOWN: return 0.686 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_GAP_MIN: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_NUM_DISPLAY_STYLE_GAP_MIN: return 0.12 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_RULE_THICKNESS: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_GAP_MIN: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_FRACTION_DENOM_DISPLAY_STYLE_GAP_MIN: return 0.12 * iEm;
                case HB_OT_MATH_CONSTANT_OVERBAR_VERTICAL_GAP: return 0.12 * iEm;
                case HB_OT_MATH_CONSTANT_OVERBAR_RULE_THICKNESS: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_OVERBAR_EXTRA_ASCENDER: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_UNDERBAR_VERTICAL_GAP: return 0.12 * iEm;
                case HB_OT_MATH_CONSTANT_UNDERBAR_RULE_THICKNESS: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_UNDERBAR_EXTRA_DESCENDER: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_VERTICAL_GAP: return 0.05 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_DISPLAY_STYLE_VERTICAL_GAP: return 0.148 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_RULE_THICKNESS: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_EXTRA_ASCENDER: return 0.04 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_KERN_BEFORE_DEGREE: return 0.278 * iEm;
                case HB_OT_MATH_CONSTANT_RADICAL_KERN_AFTER_DEGREE: return -0.556 * iEm;
                default: return 0.0;
                }
            }
        private:
            hb_font_t* iFont;
            bool iHasMath;
            double iEm = 16.0;
            mutable std::map<std::pair<char32_t, int>, hb_codepoint_t> iGlyphs;
            mutable std::unordered_map<hb_codepoint_t, glyph_metrics> iMetrics;
        };

        // ---------------------------------------------------------------------------------
        // Boxes (internally y is upwards from the baseline)
        // ---------------------------------------------------------------------------------

        struct box
        {
            double width = 0.0;
            double height = 0.0;
            double depth = 0.0;
            double italic = 0.0;
            std::optional<double> topAccent;
            bool character = false;     // a single glyph (TeX's "character box" for script placement)
            std::vector<glyph_item> glyphs;
            std::vector<rule_item> rules; // rule_item::y is the bottom edge here

            void add(box const& aOther, double aX, double aY)
            {
                for (auto g : aOther.glyphs)
                {
                    g.x += static_cast<float>(aX);
                    g.y += static_cast<float>(aY);
                    glyphs.push_back(g);
                }
                for (auto r : aOther.rules)
                {
                    r.x += static_cast<float>(aX);
                    r.y += static_cast<float>(aY);
                    rules.push_back(r);
                }
            }
            void add_rule(double aX, double aBottom, double aWidth, double aHeight)
            {
                rules.push_back(rule_item{ static_cast<float>(aX), static_cast<float>(aBottom), static_cast<float>(aWidth), static_cast<float>(aHeight) });
            }
        };

        // horizontal list builder
        struct hlist
        {
            box result;

            void append(box const& aBox, double aShiftUp = 0.0)
            {
                result.add(aBox, result.width, aShiftUp);
                result.width += aBox.width;
                result.height = std::max(result.height, aBox.height + aShiftUp);
                result.depth = std::max(result.depth, aBox.depth - aShiftUp);
                result.italic = aBox.italic;
            }
            void kern(double aWidth)
            {
                result.width += aWidth;
                result.italic = 0.0;
            }
        };

        // ---------------------------------------------------------------------------------
        // Layout
        // ---------------------------------------------------------------------------------

        struct math_style
        {
            int size = 1;       // 0 display, 1 text, 2 script, 3 scriptscript
            bool cramped = false;

            int level() const
            {
                return std::max(0, size - 1);
            }
            bool display() const
            {
                return size == 0;
            }
            math_style superscript() const
            {
                return { size <= 1 ? 2 : 3, cramped };
            }
            math_style subscript() const
            {
                return { size <= 1 ? 2 : 3, true };
            }
            math_style numerator() const
            {
                return { size == 0 ? 1 : size == 1 ? 2 : 3, cramped };
            }
            math_style denominator() const
            {
                return { size == 0 ? 1 : size == 1 ? 2 : 3, true };
            }
            math_style with_cramped() const
            {
                return { size, true };
            }
        };

        class typesetter
        {
        public:
            explicit typesetter(hb_font_t* aFont) :
                iFont{ aFont }
            {
            }
        public:
            box layout_list(node_list const& aList, math_style aStyle)
            {
                struct item
                {
                    box b;
                    std::optional<atom_class> cls; // nullopt for spaces
                };
                std::vector<item> items;
                for (auto const& n : aList)
                {
                    if (n.kind == node_kind::StyleChange)
                    {
                        aStyle = math_style{ n.style, aStyle.cramped };
                        continue;
                    }
                    if (n.kind == node_kind::Space)
                    {
                        box space;
                        space.width = (n.textSpace >= 0.0 && !aStyle.display() ? n.textSpace : n.space) * em(aStyle);
                        items.push_back(item{ std::move(space), std::nullopt });
                        continue;
                    }
                    auto b = layout_atom(n, aStyle);
                    items.push_back(item{ std::move(b), effective_class(n) });
                }
                // TeX rules 5 and 6: binary operators in the wrong context become ordinary
                std::optional<atom_class> previous;
                item* previousItem = nullptr;
                for (auto& i : items)
                {
                    if (!i.cls)
                        continue;
                    if (*i.cls == atom_class::Bin && (!previous || *previous == atom_class::Bin || *previous == atom_class::Op ||
                        *previous == atom_class::Rel || *previous == atom_class::Open || *previous == atom_class::Punct))
                        i.cls = atom_class::Ord;
                    if (previousItem && *previousItem->cls == atom_class::Bin &&
                        (*i.cls == atom_class::Rel || *i.cls == atom_class::Close || *i.cls == atom_class::Punct))
                        previousItem->cls = atom_class::Ord;
                    previous = i.cls;
                    previousItem = &i;
                }
                if (previousItem && *previousItem->cls == atom_class::Bin)
                    previousItem->cls = atom_class::Ord;
                // inter-atom spacing (TeX's table, negative entries only apply in display and text styles)
                static constexpr std::int8_t sSpacing[8][8] = {
                    //          Ord Op  Bin Rel Open Close Punct Inner
                    /* Ord   */ {  0,  1, -2, -3,  0,  0,  0, -1 },
                    /* Op    */ {  1,  1,  0, -3,  0,  0,  0, -1 },
                    /* Bin   */ { -2, -2,  0,  0, -2,  0,  0, -2 },
                    /* Rel   */ { -3, -3,  0,  0, -3,  0,  0, -3 },
                    /* Open  */ {  0,  0,  0,  0,  0,  0,  0,  0 },
                    /* Close */ {  0,  1, -2, -3,  0,  0,  0, -1 },
                    /* Punct */ { -1, -1,  0, -1, -1, -1, -1, -1 },
                    /* Inner */ { -1,  1, -2, -3, -1,  0, -1, -1 } };
                hlist result;
                std::optional<atom_class> last;
                for (auto& i : items)
                {
                    if (i.cls && last)
                    {
                        auto spacing = sSpacing[static_cast<int>(*last)][static_cast<int>(*i.cls)];
                        if (spacing < 0)
                            spacing = (aStyle.size <= 1 ? static_cast<std::int8_t>(-spacing) : 0);
                        if (spacing > 0)
                            result.kern(em(aStyle) * (spacing + 2) / 18.0);
                    }
                    result.append(i.b);
                    if (i.cls)
                        last = i.cls;
                }
                if (items.size() == 1 && items[0].cls)
                {
                    result.result.character = items[0].b.character;
                    result.result.topAccent = items[0].b.topAccent;
                }
                return std::move(result.result);
            }
        private:
            atom_class effective_class(node const& aNode) const
            {
                return aNode.cls;
            }
            double em(math_style aStyle) const
            {
                return iFont.em() * iFont.scale(aStyle.level());
            }
            double constant(hb_ot_math_constant_t aConstant, math_style aStyle) const
            {
                return iFont.constant(aConstant) * iFont.scale(aStyle.level());
            }
            double axis(math_style aStyle) const
            {
                return constant(HB_OT_MATH_CONSTANT_AXIS_HEIGHT, aStyle);
            }
            box glyph_box(hb_codepoint_t aGlyph, int aLevel)
            {
                auto const s = iFont.scale(aLevel);
                auto const& m = iFont.metrics(aGlyph);
                box result;
                result.width = m.advance * s;
                result.height = m.top * s;
                result.depth = m.bottom * s;
                result.italic = m.italic * s;
                result.topAccent = m.topAccent * s;
                result.character = true;
                result.glyphs.push_back(glyph_item{ aGlyph, static_cast<float>(s), 0.0f, 0.0f });
                return result;
            }
            box char_box(char32_t aChar, math_style aStyle)
            {
                auto glyph = iFont.glyph(aChar, aStyle.level());
                if (glyph == 0)
                {
                    // a mathematical alphanumeric the font doesn't have: fall back to the plain letter
                    auto const plain = plain_char(aChar);
                    if (plain != aChar)
                        glyph = iFont.glyph(plain, aStyle.level());
                }
                return glyph_box(glyph, aStyle.level());
            }
            static char32_t plain_char(char32_t aChar)
            {
                if (aChar >= 0x1D400 && aChar <= 0x1D6A3)
                {
                    auto const offset = (aChar - 0x1D400) % 52;
                    return offset < 26 ? U'A' + offset : U'a' + (offset - 26);
                }
                if (aChar >= 0x1D6A8 && aChar <= 0x1D7C9)
                {
                    auto const offset = (aChar - 0x1D6A8) % 58;
                    if (offset < 25)
                        return offset == 17 ? 0x03F4 : 0x0391 + offset;
                    if (offset == 25)
                        return 0x2207;
                    if (offset < 51)
                        return 0x03B1 + (offset - 26);
                    static constexpr char32_t sOthers[] = { 0x2202, 0x03F5, 0x03D1, 0x03F0, 0x03D5, 0x03F1, 0x03D6 };
                    return sOthers[offset - 51];
                }
                if (aChar >= 0x1D7CE && aChar <= 0x1D7FF)
                    return U'0' + (aChar - 0x1D7CE) % 10;
                if (aChar == 0x210E)
                    return U'h';
                return aChar;
            }
            // a glyph (or assembly) at least aSize in the given direction
            box stretched_glyph(char32_t aChar, double aSize, hb_direction_t aDirection, math_style aStyle)
            {
                auto const base = iFont.glyph(aChar, 0);
                auto const s = iFont.scale(aStyle.level());
                bool const vertical = (aDirection == HB_DIRECTION_TTB);
                auto const size_of = [&](hb_codepoint_t aGlyph)
                {
                    auto const& m = iFont.metrics(aGlyph);
                    return vertical ? (m.top + m.bottom) : m.advance;
                };
                auto const target = aSize / s;
                if (base == 0)
                    return box{};
                if (size_of(base) >= target)
                    return glyph_box(base, aStyle.level());
                auto const variants = iFont.variants(base, aDirection);
                for (auto const& v : variants)
                    if (v.advance >= target)
                        return glyph_box(v.glyph, aStyle.level());
                auto const parts = iFont.assembly(base, aDirection);
                if (parts.empty())
                    return variants.empty() ? glyph_box(base, aStyle.level()) : glyph_box(variants.back().glyph, aStyle.level());
                return assemble(parts, target, aDirection, aStyle);
            }
            box assemble(std::vector<math_font::part> const& aParts, double aTarget, hb_direction_t aDirection, math_style aStyle)
            {
                bool const vertical = (aDirection == HB_DIRECTION_TTB);
                auto const minOverlap = iFont.min_connector_overlap(aDirection);
                // repeat extenders until the assembly can reach the target size
                std::vector<math_font::part> parts;
                for (int repeats = 1; repeats <= 64; ++repeats)
                {
                    parts.clear();
                    for (auto const& p : aParts)
                        for (int r = 0; r < (p.extender ? repeats : 1); ++r)
                            parts.push_back(p);
                    double maximum = 0.0;
                    for (std::size_t i = 0; i < parts.size(); ++i)
                        maximum += parts[i].advance - (i > 0 ? minOverlap : 0.0);
                    if (maximum >= aTarget)
                        break;
                }
                // uniform overlap, as large as the connectors allow, that makes the size as close to the target as possible
                double total = 0.0;
                double maxOverlap = std::numeric_limits<double>::max();
                for (std::size_t i = 0; i < parts.size(); ++i)
                {
                    total += parts[i].advance;
                    if (i > 0)
                        maxOverlap = std::min(maxOverlap, std::min(parts[i - 1].endConnector, parts[i].startConnector));
                }
                double overlap = minOverlap;
                if (parts.size() > 1)
                {
                    overlap = (total - aTarget) / static_cast<double>(parts.size() - 1);
                    overlap = std::clamp(overlap, minOverlap, std::max(minOverlap, maxOverlap));
                }
                auto const s = iFont.scale(aStyle.level());
                box result;
                double position = 0.0;
                for (std::size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        position -= overlap;
                    auto const& m = iFont.metrics(parts[i].glyph);
                    if (vertical)
                    {
                        // parts stack upwards from the bottom; each part's ink bottom sits at the current position
                        result.glyphs.push_back(glyph_item{ parts[i].glyph, static_cast<float>(s), 0.0f,
                            static_cast<float>((position + m.bottom) * s) });
                        result.width = std::max(result.width, m.advance * s);
                    }
                    else
                    {
                        result.glyphs.push_back(glyph_item{ parts[i].glyph, static_cast<float>(s), static_cast<float>(position * s), 0.0f });
                        result.height = std::max(result.height, m.top * s);
                        result.depth = std::max(result.depth, m.bottom * s);
                    }
                    position += parts[i].advance;
                }
                if (vertical)
                {
                    result.height = position * s;
                    result.depth = 0.0;
                }
                else
                    result.width = position * s;
                return result;
            }
            box delimiter(char32_t aChar, double aSize, math_style aStyle)
            {
                // a delimiter at least aSize tall, centred on the axis
                if (aChar == 0)
                {
                    box null;
                    null.width = 0.12 * em(aStyle);
                    return null;
                }
                auto b = stretched_glyph(aChar, aSize, HB_DIRECTION_TTB, aStyle);
                auto const shift = (b.height - b.depth) / 2.0 - axis(aStyle);
                box result;
                result.add(b, 0.0, -shift);
                result.width = b.width;
                result.height = b.height - shift;
                result.depth = b.depth + shift;
                return result;
            }
            double delimiter_size(double aHeight, double aDepth, math_style aStyle)
            {
                // TeX: \delimiterfactor = 901, \delimitershortfall = 5pt
                auto const a = axis(aStyle);
                auto const extent = std::max(aHeight - a, aDepth + a);
                return std::max(extent * 2.0 * 0.901, extent * 2.0 - 0.5 * em(aStyle));
            }
            box layout_atom(node const& aNode, math_style aStyle)
            {
                box nucleus;
                bool limits = false;
                switch (aNode.kind)
                {
                case node_kind::Symbol:
                    if (aNode.delimiterSize > 0)
                    {
                        static constexpr double sSizes[] = { 1.2, 1.8, 2.4, 3.0 };
                        nucleus = delimiter(aNode.cp, sSizes[aNode.delimiterSize - 1] * em(aStyle), aStyle);
                        break;
                    }
                    if (aNode.largeOp)
                    {
                        nucleus = large_operator(aNode.cp, aStyle);
                        limits = use_limits(aNode, aStyle);
                        break;
                    }
                    nucleus = char_box(aNode.cp, aStyle);
                    break;
                case node_kind::Text:
                    nucleus = text_box(aNode.text, aNode.textVariant, aStyle);
                    limits = aNode.cls == atom_class::Op && use_limits(aNode, aStyle);
                    break;
                case node_kind::Group:
                    nucleus = layout_list(aNode.body, aStyle);
                    if (aNode.cls == atom_class::Op)
                        limits = use_limits(aNode, aStyle);
                    break;
                case node_kind::Fraction:
                    nucleus = fraction(aNode, aStyle);
                    break;
                case node_kind::Radical:
                    nucleus = radical(aNode, aStyle);
                    break;
                case node_kind::Accent:
                    nucleus = accent(aNode, aStyle);
                    break;
                case node_kind::Overline:
                case node_kind::Underline:
                    nucleus = bar(aNode, aStyle);
                    break;
                case node_kind::Delimited:
                    nucleus = delimited(aNode, aStyle);
                    break;
                case node_kind::Array:
                    nucleus = array(aNode, aStyle);
                    break;
                case node_kind::Phantom:
                    {
                        auto content = layout_list(aNode.body, aStyle);
                        nucleus.width = aNode.phantomWidth ? content.width : 0.0;
                        nucleus.height = aNode.phantomHeight ? content.height : 0.0;
                        nucleus.depth = aNode.phantomHeight ? content.depth : 0.0;
                    }
                    break;
                case node_kind::Boxed:
                    nucleus = boxed(aNode, aStyle);
                    break;
                case node_kind::Stack:
                    nucleus = stack(aNode, aStyle);
                    break;
                case node_kind::Negated:
                    nucleus = negated(aNode, aStyle);
                    break;
                case node_kind::ExtensibleArrow:
                    nucleus = extensible_arrow(aNode, aStyle);
                    break;
                default:
                    break;
                }
                if (!aNode.hasSup && !aNode.hasSub)
                    return nucleus;
                if (limits)
                    return attach_limits(nucleus, aNode, aStyle);
                return attach_scripts(nucleus, aNode, aStyle, aNode.largeOp);
            }
            bool use_limits(node const& aNode, math_style aStyle) const
            {
                if (aNode.limits == limits_e::Limits)
                    return true;
                if (aNode.limits == limits_e::NoLimits)
                    return false;
                return aStyle.display() && aNode.displayLimits;
            }
            box large_operator(char32_t aChar, math_style aStyle)
            {
                box b;
                if (aStyle.display())
                {
                    auto const minimum = iFont.constant(HB_OT_MATH_CONSTANT_DISPLAY_OPERATOR_MIN_HEIGHT) * iFont.scale(aStyle.level());
                    auto const base = iFont.glyph(aChar, 0);
                    b = glyph_box(base, aStyle.level());
                    for (auto const& v : iFont.variants(base, HB_DIRECTION_TTB))
                    {
                        b = glyph_box(v.glyph, aStyle.level());
                        if (v.advance * iFont.scale(aStyle.level()) >= minimum)
                            break;
                    }
                }
                else
                    b = char_box(aChar, aStyle);
                // centre on the axis
                auto const shift = (b.height - b.depth) / 2.0 - axis(aStyle);
                box result;
                result.add(b, 0.0, -shift);
                result.width = b.width;
                result.height = b.height - shift;
                result.depth = b.depth + shift;
                result.italic = b.italic;
                result.character = true;
                return result;
            }
            box text_box(std::u32string const& aText, font_variant aVariant, math_style aStyle)
            {
                hlist result;
                for (auto ch : aText)
                {
                    if (ch == U' ')
                    {
                        result.kern(0.25 * em(aStyle));
                        continue;
                    }
                    auto mapped = (aVariant == font_variant::Upright ? ch : map_variant(ch, aVariant));
                    if (mapped != ch && !iFont.has_glyph(mapped))
                        mapped = ch;
                    auto glyph = iFont.glyph(mapped, 0); // text is not script-styled
                    auto b = glyph_box(glyph, aStyle.level());
                    b.character = false;
                    result.append(b);
                }
                result.result.italic = 0.0;
                return std::move(result.result);
            }
            box attach_scripts(box const& aNucleus, node const& aNode, math_style aStyle, bool aLargeOp)
            {
                auto const supStyle = aStyle.superscript();
                auto const subStyle = aStyle.subscript();
                std::optional<box> sup;
                std::optional<box> sub;
                if (aNode.hasSup)
                    sup = layout_list(aNode.sup, supStyle);
                if (aNode.hasSub)
                    sub = layout_list(aNode.sub, subStyle);
                // TeX rule 18
                double u = 0.0;
                double v = 0.0;
                bool const characterNucleus = aNucleus.character && aNode.kind == node_kind::Symbol && !aLargeOp;
                if (!characterNucleus)
                {
                    u = aNucleus.height - constant(HB_OT_MATH_CONSTANT_SUPERSCRIPT_BASELINE_DROP_MAX, supStyle);
                    v = aNucleus.depth + constant(HB_OT_MATH_CONSTANT_SUBSCRIPT_BASELINE_DROP_MIN, subStyle);
                }
                double supShift = 0.0;
                double subShift = 0.0;
                if (sup)
                {
                    auto const shiftUp = aStyle.cramped ?
                        constant(HB_OT_MATH_CONSTANT_SUPERSCRIPT_SHIFT_UP_CRAMPED, aStyle) :
                        constant(HB_OT_MATH_CONSTANT_SUPERSCRIPT_SHIFT_UP, aStyle);
                    supShift = std::max({ u, shiftUp, sup->depth + constant(HB_OT_MATH_CONSTANT_SUPERSCRIPT_BOTTOM_MIN, aStyle) });
                }
                if (sub)
                {
                    subShift = std::max({ v, constant(HB_OT_MATH_CONSTANT_SUBSCRIPT_SHIFT_DOWN, aStyle),
                        sub->height - constant(HB_OT_MATH_CONSTANT_SUBSCRIPT_TOP_MAX, aStyle) });
                }
                if (sup && sub)
                {
                    auto const gapMin = constant(HB_OT_MATH_CONSTANT_SUB_SUPERSCRIPT_GAP_MIN, aStyle);
                    auto const gap = (supShift - sup->depth) - (sub->height - subShift);
                    if (gap < gapMin)
                    {
                        subShift += gapMin - gap;
                        auto const maxBottom = constant(HB_OT_MATH_CONSTANT_SUPERSCRIPT_BOTTOM_MAX_WITH_SUBSCRIPT, aStyle);
                        auto const delta = maxBottom - (supShift - sup->depth);
                        if (delta > 0.0)
                        {
                            supShift += delta;
                            subShift -= delta;
                        }
                    }
                }
                auto const italic = aNucleus.character ? aNucleus.italic : 0.0;
                auto const spaceAfter = constant(HB_OT_MATH_CONSTANT_SPACE_AFTER_SCRIPT, aStyle);
                box result = aNucleus;
                double width = aNucleus.width;
                if (sup)
                {
                    // OpenType MATH: a large operator's advance already includes its italic correction; the
                    // superscript goes at the advance and the subscript is pulled in by the correction instead
                    auto const x = aNucleus.width + (aLargeOp ? 0.0 : italic);
                    result.add(*sup, x, supShift);
                    result.height = std::max(result.height, sup->height + supShift);
                    result.depth = std::max(result.depth, sup->depth - supShift);
                    width = std::max(width, x + sup->width);
                }
                if (sub)
                {
                    // integrals and other slanted operators pull the subscript in by the italic correction
                    auto const x = aNucleus.width - (aLargeOp ? italic : 0.0);
                    result.add(*sub, x, -subShift);
                    result.height = std::max(result.height, sub->height - subShift);
                    result.depth = std::max(result.depth, sub->depth + subShift);
                    width = std::max(width, x + sub->width);
                }
                result.width = width + spaceAfter;
                result.character = false;
                result.italic = 0.0;
                result.topAccent.reset();
                return result;
            }
            box attach_limits(box const& aNucleus, node const& aNode, math_style aStyle)
            {
                std::optional<box> upper;
                std::optional<box> lower;
                if (aNode.hasSup)
                    upper = layout_list(aNode.sup, aStyle.superscript());
                if (aNode.hasSub)
                    lower = layout_list(aNode.sub, aStyle.subscript());
                auto const italic = aNucleus.italic;
                double width = aNucleus.width;
                if (upper)
                    width = std::max(width, upper->width);
                if (lower)
                    width = std::max(width, lower->width);
                box result;
                result.width = width;
                result.add(aNucleus, (width - aNucleus.width) / 2.0, 0.0);
                result.height = aNucleus.height;
                result.depth = aNucleus.depth;
                if (upper)
                {
                    auto const shift = std::max(constant(HB_OT_MATH_CONSTANT_UPPER_LIMIT_BASELINE_RISE_MIN, aStyle),
                        constant(HB_OT_MATH_CONSTANT_UPPER_LIMIT_GAP_MIN, aStyle) + upper->depth);
                    auto const y = aNucleus.height + shift;
                    result.add(*upper, (width - upper->width) / 2.0 + italic / 2.0, y);
                    result.height = y + upper->height;
                }
                if (lower)
                {
                    auto const shift = std::max(constant(HB_OT_MATH_CONSTANT_LOWER_LIMIT_BASELINE_DROP_MIN, aStyle),
                        constant(HB_OT_MATH_CONSTANT_LOWER_LIMIT_GAP_MIN, aStyle) + lower->height);
                    auto const y = aNucleus.depth + shift;
                    result.add(*lower, (width - lower->width) / 2.0 - italic / 2.0, -y);
                    result.depth = y + lower->depth;
                }
                return result;
            }
            box fraction(node const& aNode, math_style aStyle)
            {
                if (aNode.fractionStyle >= 0)
                    aStyle = math_style{ aNode.fractionStyle, aStyle.cramped };
                auto const num = layout_list(aNode.body, aStyle.numerator());
                auto const den = layout_list(aNode.body2, aStyle.denominator());
                auto const display = aStyle.display();
                auto const a = axis(aStyle);
                double numShift = 0.0;
                double denShift = 0.0;
                double thickness = 0.0;
                if (aNode.fractionRule)
                {
                    thickness = constant(HB_OT_MATH_CONSTANT_FRACTION_RULE_THICKNESS, aStyle);
                    numShift = constant(display ? HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_DISPLAY_STYLE_SHIFT_UP : HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_SHIFT_UP, aStyle);
                    denShift = constant(display ? HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_DISPLAY_STYLE_SHIFT_DOWN : HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_SHIFT_DOWN, aStyle);
                    auto const numGap = constant(display ? HB_OT_MATH_CONSTANT_FRACTION_NUM_DISPLAY_STYLE_GAP_MIN : HB_OT_MATH_CONSTANT_FRACTION_NUMERATOR_GAP_MIN, aStyle);
                    auto const denGap = constant(display ? HB_OT_MATH_CONSTANT_FRACTION_DENOM_DISPLAY_STYLE_GAP_MIN : HB_OT_MATH_CONSTANT_FRACTION_DENOMINATOR_GAP_MIN, aStyle);
                    numShift = std::max(numShift, a + thickness / 2.0 + numGap + num.depth);
                    denShift = std::max(denShift, den.height + denGap - (a - thickness / 2.0));
                }
                else
                {
                    numShift = constant(display ? HB_OT_MATH_CONSTANT_STACK_TOP_DISPLAY_STYLE_SHIFT_UP : HB_OT_MATH_CONSTANT_STACK_TOP_SHIFT_UP, aStyle);
                    denShift = constant(display ? HB_OT_MATH_CONSTANT_STACK_BOTTOM_DISPLAY_STYLE_SHIFT_DOWN : HB_OT_MATH_CONSTANT_STACK_BOTTOM_SHIFT_DOWN, aStyle);
                    auto const gapMin = constant(display ? HB_OT_MATH_CONSTANT_STACK_DISPLAY_STYLE_GAP_MIN : HB_OT_MATH_CONSTANT_STACK_GAP_MIN, aStyle);
                    auto const gap = (numShift - num.depth) - (den.height - denShift);
                    if (gap < gapMin)
                    {
                        numShift += (gapMin - gap) / 2.0;
                        denShift += (gapMin - gap) / 2.0;
                    }
                }
                auto const padding = 0.12 * em(aStyle); // \nulldelimiterspace
                auto const width = std::max(num.width, den.width);
                box result;
                result.width = width + 2.0 * padding;
                result.add(num, padding + (width - num.width) / 2.0, numShift);
                result.add(den, padding + (width - den.width) / 2.0, -denShift);
                if (aNode.fractionRule)
                    result.add_rule(padding, a - thickness / 2.0, width, thickness);
                result.height = std::max(numShift + num.height, a + thickness / 2.0);
                result.depth = std::max(denShift + den.depth, -(a - thickness / 2.0));
                if (aNode.leftDelimiter != 0 || aNode.rightDelimiter != 0)
                    return wrap_delimiters(result, aNode.leftDelimiter, aNode.rightDelimiter, aStyle,
                        iFont.constant(HB_OT_MATH_CONSTANT_DELIMITED_SUB_FORMULA_MIN_HEIGHT) * iFont.scale(aStyle.level()) * (display ? 1.0 : 0.0));
                return result;
            }
            box wrap_delimiters(box const& aInner, char32_t aLeft, char32_t aRight, math_style aStyle, double aMinimum = 0.0)
            {
                auto const size = std::max(aMinimum, delimiter_size(aInner.height, aInner.depth, aStyle));
                hlist result;
                if (aLeft != 0)
                    result.append(delimiter(aLeft, size, aStyle));
                result.append(aInner);
                if (aRight != 0)
                    result.append(delimiter(aRight, size, aStyle));
                return std::move(result.result);
            }
            box radical(node const& aNode, math_style aStyle)
            {
                auto const body = layout_list(aNode.body, aStyle.with_cramped());
                auto const display = aStyle.display();
                auto const thickness = constant(HB_OT_MATH_CONSTANT_RADICAL_RULE_THICKNESS, aStyle);
                auto gap = constant(display ? HB_OT_MATH_CONSTANT_RADICAL_DISPLAY_STYLE_VERTICAL_GAP : HB_OT_MATH_CONSTANT_RADICAL_VERTICAL_GAP, aStyle);
                auto const extraAscender = constant(HB_OT_MATH_CONSTANT_RADICAL_EXTRA_ASCENDER, aStyle);
                auto const required = body.height + body.depth + gap + thickness;
                auto sign = stretched_glyph(0x221A, required, HB_DIRECTION_TTB, aStyle);
                auto const signSize = sign.height + sign.depth;
                if (signSize > required)
                    gap += (signSize - required) / 2.0;
                // the top of the sign is level with the top of the rule
                auto const top = body.height + gap + thickness;
                auto const shift = top - sign.height;
                hlist result;
                if (!aNode.body2.empty())
                {
                    auto const degree = layout_list(aNode.body2, math_style{ 3, false });
                    auto const kernBefore = constant(HB_OT_MATH_CONSTANT_RADICAL_KERN_BEFORE_DEGREE, aStyle);
                    auto const kernAfter = constant(HB_OT_MATH_CONSTANT_RADICAL_KERN_AFTER_DEGREE, aStyle);
                    auto const raise = iFont.constant(HB_OT_MATH_CONSTANT_RADICAL_DEGREE_BOTTOM_RAISE_PERCENT) / 100.0 * signSize;
                    result.kern(kernBefore);
                    result.append(degree, raise - (sign.depth - shift) + degree.depth);
                    result.kern(kernAfter);
                    if (result.result.width < 0.0)
                        result.result.width = 0.0;
                }
                result.append(sign, shift);
                auto const ruleX = result.result.width;
                result.append(body);
                result.result.add_rule(ruleX, top - thickness, body.width, thickness);
                result.result.height = std::max(result.result.height, top + extraAscender);
                result.result.italic = 0.0;
                result.result.character = false;
                return std::move(result.result);
            }
            box accent(node const& aNode, math_style aStyle)
            {
                auto const base = layout_list(aNode.body, aStyle.with_cramped());
                box mark;
                if (aNode.stretchy)
                    mark = stretched_glyph(aNode.cp, base.width, HB_DIRECTION_LTR, aStyle);
                else
                    mark = char_box(aNode.cp, aStyle);
                auto const markMetricsAttach = mark.topAccent.value_or(mark.width / 2.0);
                auto const baseAttach = (base.character && base.topAccent) ? *base.topAccent : base.width / 2.0;
                box result;
                result.width = base.width;
                result.add(base, 0.0, 0.0);
                result.height = base.height;
                result.depth = base.depth;
                double x = baseAttach - (aNode.stretchy ? mark.width / 2.0 : markMetricsAttach);
                if (aNode.stretchy)
                    x = (base.width - mark.width) / 2.0;
                if (!aNode.under)
                {
                    double y = 0.0;
                    if (aNode.cp == 0x23DE || (aNode.cp >= 0x2190 && aNode.cp <= 0x21FF))
                    {
                        // over braces and arrows sit above the base with a gap
                        y = base.height + constant(HB_OT_MATH_CONSTANT_OVERBAR_VERTICAL_GAP, aStyle) / 2.0 + mark.depth;
                    }
                    else
                    {
                        auto const accentBase = constant(HB_OT_MATH_CONSTANT_ACCENT_BASE_HEIGHT, aStyle);
                        y = base.height - std::min(base.height, accentBase);
                    }
                    result.add(mark, x, y);
                    result.height = std::max(result.height, y + mark.height);
                }
                else
                {
                    auto const y = base.depth + constant(HB_OT_MATH_CONSTANT_UNDERBAR_VERTICAL_GAP, aStyle) / 2.0 + mark.height;
                    result.add(mark, x, -y);
                    result.depth = std::max(result.depth, y + mark.depth);
                }
                result.width = std::max(result.width, x + mark.width);
                result.character = base.character;
                result.italic = base.italic;
                return result;
            }
            box bar(node const& aNode, math_style aStyle)
            {
                bool const over = (aNode.kind == node_kind::Overline);
                auto const base = layout_list(aNode.body, over ? aStyle.with_cramped() : aStyle);
                box result = base;
                result.character = false;
                if (over)
                {
                    auto const gap = constant(HB_OT_MATH_CONSTANT_OVERBAR_VERTICAL_GAP, aStyle);
                    auto const thickness = constant(HB_OT_MATH_CONSTANT_OVERBAR_RULE_THICKNESS, aStyle);
                    auto const extra = constant(HB_OT_MATH_CONSTANT_OVERBAR_EXTRA_ASCENDER, aStyle);
                    result.add_rule(0.0, base.height + gap, base.width, thickness);
                    result.height = base.height + gap + thickness + extra;
                }
                else
                {
                    auto const gap = constant(HB_OT_MATH_CONSTANT_UNDERBAR_VERTICAL_GAP, aStyle);
                    auto const thickness = constant(HB_OT_MATH_CONSTANT_UNDERBAR_RULE_THICKNESS, aStyle);
                    auto const extra = constant(HB_OT_MATH_CONSTANT_UNDERBAR_EXTRA_DESCENDER, aStyle);
                    result.add_rule(0.0, -(base.depth + gap + thickness), base.width, thickness);
                    result.depth = base.depth + gap + thickness + extra;
                }
                return result;
            }
            box delimited(node const& aNode, math_style aStyle)
            {
                // segments between \middle delimiters
                std::vector<box> segments;
                std::vector<char32_t> middles;
                node_list current;
                for (auto const& n : aNode.body)
                {
                    if (n.kind == node_kind::Symbol && n.middle)
                    {
                        segments.push_back(layout_list(current, aStyle));
                        middles.push_back(n.cp);
                        current.clear();
                        continue;
                    }
                    current.push_back(n);
                }
                segments.push_back(layout_list(current, aStyle));
                double height = 0.0;
                double depth = 0.0;
                for (auto const& s : segments)
                {
                    height = std::max(height, s.height);
                    depth = std::max(depth, s.depth);
                }
                auto const size = delimiter_size(height, depth, aStyle);
                hlist result;
                result.append(delimiter(aNode.leftDelimiter, size, aStyle));
                for (std::size_t i = 0; i < segments.size(); ++i)
                {
                    result.append(segments[i]);
                    if (i < middles.size())
                    {
                        // a \middle delimiter is a relation: thick space either side in text and display styles
                        auto const space = aStyle.size <= 1 ? em(aStyle) * 5.0 / 18.0 : 0.0;
                        result.kern(space);
                        result.append(delimiter(middles[i], size, aStyle));
                        result.kern(space);
                    }
                }
                result.append(delimiter(aNode.rightDelimiter, size, aStyle));
                result.result.italic = 0.0;
                return std::move(result.result);
            }
            box array(node const& aNode, math_style aStyle)
            {
                auto const cellStyle = aNode.arrayStyle >= 0 ? math_style{ aNode.arrayStyle, false } : aStyle;
                auto const e = em(cellStyle);
                std::size_t columns = 0;
                for (auto const& row : aNode.rows)
                    columns = std::max(columns, row.cells.size());
                std::vector<std::vector<box>> cells(aNode.rows.size());
                std::vector<double> columnWidths(columns, 0.0);
                std::vector<double> rowHeights(aNode.rows.size(), 0.0);
                std::vector<double> rowDepths(aNode.rows.size(), 0.0);
                auto const arraySkip = 1.2 * e;
                for (std::size_t r = 0; r < aNode.rows.size(); ++r)
                {
                    rowHeights[r] = aNode.tightRows ? 0.0 : 0.7 * arraySkip;
                    rowDepths[r] = aNode.tightRows ? 0.0 : 0.3 * arraySkip;
                    for (std::size_t c = 0; c < aNode.rows[r].cells.size(); ++c)
                    {
                        cells[r].push_back(layout_list(aNode.rows[r].cells[c], cellStyle));
                        auto const& b = cells[r].back();
                        columnWidths[c] = std::max(columnWidths[c], b.width);
                        rowHeights[r] = std::max(rowHeights[r], b.height);
                        rowDepths[r] = std::max(rowDepths[r], b.depth);
                    }
                }
                // column alignment and vertical rules from the column specification
                std::vector<char> alignments;
                std::vector<int> rulesBefore;
                int pendingRules = 0;
                for (auto c : aNode.columns)
                {
                    if (c == '|')
                    {
                        ++pendingRules;
                        continue;
                    }
                    alignments.push_back(c);
                    rulesBefore.push_back(pendingRules);
                    pendingRules = 0;
                }
                auto const trailingRules = pendingRules;
                auto alignment_of = [&](std::size_t aColumn) -> char
                {
                    if (aNode.alignedPairs)
                        return aColumn % 2 == 0 ? 'r' : 'l';
                    if (alignments.empty())
                        return 'c';
                    return alignments[std::min(aColumn, alignments.size() - 1)];
                };
                auto const ruleThickness = constant(HB_OT_MATH_CONSTANT_FRACTION_RULE_THICKNESS, cellStyle);
                auto const rules_width = [&](int aCount) { return aCount * ruleThickness + (aCount - 1) * 2.0 * ruleThickness; };
                std::vector<double> columnX(columns, 0.0);
                std::vector<std::pair<double, int>> verticalRules;
                auto const columnSpacing = aNode.columnSpacing * e;
                double x = 0.0;
                for (std::size_t c = 0; c < columns; ++c)
                {
                    int const rules = (c < rulesBefore.size() ? rulesBefore[c] : 0);
                    if (c == 0)
                    {
                        if (rules > 0)
                        {
                            verticalRules.emplace_back(x, rules);
                            x += rules_width(rules) + columnSpacing / 2.0;
                        }
                    }
                    else
                    {
                        auto const gap = aNode.alignedPairs ? (c % 2 == 0 ? 2.0 * e : 0.0) : columnSpacing;
                        if (rules > 0)
                        {
                            x += gap / 2.0;
                            verticalRules.emplace_back(x, rules);
                            x += rules_width(rules) + gap / 2.0;
                        }
                        else
                            x += gap;
                    }
                    columnX[c] = x;
                    x += columnWidths[c];
                }
                if (trailingRules > 0)
                {
                    x += columnSpacing / 2.0;
                    verticalRules.emplace_back(x, trailingRules);
                    x += rules_width(trailingRules);
                }
                auto const width = x;
                // rows stack downwards from the top
                auto const rowGap = aNode.rowGap * e;
                auto const hlineGap = 0.15 * e;
                double total = 0.0;
                for (std::size_t r = 0; r < aNode.rows.size(); ++r)
                {
                    if (aNode.rows[r].hlineBefore)
                        total += ruleThickness + hlineGap;
                    total += rowHeights[r] + rowDepths[r];
                    if (r + 1 < aNode.rows.size())
                        total += rowGap;
                }
                auto const top = total / 2.0 + axis(aStyle);
                box result;
                result.width = width;
                double y = top;
                for (std::size_t r = 0; r < aNode.rows.size(); ++r)
                {
                    if (aNode.rows[r].hlineBefore)
                    {
                        result.add_rule(0.0, y - ruleThickness, width, ruleThickness);
                        y -= ruleThickness + hlineGap;
                    }
                    auto const baseline = y - rowHeights[r];
                    for (std::size_t c = 0; c < cells[r].size(); ++c)
                    {
                        auto const& b = cells[r][c];
                        double cx = columnX[c];
                        switch (alignment_of(c))
                        {
                        case 'r': cx += columnWidths[c] - b.width; break;
                        case 'c': cx += (columnWidths[c] - b.width) / 2.0; break;
                        default: break;
                        }
                        result.add(b, cx, baseline);
                    }
                    y = baseline - rowDepths[r] - (r + 1 < aNode.rows.size() ? rowGap : 0.0);
                }
                for (auto const& [rx, count] : verticalRules)
                    for (int i = 0; i < count; ++i)
                        result.add_rule(rx + i * ruleThickness * 3.0, top - total, ruleThickness, total);
                result.height = top;
                result.depth = total - top;
                if (aNode.leftDelimiter != 0 || aNode.rightDelimiter != 0)
                {
                    hlist wrapped;
                    auto const size = delimiter_size(result.height, result.depth, aStyle);
                    wrapped.append(delimiter(aNode.leftDelimiter, size, aStyle));
                    wrapped.kern(0.15 * e);
                    wrapped.append(result);
                    wrapped.kern(0.15 * e);
                    wrapped.append(delimiter(aNode.rightDelimiter, size, aStyle));
                    return std::move(wrapped.result);
                }
                return result;
            }
            box boxed(node const& aNode, math_style aStyle)
            {
                auto const content = layout_list(aNode.body, aStyle);
                auto const padding = 0.3 * em(aStyle);
                auto const thickness = 0.04 * em(aStyle);
                box result;
                result.width = content.width + 2.0 * (padding + thickness);
                result.height = content.height + padding + thickness;
                result.depth = content.depth + padding + thickness;
                result.add(content, padding + thickness, 0.0);
                result.add_rule(0.0, result.height - thickness, result.width, thickness);
                result.add_rule(0.0, -result.depth, result.width, thickness);
                result.add_rule(0.0, -result.depth, thickness, result.height + result.depth);
                result.add_rule(result.width - thickness, -result.depth, thickness, result.height + result.depth);
                return result;
            }
            box stack(node const& aNode, math_style aStyle)
            {
                auto const base = layout_list(aNode.body, aStyle);
                auto const annex = layout_list(aNode.body2, aNode.under ? aStyle.subscript() : aStyle.superscript());
                auto const width = std::max(base.width, annex.width);
                box result;
                result.width = width;
                result.add(base, (width - base.width) / 2.0, 0.0);
                result.height = base.height;
                result.depth = base.depth;
                if (!aNode.under)
                {
                    auto const shift = std::max(constant(HB_OT_MATH_CONSTANT_UPPER_LIMIT_BASELINE_RISE_MIN, aStyle),
                        constant(HB_OT_MATH_CONSTANT_UPPER_LIMIT_GAP_MIN, aStyle) + annex.depth);
                    result.add(annex, (width - annex.width) / 2.0, base.height + shift);
                    result.height = base.height + shift + annex.height;
                }
                else
                {
                    auto const shift = std::max(constant(HB_OT_MATH_CONSTANT_LOWER_LIMIT_BASELINE_DROP_MIN, aStyle),
                        constant(HB_OT_MATH_CONSTANT_LOWER_LIMIT_GAP_MIN, aStyle) + annex.height);
                    result.add(annex, (width - annex.width) / 2.0, -(base.depth + shift));
                    result.depth = base.depth + shift + annex.depth;
                }
                return result;
            }
            box extensible_arrow(node const& aNode, math_style aStyle)
            {
                // an arrow stretched to fit its labels (amsmath \xrightarrow): labels in script style above and below
                auto const over = layout_list(aNode.body2, aStyle.superscript());
                std::optional<box> under;
                if (!aNode.body.empty())
                    under = layout_list(aNode.body, aStyle.subscript());
                auto const padding = 0.5 * em(aStyle);
                auto const width = std::max({ over.width, under ? under->width : 0.0, 0.0 }) + 2.0 * padding;
                auto arrow = stretched_glyph(aNode.cp, width, HB_DIRECTION_LTR, aStyle);
                if (arrow.width < width && arrow.glyphs.size() == 1 && aNode.cp == U'=')
                {
                    // \xlongequal: no stretchy form; repeat the glyph
                    hlist repeated;
                    while (repeated.result.width + arrow.width * 0.7 < width)
                    {
                        repeated.append(arrow);
                        repeated.kern(-arrow.width * 0.3);
                    }
                    repeated.append(arrow);
                    arrow = std::move(repeated.result);
                }
                auto const total = std::max(width, arrow.width);
                box result;
                result.width = total;
                result.add(arrow, (total - arrow.width) / 2.0, 0.0);
                result.height = arrow.height;
                result.depth = arrow.depth;
                auto const gap = 0.111 * em(aStyle);
                auto const overShift = arrow.height + gap + over.depth;
                result.add(over, (total - over.width) / 2.0, overShift);
                result.height = std::max(result.height, overShift + over.height);
                if (under)
                {
                    auto const underShift = arrow.depth + gap + under->height;
                    result.add(*under, (total - under->width) / 2.0, -underShift);
                    result.depth = std::max(result.depth, underShift + under->depth);
                }
                return result;
            }
            box negated(node const& aNode, math_style aStyle)
            {
                auto const base = layout_list(aNode.body, aStyle);
                auto slash = char_box(0x0338, aStyle);
                if (slash.glyphs.empty() || slash.glyphs[0].glyph == 0)
                    slash = char_box(U'/', aStyle);
                box result = base;
                result.character = false;
                // centre the combining long solidus over the base
                auto const& m = iFont.metrics(slash.glyphs[0].glyph);
                auto const s = iFont.scale(aStyle.level());
                auto const inkCentre = (m.left + m.right) / 2.0 * s;
                result.add(slash, base.width / 2.0 - inkCentre, 0.0);
                result.height = std::max(result.height, slash.height);
                result.depth = std::max(result.depth, slash.depth);
                return result;
            }
        private:
            math_font iFont;
        };
    }

    layout typeset(hb_font_t* aFont, std::string_view aLatex, bool aDisplay)
    {
        layout result;
        if (aFont == nullptr)
        {
            result.error = true;
            return result;
        }
        auto const source = to_utf32(aLatex);
        parser p{ source };
        bool error = false;
        auto const list = p.parse(error);
        typesetter t{ aFont };
        auto const b = t.layout_list(list, math_style{ aDisplay ? 0 : 1, false });
        result.width = static_cast<float>(b.width);
        result.ascent = static_cast<float>(std::max(0.0, b.height));
        result.descent = static_cast<float>(std::max(0.0, b.depth));
        result.error = error;
        result.glyphs.reserve(b.glyphs.size());
        for (auto g : b.glyphs)
        {
            if (g.glyph == 0)
                continue;
            g.y = -g.y;
            result.glyphs.push_back(g);
        }
        result.rules.reserve(b.rules.size());
        for (auto r : b.rules)
        {
            r.y = -(r.y + r.height);
            result.rules.push_back(r);
        }
        return result;
    }
}
