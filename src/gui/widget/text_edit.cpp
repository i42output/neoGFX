// text_edit.cpp
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

#include <filesystem>
#include <fstream>
#include <future>
#include <thread>
#include <boost/algorithm/string/find.hpp>

#include <neolib/core/scoped.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/text/glyph_text.ipp>
#include <neogfx/gfx/text/i_emoticon_translator.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/core/easing.hpp>
#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/app/i_help.hpp>
#include <neolib/io/http.hpp>
#include "rich_text_parser.hpp"

namespace neogfx
{
    template class basic_glyph_text_content<text_edit::glyph_container_type>;

    namespace
    {
        constexpr char32_t ObjectReplacementCharacter = 0xFFFC;

        // an inline object's glyph (see text_edit::inline_objects::lay_out)
        bool is_inline_object(glyph_char const& aGlyph)
        {
            return is_whitespace(aGlyph) && aGlyph.value == ObjectReplacementCharacter;
        }

        uuid const& inline_object_ttid()
        {
            static uuid const sTtid{ 0x5c1e7a3b, 0x0d64, 0x4f2a, 0x9e18, { 0x73, 0x4b, 0xa2, 0x0f, 0xc6, 0x91 } };
            return sTtid;
        }

        // the marker (bullet, number or task box) that starts a list item; it hangs into the paragraph's left padding
        uuid const& list_marker_ttid()
        {
            static uuid const sTtid{ 0x2f6b9d14, 0x7a03, 0x4c58, 0xb1e2, { 0x0c, 0x95, 0x3e, 0x7d, 0x48, 0xa6 } };
            return sTtid;
        }

        // a link imported from rich text: its tag's data is the URI
        uuid const& link_tag_ttid()
        {
            static uuid const sTtid{ 0x8d2a4b7e, 0x3c1f, 0x4e95, 0xa0d6, { 0x51, 0x2b, 0x9e, 0x07, 0xc4, 0x3a } };
            return sTtid;
        }

        // a link that is opened externally (http, mailto, ...) rather than followed in place (an embedded resource, a file,
        // a path or a fragment of this document)
        bool is_external_link(std::string_view aUri)
        {
            if (aUri.empty() || aUri.starts_with("#") || aUri.starts_with(":/"))
                return false;
            auto const colon = aUri.find(':');
            bool const hasScheme = colon != std::string_view::npos && colon > 1 && std::all_of(aUri.begin(), aUri.begin() + colon,
                [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '+' || c == '-' || c == '.'; });
            bool const file = colon == 4 && std::equal(aUri.begin(), aUri.begin() + 4, "file",
                [](char lhs, char rhs) { return std::tolower(static_cast<unsigned char>(lhs)) == rhs; });
            return hasScheme && !file;
        }

        bool is_web_uri(std::string_view aUri)
        {
            auto starts_with = [&](std::string_view aScheme)
            {
                return aUri.size() >= aScheme.size() && std::equal(aScheme.begin(), aScheme.end(), aUri.begin(),
                    [](char lhs, char rhs) { return lhs == std::tolower(static_cast<unsigned char>(rhs)); });
            };
            return starts_with("http://") || starts_with("https://");
        }

        // a redirect's Location relative to the URL that was requested
        std::string resolve_url(std::string const& aBase, std::string const& aLocation)
        {
            if (is_web_uri(aLocation))
                return aLocation;
            auto const scheme = aBase.find("://");
            if (scheme == std::string::npos)
                return aLocation;
            if (aLocation.starts_with("//"))
                return aBase.substr(0, scheme + 1) + aLocation;
            auto const path = aBase.find('/', scheme + 3);
            if (path == std::string::npos)
                return aBase + (aLocation.starts_with("/") ? "" : "/") + aLocation;
            if (aLocation.starts_with("/"))
                return aBase.substr(0, path) + aLocation;
            return aBase.substr(0, aBase.rfind('/') + 1) + aLocation;
        }

        // the first installed font with an OpenType MATH table
        std::optional<string> const& math_font_family()
        {
            static std::optional<string> const sFamily = []() -> std::optional<string>
            {
                auto const& fontManager = service<i_font_manager>();
                for (auto const candidate : { "Cambria Math", "STIX Two Math", "Latin Modern Math", "XITS Math", "Libertinus Math",
                    "TeX Gyre Termes Math", "TeX Gyre Pagella Math", "Asana Math", "Fira Math", "Garamond-Math", "New Computer Modern Math",
                    "Noto Sans Math", "DejaVu Math TeX Gyre" })
                    for (std::uint32_t familyIndex = 0; familyIndex < fontManager.font_family_count(); ++familyIndex)
                        if (fontManager.font_family(familyIndex).to_std_string_view() == candidate)
                            return string{ candidate };
                return {};
            }();
            return sFamily;
        }

        std::optional<std::vector<std::uint8_t>> decode_data_uri(std::string_view aUri)
        {
            // data:[<media type>][;base64],<data>
            auto const comma = aUri.find(',');
            if (comma == std::string_view::npos)
                return {};
            auto const header = aUri.substr(0, comma);
            auto const payload = aUri.substr(comma + 1);
            std::vector<std::uint8_t> result;
            if (header.find(";base64") == std::string_view::npos)
            {
                for (std::size_t i = 0; i < payload.size(); ++i)
                    if (payload[i] == '%' && i + 2 < payload.size() &&
                        std::isxdigit(static_cast<unsigned char>(payload[i + 1])) && std::isxdigit(static_cast<unsigned char>(payload[i + 2])))
                    {
                        result.push_back(static_cast<std::uint8_t>(std::stoi(std::string{ payload.substr(i + 1, 2) }, nullptr, 16)));
                        i += 2;
                    }
                    else
                        result.push_back(static_cast<std::uint8_t>(payload[i]));
                return result;
            }
            std::uint32_t buffer = 0;
            int bits = 0;
            for (char const c : payload)
            {
                int value = -1;
                if (c >= 'A' && c <= 'Z') value = c - 'A';
                else if (c >= 'a' && c <= 'z') value = c - 'a' + 26;
                else if (c >= '0' && c <= '9') value = c - '0' + 52;
                else if (c == '+' || c == '-') value = 62;
                else if (c == '/' || c == '_') value = 63;
                else if (c == '=')
                    break;
                else
                    continue; // whitespace
                buffer = (buffer << 6) | static_cast<std::uint32_t>(value);
                bits += 6;
                if (bits >= 8)
                {
                    bits -= 8;
                    result.push_back(static_cast<std::uint8_t>((buffer >> bits) & 0xFF));
                }
            }
            return result;
        }

        std::string file_path(std::string_view aUri)
        {
            // file:///C:/x.png -> C:/x.png, file:///home/x.png -> /home/x.png, with %xx decoded
            std::string_view path = aUri;
            if (path.size() >= 5 && (path.substr(0, 5) == "file:" || path.substr(0, 5) == "FILE:"))
            {
                path.remove_prefix(5);
                if (path.starts_with("///"))
                    path.remove_prefix(2); // otherwise //server/share (UNC) is kept as is
                if (path.size() > 2 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':')
                    path.remove_prefix(1);
            }
            std::string result;
            for (std::size_t i = 0; i < path.size(); ++i)
                if (path[i] == '%' && i + 2 < path.size() && std::isxdigit(static_cast<unsigned char>(path[i + 1])) && std::isxdigit(static_cast<unsigned char>(path[i + 2])))
                {
                    result += static_cast<char>(std::stoi(std::string{ path.substr(i + 1, 2) }, nullptr, 16));
                    i += 2;
                }
                else
                    result += path[i];
            return result;
        }
    }

    // Math and images imported from rich text: each is one U+FFFC character with a tag whose data is the object. The
    // character's glyph is given the object's size when paragraphs are shaped, and the object is painted over it.
    struct text_edit::inline_objects
    {
        using object = rich_text::inline_object;

        struct object_metrics
        {
            dimension width = 0.0;
            dimension ascent = 0.0;
            dimension descent = 0.0;
        };

        struct math_entry
        {
            std::vector<std::pair<float, neogfx::font>> fonts; // by scale; the first is the math font at the text size
            math_formula formula;
            std::optional<glyph_text> glyphs; // built on first paint
            bool subpixel = false;
        };

        struct image_entry
        {
            enum class state_e
            {
                Loading,
                Loaded,
                Failed
            } state = state_e::Loading;
            std::optional<texture> image;
            std::future<std::vector<std::uint8_t>> pending;
            std::string redirect; // where a web server redirected the request
            std::uint32_t redirects = 0;
            std::unique_ptr<neolib::http> web; // last, so it goes (with its event handlers) before the members they use
        };

        text_edit& owner;
        std::map<std::pair<object, font_id>, math_entry> maths;
        std::map<std::string, image_entry> images;
        std::map<std::pair<object, font_id>, object_metrics> metrics;
        std::unique_ptr<widget_timer> loadTimer;
        // FollowLinks: the document a followed link refers to, being loaded; and where the current one came from (if a link)
        std::future<std::vector<std::uint8_t>> pendingDocument;
        std::string pendingDocumentUri;
        std::string documentUri;
        // the external link under the mouse, whose full URL the status bar shows
        struct link_help : i_help_source
        {
            string text;
            i_widget const* widget = nullptr;

            bool help_active() const override { return true; }
            neogfx::help_type help_type() const override { return neogfx::help_type::Action; }
            i_string const& help_text() const override { return text; }
            i_widget const* help_widget() const override { return widget; }
        };
        std::optional<std::string> hoveredLink;
        link_help linkHelp;
        bool linkHelpActive = false;
        // a formula being edited: expanded to its source text ($...$ or $$...$$) in [start, end)
        struct edit_state
        {
            position_type start;
            position_type end;
            object original;
            style originalStyle;
        };
        std::optional<edit_state> editing;
        dimension clampWidth = 0.0;
        dimension widestImage = 0.0; // the widest any image asked to be, before clamping

        explicit inline_objects(text_edit& aOwner) :
            owner{ aOwner }
        {
        }
        ~inline_objects()
        {
            hover_link(std::nullopt); // the help service must not keep a pointer to linkHelp
        }

        static object const* object_at(text_edit const& aOwner, position_type aTextIndex)
        {
            if (aTextIndex < 0 || aTextIndex >= static_cast<position_type>(aOwner.iText.size()))
                return nullptr;
            auto const& ch = aOwner.iText[aTextIndex];
            if (ch.character != ObjectReplacementCharacter || ch.tag == neolib::invalid_cookie<tag_cookie>)
                return nullptr;
            auto const existing = aOwner.iTagMap.find(ch.tag);
            if (existing == aOwner.iTagMap.end())
                return nullptr;
            auto const& tag = **existing;
            if (tag.ttid() != inline_object_ttid() || !tag.has_data())
                return nullptr;
            return static_cast<object const*>(tag.data());
        }

        math_entry& math_for(object const& aObject, neogfx::font const& aFont)
        {
            auto key = std::make_pair(aObject, aFont.id());
            auto existing = maths.find(key);
            if (existing != maths.end())
                return existing->second;
            math_entry entry;
            neogfx::font mathFont = aFont;
            if (math_font_family())
            {
                try
                {
                    mathFont = neogfx::font{ *math_font_family(), font_style::Normal, aFont.size() };
                }
                catch (...)
                {
                }
            }
            entry.fonts.emplace_back(1.0f, mathFont);
            entry.formula = service<i_font_manager>().typeset_math(mathFont, aObject.source, aObject.display);
            return maths.emplace(std::move(key), std::move(entry)).first->second;
        }

        static glyph_text const& math_glyphs(math_entry& aEntry, i_graphics_context const& aGc)
        {
            bool const subpixel = aGc.is_subpixel_rendering_on();
            if (aEntry.glyphs && aEntry.subpixel == subpixel)
                return *aEntry.glyphs;
            auto const baseFont = aEntry.fonts[0].second; // a copy: font_for() grows aEntry.fonts
            aEntry.glyphs.emplace(baseFont);
            aEntry.subpixel = subpixel;
            auto font_for = [&](float aScale) -> neogfx::font const&
            {
                for (auto const& f : aEntry.fonts)
                    if (std::abs(f.first - aScale) < 0.001f)
                        return f.second;
                aEntry.fonts.emplace_back(aScale, neogfx::font{ baseFont, font_style::Normal, baseFont.size() * aScale });
                return aEntry.fonts.back().second;
            };
            for (auto const& item : aEntry.formula.glyphs)
            {
                auto const& f = font_for(item.scale);
                glyph_char g{ item.glyph, { 0u, 1u }, character_type{ text_category::LTR, text_direction::LTR }, glyph_char::flags_e{}, f.id(), quadf_2d{}, quadf_2d{} };
                if (subpixel && !f.is_bitmap_font())
                    set_subpixel(g, true);
                // as the glyph text factory does: the cell's top is the baseline less the font's ascent
                auto const& fontGlyph = f.glyph(g);
                auto const glyphExtents = fontGlyph.texture().extents().as<float>();
                auto const& glyphMetrics = fontGlyph.metrics();
                auto const cellHeight = static_cast<float>(f.height());
                vec2f const origin{ item.x, item.y - (cellHeight + static_cast<float>(f.descender())) };
                g.cell = quadf_2d{
                    origin,
                    origin + vec2f{ glyphExtents.cx, 0.0f },
                    origin + vec2f{ glyphExtents.cx, cellHeight },
                    origin + vec2f{ 0.0f, cellHeight } };
                g.shape = quadf_2d{
                    vec2f{ 0.0f, 0.0f },
                    vec2f{ glyphExtents.cx, 0.0f },
                    vec2f{ glyphExtents.cx, glyphExtents.cy },
                    vec2f{ 0.0f, glyphExtents.cy } };
                g.shape += vec2{ glyphMetrics.bearing.x, glyphMetrics.bearing.y - glyphMetrics.extents.y + -f.descender() }.as<float>();
                if (aGc.logical_coordinate_system() == logical_coordinate_system::AutomaticGui)
                    for (auto& v : g.shape)
                        v.y = -v.y + cellHeight;
                aEntry.glyphs->content().push_back(g);
            }
            return *aEntry.glyphs;
        }

        image_entry& image_for(std::string const& aUri)
        {
            auto existing = images.find(aUri);
            if (existing != images.end())
                return existing->second;
            auto& entry = images[aUri];
            if (aUri.starts_with(":/"))
                finish(entry, aUri, {});
            else if (aUri.size() > 5 && (aUri.substr(0, 5) == "data:" || aUri.substr(0, 5) == "DATA:"))
            {
                auto const data = decode_data_uri(aUri);
                if (data && !data->empty())
                    finish(entry, "neogfx-text-edit-image:" + std::to_string(std::hash<std::string>{}(aUri)), *data);
                else
                    entry.state = image_entry::state_e::Failed;
            }
            else if ((owner.iCaps & text_edit_caps::AllowExternalResources) != text_edit_caps::AllowExternalResources)
                entry.state = image_entry::state_e::Failed; // shown as its alt text
            else if (is_web_uri(aUri))
                fetch(entry, aUri);
            else
            {
                entry.pending = read_file(file_path(aUri));
                start_polling();
            }
            return entry;
        }

        // files are read on another thread so the GUI thread never blocks on I/O
        static std::future<std::vector<std::uint8_t>> read_file(std::string const& aPath)
        {
            auto task = std::make_shared<std::packaged_task<std::vector<std::uint8_t>()>>([aPath]()
            {
                std::vector<std::uint8_t> data;
                std::ifstream file{ std::filesystem::path{ std::u8string{ aPath.begin(), aPath.end() } }, std::ios::binary };
                if (file)
                    data.assign(std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{});
                return data;
            });
            auto result = task->get_future();
            std::thread{ [task]() { (*task)(); } }.detach();
            return result;
        }

        // FollowLinks: a link to an embedded resource (":/...") or a file (relative ones against the current document, if
        // that came from a link) replaces the content; anything else (http, mailto, ...) is opened externally. The content
        // is replaced from poll(), not here: this runs in the link's own tag's event handler, and replacing the content
        // destroys the tag.
        void follow(std::string const& aUri)
        {
            if (aUri.empty() || aUri.starts_with("#"))
                return; // within this document: nothing to load
            auto const colon = aUri.find(':');
            bool const hasScheme = colon != std::string::npos && colon > 1 && std::all_of(aUri.begin(), aUri.begin() + colon,
                [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '+' || c == '-' || c == '.'; });
            bool const resource = aUri.starts_with(":/");
            if (is_external_link(aUri))
            {
                if (!service<i_basic_services>().open_uri(aUri))
                    service<i_basic_services>().system_beep();
                return;
            }
            std::string uri = aUri;
            bool const absolute = resource || hasScheme || uri.starts_with("/") || uri.starts_with("\\") ||
                (uri.size() > 2 && std::isalpha(static_cast<unsigned char>(uri[0])) && uri[1] == ':');
            if (!absolute && !documentUri.empty())
                uri = documentUri.substr(0, documentUri.find_last_of("/\\") + 1) + uri;
            if (auto const fragment = uri.find('#'); fragment != std::string::npos)
                uri.erase(fragment);
            if (uri.starts_with(":/"))
            {
                // embedded: already in memory
                std::promise<std::vector<std::uint8_t>> loaded;
                std::vector<std::uint8_t> data;
                try
                {
                    auto const embedded = service<i_resource_manager>().load_resource(uri);
                    if (embedded && embedded->available() && !embedded->is_empty())
                        data.assign(static_cast<std::uint8_t const*>(embedded->cdata()), static_cast<std::uint8_t const*>(embedded->cdata()) + embedded->size());
                }
                catch (...)
                {
                }
                loaded.set_value(std::move(data));
                pendingDocument = loaded.get_future();
            }
            else if ((owner.iCaps & text_edit_caps::AllowExternalResources) == text_edit_caps::AllowExternalResources)
                pendingDocument = read_file(file_path(uri));
            else
            {
                service<i_basic_services>().system_beep();
                return;
            }
            pendingDocumentUri = uri;
            start_polling();
        }

        // the external link under the mouse: its full URL is shown in the status bar, as help
        void hover_link(std::optional<std::string> const& aUri)
        {
            if (aUri == hoveredLink)
                return;
            if (linkHelpActive)
            {
                linkHelpActive = false;
                service<i_app>().help().deactivate(linkHelp);
            }
            hoveredLink = aUri;
            if (hoveredLink)
            {
                linkHelp.text = string{ *hoveredLink };
                linkHelp.widget = &owner;
                service<i_app>().help().activate(linkHelp);
                linkHelpActive = true;
            }
        }

        // a followed link's document replaces the content: HTML and Markdown (by extension) as rich text if allowed
        void show_document(std::string const& aUri, std::vector<std::uint8_t> const& aData)
        {
            auto extension = std::filesystem::path{ std::u8string{ aUri.begin(), aUri.end() } }.extension().string();
            for (auto& ch : extension)
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            string const content{ std::string{ aData.begin(), aData.end() } };
            bool const richText = (owner.iCaps & text_edit_caps::AllowRichText) == text_edit_caps::AllowRichText;
            if (richText && (extension == ".html" || extension == ".htm" || extension == ".xhtml"))
                owner.set_rich_text(content, rich_text_format::Html);
            else if (richText && (extension == ".md" || extension == ".markdown"))
                owner.set_rich_text(content, rich_text_format::Markdown);
            else
                owner.set_plain_text(content);
            documentUri = aUri;
        }

        // web images are fetched by neolib::http on the app thread's I/O service: resolving, connecting, the TLS handshake
        // and the transfer are all asynchronous, so the GUI thread never waits on the network
        void fetch(image_entry& aEntry, std::string const& aUrl)
        {
            auto const result = std::make_shared<std::promise<std::vector<std::uint8_t>>>();
            auto const settled = std::make_shared<bool>(false);
            auto settle = [result, settled](std::vector<std::uint8_t> aData)
            {
                if (!*settled) // a failed transfer can report more than once
                {
                    *settled = true;
                    result->set_value(std::move(aData));
                }
            };
            aEntry.pending = result->get_future();
            aEntry.redirect.clear();
            aEntry.web = std::make_unique<neolib::http>(static_cast<neolib::async_task&>(service<i_async_task>()));
            auto& web = *aEntry.web;
            web.Completed([&web, settle]()
            {
                settle({ web.body().begin(), web.body().end() });
            });
            web.Failure([&web, &aEntry, aUrl, settle]()
            {
                if (web.status_code() / 100 == 3)
                    if (auto const location = web.response_headers().find("Location"); location != web.response_headers().end())
                        aEntry.redirect = resolve_url(aUrl, location->second);
                settle({});
            });
            web.request(aUrl, neolib::http::Get, { { "User-Agent", "neoGFX" }, { "Accept", "image/*" } });
            start_polling();
        }

        void start_polling()
        {
            if (!loadTimer)
                loadTimer = std::make_unique<widget_timer>(owner, [this](widget_timer& aTimer) { poll(aTimer); }, std::chrono::milliseconds{ 50 });
            else
                loadTimer->again_if(); // may be called from poll() (a redirect), which then also restarts the timer
        }

        void finish(image_entry& aEntry, std::string const& aResource, std::vector<std::uint8_t> const& aData)
        {
            try
            {
                if (!aData.empty())
                    service<i_resource_manager>().add_resource(aResource, aData.data(), aData.size());
                image const loaded{ string{ aResource } };
                if (loaded.error() || loaded.extents().cx <= 0.0 || loaded.extents().cy <= 0.0)
                    aEntry.state = image_entry::state_e::Failed;
                else
                {
                    aEntry.image.emplace(loaded);
                    aEntry.state = image_entry::state_e::Loaded;
                }
            }
            catch (...)
            {
                aEntry.state = image_entry::state_e::Failed;
            }
        }

        void poll(widget_timer& aTimer)
        {
            bool stillLoading = false;
            bool changed = false;
            for (auto& [uri, entry] : images)
            {
                if (entry.state != image_entry::state_e::Loading || !entry.pending.valid())
                    continue;
                if (entry.pending.wait_for(std::chrono::seconds{ 0 }) != std::future_status::ready)
                {
                    stillLoading = true;
                    continue;
                }
                std::vector<std::uint8_t> data;
                try
                {
                    data = entry.pending.get();
                }
                catch (...)
                {
                }
                if (data.empty() && !entry.redirect.empty() && entry.redirects < 5)
                {
                    // follow it; the previous request is finished with, so it can go now (not from within its own event)
                    ++entry.redirects;
                    fetch(entry, std::string{ entry.redirect });
                    stillLoading = true;
                    continue;
                }
                entry.web.reset();
                if (data.empty())
                    entry.state = image_entry::state_e::Failed;
                else
                    finish(entry, "neogfx-text-edit-image:" + uri, data);
                changed = true;
            }
            if (changed)
            {
                metrics.clear();
                owner.refresh_paragraph(owner.iText.begin(), 0);
                owner.update();
            }
            if (pendingDocument.valid())
            {
                if (pendingDocument.wait_for(std::chrono::seconds{ 0 }) != std::future_status::ready)
                    stillLoading = true;
                else
                {
                    std::vector<std::uint8_t> data;
                    try
                    {
                        data = pendingDocument.get();
                    }
                    catch (...)
                    {
                    }
                    if (data.empty())
                        service<i_basic_services>().system_beep();
                    else
                        show_document(pendingDocumentUri, data);
                }
            }
            if (stillLoading)
                aTimer.again_if(); // already restarted if a redirect was followed above
        }

        object_metrics const& measure(i_graphics_context const& aGc, object const& aObject, neogfx::font const& aFont)
        {
            auto key = std::make_pair(aObject, aFont.id());
            if (auto const existing = metrics.find(key); existing != metrics.end())
                return existing->second;
            object_metrics result;
            if (aObject.kind == object::kind_e::Math)
            {
                auto const& entry = math_for(aObject, aFont);
                result = { entry.formula.metrics.width, entry.formula.metrics.ascent, entry.formula.metrics.descent };
            }
            else if (aObject.kind == object::kind_e::Rule)
                result = { 0.0, aFont.ascender(), -aFont.descender() }; // a line of text's height; drawn across the paragraph
            else
            {
                auto const& entry = image_for(aObject.source);
                switch (entry.state)
                {
                case image_entry::state_e::Loaded:
                    {
                        auto extents = entry.image->extents();
                        if (aObject.width && aObject.height)
                            extents = size{ *aObject.width, *aObject.height };
                        else if (aObject.width)
                            extents = size{ *aObject.width, extents.cy * *aObject.width / extents.cx };
                        else if (aObject.height)
                            extents = size{ extents.cx * *aObject.height / extents.cy, *aObject.height };
                        auto const maxWidth = owner.column_rect(0).width();
                        clampWidth = maxWidth;
                        widestImage = std::max(widestImage, extents.cx);
                        if (maxWidth > 0.0 && extents.cx > maxWidth)
                            extents = size{ maxWidth, extents.cy * maxWidth / extents.cx };
                        result = { extents.cx, extents.cy, 0.0 };
                    }
                    break;
                case image_entry::state_e::Loading:
                    result = { aFont.height(), aFont.ascender(), 0.0 };
                    break;
                case image_entry::state_e::Failed:
                    {
                        auto const extents = aGc.text_extent(string{ aObject.alt.empty() ? "[image]" : aObject.alt }, aFont);
                        result = { extents.cx, aFont.ascender(), -aFont.descender() };
                    }
                    break;
                }
            }
            // whole pixels vertically: a fractional height (math, scaled images) would put every line after it, and the
            // caret, off the pixel grid, and a partial repaint (e.g. of the flashing caret) would leave a row behind
            result.ascent = std::ceil(result.ascent);
            result.descent = std::ceil(result.descent);
            return metrics.emplace(std::move(key), result).first->second;
        }

        // images are no wider than the column: re-measure when a resize changes that for a loaded image
        bool column_width_changed(dimension aWidth) const
        {
            if (aWidth == clampWidth || widestImage <= 0.0)
                return false;
            auto const narrower = std::min(aWidth, clampWidth);
            return narrower <= 0.0 || widestImage > narrower;
        }

        object_metrics const* cached_metrics(object const& aObject, neogfx::font const& aFont) const
        {
            auto const existing = metrics.find(std::make_pair(aObject, aFont.id()));
            return existing != metrics.end() ? &existing->second : nullptr;
        }

        // give each object's glyph in a newly shaped paragraph the object's size
        static void lay_out(text_edit& aOwner, i_graphics_context const& aGc, document_text::const_iterator aParagraph, glyph_text& aGlyphs,
            std::function<neogfx::font(std::u32string::size_type)> const& aFontSelector)
        {
            if (!aOwner.iInlineObjects)
                return;
            auto const paragraphStart = std::distance(aOwner.iText.cbegin(), aParagraph);
            float shift = 0.0f;
            for (auto& g : aGlyphs.content())
            {
                if (shift != 0.0f)
                {
                    if (is_whitespace(g) && g.value == U'\t' && aGc.has_tab_stops() && aGc.tab_stops().default_stop().pos > 0.0)
                    {
                        // a tab after an object still ends on a tab stop; what follows it moves by the change in its end
                        auto const tabStop = static_cast<float>(aGc.tab_stops().default_stop().pos);
                        auto const x = g.cell[0].x + shift;
                        auto const width = tabStop - std::fmod(x, tabStop);
                        auto const oldEnd = g.cell[1].x;
                        auto const height = g.cell[3].y - g.cell[0].y;
                        auto const top = g.cell[0].y;
                        g.cell = quadf_2d{ vec2f{ x, top }, vec2f{ x + width, top }, vec2f{ x + width, top + height }, vec2f{ x, top + height } };
                        g.cellExtents = std::nullopt;
                        shift = (x + width) - oldEnd;
                        continue;
                    }
                    for (auto& v : g.cell)
                        v.x += shift;
                    g.cellExtents = std::nullopt;
                }
                auto const* object = object_at(aOwner, paragraphStart + g.clusters.first);
                if (!object)
                    continue;
                auto const objectFont = aFontSelector(g.clusters.first);
                auto const& m = aOwner.iInlineObjects->measure(aGc, *object, objectFont);
                auto const width = static_cast<float>(m.width);
                // with the font's descender, align_baselines() puts this cell's baseline at the object's ascent
                auto const height = static_cast<float>(m.ascent - objectFont.descender());
                auto const oldWidth = g.cell[1].x - g.cell[0].x;
                set_category(g, text_category::Whitespace); // the glyph itself is not drawn
                g.value = ObjectReplacementCharacter;
                g.font = objectFont.id();
                g.shape = {};
                g.outlineShape = std::nullopt;
                auto const origin = g.cell[0];
                g.cell = quadf_2d{ origin, origin + vec2f{ width, 0.0f }, origin + vec2f{ width, height }, origin + vec2f{ 0.0f, height } };
                g.cellExtents = std::nullopt;
                shift += width - oldWidth;
            }
        }

        // aLineLeft and aLineRight: where the paragraph's lines may go (inside its margins and padding), which a rule spans
        void draw(i_graphics_context& aGc, object const& aObject, neogfx::font const& aFont, point const& aBaseline, color_or_gradient const& aInk,
            coordinate aLineLeft, coordinate aLineRight)
        {
            auto const& m = measure(aGc, aObject, aFont);
            if (aObject.kind == object::kind_e::Rule)
            {
                auto const thickness = std::max(1.0, std::round(owner.dpi_scale(1.0)));
                auto const y = std::floor(aBaseline.y - (m.ascent - m.descent) / 2.0) + thickness / 2.0;
                aGc.draw_line(point{ aLineLeft, y }, point{ aLineRight, y }, pen{ aInk, thickness });
                return;
            }
            if (aObject.kind == object::kind_e::Math)
            {
                auto& entry = math_for(aObject, aFont);
                auto const& glyphs = math_glyphs(entry, aGc);
                if (!glyphs.content().empty())
                    aGc.draw_glyph_text(aBaseline, glyphs, text_format{ aInk });
                for (auto const& r : entry.formula.rules)
                    aGc.fill_rect(rect{ point{ aBaseline.x + r.x, aBaseline.y + r.y }, size{ r.width, std::max<dimension>(1.0, r.height) } }, to_brush(aInk));
                return;
            }
            rect const bounds{ point{ aBaseline.x, aBaseline.y - m.ascent }, size{ m.width, m.ascent + m.descent } };
            auto const& entry = image_for(aObject.source);
            switch (entry.state)
            {
            case image_entry::state_e::Loaded:
                aGc.draw_texture(bounds, *entry.image);
                break;
            case image_entry::state_e::Loading:
                aGc.draw_rect(bounds.deflated(size{ 1.0, 1.0 }), pen{ aInk, 1.0 });
                break;
            case image_entry::state_e::Failed:
                aGc.draw_text(point{ aBaseline.x, aBaseline.y - (aFont.height() + aFont.descender()) },
                    string{ aObject.alt.empty() ? "[image]" : aObject.alt }, aFont, text_format{ aInk });
                break;
            }
        }

        // the math object whose glyph is under a point (in widget coordinates), if any
        static std::optional<position_type> math_at(text_edit const& aOwner, point const& aPosition);

        void begin_edit(position_type aPosition);
        void end_edit(bool aCancel);

        // keep the edited range in step with changes to the text
        void text_inserted(position_type aPosition, position_type aLength)
        {
            if (!editing)
                return;
            if (aPosition <= editing->start)
            {
                editing->start += aLength;
                editing->end += aLength;
            }
            else if (aPosition < editing->end)
                editing->end += aLength;
        }
        void text_deleted(position_type aStart, position_type aEnd)
        {
            if (!editing)
                return;
            auto const adjusted = [&](position_type aPosition) { return aPosition >= aEnd ? aPosition - (aEnd - aStart) : aPosition > aStart ? aStart : aPosition; };
            editing->start = adjusted(editing->start);
            editing->end = adjusted(editing->end);
            if (editing->start == editing->end)
                editing.reset();
        }

        // the object's source, for copying as text
        static std::u32string source_text(object const& aObject)
        {
            std::string result;
            if (aObject.kind == object::kind_e::Math)
                result = aObject.display ? "$$" + aObject.source + "$$" : "$" + aObject.source + "$";
            else if (aObject.kind == object::kind_e::Rule)
                result = "---";
            else
                result = "![" + aObject.alt + "](" + aObject.source + ")";
            return neolib::utf8_to_utf32(result);
        }
    };

    text_edit::password_bits::password_bits(text_edit& aParent) :
        parent{ aParent },
        previousPadding{ aParent.padding() },
        showPassword{ aParent, image{ ":/neogfx/resources/icons/eye-outline.png" } }
    {
        showPassword.set_size_policy(size_constraint::Minimum);
        showPassword.set_image_extents(size{ 16_dip, 16_dip });
        showPassword.image_widget().set_image_color(service<i_app>().current_style().palette().color(color_role::Text));
        showPassword.show(!aParent.iText.empty());

        parent.set_padding(previousPadding.with_right(previousPadding.right + showPassword.minimum_size().cx));

        showPassword.pressed([&]() { aParent.refresh_paragraph(aParent.iText.begin(), 0); });
        showPassword.released([&]() { aParent.refresh_paragraph(aParent.iText.begin(), 0); });
    }

    text_edit::password_bits::~password_bits()
    {
        parent.set_padding(previousPadding);
    }

    class text_edit::dragger : public widget_timer
    {
    public:
        dragger(text_edit& aOwner) :
            widget_timer{ aOwner, [&](widget_timer& aTimer)
            {
                aTimer.again();
                aOwner.set_cursor_position(aOwner.mouse_position(), false);
            }, std::chrono::milliseconds{ 250 } },
            iSts1{ aOwner.vertical_scrollbar().Position },
            iSts2{ aOwner.horizontal_scrollbar().Position }
            {
            }
            ~dragger()
            {
            }
    private:
        scoped_property_transition_suppression iSts1;
        scoped_property_transition_suppression iSts2;
    };

    text_edit::character_style::character_style() :
        iSmartUnderline{ false },
        iIgnoreEmoji{ true }
    {
    }

    text_edit::character_style::character_style(character_style const& aOther) :
        iFont{ aOther.iFont },
        iGlyphColor{ aOther.iGlyphColor },
        iTextColor{ aOther.iTextColor },
        iPaperColor{ aOther.iPaperColor },
        iSmartUnderline{ aOther.iSmartUnderline },
        iIgnoreEmoji{ aOther.iIgnoreEmoji },
        iTextEffect{ aOther.iTextEffect },
        iTextAnimation{ aOther.iTextAnimation }
    {
    }
        
    text_edit::character_style::character_style(
        optional_font const& aFont,
        const color_or_gradient& aTextColor,
        const color_or_gradient& aPaperColor,
        const optional_text_effect& aTextEffect,
        const optional_text_animation& aTextAnimation) :
        iFont{ aFont },
        iTextColor{ aTextColor },
        iPaperColor{ aPaperColor },
        iSmartUnderline{ false },
        iIgnoreEmoji{ true },
        iTextEffect{ aTextEffect },
        iTextAnimation{ aTextAnimation }
    {
    }

    optional_font const& text_edit::character_style::font() const
    {
        return iFont;
    }

    const color_or_gradient& text_edit::character_style::glyph_color() const
    {
        return iGlyphColor;
    }

    const color_or_gradient& text_edit::character_style::text_color() const
    {
        return iTextColor;
    }

    const color_or_gradient& text_edit::character_style::paper_color() const
    {
        return iPaperColor;
    }

    bool text_edit::character_style::smart_underline() const
    {
        return iSmartUnderline;
    }

    bool text_edit::character_style::ignore_emoji() const
    {
        return iIgnoreEmoji;
    }

    const optional_text_effect& text_edit::character_style::text_effect() const
    {
        return iTextEffect;
    }

    const optional_text_animation& text_edit::character_style::text_animation() const
    {
        return iTextAnimation;
    }

    text_format text_edit::character_style::as_text_format() const
    {
        return text_format{ 
            glyph_color() != neolib::none ? glyph_color() : text_color(), 
            paper_color() != neolib::none ? optional_text_color{ paper_color() } : optional_text_color{}, 
            text_effect(),
            text_animation() }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
    }

    text_edit::character_style& text_edit::character_style::set_font(optional_font const& aFont)
    {
        iFont = aFont;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_font_if_none(neogfx::font const& aFont)
    {
        if (iFont == std::nullopt)
            iFont = aFont;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_glyph_color(const color_or_gradient& aColor)
    {
        iGlyphColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_text_color(const color_or_gradient& aColor)
    {
        iTextColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_paper_color(const color_or_gradient& aColor)
    {
        iPaperColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_text_effect(const optional_text_effect& aEffect)
    {
        iTextEffect = aEffect;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_text_animation(const optional_text_animation& aAnimation)
    {
        iTextAnimation = aAnimation;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_from_text_format(const text_format& aTextFormat)
    {
        iGlyphColor = aTextFormat.ink();
        iTextColor = neolib::none; // todo
        iPaperColor = aTextFormat.paper() ? *aTextFormat.paper() : neolib::none;
        iSmartUnderline = aTextFormat.smart_underline();
        iIgnoreEmoji = aTextFormat.ignore_emoji();
        iTextEffect = aTextFormat.effect();
        iTextAnimation = aTextFormat.animation();
        return *this;
    }

    text_edit::character_style& text_edit::character_style::merge(const character_style& aRhs)
    {
        if (aRhs.font() != std::nullopt)
            iFont = aRhs.font();
        if (aRhs.glyph_color() != neolib::none)
            iGlyphColor = aRhs.glyph_color();
        if (aRhs.text_color() != neolib::none)
            iTextColor = aRhs.text_color();
        if (aRhs.paper_color() != neolib::none)
            iPaperColor = aRhs.paper_color();
        iSmartUnderline = aRhs.smart_underline();
        iIgnoreEmoji = aRhs.ignore_emoji();
        if (aRhs.text_effect() != std::nullopt)
            iTextEffect = aRhs.text_effect();
        if (aRhs.text_animation() != std::nullopt)
            iTextAnimation = aRhs.text_animation();
        return *this;
    }

    bool text_edit::character_style::operator==(const character_style& aRhs) const
    {
        return std::forward_as_tuple(iFont, iGlyphColor, iTextColor, iPaperColor, iSmartUnderline, iIgnoreEmoji, iTextEffect, iTextAnimation) == 
            std::forward_as_tuple(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iSmartUnderline, aRhs.iIgnoreEmoji, aRhs.iTextEffect, aRhs.iTextAnimation);
    }

    bool text_edit::character_style::operator!=(const character_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::character_style::operator<(const character_style& aRhs) const
    {
        return std::forward_as_tuple(iFont, iGlyphColor, iTextColor, iPaperColor, iSmartUnderline, iIgnoreEmoji, iTextEffect, iTextAnimation) < 
            std::forward_as_tuple(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iSmartUnderline, aRhs.iIgnoreEmoji, aRhs.iTextEffect, aRhs.iTextAnimation);
    }

    text_edit::paragraph_style::paragraph_style()
    {
    }

    text_edit::paragraph_style::paragraph_style(paragraph_style const& aOther) :
        iAlignment{ aOther.iAlignment },
        iPadding{ aOther.iPadding },
        iLineSpacing{ aOther.iLineSpacing },
        iMargin{ aOther.iMargin }
    {
    }

    text_edit::paragraph_style::paragraph_style(optional_alignment const& aAlignment, optional_padding const& aPadding, optional<double> const& aLineSpacing) :
        iAlignment{ aAlignment },
        iPadding{ aPadding },
        iLineSpacing{ aLineSpacing }
    {
    }

    optional_alignment const& text_edit::paragraph_style::alignment() const
    {
        return iAlignment;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_alignment(optional_alignment const& aAlignment)
    {
        iAlignment = aAlignment;
        return *this;
    }

    optional_padding const& text_edit::paragraph_style::padding() const
    {
        return iPadding;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_padding(optional_padding const& aPadding)
    {
        iPadding = aPadding;
        return *this;
    }

    optional_margin const& text_edit::paragraph_style::margin() const
    {
        return iMargin;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_margin(optional_margin const& aMargin)
    {
        iMargin = aMargin;
        return *this;
    }

    optional<double> const& text_edit::paragraph_style::line_spacing() const
    {
        return iLineSpacing;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_line_spacing(optional<double> const& aLineSpacing)
    {
        iLineSpacing = aLineSpacing;
        return *this;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::merge(const paragraph_style& aRhs)
    {
        if (aRhs.alignment() != std::nullopt)
            iAlignment = aRhs.alignment();
        if (aRhs.padding() != std::nullopt)
            iPadding = aRhs.padding();
        if (aRhs.line_spacing() != std::nullopt)
            iLineSpacing = aRhs.line_spacing();
        if (aRhs.margin() != std::nullopt)
            iMargin = aRhs.margin();
        return *this;
    }

    bool text_edit::paragraph_style::operator==(const paragraph_style& aRhs) const
    {
        return std::forward_as_tuple(iAlignment, iPadding, iLineSpacing, iMargin) == std::forward_as_tuple(aRhs.iAlignment, aRhs.iPadding, aRhs.iLineSpacing, aRhs.iMargin);
    }

    bool text_edit::paragraph_style::operator!=(const paragraph_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::paragraph_style::operator<(const paragraph_style& aRhs) const
    {
        return std::forward_as_tuple(iAlignment, iPadding, iLineSpacing, iMargin) < std::forward_as_tuple(aRhs.iAlignment, aRhs.iPadding, aRhs.iLineSpacing, aRhs.iMargin);
    }

    text_edit::style::style() : 
        iParent{ nullptr },
        iUseCount{ 0 }
    {
    }

    text_edit::style::style(character_style const& aCharacter) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iCharacter{ aCharacter }
    {
    }
        
    text_edit::style::style(character_style const& aCharacter, paragraph_style const& aParagraph) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iCharacter{ aCharacter },
        iParagraph{ aParagraph }
    {
    }

    text_edit::style::style(paragraph_style const& aParagraph) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iParagraph{ aParagraph }
    {
    }

    text_edit::style::style(text_edit& aParent, const style& aOther) :
        iParent{ &aParent },
        iUseCount{ 0 },
        iCharacter{ aOther.iCharacter },
        iParagraph{ aOther.iParagraph }
    {
    }

    bool text_edit::style::has_cookie() const
    {
        return iCookie != neolib::invalid_cookie<style_cookie>;
    }

    text_edit::style_cookie text_edit::style::cookie() const
    {
        return iCookie;
    }

    void text_edit::style::set_cookie(style_cookie aCookie)
    {
        iCookie = aCookie;
    }

    void text_edit::style::add_ref() const
    {
        ++iUseCount;
    }

    void text_edit::style::release() const
    {
        if (iParent && &iParent->iDefaultStyle != this && --iUseCount == 0u)
        {
            iParent->iStyles.erase(iParent->iStyleMap[cookie()]);
            iParent->iStyleMap.remove(cookie());
        }
    }

    text_edit::style& text_edit::style::merge(const style& aOverridingStyle)
    {
        iCharacter.merge(aOverridingStyle.iCharacter);
        iParagraph.merge(aOverridingStyle.iParagraph);
        return *this;
    }

    bool text_edit::style::operator==(const style& aRhs) const
    {
        return std::forward_as_tuple(iCharacter, iParagraph) == std::forward_as_tuple(aRhs.iCharacter, aRhs.iParagraph);
    }

    bool text_edit::style::operator!=(const style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::style::operator<(const style& aRhs) const
    {
        return std::forward_as_tuple(iCharacter, iParagraph) < std::forward_as_tuple(aRhs.iCharacter, aRhs.iParagraph);
    }

    text_edit::character_style const& text_edit::style::character() const
    {
        return iCharacter;
    }

    text_edit::character_style& text_edit::style::character()
    {
        return iCharacter;
    }

    text_edit::paragraph_style const& text_edit::style::paragraph() const
    {
        return iParagraph;
    }

    text_edit::paragraph_style& text_edit::style::paragraph()
    {
        return iParagraph;
    }

    text_edit::style text_edit::style::with_font(optional_font const& aFont) const
    {
        style newStyle = *this;
        newStyle.character().set_font(aFont);
        return newStyle;
    }

    text_edit::style text_edit::style::with_glyph_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_glyph_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_text_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_text_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_paper_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_paper_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_text_effect(const optional_text_effect& aEffect) const
    {
        style newStyle = *this;
        newStyle.character().set_text_effect(aEffect);
        return newStyle;
    }

    text_edit::style text_edit::style::with_text_animation(const optional_text_animation& aAnimation) const
    {
        style newStyle = *this;
        newStyle.character().set_text_animation(aAnimation);
        return newStyle;
    }

    class text_edit::multiple_text_changes
    {
    public:
        multiple_text_changes(text_edit& aOwner) : 
            iOwner(aOwner)
        {
            ++iOwner.iSuppressTextChangedNotification;
        }
        ~multiple_text_changes()
        {
            if (--iOwner.iSuppressTextChangedNotification == 0u)
            {
                bool notify = (iOwner.iWantedToNotifyTextChanged > 0u);
                iOwner.iWantedToNotifyTextChanged = 0u;
                if (notify)
                    iOwner.TextChanged();
            }
        }
    private:
        text_edit & iOwner;
    };

    struct text_edit::position_info
    {
        glyph_paragraphs::const_iterator paragraph;
        std::optional<glyph_columns::const_iterator> column;
        std::optional<glyph_lines::const_iterator> line;
        std::optional<document_glyphs::const_iterator> glyph;
        std::optional<document_glyphs::const_iterator> lineStart;
        std::optional<document_glyphs::const_iterator> lineEnd;
        point pos;

        glyph_columns::size_type column_index() const
        {
            return column.has_value() ? column.value()->index() : 0;
        }
    };

    void text_edit::inline_objects::begin_edit(position_type aPosition)
    {
        if (editing)
        {
            // finish another edit first; it may move this formula
            auto const previous = *editing;
            auto const sizeBefore = static_cast<position_type>(owner.iText.size());
            end_edit(false);
            if (aPosition >= previous.end)
                aPosition -= sizeBefore - static_cast<position_type>(owner.iText.size());
        }
        auto const* object = object_at(owner, aPosition);
        if (!object || object->kind != object::kind_e::Math)
            return;
        auto const original = *object;
        auto const originalStyle = owner.style_at(aPosition);
        std::string const delimiter = original.display ? "$$" : "$";
        auto editStyle = originalStyle;
        editStyle.character().set_paper_color(owner.default_text_color().with_alpha(0.12));
        {
            multiple_text_changes mtc{ owner };
            owner.delete_text(aPosition, aPosition + 1);
            auto const length = static_cast<position_type>(owner.insert_text(aPosition, string{ delimiter + original.source + delimiter }, format{ editStyle }));
            editing = edit_state{ aPosition, aPosition + length, original, originalStyle };
        }
        // the caret goes before the closing delimiter
        owner.cursor().set_position(editing->end - static_cast<position_type>(delimiter.size()));
    }

    void text_edit::inline_objects::end_edit(bool aCancel)
    {
        if (!editing)
            return;
        auto const state = *editing;
        editing.reset(); // before changing the text: the changes below must not be tracked as edits
        auto const size = static_cast<position_type>(owner.iText.size());
        auto const start = std::clamp<position_type>(state.start, 0, size);
        auto const end = std::clamp<position_type>(state.end, start, size);
        auto result = state.original;
        if (!aCancel)
        {
            std::u32string text;
            for (auto i = start; i != end; ++i)
                text += owner.iText[i].character;
            auto const source = neolib::utf32_to_utf8(text);
            auto const trim = [](std::string_view aText)
            {
                auto const first = aText.find_first_not_of(" \t\r\n");
                if (first == std::string_view::npos)
                    return std::string_view{};
                return aText.substr(first, aText.find_last_not_of(" \t\r\n") - first + 1);
            };
            auto const trimmed = trim(source);
            if (trimmed.size() >= 4 && trimmed.starts_with("$$") && trimmed.ends_with("$$"))
            {
                result.display = true;
                result.source = trim(trimmed.substr(2, trimmed.size() - 4));
            }
            else if (trimmed.size() >= 2 && trimmed.front() == '$' && trimmed.back() == '$')
            {
                result.display = false;
                result.source = trim(trimmed.substr(1, trimmed.size() - 2));
            }
            else
            {
                // the delimiters were removed: the formula is now ordinary text
                owner.apply_style(start, end, state.originalStyle);
                return;
            }
        }
        auto const position = owner.cursor().position();
        auto const anchor = owner.cursor().anchor();
        bool const remove = result.source.empty();
        {
            multiple_text_changes mtc{ owner };
            owner.delete_text(start, end);
            if (!remove)
            {
                format objectFormat;
                objectFormat.style = state.originalStyle;
                objectFormat.tag = owner.create_tag(inline_object_ttid(), result).first;
                owner.insert_text(start, string{ "\xEF\xBF\xBC" }, objectFormat);
            }
        }
        // keep the caret where it was relative to the text around the formula
        auto const removed = (end - start) - (remove ? 0 : 1);
        auto const adjusted = [&](position_type aPosition)
        {
            return aPosition >= end ? aPosition - removed : aPosition > start ? start + (remove ? 0 : 1) : aPosition;
        };
        owner.cursor().set_anchor(adjusted(anchor));
        owner.cursor().set_position(adjusted(position), false);
    }

    std::optional<text_edit::position_type> text_edit::inline_objects::math_at(text_edit const& aOwner, point const& aPosition)
    {
        if (!aOwner.iInlineObjects || aOwner.glyphs().empty())
            return {};
        // the hit test gives the nearest caret position: the formula is the character after or before it
        auto const position = aOwner.document_hit_test(aPosition);
        for (auto const candidate : { position, position - 1 })
        {
            auto const* object = object_at(aOwner, candidate);
            if (!object || object->kind != object::kind_e::Math)
                continue;
            auto const glyph = aOwner.to_glyph(std::next(aOwner.iText.begin(), candidate));
            if (glyph == aOwner.glyphs().end())
                continue;
            auto const glyphIndex = std::distance(aOwner.glyphs().begin(), glyph);
            auto const info = aOwner.glyph_position(glyphIndex, true);
            auto const columnRect = aOwner.column_rect(info.column_index());
            auto const left = info.pos.x - aOwner.horizontal_scrollbar().position() + columnRect.x;
            if (aPosition.x < left || aPosition.x >= left + glyph->cell_extents().x)
                continue;
            if (info.line)
            {
                auto const top = (*info.line)->ypos() - aOwner.vertical_scrollbar().position() + columnRect.y;
                if (aPosition.y < top || aPosition.y >= top + (*info.line)->extents.cy)
                    continue;
            }
            return candidate;
        }
        return {};
    }

    dimension text_edit::glyph_paragraph::height(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const
    {
        if (heightMap.empty())
        {
            dimension previousHeight = 0.0;
            auto iterGlyph = std::next(owner->glyphs().begin(), span.glyphsFirst);
            for (auto i = span.glyphsFirst; i != span.glyphsLast; ++i)
            {
                auto const& glyph = *(iterGlyph++);
                dimension cy = owner->glyphs().extents(glyph).cy;
                if (i == span.glyphsFirst || cy != previousHeight)
                {
                    heightMap.emplace_back(i, cy);
                    previousHeight = cy;
                }
            }
            heightMap.emplace_back(span.glyphsLast, 0.0);
        }
        dimension result = 0.0;
        auto start = std::lower_bound(heightMap.begin(), heightMap.end(), height_map_entry{ aStart - owner->glyphs().begin() },
            [](auto const& lhs, auto const& rhs) { return lhs.glyphIndex < rhs.glyphIndex; });
        if (start != heightMap.begin() && aStart < owner->glyphs().begin() + start->glyphIndex)
            --start;
        auto stop = std::lower_bound(heightMap.begin(), heightMap.end(), height_map_entry{ aEnd - owner->glyphs().begin() },
            [](auto const& lhs, auto const& rhs) { return lhs.glyphIndex < rhs.glyphIndex; });
        if (start == stop && stop != heightMap.end())
            ++stop;
        for (auto i = start; i != stop; ++i)
            result = std::max(result, (*i).height);
        return result;
    }

    text_edit::text_edit(text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingContents{ false },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
        iBookmarkIcon{ image{ ":/neogfx/resources/icons/bookmark_small.png", 1.0, texture_sampling::Normal } },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_widget& aParent, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aParent, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingContents{ false },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
        iBookmarkIcon{ image{ ":/neogfx/resources/icons/bookmark_small.png", 1.0, texture_sampling::Normal } },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_layout& aLayout, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aLayout, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingContents{ false },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
        iBookmarkIcon{ image{ ":/neogfx/resources/icons/bookmark_small.png", 1.0, texture_sampling::Normal } },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::~text_edit()
    {
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
    }

    void text_edit::moved()
    {
        framed_scrollable_widget::moved();
    }

    void text_edit::resized()
    {
        framed_scrollable_widget::resized();
        if (iInlineObjects && iInlineObjects->column_width_changed(column_rect(0).width()))
        {
            iInlineObjects->metrics.clear();
            refresh_paragraph(iText.begin(), 0);
        }
    }

    void text_edit::layout_items(bool aDefer)
    {
        framed_scrollable_widget::layout_items(aDefer);
        if (!aDefer && iPasswordBits)
        {
            auto r = client_rect();
            r.x = r.right() - padding().right;
            auto& spb = iPasswordBits.value().showPassword;
            spb.resize(spb.minimum_size());
            spb.move(point{ r.x, std::ceil(r.cy - spb.extents().cy) / 2.0 });
        }
    }

    size text_edit::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        scoped_units su{ *this, units::Pixels };
        auto const childLayoutSize = has_layout() ? layout().minimum_size(aAvailableSpace) : size{};
        auto const frameBits = framed_scrollable_widget::minimum_size(aAvailableSpace) - childLayoutSize;
        auto result = frameBits;
        if (!size_hint())
            result += size{ font().height() }.max(childLayoutSize);
        else
        {
            if (iHintedSize == std::nullopt || iHintedSize->first != font())
            {
                iHintedSize.emplace(font(), size{});
                graphics_context gc{ *this, graphics_context::type::Unattached };
                iHintedSize->second = gc.text_extent(size_hint().primary_hint(), font()).max(gc.text_extent(size_hint().secondary_hint(), font()));
                if (iHintedSize->second.cy == 0.0)
                    iHintedSize->second.cy = font().height();
            }
            result += iHintedSize->second.max(childLayoutSize);
        }
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::GrowLines)
            result.cy = std::max(result.cy, frameBits.cy + std::min(iTextExtents.value().cy, grow_lines() * font().height()));
        return to_units(*this, su.saved_units(), result);
    }

    size text_edit::maximum_size(optional_size const& aAvailableSpace) const
    {
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::MultiLine || has_maximum_size())
            return framed_scrollable_widget::maximum_size(aAvailableSpace);
        return size{ framed_scrollable_widget::maximum_size(aAvailableSpace).cx, minimum_size(aAvailableSpace).cy };
    }

    neogfx::padding text_edit::padding() const
    {
        scoped_units su{ *this, units::Pixels };
        auto result = framed_scrollable_widget::padding();
        if (iRichText)
            result += iMarkupPadding.value_or(neogfx::padding{ dpi_scale(8.0) }); // CSS default: body { margin: 8px }
        return to_units(*this, su.saved_units(), result + padding_adjust());
    }

    void text_edit::paint(i_graphics_context& aGc) const
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_edit::paint(...)" << std::endl;
#endif // NEOGFX_DEBUG

        framed_scrollable_widget::paint(aGc);
        // the padding is included: glyphs can draw beyond their cells (italic overhangs, text effects)
        rect const clipRect = default_clip_rect().intersection(client_rect(true));

        if (iOutOfMemory)
        {
            draw_alpha_background(aGc, clipRect);
            return;
        }

        scoped_scissor scissor{ aGc, clipRect };
        if (default_style().character().paper_color() != neolib::none)
            aGc.fill_rect(client_rect(), to_brush(default_style().character().paper_color()));

        auto const top = vertical_scrollbar().position();

        iHasAnimations = false;

        coordinate x = 0.0;
        for (auto columnIndex = 0u; columnIndex < iGlyphColumns.size(); ++columnIndex)
        {
            auto const& columnRectSansPadding = column_rect(columnIndex);

            auto const& glyphColumn = iGlyphColumns[columnIndex];

            // a column clips its neighbours' glyphs but not the widget's padding: the first column extends into the left padding,
            // every column to the right of the page (a paragraph's last segment spans the remaining columns) and into the
            // padding there, and into the top and bottom padding
            auto columnExtent = column_rect(columnIndex, true);
            if (columnIndex == 0)
                columnExtent.x = clipRect.x;
            columnExtent.cx = std::max(0.0, clipRect.right() - columnExtent.x);
            columnExtent.y = clipRect.y;
            columnExtent.cy = clipRect.cy;
            auto columnClipRect = clipRect.intersection(columnExtent);
            columnClipRect.inflate(size{ std::max(calc_padding_adjust(column_style(columnIndex)), padding_adjust()) });
            scoped_scissor scissor2{ aGc, columnClipRect };

            auto const& lines = glyphColumn.lines;
            auto line = std::lower_bound(lines.begin(), lines.end(), top,
                [](const glyph_line& left, coordinate const& right) { return left.ypos() < right; });
            if (line != lines.begin() && (line == lines.end() || top < line->ypos()))
                --line;
            if (line == lines.end())
                continue;
            for (auto paintLine = line; paintLine != lines.end(); paintLine++)
            {
                point linePos = columnRectSansPadding.top_left() + point{ -horizontal_scrollbar().position(), paintLine->ypos() - top};
                if (linePos.y + paintLine->extents.cy < clipRect.top() || linePos.y + paintLine->extents.cy < update_rect().top())
                    continue;
                if (linePos.y > clipRect.bottom() || linePos.y > update_rect().bottom())
                    break;
                linePos.x += paintLine->xpos();
                draw_glyphs(aGc, linePos, glyphColumn, paintLine);
            }
            x += glyphColumn.width;
        }

        for (auto columnIndex = 0u; columnIndex < iColumns.size(); ++columnIndex)
            if (iColumns[columnIndex].info.separator)
            {
                auto const separatorX = std::floor(column_rect(columnIndex, true).right() - horizontal_scrollbar().position()) + 0.5;
                aGc.draw_line(point{ separatorX, client_rect(false).top() }, point{ separatorX, client_rect(false).bottom() },
                    pen{ iColumns[columnIndex].info.separator.value(), false });
            }

        draw_bookmarks(aGc);

        if (has_focus() && !read_only())
            draw_cursor(aGc);
    }

    color text_edit::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return framed_scrollable_widget::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return palette_color(color_role::Base);
        return framed_scrollable_widget::palette_color(aColorRole);
    }

    const font& text_edit::font() const
    {
        return default_style().character().font() != std::nullopt ? *default_style().character().font() : framed_scrollable_widget::font();
    }

    void text_edit::set_font(optional_font const& aFont)
    {
        framed_scrollable_widget::set_font(aFont);
        if (!default_style().character().font())
        {
            glyphs().set_major_font(font());
            refresh_paragraph(iText.begin(), 0);
        }
    }

    bool text_edit::capture_locks_cursor() const
    {
        return false;
    }

    void text_edit::focus_gained(focus_reason aFocusReason)
    {
        framed_scrollable_widget::focus_gained(aFocusReason);

        if (neolib::service<i_keyboard>().layout().ime_open())
            neolib::service<i_keyboard>().layout().activate_ime(*this, cursor_rect().bottom_left());

        neolib::service<neolib::i_power>().register_activity();

        service<i_clipboard>().activate(*this);

        iCursorAnimationStartTime = neolib::this_process::elapsed_ms();

        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine && aFocusReason == focus_reason::Tab)
        {
            cursor().set_anchor(0);
            cursor().set_position(iText.size(), false);
        }

        if ((iCaps & text_edit_caps::TranslateEmoticons) == text_edit_caps::TranslateEmoticons)
            service<i_emoticon_translator>().activate(*this);

        update(true);
    }

    void text_edit::focus_lost(focus_reason aFocusReason)
    {
        destroyed_flag destroyed{ *this };

        if (iInlineObjects)
            iInlineObjects->end_edit(false);

        framed_scrollable_widget::focus_lost(aFocusReason);

        if (destroyed)
            return;

        if (neolib::service<i_keyboard>().layout().ime_open())
            neolib::service<i_keyboard>().layout().deactivate_ime(*this);

        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);

        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
            cursor().set_position(iText.size());

        if ((iCaps & text_edit_caps::TranslateEmoticons) == text_edit_caps::TranslateEmoticons &&
            service<i_emoticon_translator>().active(*this))
            service<i_emoticon_translator>().deactivate();

        update(true);
    }

    void text_edit::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        framed_scrollable_widget::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto const docPos = document_hit_test_ex(aPosition);
            if ((iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() && docPos.second)
            {
                auto wordSpan = word_at(docPos.first, true);
                thread_local std::u32string utf32word;
                utf32word.clear();
                std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                if (utf32word.find(U"://") != std::u32string::npos)
                    iSelectedUri = wordSpan;
            }
            set_cursor_position(aPosition, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None, capturing());
        }
        else if (aButton == mouse_button::Right && focus_policy() == neogfx::focus_policy::NoFocus && capture_ok(hit_test(aPosition)) && can_capture())
        {
            set_capture(capture_reason::MouseEvent, aPosition);
            if (cursor().position() == cursor().anchor())
                set_cursor_position(aPosition, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None, capturing());
        }
    }

    void text_edit::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        framed_scrollable_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifier);
        if (!read_only() && aButton == mouse_button::Left && client_rect().contains(aPosition))
            if (auto const formula = inline_objects::math_at(*this, aPosition))
            {
                // edit the formula's source in place
                iInlineObjects->begin_edit(*formula);
                return;
            }
        if (!password() && aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto word = word_at(document_hit_test(aPosition));
            cursor().set_anchor(word.first);
            cursor().set_position(word.second, false);
        }
    }
    
    void text_edit::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool wasCapturing = capturing();
        framed_scrollable_widget::mouse_button_released(aButton, aPosition);
        iDragger = nullptr;
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto const docPos = document_hit_test_ex(aPosition);
            if ((iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() && docPos.second &&
                cursor().position() == cursor().anchor() && iSelectedUri)
            {
                auto wordSpan = word_at(docPos.first, true);
                if (iSelectedUri == wordSpan)
                {
                    thread_local std::u32string utf32word;
                    utf32word.clear();
                    std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                    if (utf32word.find(U"://") != std::u32string::npos)
                    {
                        if (!service<i_basic_services>().open_uri(neolib::utf32_to_utf8(utf32word)))
                            service<i_basic_services>().system_beep();
                    }
                }
            }
        }
        else if (aButton == mouse_button::Right)
        {
            iMenu = std::make_unique<neogfx::context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
            ContextMenu(iMenu->menu());
            if (!read_only())
                if (auto const formula = inline_objects::math_at(*this, aPosition))
                {
                    iMenu->menu().add_action(make_ref<action>("Edit Math"_t)).triggered(
                        [this, position = *formula, formulaObject = *inline_objects::object_at(*this, *formula)]()
                    {
                        if (!iInlineObjects)
                            return;
                        // the text may have changed since the menu opened (e.g. another edit finished): find the formula again
                        auto target = position;
                        auto const* atTarget = inline_objects::object_at(*this, target);
                        if (!atTarget || *atTarget != formulaObject)
                        {
                            std::optional<position_type> nearest;
                            for (position_type i = 0; i < static_cast<position_type>(iText.size()); ++i)
                                if (auto const* candidate = inline_objects::object_at(*this, i); candidate && *candidate == formulaObject)
                                    if (!nearest || std::abs(i - position) < std::abs(*nearest - position))
                                        nearest = i;
                            if (!nearest)
                                return;
                            target = *nearest;
                        }
                        iInlineObjects->begin_edit(target);
                    });
                    iMenu->menu().add_separator();
                }
            if (!read_only())
            {
                iMenu->menu().add_action(service<i_app>().action_undo());
                iMenu->menu().add_action(service<i_app>().action_redo());
                iMenu->menu().add_separator();
                iMenu->menu().add_action(service<i_app>().action_cut());
            }
            iMenu->menu().add_action(service<i_app>().action_copy());
            if (!read_only())
            {
                iMenu->menu().add_action(service<i_app>().action_paste());
            }
            auto pastePlainText = make_ref<action>("Plain Text"_t);
            auto pasteRichText = make_ref<action>("Rich Text (HTML)"_t);
            auto pasteMarkdown = make_ref<action>("Rich Text (Markdown)"_t);
            pastePlainText->Triggered([this]() { paste_plain_text(); });
            pasteRichText->Triggered([this]() { paste_rich_text(); });
            pasteMarkdown->Triggered([this]() { paste_rich_text(rich_text_format::Markdown); });
            bool const richText = (iCaps & text_edit_caps::AllowRichText) == text_edit_caps::AllowRichText;
            if (!richText)
            {
                pasteRichText->disable();
                pasteMarkdown->disable();
            }
            sink tempSink;
            tempSink += service<i_app>().action_paste().enabled([&pastePlainText, &pasteRichText, &pasteMarkdown, richText]()
                { pastePlainText->enable(); if (richText) { pasteRichText->enable(); pasteMarkdown->enable(); } });
            tempSink += service<i_app>().action_paste().disabled([&pastePlainText, &pasteRichText, &pasteMarkdown]() { pastePlainText->disable(); pasteRichText->disable(); pasteMarkdown->disable(); });
            if (!read_only())
            {
                auto& pasteAs = iMenu->menu().add_sub_menu("Paste As"_t);
                pasteAs.add_action(pastePlainText);
                pasteAs.add_action(pasteRichText);
                pasteAs.add_action(pasteMarkdown);
                iMenu->menu().add_action(service<i_app>().action_delete());
            }
            iMenu->menu().add_separator();
            iMenu->menu().add_action(service<i_app>().action_select_all());
            if (!read_only())
            {
                iMenu->menu().add_separator();
                if (!neolib::service<i_keyboard>().layout().ime_open())
                {
                    iMenu->menu().add_action(make_ref<action>("Open IME"_t)).triggered([&]()
                    {
                        neolib::service<i_keyboard>().layout().open_ime();
                        neolib::service<i_keyboard>().layout().activate_ime(*this, cursor_rect().bottom_left());
                    });
                }
                else
                {
                    iMenu->menu().add_action(make_ref<action>("Close IME"_t)).triggered([&]()
                    {
                        neolib::service<i_keyboard>().layout().close_ime();
                    });
                }
            }
            bool selectAll = false;
            tempSink += service<i_app>().action_select_all().triggered([&]() { selectAll = true; });
            scoped_clipboard_sink scs{ *this };
            iMenu->exec();
            iMenu.reset();
            if (!selectAll && focus_policy() == neogfx::focus_policy::NoFocus && wasCapturing)
                cursor().set_anchor(cursor().position());
        }
        iSelectedUri = std::nullopt;
    }

    void text_edit::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        framed_scrollable_widget::mouse_moved(aPosition, aKeyModifier);
        neolib::service<neolib::i_power>().register_activity();
        if (iDragger != nullptr)
            set_cursor_position(aPosition, false);
        // the full URL of an external link under the mouse, in the status bar
        std::optional<std::string> link;
        if (iDragger == nullptr && client_rect(false).contains(aPosition))
        {
            auto const docPos = document_hit_test_ex(aPosition);
            if (docPos.second && docPos.first < static_cast<position_type>(iText.size()) && iText[docPos.first].tag != neolib::invalid_cookie<tag_cookie>)
                if (auto const existing = iTagMap.find(iText[docPos.first].tag); existing != iTagMap.end() && (**existing).ttid() == link_tag_ttid())
                    if (auto const& uri = *static_cast<std::string const*>((**existing).data()); is_external_link(uri))
                        link = uri;
        }
        if (link && !iInlineObjects)
            iInlineObjects = std::make_unique<inline_objects>(*this);
        if (iInlineObjects)
            iInlineObjects->hover_link(link);
    }

    void text_edit::mouse_entered(const point& aPosition)
    {
        framed_scrollable_widget::mouse_entered(aPosition);
    }

    void text_edit::mouse_left()
    {
        if (iInlineObjects)
            iInlineObjects->hover_link(std::nullopt);
        framed_scrollable_widget::mouse_left();
    }

    neogfx::mouse_cursor text_edit::mouse_cursor() const
    {
        auto const mousePosition = mouse_position();
        auto const clientRect = client_rect(false);
        if (clientRect.contains(mousePosition) || iDragger != nullptr)
        {
            auto const docPos = document_hit_test_ex(mousePosition);
            if (!docPos.second)
                return mouse_system_cursor::IBeam;
            if (iText[docPos.first].tag != neolib::invalid_cookie<tag_cookie> &&
                cursor().position() == cursor().anchor())
            {
                auto const& tag = *iTagMap[iText[docPos.first].tag];
                if (tag.query_mouse_cursor().has_slots())
                {
                    neogfx::mouse_cursor cursor = mouse_system_cursor::IBeam;
                    tag.ev_query_mouse_cursor().trigger(cursor);
                    return cursor;
                }
                else if (tag.mouse_event().has_slots())
                    return mouse_system_cursor::Hand;
            }
            if ((iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() &&
                cursor().position() == cursor().anchor())
            {
                auto wordSpan = word_at(docPos.first, true);
                thread_local std::u32string utf32word;
                utf32word.clear();
                std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                if (utf32word.find(U"://") != std::u32string::npos)
                    return mouse_system_cursor::Hand;
            }
            return mouse_system_cursor::IBeam;
        }
        else
            return framed_scrollable_widget::mouse_cursor();
    }

    bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier)
    {
        neolib::service<neolib::i_power>().register_activity();

        if (iInlineObjects && iInlineObjects->editing &&
            (aScanCode == ScanCode_RETURN || aScanCode == ScanCode_KEYPAD_ENTER || aScanCode == ScanCode_ESCAPE))
        {
            // Enter typesets the formula being edited; Escape abandons the changes
            iInlineObjects->end_edit(aScanCode == ScanCode_ESCAPE);
            return true;
        }

        neolib::scoped_flag sf{ iHandlingKeyPress };

        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_TAB:
            if (!read_only())
            {
                multiple_text_changes mtc{ *this };
                delete_any_selection();
                insert_text(string{ "\t" }, { next_style() });
                cursor().set_position(cursor().position() + 1);
            }
            break;
        case ScanCode_RETURN:
        case ScanCode_KEYPAD_ENTER:
            if (!read_only())
            {
                if ((aKeyModifier & key_modifier::CTRL) == key_modifier::None)
                {
                    bool canAccept = false;
                    CanAcceptText(text(), canAccept);
                    if (canAccept)
                    {
                        AcceptText(text());
                        break;
                    }
                    else if ((iCaps & text_edit_caps::OnlyAccept) == text_edit_caps::OnlyAccept)
                        break;
                }
                if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
                {
                    multiple_text_changes mtc{ *this };
                    delete_any_selection();
                    if ((aKeyModifier & key_modifier::SHIFT) == key_modifier::None)
                        insert_text(string{ "\n" }, { next_style() });
                    else
                        insert_text(string{ "\r" }, { next_style() });
                    cursor().set_position(cursor().position() + 1);
                    iCursorHint.x = std::nullopt;
                }
                else
                    handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
            }
            else
                handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
            break;
        case ScanCode_BACKSPACE:
            if (!read_only())
            {
                if (cursor().position() == cursor().anchor())
                {
                    if (cursor().position() > 0)
                    {
                        auto const from = cursor().position() - 1;
                        auto const to = cursor().position();
                        if (iInlineObjects && iInlineObjects->editing)
                        {
                            // delete first: moving the caret onto the formula's start would typeset it before the delete
                            delete_text(from, to);
                            cursor().set_position(from);
                        }
                        else
                        {
                            cursor().set_position(cursor().position() - 1);
                            delete_text(from, to);
                        }
                        make_cursor_visible(true);
                    }
                }
                else
                    delete_any_selection();
            }
            break;
        case ScanCode_DELETE:
            if (!read_only())
            {
                if (cursor().position() == cursor().anchor())
                {
                    if (cursor().position() < iText.size())
                    {
                        auto const from = cursor().position();
                        auto const to = cursor().position() + 1;
                        delete_text(from, to);
                        make_cursor_visible(true);
                    }
                }
                else
                    delete_any_selection();
            }
            break;
        case ScanCode_UP:
            if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
            {
                if ((aKeyModifier & key_modifier::CTRL) != key_modifier::None)
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
                else
                    move_cursor(cursor::Up, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            }
            else
                handled = false;
            break;
        case ScanCode_DOWN:
            if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
            {
                if ((aKeyModifier & key_modifier::CTRL) != key_modifier::None)
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
                else
                    move_cursor(cursor::Down, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            }
            else
                handled = false;
            break;
        case ScanCode_LEFT:
            move_cursor((aKeyModifier & key_modifier::CTRL) != key_modifier::None ? cursor::PreviousWord : cursor::Left, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            break;
        case ScanCode_RIGHT:
            move_cursor((aKeyModifier & key_modifier::CTRL) != key_modifier::None ? cursor::NextWord : cursor::Right, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            break;
        case ScanCode_HOME:
            move_cursor((aKeyModifier & key_modifier::CTRL) != key_modifier::None ? cursor::StartOfDocument : cursor::StartOfLine, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            break;
        case ScanCode_END:
            move_cursor((aKeyModifier & key_modifier::CTRL) != key_modifier::None ? cursor::EndOfDocument : cursor::EndOfLine, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
            break;
        case ScanCode_PAGEUP:
        case ScanCode_PAGEDOWN:
            if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::MultiLine)
            {
                if (aScanCode == ScanCode_PAGEUP && vertical_scrollbar().position() == vertical_scrollbar().minimum())
                    move_cursor(cursor::StartOfDocument, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
                else if (aScanCode == ScanCode_PAGEDOWN && vertical_scrollbar().position() == vertical_scrollbar().maximum() - vertical_scrollbar().page())
                    move_cursor(cursor::EndOfDocument, (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
                else
                {
                    auto pos = point{ glyph_position(cursor_glyph_position()).pos - point{ horizontal_scrollbar().Position.effective_value(), vertical_scrollbar().Position.effective_value()}};
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
                    set_cursor_position(pos + client_rect(false).top_left(), (aKeyModifier & key_modifier::SHIFT) == key_modifier::None);
                }
            }
            else
                handled = false;
            break;
        case ScanCode_ESCAPE:
            if (cursor().anchor() != cursor().position())
                cursor().set_anchor(cursor().position());
            else if (!read_only() && (iCaps & (text_edit_caps::SingleLine | text_edit_caps::OnlyAccept)) != text_edit_caps::None)
                set_text(string{});
            break;
        default:
            handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifier);
            break;
        }
        return handled;
    }

    bool text_edit::key_released(scan_code_e, key_code_e, key_modifier)
    {
        return false;
    }

    bool text_edit::text_input(i_string const& aText)
    {
        if (aText[0] == '\r' || aText[0] == '\n') // handled in key_pressed()
        {
            if ((focus_policy() & neogfx::focus_policy::ConsumeReturnKey) == neogfx::focus_policy::ConsumeReturnKey)
                return true;
            else
                return framed_scrollable_widget::text_input(aText);
        }
        else if (aText[0] == '\t') // handled in key_pressed()
        {
            if ((focus_policy() & neogfx::focus_policy::ConsumeTabKey) == neogfx::focus_policy::ConsumeTabKey)
                return true;
            else
                return framed_scrollable_widget::text_input(aText);
        }
        if (!read_only())
        {
            multiple_text_changes mtc{ *this };
            if (cursor().position() == cursor().anchor() &&
                overwrite_cursor_available() &&
                (service<i_keyboard>().locks() & keyboard_locks::InsertLock) != keyboard_locks::InsertLock)
            {
                if (cursor().position() < iText.size())
                {
                    auto const from = cursor().position();
                    auto const to = cursor().position() + 1;
                    delete_text(from, to);
                    make_cursor_visible(true);
                }
            }
            else
                delete_any_selection();
            insert_text(aText, { next_style() }, true);
        }
        return true;
    }

    scrolling_disposition text_edit::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::DontScrollChildWidget;
    }

    scrolling_disposition text_edit::scrolling_disposition(const i_widget& aWidget) const
    {
        if (iPasswordBits && &iPasswordBits.value().showPassword == &aWidget)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        return framed_scrollable_widget::scrolling_disposition(aWidget);
    }

    rect text_edit::scroll_area() const
    {
        auto const& internalPadding = padding();
        auto const& columnPadding = column(0).padding; ///< todo: other columns?
        auto const& totalPadding = internalPadding + columnPadding;
        return rect{ point{}, iTextExtents.value_or(size{}) + totalPadding.size() };
    }

    rect text_edit::scroll_page() const
    {
        return framed_scrollable_widget::scroll_page();
    }

    bool text_edit::use_scrollbar_container_updater() const
    {
        return false;
    }

    bool text_edit::update_scrollbar_visibility(usv_stage_e aStage)
    {
        std::optional<scoped_property_transition_suppression> sts1;
        std::optional<scoped_property_transition_suppression> sts2;

        if (iTextExtents != std::nullopt) // must be a resize event
        {
            sts1.emplace(vertical_scrollbar().Position);
            sts2.emplace(horizontal_scrollbar().Position);
        }

        switch (aStage)
        {
        case UsvStageInit:
            if (resizing())
                vertical_scrollbar().push_zone();
            vertical_scrollbar().set_step(font().height());
            horizontal_scrollbar().set_step(font().height());
            if (!framed_scrollable_widget::update_scrollbar_visibility(aStage))
                refresh_lines();
            break;
        case UsvStageDone:
            framed_scrollable_widget::update_scrollbar_visibility(aStage);
            if (has_focus() && !read_only())
                make_cursor_visible();
            else if (resizing())
            {
                auto zone = vertical_scrollbar().pop_zone();
                switch(zone)
                {
                case scrollbar_zone::Top:
                    make_cursor_visible();
                    break;
                case scrollbar_zone::Middle:
                    make_visible(glyph_position(glyph_hit_test(column_rect(0).top_left()).first, true));
                    break;
                case scrollbar_zone::Bottom:
                    vertical_scrollbar().set_position(vertical_scrollbar().maximum());
                    horizontal_scrollbar().set_position(0.0);
                    break;
                }
            }
            break;
        default:
            return framed_scrollable_widget::update_scrollbar_visibility(aStage);
        }

        return true;
    }

    void text_edit::scroll_page_updated()
    {
        refresh_lines();
    }

    color text_edit::frame_color() const
    {
        if (has_frame_color())
            return framed_scrollable_widget::frame_color();
        else if (has_focus())
            return service<i_app>().current_style().palette().color(color_role::Focus);
        else if (service<i_app>().current_style().palette().color(color_role::Theme).similar_intensity(background_color(), 0.03125))
            return framed_scrollable_widget::frame_color();
        else
            return service<i_app>().current_style().palette().color(color_role::Theme).mid(background_color());
    }

    bool text_edit::can_undo() const
    {
        return iPreviousText != iText;
    }

    bool text_edit::can_redo() const
    {
        return iPreviousText != iText;
    }

    bool text_edit::can_cut() const
    {
        return !read_only() && !iText.empty() && cursor().position() != cursor().anchor();
    }

    bool text_edit::can_copy() const
    {
        return !iText.empty() && cursor().position() != cursor().anchor();
    }

    bool text_edit::can_paste() const
    {
        return !read_only();
    }

    bool text_edit::can_delete_selected() const
    {
        return !read_only() && !iText.empty();
    }

    bool text_edit::can_select_all() const
    {
        return !iText.empty();
    }

    void text_edit::undo(i_clipboard&)
    {
        // todo: more intelligent undo
        if (iInlineObjects)
            iInlineObjects->editing.reset();
        iText.swap(iPreviousText);
        iUtf8TextCache = std::nullopt;
        std::erase_if(iBookmarks, [&](position_type aBookmark) { return aBookmark > static_cast<position_type>(iText.size()); });

        refresh_paragraph(iText.begin(), 0);
        update();
        cursor().set_position(iText.size());
        if (iPreviousText != iText)
            notify_text_changed();
    }

    void text_edit::redo(i_clipboard& aClipboard)
    {
        // todo: more intelligent redo
        undo(aClipboard);
    }

    void text_edit::cut(i_clipboard& aClipboard)
    {
        if (cursor().position() != cursor().anchor())
        {
            copy(aClipboard);
            delete_selected();
        }
    }

    void text_edit::copy(i_clipboard& aClipboard)
    {
        if (cursor().position() != cursor().anchor())
        {
            std::u32string selectedText;
            auto selectionStart = std::min(cursor().position(), cursor().anchor());
            auto selectionEnd = std::max(cursor().position(), cursor().anchor());
            for (auto i = selectionStart; i != selectionEnd; ++i)
            {
                // inline objects copy as their source (e.g. $x^2$)
                if (auto const* object = inline_objects::object_at(*this, i))
                    selectedText += inline_objects::source_text(*object);
                else
                    selectedText += iText[i].character;
            }
            for (auto sti = selectedText.begin(); sti != selectedText.end();)
            {
                auto const ch = *sti++;
                if (ch == U'\r' && (sti == selectedText.end() || *sti != U'\n'))
                    *std::prev(sti) = U'\n';
            }
            aClipboard.set_text(string{ neolib::utf32_to_utf8(selectedText) });
        }
    }

    void text_edit::paste(i_clipboard& aClipboard)
    {
        {
            multiple_text_changes mtc{ *this };
            if (cursor().position() != cursor().anchor())
                delete_selected();
            auto len = insert_text(aClipboard.text(), { next_style() });
            cursor().set_position(cursor().position() + len);
        }
    }

    void text_edit::delete_selected()
    {
        multiple_text_changes mtc{ *this };
        if (cursor().position() != cursor().anchor())
            delete_any_selection();
        else if(cursor().position() < iText.size())
            delete_text(cursor().position(), cursor().position() + 1);
    }

    void text_edit::select_all()
    {
        cursor().set_anchor(0);
        cursor().set_position(iText.size(), false);
    }

    std::size_t text_edit::document_length() const
    {
        return iText.size();
    }

    void text_edit::move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor)
    {
        if (glyphs().empty())
            return;
        switch (aMoveOperation)
        {
        case cursor::Up:
        case cursor::Down:
            if (iCursorHint.x == std::nullopt)
                iCursorHint.x = glyph_position(cursor_glyph_position(), true).pos.x;
            break;
        default:
            iCursorHint.x = std::nullopt;
            break;
        }
        switch (aMoveOperation)
        {
        case cursor::StartOfDocument:
            cursor().set_position(0, aMoveAnchor);
            break;
        case cursor::StartOfParagraph:
            break;
        case cursor::StartOfLine:
            {
                auto p = cursor().position();
                while (p > 0)
                {
                    if (iText[p - 1] == U'\n')
                        break;
                    --p;
                }
                cursor().set_position(p, aMoveAnchor);
            }
            break;
        case cursor::StartOfWord:
            break;
        case cursor::EndOfDocument:
            cursor().set_position(iText.size(), aMoveAnchor);
            break;
        case cursor::EndOfParagraph:
            break;
        case cursor::EndOfLine:
            {
                auto p = cursor().position();
                while (p < iText.size())
                {
                    if (iText[p] == U'\n')
                        break;
                    ++p;
                }
                cursor().set_position(p, aMoveAnchor);
            }
            break;
        case cursor::EndOfWord:
            break;
        case cursor::PreviousParagraph:
            break;
        case cursor::PreviousLine:
            break;
        case cursor::PreviousWord:
            if (!iText.empty())
            {
                auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
                auto p = cursor().position();
                if (p == iText.size())
                    --p;
                while (p > 0 && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
                    --p;
                if (p > 0)
                {
                    auto c = get_text_category(emojiAtlas, iText[p == cursor().position() ? p - 1 : p]);
                    while (p > 0 && category(glyphs()[p - 1]) == c)
                        --p;
                    if (p > 0 && c == text_category::Whitespace)
                    {
                        c = get_text_category(emojiAtlas, iText[p - 1]);
                        while (p > 0 && get_text_category(emojiAtlas, iText[p - 1]) == c)
                            --p;
                    }
                }
                cursor().set_position(p, aMoveAnchor);
            }
            break;
        case cursor::PreviousCharacter:
            if (cursor().position() > 0)
                cursor().set_position(cursor().position() - 1, aMoveAnchor);
            break;
        case cursor::NextParagraph:
            break;
        case cursor::NextLine:
            break;
        case cursor::NextWord:
            if (!iText.empty())
            {
                auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
                auto p = cursor().position();
                while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
                    ++p;
                if (p < iText.size() && p == cursor().position())
                {
                    auto c = get_text_category(emojiAtlas, iText[p]);
                    while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == c)
                        ++p;
                    while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
                        ++p;
                }
                cursor().set_position(p, aMoveAnchor);
            }
            break;
        case cursor::NextCharacter:
            if (cursor().position() < iText.size())
                cursor().set_position(cursor().position() + 1, aMoveAnchor);
            break;
        case cursor::Up:
            {
                auto currentPosition = glyph_position(cursor_glyph_position());
                auto previousLine = previous_line(currentPosition.line);
                if (previousLine)
                {
                    auto const columnRectSansPadding = column_rect(currentPosition.column_index());
                    auto const cursorPos = point{ *iCursorHint.x, previousLine.value()->ypos()} + columnRectSansPadding.top_left();
                    auto const documentPos = document_hit_test(cursorPos, false);
                    cursor().set_position(documentPos, aMoveAnchor);
                }
            }
            break;
        case cursor::Down:
            {
                auto currentPosition = glyph_position(cursor_glyph_position());
                auto nextLine = next_line(currentPosition.line);
                if (nextLine)
                {
                    auto const columnRectSansPadding = column_rect(currentPosition.column_index());
                    auto const cursorPos = point{ *iCursorHint.x, nextLine.value()->ypos() } + columnRectSansPadding.top_left();
                    auto const documentPos = document_hit_test(cursorPos, false);
                    cursor().set_position(documentPos, aMoveAnchor);
                }
                else if (currentPosition.lineEnd.value() == glyphs().end() || is_line_breaking_whitespace(*currentPosition.lineEnd.value()))
                    cursor().set_position(iText.size(), aMoveAnchor);
            }
            break;
        case cursor::Left:
            if (cursor().position() > 0)
                cursor().set_position(cursor().position() - 1, aMoveAnchor);
            break;
        case cursor::Right:
            if (cursor().position() < iText.size())
                cursor().set_position(cursor().position() + 1, aMoveAnchor);
            break;
        default:
            break;
        }
    }

    bool text_edit::overwrite_cursor_available() const
    {
        return (iCaps & text_edit_caps::OverwriteMode) == text_edit_caps::OverwriteMode;
    }

    i_string const& text_edit::plain_text() const
    {
        return text();
    }

    bool text_edit::set_plain_text(i_string const& aPlainText)
    {
        return set_text(aPlainText) != 0 || aPlainText.empty();
    }

    i_string const& text_edit::rich_text(rich_text_format aFormat) const
    {
        // todo
        return text();
    }

    bool text_edit::set_rich_text(i_string const& aRichText, rich_text_format aFormat)
    {
        return do_insert_rich_text(0, aRichText, aFormat, true, true) != 0 || aRichText.empty();
    }

    void text_edit::paste_plain_text()
    {
        paste(service<i_clipboard>());
    }

    void text_edit::paste_rich_text(rich_text_format aFormat)
    {
        auto& clipboard = service<i_clipboard>();
        if (!clipboard.has_text())
            return;
        multiple_text_changes mtc{ *this };
        if (cursor().position() != cursor().anchor())
            delete_selected();
        auto const len = do_insert_rich_text(cursor().position(), clipboard.text(), aFormat, false, false);
        cursor().set_position(cursor().position() + len);
    }

    void text_edit::begin_update()
    {
        iUpdatingDocument = true;
    }

    void text_edit::end_update()
    {
        iUpdatingDocument = false;
        refresh_paragraph(iText.begin(), 0);
        update();
    }

    text_edit_line_ending text_edit::line_ending() const
    {
        return iLineEnding;
    }

    void text_edit::set_line_ending(text_edit_line_ending aLineEnding)
    {
        iLineEnding = aLineEnding;
    }

    bool text_edit::read_only() const
    {
        return ReadOnly;
    }

    void text_edit::set_read_only(bool aReadOnly)
    {
        ReadOnly = aReadOnly;
        update();
    }

    bool text_edit::word_wrap() const
    {
        return WordWrap;
    }
    
    void text_edit::set_word_wrap(bool aWordWrap)
    {
        if (WordWrap != aWordWrap)
        {
            WordWrap = aWordWrap;
            refresh_columns();
        }
    }

    std::uint32_t text_edit::grow_lines() const
    {
        return GrowLines;
    }

    void text_edit::set_grow_lines(std::uint32_t aGrowLines)
    {
        if (GrowLines != aGrowLines)
        {
            GrowLines = aGrowLines;
            update_layout();
        }
    }

    bool text_edit::password() const
    {
        return Password;
    }

    i_string const& text_edit::password_mask() const
    {
        return PasswordMask;
    }

    void text_edit::set_password(bool aPassword, i_string const& aMask)
    {
        if (Password != aPassword || PasswordMask != aMask)
        {
            Password = aPassword;
            PasswordMask = string{ aMask };
            refresh_paragraph(iText.begin(), 0);
        }
    }

    const text_edit::style& text_edit::default_style() const
    {
        return iDefaultStyle;
    }

    void text_edit::set_default_style(const style& aDefaultStyle, bool aPersist)
    {
        neogfx::font oldFont = font();
        auto oldEffect = (default_style().character().text_effect() == std::nullopt);
        iDefaultStyle = aDefaultStyle;
        if (oldFont != font() || oldEffect != (default_style().character().text_effect() == std::nullopt))
        {
            glyphs().set_major_font(font());
            refresh_paragraph(iText.begin(), 0);
        }
        iPersistDefaultStyle = aPersist;
        DefaultStyleChanged();
        update();
    }

    color text_edit::default_text_color() const
    {
        if (std::holds_alternative<color>(default_style().character().text_color()))
            return static_variant_cast<const color&>(default_style().character().text_color());
        else if (std::holds_alternative<gradient>(default_style().character().text_color()))
            return static_variant_cast<const gradient&>(default_style().character().text_color()).at(0.0);
        else
            return service<i_app>().current_style().palette().default_text_color_for_widget(*this);
    }

    alignment text_edit::alignment() const
    {
        return default_style().paragraph().alignment().as_std_optional().value_or(
            neogfx::alignment::Left | 
            ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine ? 
                neogfx::alignment::VCenter : neogfx::alignment::Top));
    }

    void text_edit::set_alignment(neogfx::alignment aAlignment)
    {
        if (alignment() != aAlignment)
        {
            iDefaultStyle.paragraph().set_alignment(aAlignment);
            DefaultStyleChanged();
            update();
        }
    }

    text_edit::style text_edit::current_style() const
    {
        // a style applied with no selection is pending for the text typed next; it is the current style until the cursor moves
        if (iNextStyle != std::nullopt && cursor().position() == cursor().anchor())
            return *iNextStyle;
        auto position = static_cast<position_type>(cursor().anchor());
        if (position != 0 && cursor().position() == cursor().anchor())
            --position;
        return style_at(position);
    }

    text_edit::style text_edit::style_at(position_type aPosition) const
    {
        if (iText.empty())
        {
            style defaultStyle{ default_style() };
            defaultStyle.character().set_font_if_none(font());
            return defaultStyle;
        }
        auto const t = std::next(iText.begin(), std::min(aPosition, static_cast<position_type>(iText.size() - 1u)));
        auto const g = to_glyph(t);
        auto style = glyph_style(g, iColumns.at(glyph_position(g - glyphs().begin(), true).column_index()));
        style.character().set_font_if_none(g != glyphs().end() ? glyphs().glyph_font(*g) : font());
        return style;
    }

    void text_edit::apply_style(style const& aStyle)
    {
        if (cursor().position() == cursor().anchor())
        {
            // style the text that is typed next; changing the default style here would restyle
            // every character that still refers to it
            iNextStyle = aStyle;
        }
        else
            apply_style(std::min(cursor().anchor(), cursor().position()), std::max(cursor().anchor(), cursor().position()), aStyle);
    }

    void text_edit::apply_style(position_type aStart, position_type aEnd, style const& aStyle)
    {
        if (aStart >= aEnd)
            return;
        // a style that sets a font (or that replaces one that does) changes the shapes of the glyphs: the text is inserted again (so its 
        // paragraphs are refreshed); otherwise (e.g. colours, as syntax highlighting sets) the characters' styles are changed in place, as 
        // the glyphs' colours and effects are looked up when they are drawn: the text does not change (and is not notified as changed)
        auto sets_font = [&](style_cookie aCookie)
        {
            auto const existingStyle = iStyleMap.find(aCookie);
            return existingStyle != iStyleMap.end() && (**existingStyle).character().font() != std::nullopt;
        };
        bool reinsert = aStyle.character().font() != std::nullopt;
        for (auto i = aStart; !reinsert && i < aEnd; ++i)
            reinsert = sets_font(iText[i].style);
        if (reinsert)
        {
            std::u32string part;
            part.assign(iText.begin() + aStart, iText.begin() + aEnd);
            delete_text(aStart, aEnd);
            insert_text(aStart, string{ neolib::utf32_to_utf8(part) }, { aStyle });
            return;
        }
        // (the style's cookie, as inserting text with it would give it (the default style: none, unless it is persisted))
        style const* newStyle = nullptr;
        if (&aStyle != &iDefaultStyle || iPersistDefaultStyle)
        {
            style key = aStyle;
            auto existingStyle = iStyles.find(style_ptr{ style_ptr{}, &key });
            if (existingStyle == iStyles.end())
                existingStyle = iStyles.insert(std::make_shared<style>(*this, aStyle)).first;
            if (!(**existingStyle).has_cookie())
                (**existingStyle).set_cookie(iStyleMap.insert(*existingStyle));
            newStyle = &**existingStyle;
        }
        auto const newCookie = newStyle != nullptr ? newStyle->cookie() : neolib::invalid_cookie<style_cookie>;
        bool changed = false;
        for (auto i = aStart; i < aEnd; ++i)
        {
            auto& ch = iText[i];
            if (ch.style == newCookie)
                continue;
            // (the new style is referenced before the old one is released: releasing a style's last reference removes it)
            if (newStyle != nullptr)
                newStyle->add_ref();
            auto const existingStyle = iStyleMap.find(ch.style);
            if (existingStyle != iStyleMap.end())
                (**existingStyle).release();
            ch.style = newCookie;
            changed = true;
        }
        if (changed)
            update();
    }

    text_edit::style text_edit::next_style() const
    {
        if (iNextStyle != std::nullopt)
        {
            auto nextStyle = *iNextStyle;
            iNextStyle = std::nullopt;
            return nextStyle;
        }
        return current_style();
    }

    neogfx::cursor& text_edit::cursor() const
    {
        return iCursor;
    }

    void text_edit::set_cursor_position(const point& aPosition, bool aMoveAnchor, bool aEnableDragger)
    {
        auto const docPos = document_hit_test(aPosition);
        cursor().set_position(docPos, aMoveAnchor);
        neolib::service<neolib::i_power>().register_activity();
        if (aEnableDragger)
        {
            if (!capturing())
                set_capture();
            iDragger = std::make_unique<dragger>(*this);
        }
    }

    void text_edit::cancel_object_selection()
    {
        iSelectedUri = std::nullopt;
    }

    rect text_edit::column_rect(std::size_t aColumnIndex, bool aExtendIntoPadding) const
    {
        auto result = page_rect();
        for (std::size_t ci = 0; ci < aColumnIndex; ++ci)
            result.x += iColumns.at(ci).width;
        if (iColumns.size() > 1)
        {
            if (aColumnIndex + 1 < iColumns.size())
                result.cx = iColumns.at(aColumnIndex).width;
            else
                result.cx = std::max(0.0, page_rect().right() - result.x); // last column takes the remaining width
        }
        if (!aExtendIntoPadding)
            result.deflate(iColumns.at(aColumnIndex).info.padding);
        return result;
    }

    std::size_t text_edit::column_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        // by x only: column rects are page (not document) rects, so they don't contain a scrolled document position's y; 
        // the padding either side of a column belongs to it
        auto const x = (aAdjustForScrollPosition ? aPosition.x + horizontal_scrollbar().position() : aPosition.x);
        for (std::size_t ci = 0; ci + 1 < columns(); ++ci)
            if (x < column_rect(ci, true).right())
                return ci;
        return columns() - 1;
    }

    // Absolute glyph range of a paragraph's segment for a column; the column delimiter glyphs belong to no segment.
    // A paragraph with fewer segments than there are columns has no segment in the remaining columns (empty range);
    // its last segment spans them instead.
    std::pair<text_edit::document_glyphs::difference_type, text_edit::document_glyphs::difference_type> text_edit::column_glyph_span(glyph_paragraph const& aParagraph, std::size_t aColumnIndex) const
    {
        auto const breakCount = aParagraph.columnBreaks.size();
        if (aColumnIndex > breakCount)
            return { aParagraph.glyph_end_index(), aParagraph.glyph_end_index() };
        auto const break_glyph = [&](std::size_t aBreak) -> document_glyphs::difference_type
        {
            return aParagraph.glyph_begin_index() + aParagraph.columnBreaks[aBreak];
        };
        auto const first = aColumnIndex == 0 ? aParagraph.glyph_begin_index() : std::min(break_glyph(aColumnIndex - 1) + 1, aParagraph.glyph_end_index());
        auto const last = aColumnIndex < breakCount ? break_glyph(aColumnIndex) : aParagraph.glyph_end_index();
        return { first, std::max(first, last) };
    }

    text_edit::position_info text_edit::glyph_position(position_type aGlyphPosition, bool aForCursor) const
    {
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), document_span{ 0, 0, aGlyphPosition, aGlyphPosition },
            [](auto const& p, auto const& s) { return p.glyph_begin_index() < s.glyphsFirst; });
        if (paragraph != iGlyphParagraphs.begin())
        {
            if (paragraph == iGlyphParagraphs.end() || aGlyphPosition < paragraph->glyph_begin_index())
                paragraph = std::prev(paragraph);
        }
        if (paragraph == iGlyphParagraphs.end())
            return { paragraph };
        std::optional<glyph_lines::const_iterator> match;
        std::optional<glyph_lines::const_iterator> fallback;
        for(auto column = iGlyphColumns.begin(); column != iGlyphColumns.end(); ++column)
        {
            if (column->lines.empty())
                continue;
            auto lineTry = std::lower_bound(column->lines.begin(), column->lines.end(), document_span{ 0, 0, aGlyphPosition, aGlyphPosition },
                [](auto const& l, auto const& s) { return l.glyph_begin_index() < s.glyphsFirst; });
            if (lineTry != column->lines.begin())
            {
                if (lineTry == column->lines.end() || aGlyphPosition < lineTry->glyph_begin_index())
                    lineTry = std::prev(lineTry);
            }
            if (lineTry == column->lines.end())
                continue;
            if (lineTry->glyph_begin_index() > aGlyphPosition)
            {
                // this column's lines all start after the glyph; only use if no column has a line containing it
                if (fallback == std::nullopt || lineTry->glyph_begin_index() < fallback.value()->glyph_begin_index())
                    fallback = lineTry;
                continue;
            }
            if (match == std::nullopt || match.value()->glyph_begin_index() < lineTry->glyph_begin_index())
                match = lineTry;
        }
        if (match == std::nullopt)
            match = fallback;
        if (match == std::nullopt)
            return position_info{ iGlyphParagraphs.end(), {}, {}, glyphs().end(), glyphs().end(), glyphs().end(), point{} };
        auto const line = match.value();
        auto const columnIndex = line->columnIndex;
        auto const column = std::next(iGlyphColumns.begin(), columnIndex);
        auto const& columnRectSansPadding = column_rect(columnIndex);
        auto const& lines = column->lines;
        if (line != lines.end())
        {
            position_type lineStart = line->glyph_begin_index();
            position_type lineEnd = line->glyph_end_index();
            auto textDirection = glyph_text_direction(line->glyph_begin(), line->glyph_end());
            bool placeCursorToRight = (aGlyphPosition == lineEnd);
            if (aForCursor)
            {
                if (aGlyphPosition == static_cast<position_type>(glyphs().size()) || aGlyphPosition == paragraph->glyph_end_index())
                {
                    auto iterChar = iText.begin() + from_glyph(glyphs().begin() + aGlyphPosition).first;
                    if (iterChar != iText.begin())
                    {
                        auto iterGlyph = to_glyph(iterChar - 1);
                        auto const& g = *iterGlyph;
                        if (direction(g) == text_direction::RTL)
                        {
                            aGlyphPosition = iterGlyph - glyphs().begin();
                            placeCursorToRight = (direction(g) == text_direction::RTL ? false : true);
                        }
                    }
                }
                else if (direction(glyphs()[aGlyphPosition]) == text_direction::RTL)
                    placeCursorToRight = true;
                else if (aGlyphPosition == lineEnd)
                    placeCursorToRight = (textDirection != text_direction::RTL);
            }
            if (aGlyphPosition >= lineStart && aGlyphPosition <= lineEnd)
            {
                delta alignmentAdjust; // alignment and indent: where refresh_lines() put the line, as painting and hit testing use
                alignmentAdjust.dx = line->xpos();
                if (lineStart != lineEnd)
                {
                    auto iterGlyph = glyphs().begin() + aGlyphPosition;
                    auto const& glyphForPos = aGlyphPosition < lineEnd ? *iterGlyph : 
                        (textDirection == text_direction::LTR ? 
                            *(iterGlyph - 1) : 
                            *(glyphs().begin() + lineStart));
                    point linePos{ glyphForPos.cell[0].x - line->glyph_begin()->cell[0].x, line->ypos()};
                    if (placeCursorToRight)
                        linePos.x += glyphForPos.cell_extents().x;
                    return position_info{ paragraph, column, line, iterGlyph, line->glyph_begin(), line->glyph_end(), linePos + alignmentAdjust};
                }
                else
                    return position_info{ paragraph, column, line, line->glyph_begin(), line->glyph_begin(), line->glyph_end(), point{ 0.0, line->ypos()} + alignmentAdjust};
            }
        }
        point pos;
        if (!lines.empty())
        {
            pos.x = 0.0;
            auto textDirection = glyph_text_direction(lines.back().glyph_begin(), lines.back().glyph_end());
            auto const& paragraphStyle = glyph_style(lines.back().glyph_begin(), iColumns.at(columnIndex));
            auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(alignment());
            if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                pos.x = columnRectSansPadding.cx;
            else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                pos.x = columnRectSansPadding.cx / 2.0;
            pos.y = lines.back().ypos() + lines.back().extents.cy;
        }
        return position_info{ iGlyphParagraphs.end(), {}, {}, glyphs().end(), glyphs().end(), glyphs().end(), pos };
    }

    text_edit::position_type text_edit::cursor_glyph_position() const
    {
        return to_glyph(iText.begin() + cursor().position()) - glyphs().begin();
    }

    text_edit::position_type text_edit::cursor_glyph_anchor() const
    {
        return to_glyph(iText.begin() + cursor().anchor()) - glyphs().begin();
    }

    void text_edit::set_cursor_glyph_position(position_type aGlyphPosition, bool aMoveAnchor)
    {
        cursor().set_position(from_glyph(glyphs().begin() + aGlyphPosition).first, aMoveAnchor);
    }

    std::pair<text_edit::document_glyphs::difference_type, bool> text_edit::glyph_hit_test(const point& aPosition, bool aAdjustForScrollPosition, position_type* aClusterOffset) const
    {
        if (aClusterOffset != nullptr)
            *aClusterOffset = 0;
        if (iGlyphParagraphs.empty())
            return std::make_pair(0, false);
        auto columnIndex = column_hit_test(aPosition, aAdjustForScrollPosition);
        point const documentPosition = (aAdjustForScrollPosition ? aPosition + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } : aPosition);
        point adjustedPosition = (documentPosition - column_rect(columnIndex).top_left()).max(point{});
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), adjustedPosition.y,
            [](glyph_paragraph const& p, coordinate y)
            {
                return p.ypos < y;
            });
        if (paragraph == iGlyphParagraphs.end())
            paragraph = std::prev(paragraph);
        if (paragraph != iGlyphParagraphs.begin() && adjustedPosition.y < paragraph->ypos)
            paragraph = std::prev(paragraph);
        if (paragraph == iGlyphParagraphs.begin() && adjustedPosition.y < paragraph->ypos)
            return std::make_pair(0, false);
        if (columnIndex > paragraph->columnBreaks.size())
        {
            // the paragraph's last segment spans the remaining columns
            columnIndex = paragraph->columnBreaks.size();
            adjustedPosition = (documentPosition - column_rect(columnIndex).top_left()).max(point{});
        }
        auto const& column = iGlyphColumns.at(columnIndex);
        // only this paragraph's lines: a column's segment can be shorter than the paragraph (another column wrapped)
        auto const paragraphIndex = static_cast<std::size_t>(std::distance(iGlyphParagraphs.begin(), paragraph));
        auto const linesBegin = std::lower_bound(column.lines.begin(), column.lines.end(), paragraphIndex,
            [](const glyph_line& left, std::size_t p) { return left.paragraphIndex < p; });
        auto const linesEnd = std::upper_bound(linesBegin, column.lines.end(), paragraphIndex,
            [](std::size_t p, const glyph_line& right) { return p < right.paragraphIndex; });
        auto line = std::lower_bound(linesBegin, linesEnd, adjustedPosition.y,
            [](const glyph_line& left, coordinate y) { return left.ypos() < y; });
        if (line == linesEnd && linesBegin != linesEnd && 
            (adjustedPosition.y < std::prev(linesEnd)->ypos() + std::prev(linesEnd)->extents.cy || std::next(paragraph) != iGlyphParagraphs.end()))
            --line;
        if (line != linesEnd)
        {
            if (line != linesBegin && adjustedPosition.y < line->ypos())
                --line;
            adjustedPosition.x -= line->xpos();
            auto const lineStart = line->glyph_begin_index();
            auto const lineEnd = line->glyph_end_index();
            auto const lineStartX = line->glyph_begin()->cell[0].x;
            auto const lineEndX = (lineEnd != lineStart ? std::prev(line->glyph_end())->cell[1].x : lineStartX);
            if (adjustedPosition.x >= lineEndX)
            {
                if (lineEnd > lineStart && is_line_breaking_whitespace(glyphs()[lineEnd - 1]))
                    return std::make_pair(lineEnd - 1, true);
                return std::make_pair(lineEnd, true);
            }
            for (auto gi = lineStart; gi != lineEnd; ++gi)
            {
                auto const& glyph = glyphs()[gi];
                auto const glyphAdvance = glyph.cell_extents().x;
                if (adjustedPosition.x >= glyph.cell[0].x - lineStartX && adjustedPosition.x < glyph.cell[0].x - lineStartX + glyphAdvance)
                {
                    auto const clusterSize = glyph.clusters.second - glyph.clusters.first;
                    if (aClusterOffset != nullptr && clusterSize > 1 && glyphAdvance != 0.0)
                    {
                        // Ligature (or other multi-character glyph): pick the nearest character boundary
                        // within the glyph assuming the characters are evenly spaced across it.
                        auto const fraction = (adjustedPosition.x - (glyph.cell[0].x - lineStartX)) / glyphAdvance;
                        auto const offset = static_cast<position_type>(std::round(
                            (direction(glyph) == text_direction::LTR ? fraction : 1.0 - fraction) * clusterSize));
                        if (offset > 0 && offset < static_cast<position_type>(clusterSize))
                        {
                            *aClusterOffset = offset;
                            return std::make_pair(gi, true);
                        }
                    }
                    bool const inLeftHalf = adjustedPosition.x < glyph.cell[0].x - lineStartX + glyphAdvance / 2.0 || glyphAdvance == 0.0;
                    if (direction(glyph) == text_direction::LTR)
                        return std::make_pair(inLeftHalf ? gi : gi + 1, true);
                    else
                        return std::make_pair(inLeftHalf && gi != lineStart ? gi - 1 : (inLeftHalf ? lineEnd : gi), true);
                }
            }
            if (lineEnd > lineStart && line != std::prev(linesEnd) && !is_whitespace(glyphs()[lineEnd]))
                return std::make_pair(lineEnd - 1, true);
            if (lineEnd > lineStart && direction(glyphs()[lineEnd - 1]) == text_direction::RTL)
                return std::make_pair(lineEnd - 1, true);
            return std::make_pair(lineEnd, true);
        }
        else
        {
            return std::make_pair(glyphs().size(), false);
        }
    }

    text_edit::position_type text_edit::document_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        return document_hit_test_ex(aPosition, aAdjustForScrollPosition).first;
    }

    std::pair<text_edit::position_type, bool> text_edit::document_hit_test_ex(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        position_type clusterOffset = 0;
        auto const glyphPosition = glyph_hit_test(aPosition, aAdjustForScrollPosition, &clusterOffset);
        if (glyphPosition.first < static_cast<document_glyphs::difference_type>(glyphs().size()))
        {
            auto const glyphParagraph = glyph_to_paragraph(glyphPosition.first);
            if (glyphParagraph != iGlyphParagraphs.end())
                return std::make_pair(glyphParagraph->text_begin_index() + glyphs()[glyphPosition.first].clusters.first + clusterOffset, glyphPosition.second);
        }
        return std::make_pair(iText.size(), false);
    }

    bool text_edit::same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const
    {
        if (aTextPositionRight == static_cast<position_type>(iText.size()))
            return false;
        if (iText[aTextPositionLeft] == U'\n' || iText[aTextPositionRight] == U'\n' ||
            iText[aTextPositionLeft] == U'\r' || iText[aTextPositionRight] == U'\r')
            return false;
        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
        return get_text_category(emojiAtlas, iText[aTextPositionLeft]) == get_text_category(emojiAtlas, iText[aTextPositionRight]);
    }

    std::pair<text_edit::position_type, text_edit::position_type> text_edit::word_at(position_type aTextPosition, bool aWordBreakIsWhitespace) const
    {
        auto is_space =
            [&](char32_t ch)
            {
                switch (ch)
                {
                case U'\r':
                case U'\n':
                case U'\t':
                case U'\f':
                case U'\v':
                case U' ':
                    return true;
                default:
                    if ((iCaps & text_edit_caps::NonPrintableWhitespace) == text_edit_caps::NonPrintableWhitespace)
                        return ch < U' ' || ch == U'\x7F';
                    return false;
                }
            };
        auto start = aTextPosition;
        auto end = aTextPosition;
        if (aWordBreakIsWhitespace && (start == static_cast<position_type>(iText.size()) || is_space(iText[start])))
            return std::make_pair(start, end);
        while (start > 0 && (aWordBreakIsWhitespace ? !is_space(iText[start - 1]) : same_word(start - 1, aTextPosition)))
            --start;
        while (end < static_cast<position_type>(iText.size()) && (aWordBreakIsWhitespace ? !is_space(iText[end]) : same_word(aTextPosition, end)))
            ++end;
        return std::make_pair(start, end);
    }

    void text_edit::clear()
    {
        if (iInlineObjects)
        {
            iInlineObjects->editing.reset();
            iInlineObjects->hover_link(std::nullopt);
        }
        cursor().set_position(0);
        iPreviousText = iText;
        iText.clear();
        glyphs().clear();
        iGlyphParagraphs.clear();
        iUtf8TextCache = std::nullopt;
        iStyles.clear();
        iStyleMap.clear();
        iTags.clear();
        iTagMap.clear();
        iBookmarks.clear();
        iRichText = false;
        iMarkupPadding = std::nullopt;
        refresh_columns();
        if (iPreviousText != iText && !iUpdatingContents)
            notify_text_changed();
    }

    std::size_t text_edit::paragraph_count() const
    {
        return std::count(text().begin(), text().end(), '\n') + 1;
    }

    void text_edit::delete_paragraph(std::size_t aParagraphIndex)
    {
        if (paragraph_count() > 1)
        {
            auto start = iText.cbegin();
            for (std::size_t i = 0; i < aParagraphIndex; ++i)
            {
                start = std::find(start, iText.cend(), U'\n');
                if (start == iText.cend())
                    return;
                ++start;
            }
            auto end = std::find(start, iText.cend(), U'\n');
            if (end != iText.cend())
                ++end;
            delete_text(std::distance(iText.cbegin(), start), std::distance(iText.cbegin(), end));
        }
        else
            clear();
    }

    i_string const& text_edit::text() const
    {
        if (iUtf8TextCache == std::nullopt)
        {
            thread_local std::u32string u32text;
            u32text.assign(iText.begin(), iText.end());
            iUtf8TextCache = neolib::utf32_to_utf8(u32text);
        }
        return *iUtf8TextCache;
    }

    std::size_t text_edit::set_text(i_string const& aText)
    {
        return set_text(aText, {});
    }

    std::size_t text_edit::set_text(i_string const& aText, format const& aFormat)
    {
        return do_insert_text(0, aText, aFormat, true, true);
    }

    std::size_t text_edit::append_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::append_text(i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, aFormat, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, aFormat, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, aFormat, aMoveCursor, false);
    }

    void text_edit::delete_text(position_type aStart, position_type aEnd)
    {
        if (aStart == aEnd)
            return;

        auto eraseBegin = iText.begin() + aStart;
        auto eraseEnd = iText.begin() + aEnd;
        auto eraseAmount = eraseEnd - eraseBegin;

        iPreviousText = iText;
        iUtf8TextCache = std::nullopt;

        for (auto& ch : std::ranges::subrange(eraseBegin, eraseEnd))
        {
            auto existingStyle = iStyleMap.find(ch.style);
            if (existingStyle != iStyleMap.end())
                (**existingStyle).release();
            auto existingTag = iTagMap.find(ch.tag);
            if (existingTag != iTagMap.end())
                (**existingTag).release();
        }
        for (auto& bookmark : iBookmarks)
        {
            if (bookmark >= aEnd)
                bookmark -= eraseAmount;
            else if (bookmark > aStart)
                bookmark = aStart;
        }
        if (iInlineObjects)
            iInlineObjects->text_deleted(aStart, aEnd);
        iBookmarks.erase(std::unique(iBookmarks.begin(), iBookmarks.end()), iBookmarks.end());
        refresh_paragraph(iText.erase(eraseBegin, eraseEnd), -eraseAmount);

        update();

        if (iPreviousText != iText)
            notify_text_changed();
    }

    std::vector<text_edit::position_type> const& text_edit::bookmarks() const
    {
        return iBookmarks;
    }

    void text_edit::add_bookmark(position_type aPosition)
    {
        aPosition = std::clamp<position_type>(aPosition, 0, static_cast<position_type>(iText.size()));
        auto const existing = std::lower_bound(iBookmarks.begin(), iBookmarks.end(), aPosition);
        if (existing == iBookmarks.end() || *existing != aPosition)
        {
            iBookmarks.insert(existing, aPosition);
            update();
        }
    }

    void text_edit::remove_bookmark(position_type aPosition)
    {
        auto const existing = std::lower_bound(iBookmarks.begin(), iBookmarks.end(), aPosition);
        if (existing != iBookmarks.end() && *existing == aPosition)
        {
            iBookmarks.erase(existing);
            update();
        }
    }

    void text_edit::clear_bookmarks()
    {
        if (!iBookmarks.empty())
        {
            iBookmarks.clear();
            update();
        }
    }

    bool text_edit::same_paragraph(position_type aFirstGlyphPos, position_type aSecondGlyphPos) const
    {
        return glyph_to_paragraph(aFirstGlyphPos) == glyph_to_paragraph(aSecondGlyphPos);
    }

    text_edit::paragraph_span text_edit::character_to_paragraph(position_type aCharacterPos) const
    {
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), aCharacterPos,
            [](glyph_paragraph const& p, position_type cp)
            {
                return p.text_begin_index() < cp;
            });
        if (paragraph != iGlyphParagraphs.begin() && (paragraph == iGlyphParagraphs.end() || aCharacterPos < paragraph->text_begin_index()))
            paragraph = std::prev(paragraph);
        return { static_cast<std::size_t>(std::distance(iGlyphParagraphs.begin(), paragraph)), paragraph->span };
    }

    text_edit::paragraph_line_span text_edit::character_to_line(position_type aCharacterPos) const
    {
        for (auto const& column : iGlyphColumns)
        {
            auto line = std::lower_bound(column.lines.begin(), column.lines.end(), aCharacterPos,
                [](glyph_line const& l, position_type cp)
                {
                    return l.text_begin_index() < cp;
                });
            if (line != column.lines.begin() && (line == column.lines.end() || aCharacterPos < line->text_begin_index()))
                line = std::prev(line);
            if (line == column.lines.end())
                continue;
            if (aCharacterPos < line->text_begin_index())
                continue;
            if (aCharacterPos < line->text_end_index())
                return line->paragraph_span();
            if (&column == &*std::prev(iGlyphColumns.end()) && std::next(line) == column.lines.end())
                return line->paragraph_span();
        }
        return paragraph_line_span{};
    }

    text_edit::glyph_paragraphs::const_iterator text_edit::glyph_to_paragraph(position_type aGlyphPos) const
    {
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), aGlyphPos,
            [](glyph_paragraph const& p, position_type gp) { return p.glyph_begin_index() < gp; });
        if (paragraph != iGlyphParagraphs.begin() && 
            (paragraph == iGlyphParagraphs.end() || aGlyphPos < paragraph->glyph_begin_index()))
            paragraph = std::prev(paragraph);
        return paragraph;
    }

    std::size_t text_edit::columns() const
    {
        return iColumns.size();
    }

    void text_edit::set_columns(std::size_t aColumnCount)
    {
        iColumns.resize(aColumnCount);
        if (iGlyphColumns.size() > iColumns.size())
            iGlyphColumns.resize(iColumns.size(), { this });
        refresh_paragraph(iText.begin(), 0);
    }

    void text_edit::remove_columns()
    {
        iColumns.resize(1);
        iColumns[0] = {};
        if (iGlyphColumns.size() > iColumns.size())
            iGlyphColumns.resize(iColumns.size(), { this });
        refresh_paragraph(iText.begin(), 0);
    }

    const text_edit::column_info& text_edit::column(std::size_t aColumnIndex) const
    {
        if (aColumnIndex >= iColumns.size())
            throw bad_column_index();
        return iColumns[aColumnIndex].info;
    }

    void text_edit::set_column(std::size_t aColumnIndex, const column_info& aColumn)
    {
        if (aColumnIndex >= iColumns.size())
            throw bad_column_index();
        if (iColumns[aColumnIndex].info != aColumn)
        {
            iColumns[aColumnIndex].info = aColumn;
            refresh_paragraph(iText.begin(), 0);
        }
    }

    const text_edit::style& text_edit::column_style(std::size_t aColumnIndex) const
    {
        return column_style(column(aColumnIndex));
    }

    const text_edit::style& text_edit::column_style(const column_info& aColumn) const
    {
        if (aColumn.style)
            return *aColumn.style;
        return default_style();
    }

    bool text_edit::has_page_rect() const
    {
        return iPageRect != std::nullopt;
    }

    rect text_edit::page_rect() const
    {
        if (has_page_rect())
            return iPageRect.value();
        return client_rect(false);
    }

    void text_edit::set_page_rect(optional_rect const& aPageRect)
    {
        iPageRect = aPageRect;
        refresh_columns();
    }

    const neogfx::size_hint& text_edit::size_hint() const
    {       
        return iSizeHint;
    }

    void text_edit::set_size_hint(const neogfx::size_hint& aHint)
    {
        if (iSizeHint != aHint)
        {
            iSizeHint = aHint;
            iHintedSize = std::nullopt;
            update_layout();
            update();
        }
    }

    neogfx::tab_stops const& text_edit::tab_stops() const
    {
        if (iTabStops != std::nullopt)
            return iTabStops.value();
        if (iCalculatedTabStops == std::nullopt || iCalculatedTabStops->first != font())
            iCalculatedTabStops = std::make_pair(font(), tab_stop{ graphics_context{ *this, graphics_context::type::Unattached }.text_extent(iTabStopHint, font()).cx });
        return iCalculatedTabStops->second;
    }

    void text_edit::set_tab_stop_hint(i_string const& aTabStopHint)
    {
        if (iTabStopHint != aTabStopHint)
        {
            iTabStopHint = aTabStopHint;
            iCalculatedTabStops.reset();
            refresh_columns();
        }
    }

    void text_edit::set_tab_stops(std::optional<neogfx::tab_stops> const& aTabStops)
    {
        if (iTabStops != aTabStops)
        {
            iTabStops = aTabStops;
            iCalculatedTabStops.reset();
            refresh_columns();
        }
    }

    void text_edit::init()
    {
        set_frame_radius(vec4{ 4.0, 4.0, 4.0, 4.0 });

        iSink += neolib::service<neolib::i_power>().green_mode_entered([this]()
        {
            if (has_focus())
                update_cursor();
        });

        iDefaultFont = service<i_app>().current_style().font_info();
        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if (iDefaultFont != service<i_app>().current_style().font_info())
            {
                iDefaultFont = service<i_app>().current_style().font_info();
                refresh_paragraph(iText.begin(), 0);
            }
        });

        iSink += service<i_rendering_engine>().subpixel_rendering_changed([this]()
        {
            refresh_paragraph(iText.begin(), 0);
        });

        auto focusPolicy = neogfx::focus_policy::ClickTabFocus;
        if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
            focusPolicy |= neogfx::focus_policy::ConsumeReturnKey;
        set_focus_policy(focusPolicy);

        iSink += Mouse([this](const neogfx::mouse_event& aEvent)
            {
                auto const docPos = document_hit_test_ex(aEvent.position() - origin());
                if (docPos.second)
                {
                    auto const& ch = iText[docPos.first];
                    if (ch.tag != neolib::invalid_cookie<tag_cookie>)
                    {
                        auto tagPtr = iTagMap[ch.tag];
                        auto const& tag = *tagPtr;
                        if (!tag.mouse_event().has_slots())
                            return;
                        if (aEvent.type() == mouse_event_type::ButtonReleased)
                        {
                            bool const wasCapturingTag = (tagPtr == iTagCapturing);
                            iTagCapturing = nullptr;
                            if (!wasCapturingTag || cursor().position() != cursor().anchor())
                                return;
                            Mouse.accept();
                            if (capturing())
                                release_capture();
                            iDragger = nullptr;
                        }
                        auto const result = tag.mouse_event().trigger(aEvent);
                        if (result == trigger_result::Accepted)
                            Mouse.accept();
                        else if (aEvent.type() == mouse_event_type::ButtonClicked)
                            iTagCapturing = tagPtr;
                    }
                }
            });
        
        iSink += cursor().PositionChanged([this]()
        {
            // moving the caret out of a formula being edited typesets it again
            if (iInlineObjects && iInlineObjects->editing &&
                (static_cast<position_type>(cursor().position()) <= iInlineObjects->editing->start ||
                    static_cast<position_type>(cursor().position()) >= iInlineObjects->editing->end))
                iInlineObjects->end_edit(false);
            if (service<i_keyboard>().layout().ime_active(*this))
                service<i_keyboard>().layout().update_ime_position(cursor_rect().bottom_left());
            if (service<i_emoticon_translator>().active(*this))
                service<i_emoticon_translator>().update_cursor_position(cursor_rect().bottom_left());
            iNextStyle = std::nullopt;
            iCursorAnimationStartTime = neolib::this_process::elapsed_ms();
            make_cursor_visible();
            update();
        });
        iSink += cursor().AnchorChanged([this]()
        {
            update();
        });
        iSink += cursor().AppearanceChanged([this]()
        {
            update();
        });

        iSink += Password.Changed([&](bool const& aPassword)
        {
            if (aPassword && (iCaps & text_edit_caps::ShowPassword) == text_edit_caps::ShowPassword)
                iPasswordBits.emplace(*this);
            else
                iPasswordBits = std::nullopt;
            layout_items(false);
        });

        if ((iCaps & text_edit_caps::Password) == text_edit_caps::Password)
            set_password(true);
    }

    text_edit::document_glyphs const& text_edit::glyphs() const
    {
        if (iGlyphs == std::nullopt)
            iGlyphs = *make_ref<basic_glyph_text_content<text_edit::glyph_container_type>>(font());
        return *iGlyphs;
    }

    text_edit::document_glyphs& text_edit::glyphs()
    {
        return const_cast<document_glyphs&>(to_const(*this).glyphs());
    }

    std::size_t text_edit::do_insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor, bool aClearFirst)
    {
        bool accept = true;
        TextFilter(aText, accept);
        if (!accept)
            return 0;

        neolib::scoped_flag sf{ iUpdatingContents };

        iPreviousText = iText;
        iUtf8TextCache = std::nullopt;

        if (aClearFirst)
            clear();

        auto insertionPoint = iText.begin() + aPosition;
        std::size_t insertionSize = 0;

        thread_local std::vector<std::ptrdiff_t> indexMap;
        indexMap.clear();
        indexMap.resize(aText.size(), -1);
        std::size_t readPos = 0;

        auto insert = [&](document_text::const_iterator aWhere, const style& aStyle, std::u32string::const_iterator begin, std::u32string::const_iterator end)
        {
            auto existingStyle = iStyles.end();
            if (&aStyle != &iDefaultStyle || iPersistDefaultStyle)
            {
                style key = aStyle;
                existingStyle = iStyles.find(style_ptr{ style_ptr{}, &key });
                if (existingStyle == iStyles.end())
                    existingStyle = iStyles.insert(std::make_shared<style>(*this, aStyle)).first;
            }
            auto& style = (existingStyle != iStyles.end() ? **existingStyle : iDefaultStyle);
            if (existingStyle != iStyles.end())
            {
                if (!style.has_cookie())
                    style.set_cookie(iStyleMap.insert(*existingStyle));
                for (auto i = std::distance(begin, end); i > 0; --i)
                    style.add_ref();
            }

            auto const pos = std::distance(iText.cbegin(), aWhere);
            auto next = std::next(iText.begin(), pos);
            char32_t previousChar = (aWhere != iText.begin() ? std::prev(aWhere)->character : 0);
            for (auto const ch : std::ranges::subrange(begin, end))
            {
                bool discard = false;
                if (!iHandlingKeyPress)
                {
                    switch (ch)
                    {
                    case '\n':
                        if (previousChar == '\r')
                        {
                            std::prev(next)->character = '\n';
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticLfCr;
                            discard = true;
                        }
                        else
                        {
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticLf;
                        }
                        break;
                    case '\r':
                        if (previousChar == '\n')
                        {
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticCrLf;
                            discard = true;
                        }
                        break;
                    default:
                        /* do nothing */
                        break;
                    }
                }
                if (!discard)
                {
                    if (readPos < indexMap.size())
                        indexMap[readPos] = insertionSize;
                    next = std::next(iText.insert(next, document_char{ ch, style.cookie() }));
                    ++insertionSize;
                }
                readPos += (ch < 0x80 ? 1 : ch < 0x800 ? 2 : ch < 0x10000 ? 3 : 4); // indexMap is keyed on UTF-8 byte offsets into aText
                previousChar = ch;
            }
            return std::next(iText.begin(), pos);
        };

        if (std::holds_alternative<std::monostate>(aFormat.style) || std::holds_alternative<style>(aFormat.style))
        {
            thread_local std::u32string text;
            text = neolib::utf8_to_utf32(aText);
            if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
            {
                auto eol = text.find_first_of(U"\r\n");
                if (eol != std::u32string::npos)
                    text.erase(eol);
            }
            auto const& formatStyle = std::holds_alternative<std::monostate>(aFormat.style) ? default_style() : std::get<style>(aFormat.style);
            insert(insertionPoint, formatStyle, text.begin(), text.end());
        }
        else if (std::holds_alternative<style_callback>(aFormat.style))
        {
            std::ptrdiff_t next = 0;
            bool gotSingleLine = false;
            while (next != static_cast<std::ptrdiff_t>(aText.size()) && !gotSingleLine)
            {
                auto const& [s, nextEnd] = std::get<style_callback>(aFormat.style)(next);
                thread_local std::u32string text;
                text = neolib::utf8_to_utf32(std::string_view{ &aText[next], std::next(&aText[next], nextEnd - next) });
                if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
                {
                    auto eol = text.find_first_of(U"\r\n");
                    if (eol != std::u32string::npos)
                    {
                        text.erase(eol);
                        gotSingleLine = true;
                    }
                }
                insertionPoint = std::next(insert(insertionPoint, s, text.begin(), text.end()), text.size());
                next = nextEnd;
            }
        }
        else if (std::holds_alternative<ansi>(aFormat.style))
        {
            throw not_implemented();
        }

        insertionPoint = iText.begin() + aPosition;

        if (std::holds_alternative<tag_ptr>(aFormat.tag))
        {
            auto const& tag = *std::get<tag_ptr>(aFormat.tag);
            for (auto& ch : std::ranges::subrange(insertionPoint, insertionPoint + insertionSize))
                ch.tag = tag.cookie();
        }
        else if (std::holds_alternative<tag_ptr_callback>(aFormat.tag))
        {
            std::ptrdiff_t next = 0;
            while (next != static_cast<std::ptrdiff_t>(aText.size()))
            {
                auto const& [t, nextEnd] = std::get<tag_ptr_callback>(aFormat.tag)(next);
                if (t)
                {
                    auto const& tag = *t;
                    for (auto index = next; index != nextEnd; ++index)
                        if (indexMap[index] != -1)
                        {
                            auto& ch = *std::next(insertionPoint, indexMap[index]);
                            ch.tag = tag.cookie();
                        }
                }
                next = nextEnd;
            }
        }

        for (auto& bookmark : iBookmarks)
            if (bookmark > aPosition)
                bookmark += insertionSize;
        if (iInlineObjects)
            iInlineObjects->text_inserted(aPosition, static_cast<position_type>(insertionSize));

        if (!aClearFirst)
            refresh_paragraph(insertionPoint, insertionSize);
        else
            refresh_paragraph(iText.begin(), 0);

        update();
        
        if (aMoveCursor)
        {
            cursor().set_position(insertionPoint - iText.begin() + insertionSize);
            iCursorHint.x = glyph_position(cursor_glyph_position(), true).pos.x;
        }
        
        if (iPreviousText != iText)
            notify_text_changed();
        
        return insertionSize;
    }

    std::size_t text_edit::do_insert_rich_text(position_type aPosition, i_string const& aRichText, rich_text_format aFormat, bool aMoveCursor, bool aClearFirst)
    {
        auto const document = (aFormat == rich_text_format::Markdown) ?
            rich_text::parse_markdown(aRichText.to_std_string_view()) :
            rich_text::parse_html(aRichText.to_std_string_view());

        auto const& baseFont = font();
        auto find_font_family = [](std::string_view aFamily) -> std::optional<string>
        {
            auto const& fontManager = service<i_font_manager>();
            for (std::uint32_t familyIndex = 0; familyIndex < fontManager.font_family_count(); ++familyIndex)
            {
                auto const family = fontManager.font_family(familyIndex).to_std_string_view();
                if (std::ranges::equal(family, aFamily, [](char lhs, char rhs) { return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs)); }))
                    return string{ family };
            }
            return {};
        };
        std::optional<std::optional<string>> monospaceFamily;
        auto monospace_family = [&]() -> std::optional<string> const&
        {
            if (!monospaceFamily)
            {
                monospaceFamily.emplace();
                for (auto const candidate : { "Consolas", "Cascadia Mono", "Cascadia Code", "Courier New", "DejaVu Sans Mono", "Liberation Mono", "Menlo", "Monaco", "Courier" })
                    if ((*monospaceFamily = find_font_family(candidate)))
                        break;
            }
            return *monospaceFamily;
        };
        auto parse_color = [](std::string const& aColor) -> std::optional<color>
        {
            if (auto const named = color::from_name(aColor))
                return *named;
            if (aColor.starts_with("#") || aColor.starts_with("rgb"))
                return color{ aColor };
            return {};
        };
        auto const textColor = default_text_color();
        auto const linkColor = textColor.light() ? color::from_argb(0xFF58A6FFu) : color::from_argb(0xFF0645ADu);
        auto const codeShade = textColor.with_alpha(0.12);
        auto const indentStep = dpi_scale(40.0); // per list or block quote level, as browsers and Qt
        auto const em = baseFont.em_size().cy; // default block margins are in ems of the widget's font

        auto to_style = [&](rich_text::span_format const& aSpan)
        {
            style result;
            auto& character = result.character();
            font_style fontStyle = font_style::Invalid;
            if (aSpan.bold)
                fontStyle |= font_style::Bold;
            if (aSpan.italic)
                fontStyle |= font_style::Italic;
            if (fontStyle == font_style::Invalid)
                fontStyle = font_style::Normal;
            if (aSpan.underline || aSpan.link)
                fontStyle |= font_style::Underline;
            if (aSpan.strike)
                fontStyle |= font_style::Strike;
            if (aSpan.superscript)
                fontStyle |= font_style::Superscript;
            else if (aSpan.subscript)
                fontStyle |= font_style::Subscript;
            auto const size = aSpan.pointSize ? *aSpan.pointSize : baseFont.size() * aSpan.fontScale;
            auto const family = aSpan.monospace ? monospace_family() : aSpan.fontFamily ? find_font_family(*aSpan.fontFamily) : std::nullopt;
            if (family || fontStyle != font_style::Normal || size != baseFont.size())
            {
                try
                {
                    // by family name, so that a style in another of the family's files (e.g. tahomabd.ttf) is found: font{ baseFont, ... }
                    // only looks in the base font's own file and would emulate it (Tahoma Bold became an emulated bold italic)
                    character.set_font(neogfx::font{ family ? *family : string{ baseFont.family_name() }, fontStyle, size });
                }
                catch (...)
                {
                    character.set_font(neogfx::font{ baseFont, fontStyle, size });
                }
            }
            if (auto const explicitColor = aSpan.textColor ? parse_color(*aSpan.textColor) : std::nullopt)
                character.set_text_color(*explicitColor);
            else if (aSpan.link)
                character.set_text_color(linkColor);
            if (auto const background = aSpan.backgroundColor ? parse_color(*aSpan.backgroundColor) : std::nullopt)
                character.set_paper_color(*background);
            else if (aSpan.codeShade)
                character.set_paper_color(codeShade);
            if (aSpan.alignment != rich_text::block_alignment::Default)
            {
                auto horizontal = neogfx::alignment::Left;
                if (aSpan.alignment == rich_text::block_alignment::Center)
                    horizontal = neogfx::alignment::Center;
                else if (aSpan.alignment == rich_text::block_alignment::Right)
                    horizontal = neogfx::alignment::Right;
                else if (aSpan.alignment == rich_text::block_alignment::Justify)
                    horizontal = neogfx::alignment::Justify;
                result.paragraph().set_alignment((alignment() & neogfx::alignment::Vertical) | horizontal);
            }
            // lists indent with padding (as browsers' ul/ol padding-left), block quotes with margins (blockquote margin: 0 40px)
            auto const listLevels = aSpan.indent - aSpan.rightIndent;
            auto const quoteLevels = aSpan.rightIndent;
            auto const vertical = [&](rich_text::css_length const& aLength) { return std::round(dpi_scale(aLength.px) + aLength.em * em); };
            neogfx::margin const margin{
                quoteLevels * indentStep + dpi_scale(aSpan.marginLeft), vertical(aSpan.marginTop),
                quoteLevels * indentStep + dpi_scale(aSpan.marginRight), vertical(aSpan.marginBottom) };
            neogfx::padding const padding{
                listLevels * indentStep + dpi_scale(aSpan.paddingLeft), vertical(aSpan.paddingTop),
                dpi_scale(aSpan.paddingRight), vertical(aSpan.paddingBottom) };
            // always set, even when zero: the markup's box model (CSS defaults or its own) replaces the default style's
            // paragraph margin and padding, which would otherwise add to it
            result.paragraph().set_margin(margin);
            result.paragraph().set_padding(padding);
            return result;
        };

        // one style per distinct span format (unformatted runs too, for their paragraph spacing: see to_style)
        std::vector<rich_text::span_format const*> spans;
        std::vector<style> styles;
        std::vector<style const*> runStyles;
        runStyles.reserve(document.runs.size());
        styles.reserve(document.runs.size());
        for (auto const& run : document.runs)
        {
            auto existing = std::find_if(spans.begin(), spans.end(), [&](auto const* aSpan) { return *aSpan == run.format; });
            if (existing == spans.end())
            {
                spans.push_back(&run.format);
                styles.push_back(to_style(run.format));
                existing = std::prev(spans.end());
            }
            runStyles.push_back(&styles[std::distance(spans.begin(), existing)]);
        }

        auto run_at = [&](std::ptrdiff_t aOffset)
        {
            return static_cast<std::size_t>(std::distance(document.runs.begin(), std::upper_bound(document.runs.begin(), document.runs.end(), static_cast<std::size_t>(aOffset),
                [](std::size_t aValue, rich_text::run const& aRun) { return aValue < aRun.end; })));
        };

        // links are tags created on first use (after any clear), each opening its URI when clicked
        std::vector<tag_ptr> runTags(document.runs.size());
        auto link_tag = [&](std::string const& aUri)
        {
            auto const [tagPtr, created] = create_tag(link_tag_ttid(), aUri);
            if (created)
            {
                auto& linkTag = *tagPtr;
                linkTag.mouse_event()([this, &linkTag](const neogfx::mouse_event& aEvent)
                {
                    if (aEvent.type() != mouse_event_type::ButtonReleased || !aEvent.is_left_button())
                        return;
                    // a click (Ctrl+click when editable), not the end of a selection
                    if (!read_only() && (aEvent.key_modifiers() & key_modifier::CTRL) == key_modifier::None)
                        return;
                    if (cursor().position() != cursor().anchor())
                        return;
                    linkTag.mouse_event().accept();
                    std::string const uri{ *static_cast<std::string const*>(linkTag.data()) };
                    if (UriClicked.trigger(string{ uri }) == trigger_result::Accepted)
                        return;
                    if ((iCaps & text_edit_caps::FollowLinks) == text_edit_caps::FollowLinks)
                    {
                        if (!iInlineObjects)
                            iInlineObjects = std::make_unique<inline_objects>(*this);
                        iInlineObjects->follow(uri);
                    }
                });
                linkTag.query_mouse_cursor()([this](neogfx::mouse_cursor& aCursor)
                {
                    aCursor = ((iCaps & text_edit_caps::FollowLinks) == text_edit_caps::FollowLinks || UriClicked.has_slots()) &&
                        (read_only() || (service<i_keyboard>().modifiers() & key_modifier::CTRL) != key_modifier::None) ?
                        mouse_system_cursor::Hand : mouse_system_cursor::IBeam;
                });
            }
            return tagPtr;
        };

        format richFormat;
        richFormat.style = style_callback{ [&](std::ptrdiff_t aOffset) -> std::tuple<style const&, std::ptrdiff_t>
        {
            auto const runIndex = run_at(aOffset);
            return { *runStyles[runIndex], static_cast<std::ptrdiff_t>(document.runs[runIndex].end) };
        } };
        if (!iInlineObjects && std::any_of(document.runs.begin(), document.runs.end(), [](auto const& aRun) { return aRun.format.object.has_value(); }))
            iInlineObjects = std::make_unique<inline_objects>(*this);
        richFormat.tag = tag_ptr_callback{ [&](std::ptrdiff_t aOffset) -> std::tuple<tag_ptr, std::ptrdiff_t>
        {
            auto const runIndex = run_at(aOffset);
            auto const& run = document.runs[runIndex];
            if (run.format.object)
            {
                // math and images: the run's text is U+FFFC; the tag carries the object
                if (!runTags[runIndex])
                    runTags[runIndex] = create_tag(inline_object_ttid(), *run.format.object).first;
                return { runTags[runIndex], static_cast<std::ptrdiff_t>(run.end) };
            }
            if (run.format.listMarker)
            {
                if (!runTags[runIndex])
                    runTags[runIndex] = create_tag(list_marker_ttid(), std::string{}).first;
                return { runTags[runIndex], static_cast<std::ptrdiff_t>(run.end) };
            }
            if (!run.format.link)
                return { tag_ptr{}, static_cast<std::ptrdiff_t>(run.end) };
            if (!runTags[runIndex])
                runTags[runIndex] = link_tag(*run.format.link);
            return { runTags[runIndex], static_cast<std::ptrdiff_t>(run.end) };
        } };

        auto const result = do_insert_text(aPosition, string{ document.text }, richFormat, aMoveCursor, aClearFirst);

        // rich text is inset by its <body style> margin and padding (clear() forgets them, so a document replaces them; pasted
        // HTML only supplies them if there are none yet), else by CSS defaults (see padding())
        auto const previousPadding = padding();
        iRichText = true;
        if (document.body && !iMarkupPadding)
        {
            auto const length = [&](rich_text::css_length const& aLength) { return std::round(dpi_scale(aLength.px) + aLength.em * em); };
            iMarkupPadding = neogfx::padding{ length(document.body->left), length(document.body->top), length(document.body->right), length(document.body->bottom) };
        }
        if (padding() != previousPadding)
            refresh_columns();
        return result;
    }

    void text_edit::delete_any_selection()
    {
        if (cursor().position() != cursor().anchor())
        {
            auto const from = std::min(cursor().position(), cursor().anchor());
            auto const to = std::max(cursor().position(), cursor().anchor());
            cursor().set_position(std::min(cursor().position(), cursor().anchor()));
            delete_text(from, to);
        }
    }

    void text_edit::notify_text_changed()
    {
        if (!iSuppressTextChangedNotification)
            TextChanged();
        else
            ++iWantedToNotifyTextChanged;
    }

    text_edit::document_glyphs::const_iterator text_edit::to_glyph(document_text::const_iterator aWhere) const
    {
        auto const textIndex = aWhere - iText.begin();
        auto const paragraphLineSpan = character_to_line(textIndex);
        auto const documentSpan = paragraphLineSpan.lineSpan + paragraphLineSpan.paragraphSpan;
        auto const lineStart = std::next(glyphs().begin(), documentSpan.glyphsFirst);
        auto const lineEnd = std::next(glyphs().begin(), documentSpan.glyphsLast);
        auto const paragraphTextIndex = textIndex - paragraphLineSpan.paragraphSpan.textFirst;
        for (auto g = lineStart; g != lineEnd; ++g)
            if (g->clusters.first <= paragraphTextIndex && g->clusters.second > paragraphTextIndex)
                return g;
        return lineEnd;
    }

    std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
    {
        if (aWhere == glyphs().end())
            return std::make_pair(iText.size(), iText.size());
        auto paragraph = glyph_to_paragraph(aWhere - glyphs().begin());
        if (paragraph == iGlyphParagraphs.end() && paragraph != iGlyphParagraphs.begin() && aWhere <= (paragraph - 1)->glyph_end())
            --paragraph;
        if (paragraph != iGlyphParagraphs.end())
        {
            if (paragraph->glyph_begin() > aWhere)
                --paragraph;
            auto const textStart = paragraph->span.textFirst;
            auto const& clusters = aWhere->clusters;
            return std::make_pair(textStart + clusters.first, textStart + clusters.second);
        }
        return std::make_pair(iText.size(), iText.size());
    }

    std::optional<text_edit::glyph_lines::const_iterator> text_edit::next_line(std::optional<glyph_lines::const_iterator> const& aFrom) const
    {
        if (!aFrom)
            return {};
        if (aFrom.value() < std::prev(aFrom.value()->column().lines.end()))
            return std::next(aFrom.value());
        return {};
    }

    std::optional<text_edit::glyph_lines::const_iterator> text_edit::previous_line(std::optional<glyph_lines::const_iterator> const& aFrom) const
    {
        if (!aFrom)
            return {};
        if (aFrom.value() > aFrom.value()->column().lines.begin())
            return std::prev(aFrom.value());
        return {};
    }

    void text_edit::refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta)
    {
        if (iUpdatingDocument)
            return;

        document_text::const_iterator first;
        document_text::const_iterator last;
        document_glyphs::const_iterator glyphsInsertPos;
        glyph_paragraphs::const_iterator glyphParagraphsInsertPos;

        std::ptrdiff_t charsInserted = 0;
        std::ptrdiff_t glyphsInserted = 0;

        if (aDelta == 0 || iGlyphParagraphs.empty())
        {
            (void)aWhere;
            glyphs().clear();
            iGlyphParagraphs.clear();
            first = iText.begin();
            last = iText.end();
            glyphsInsertPos = glyphs().end();
            glyphParagraphsInsertPos = iGlyphParagraphs.end();
        }
        else if (aDelta > 0)
        {   
            auto const fromParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere));
            first = std::next(iText.begin(), fromParagraph.paragraphSpan.textFirst);
            last = std::next(iText.begin(), fromParagraph.paragraphSpan.textLast + aDelta);
            glyphsInsertPos = glyphs().erase(
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsFirst), 
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsLast));
            glyphParagraphsInsertPos = iGlyphParagraphs.erase(std::next(iGlyphParagraphs.begin(), fromParagraph.paragraphIndex));
            charsInserted -= (fromParagraph.paragraphSpan.textLast - fromParagraph.paragraphSpan.textFirst);
            glyphsInserted -= (fromParagraph.paragraphSpan.glyphsLast - fromParagraph.paragraphSpan.glyphsFirst);
        }
        else // aDelta < 0
        {
            auto const fromParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere));
            auto const toParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere) + -aDelta);
            first = std::next(iText.begin(), fromParagraph.paragraphSpan.textFirst);
            last = std::next(iText.begin(), toParagraph.paragraphSpan.textLast + aDelta);
            glyphsInsertPos = glyphs().erase(
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsFirst),
                std::next(glyphs().begin(), toParagraph.paragraphSpan.glyphsLast));
            glyphParagraphsInsertPos = iGlyphParagraphs.erase(
                std::next(iGlyphParagraphs.begin(), fromParagraph.paragraphIndex),
                std::next(iGlyphParagraphs.begin(), toParagraph.paragraphIndex + 1));
            charsInserted -= (toParagraph.paragraphSpan.textLast - fromParagraph.paragraphSpan.textFirst);
            glyphsInserted -= (toParagraph.paragraphSpan.glyphsLast - fromParagraph.paragraphSpan.glyphsFirst);
        }

        graphics_context gc{ *this, graphics_context::type::Unattached };

        if (password() && (!iPasswordBits || !iPasswordBits.value().showPassword.is_pressed()))
            gc.set_password(true, PasswordMask.value().empty() ? "\xE2\x97\x8F"_s : PasswordMask);

        auto nextParagraph = first;
        thread_local std::vector<std::u32string::difference_type> cachedColumnDelimiters;
        auto& columnDelimiters = cachedColumnDelimiters;
 
        auto fs = [this, &nextParagraph, &columnDelimiters](std::u32string::size_type aSourceIndex)
        {
            auto characterStyle = iStyleMap.find(std::next(nextParagraph, aSourceIndex)->style);
            std::size_t indexColumn = std::lower_bound(columnDelimiters.begin(), columnDelimiters.end(), 
                static_cast<std::u32string::difference_type>(aSourceIndex)) - columnDelimiters.begin();
            if (indexColumn > columns() - 1)
                indexColumn = columns() - 1;
            auto const& columnStyle = column_style(indexColumn);
            auto const& style =
                characterStyle != iStyleMap.end() ? **characterStyle :
                columnStyle.character().font() != std::nullopt ? columnStyle : iDefaultStyle;
            return style.character().font() != std::nullopt ? style.character().font().value() : font();
        };
        
        columnDelimiters.clear();
        std::size_t columnCount = 0;

        for (auto iterChar = first; iterChar != last; ++iterChar)
        {
            auto ch = iterChar->character;

            auto& column = iColumns[std::min(columnDelimiters.size(), iColumns.size() - 1)];

            if (ch == column.info.delimiter && columnDelimiters.size() + 1 < iColumns.size())
                columnDelimiters.push_back(std::distance(nextParagraph, iterChar));
            
            bool newParagraph = (ch == U'\n');
            
            if (newParagraph || iterChar == std::prev(last))
            {
                thread_local std::u32string paragraphBuffer;
                paragraphBuffer.assign(nextParagraph, std::next(iterChar));
                scoped_tab_stops sts{ gc, tab_stops() };
                auto gt = service<i_font_manager>().glyph_text_factory().to_glyph_text(gc, std::u32string_view{ paragraphBuffer.begin(), paragraphBuffer.end() }, fs, false);
                if (paragraphBuffer.find(ObjectReplacementCharacter) != std::u32string::npos)
                    inline_objects::lay_out(*this, gc, nextParagraph, gt, fs);
                if (gt.cbegin() != gt.cend())
                {
                    auto const paragraphGlyphs = glyphs().insert(glyphsInsertPos, gt.cbegin(), gt.cend());
                    glyphsInsertPos = std::next(paragraphGlyphs, gt.size());
                    for (auto& newGlyph : gt)
                        glyphs().glyph_font(newGlyph);
                    document_span const span{
                        std::distance(iText.cbegin(), nextParagraph),
                        std::distance(iText.cbegin(), std::next(iterChar)),
                        std::distance(glyphs().begin(), paragraphGlyphs),
                        std::distance(glyphs().begin(), std::next(paragraphGlyphs, gt.size())) };
                    auto const paragraph = iGlyphParagraphs.emplace(glyphParagraphsInsertPos, this, span);
                    paragraph->columnBreaks.assign(columnDelimiters.begin(), columnDelimiters.end());
                    // column breaks: text offsets of the delimiters -> glyph offsets (both relative to the paragraph)
                    std::size_t nextBreak = 0;
                    for (auto gi = paragraph->glyph_begin_index(); gi != paragraph->glyph_end_index() && nextBreak < paragraph->columnBreaks.size(); ++gi)
                        if (static_cast<document_glyphs::difference_type>(glyphs()[gi].clusters.first) == paragraph->columnBreaks[nextBreak])
                            paragraph->columnBreaks[nextBreak++] = gi - paragraph->glyph_begin_index();
                    for (; nextBreak < paragraph->columnBreaks.size(); ++nextBreak)
                        paragraph->columnBreaks[nextBreak] = paragraph->glyph_end_index() - paragraph->glyph_begin_index();
                    paragraph->lineBreaks.assign(gt.content().line_breaks().begin(), gt.content().line_breaks().end());
                    glyphParagraphsInsertPos = std::next(paragraph);
                    charsInserted += (paragraph->span.textLast - paragraph->span.textFirst);
                    glyphsInserted += (paragraph->span.glyphsLast - paragraph->span.glyphsFirst);
                }
                nextParagraph = std::next(iterChar);
                columnCount = std::max(columnCount, columnDelimiters.size() + 1);
                columnDelimiters.clear();
            }
        }

        for (auto paragraphToAdjust = std::next(iGlyphParagraphs.begin(), std::distance(iGlyphParagraphs.cbegin(), glyphParagraphsInsertPos)); 
            paragraphToAdjust != iGlyphParagraphs.end(); ++paragraphToAdjust)
        {
            auto& p = *paragraphToAdjust;
            p.span.textFirst += charsInserted;
            p.span.textLast += charsInserted;
            p.span.glyphsFirst += glyphsInserted;
            p.span.glyphsLast += glyphsInserted;
            p.heightMap.clear();
            p.ypos = 0.0;
        }

        iGlyphColumns.resize(std::max(columnCount, iGlyphColumns.size()), { this });
        for (auto& column : iGlyphColumns)
            column.lines.clear();

        if (iPasswordBits)
            iPasswordBits.value().showPassword.show(!iText.empty());

        refresh_columns();
    }

    void text_edit::refresh_columns()
    {
        iTextExtents = std::nullopt;
        update_scrollbar_visibility();
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::GrowLines)
            update_layout();
        update();
    }

    void text_edit::refresh_lines()
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_edit::refresh_lines()" << std::endl;
#endif // NEOGFX_DEBUG

        try
        {
            iOutOfMemory = false;

            /* simple (naive) implementation just to get things moving... */
            for (auto& column : iGlyphColumns)
                column.lines.clear();
            
            // column widths: each column except the last is as wide as its widest segment (that is followed by a
            // column break, i.e. doesn't span the remaining columns); the last column takes the remaining width
            if (iColumns.size() > 1)
            {
                for (auto& c : iColumns)
                    c.width = 0.0;
                for (auto const& paragraph : iGlyphParagraphs)
                {
                    for (std::size_t ci = 0; ci < paragraph.columnBreaks.size() && ci + 1 < iColumns.size(); ++ci)
                    {
                        auto const segment = column_glyph_span(paragraph, ci);
                        dimension segmentWidth = 0.0;
                        std::optional<coordinate> lineStartX;
                        for (auto gi = segment.first; gi != segment.second; ++gi)
                        {
                            auto const& g = glyphs()[gi];
                            if (is_line_breaking_whitespace(g))
                            {
                                lineStartX = std::nullopt;
                                continue;
                            }
                            if (!lineStartX)
                                lineStartX = g.cell[0].x;
                            segmentWidth = std::max<dimension>(segmentWidth, g.cell[1].x - lineStartX.value());
                        }
                        auto& c = iColumns[ci];
                        c.width = std::max(c.width, std::ceil(segmentWidth) + c.info.padding.size().cx);
                    }
                }
                for (std::size_t ci = 0; ci + 1 < iColumns.size(); ++ci)
                {
                    auto& c = iColumns[ci];
                    if (c.info.minWidth)
                        c.width = std::max(c.width, c.info.minWidth.value());
                    if (c.info.maxWidth)
                        c.width = std::min(c.width, c.info.maxWidth.value());
                }
            }

            auto availableRect = page_rect();
            availableRect.deflate(iColumns[0].info.padding);
            dimension availableWidth = availableRect.width(); // the whole page; each column narrows this (see below)
            dimension availableHeight = column_rect(0).height();
            bool showVerticalScrollbar = false;
            bool showHorizontalScrollbar = false;

            iTextExtents = size{};
            
            std::uint32_t pass = 1;
            dimension yposParagraph = 0.0;
            dimension previousMarginBottom = 0.0;

            for (auto iterParagraph = iGlyphParagraphs.begin(); iterParagraph != iGlyphParagraphs.end();)
            {
                auto& paragraph = *iterParagraph;

                paragraph.ypos = yposParagraph;
                dimension yColumn = 0.0;
                dimension marginBottom = 0.0;

                for (auto& column : iGlyphColumns)
                {
                    dimension yLine = 0.0;

                    auto const columnIndex = std::distance(&iGlyphColumns[0], &column);
                    auto& lines = column.lines;

                    // this paragraph's segment for this column; its last segment spans any remaining columns
                    auto const segmentIndex = static_cast<std::size_t>(columnIndex);
                    if (segmentIndex > paragraph.columnBreaks.size())
                        continue;
                    bool const spansRemainingColumns = (segmentIndex == paragraph.columnBreaks.size());
                    auto const segment = column_glyph_span(paragraph, segmentIndex);
                    auto const columnOffsetX = column_rect(segmentIndex).x - column_rect(0).x;
                    dimension const columnAvailableWidth = spansRemainingColumns ?
                        std::max(0.0, availableWidth - columnOffsetX) : column_rect(segmentIndex).width();

                    thread_local std::vector<std::pair<document_glyphs::difference_type, document_glyphs::difference_type>> paragraphLines;
                    paragraphLines.clear();

                    // todo: line segments to correct column

                    glyph_text::size_type lastBreak = 0;
                    for (auto lineBreak : paragraph.lineBreaks)
                    {
                        paragraphLines.emplace_back(lastBreak + paragraph.span.glyphsFirst, lineBreak + paragraph.span.glyphsFirst);
                        lastBreak = lineBreak + 1;
                    }
                    paragraphLines.emplace_back(lastBreak + paragraph.span.glyphsFirst, paragraph.span.glyphsLast);
                    if (paragraphLines.back().first != paragraphLines.back().second &&
                        is_line_breaking_whitespace(glyphs().back()) && std::next(iterParagraph) == iGlyphParagraphs.end())
                        paragraphLines.emplace_back(paragraph.span.glyphsLast, paragraph.span.glyphsLast);

                    if (!paragraph.columnBreaks.empty())
                    {
                        // clip the paragraph's lines to this column's segment
                        std::size_t kept = 0;
                        for (auto paragraphLine : paragraphLines)
                        {
                            if (paragraphLine.first == paragraphLine.second) // empty line
                            {
                                if (paragraphLine.first < segment.first || paragraphLine.first > segment.second)
                                    continue;
                            }
                            else
                            {
                                paragraphLine.first = std::max(paragraphLine.first, segment.first);
                                paragraphLine.second = std::min(paragraphLine.second, segment.second);
                                if (paragraphLine.first >= paragraphLine.second)
                                    continue;
                            }
                            paragraphLines[kept++] = paragraphLine;
                        }
                        paragraphLines.resize(kept);
                        if (paragraphLines.empty())
                            paragraphLines.emplace_back(segment.first, segment.first);
                    }

                    auto const& paragraphStyle = glyph_style(paragraph.glyph_begin(), iColumns[columnIndex]);

                    // a line is also tall enough for the descent of any inline objects (math, images)
                    auto const line_height = [&](auto aFirst, auto aLast, auto const& aAlignment) -> float
                    {
                        float result = aAlignment.yExtent;
                        if (!iInlineObjects)
                            return result;
                        for (auto g = aFirst; g != aLast; ++g)
                            if (is_inline_object(*g))
                                if (auto const* object = inline_objects::object_at(*this, paragraph.span.textFirst + g->clusters.first))
                                    if (auto const* m = iInlineObjects->cached_metrics(*object, glyphs().glyph_font(*g)))
                                        result = std::max(result, aAlignment.baseline + static_cast<float>(m->descent));
                        return result;
                    };

                    // left and right margins and padding indent the paragraph (lists, block quotes); a list item's marker hangs into the left
                    auto const paragraphPadding = paragraphStyle.paragraph().padding() ? paragraphStyle.paragraph().padding().value() : neogfx::padding{};
                    auto const paragraphMargin = paragraphStyle.paragraph().margin() ? paragraphStyle.paragraph().margin().value() : neogfx::margin{};
                    dimension const indentLeft = std::max(0.0, paragraphMargin.left + paragraphPadding.left);
                    dimension const indentRight = std::max(0.0, paragraphMargin.right + paragraphPadding.right);
                    dimension const lineAvailableWidth = std::max(0.0, columnAvailableWidth - indentLeft - indentRight);
                    dimension hangingIndent = 0.0;
                    if (segmentIndex == 0 && indentLeft > 0.0)
                    {
                        auto is_list_marker = [&](glyph_char const& aGlyph)
                        {
                            auto const textIndex = static_cast<std::size_t>(paragraph.span.textFirst + aGlyph.clusters.first);
                            if (textIndex >= iText.size() || iText[textIndex].tag == neolib::invalid_cookie<tag_cookie>)
                                return false;
                            auto const existing = iTagMap.find(iText[textIndex].tag);
                            return existing != iTagMap.end() && (**existing).ttid() == list_marker_ttid();
                        };
                        for (auto g = std::next(glyphs().begin(), paragraph.span.glyphsFirst); g != std::next(glyphs().begin(), paragraph.span.glyphsLast) && is_list_marker(*g); ++g)
                            hangingIndent += g->cell_extents().x;
                        hangingIndent = std::min(hangingIndent, indentLeft);
                    }
                    auto const hang = [&](auto aLineStart) -> dimension
                    {
                        return (aLineStart - glyphs().begin() == paragraph.span.glyphsFirst) ? hangingIndent : 0.0;
                    };
                    auto const line_x = [&](auto aLineStart, auto aLineEnd, dimension aLineWidth) -> dimension
                    {
                        dimension const width = lineAvailableWidth + hang(aLineStart);
                        dimension result = indentLeft - hang(aLineStart);
                        auto const textDirection = glyph_text_direction(aLineStart, aLineEnd);
                        auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(alignment());
                        if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                            ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                            result += (width - aLineWidth);
                        else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                            result += std::ceil(std::max(width - aLineWidth, 0.0) / 2.0);
                        return result;
                    };
                    auto const line_extent_x = [&](auto aLineStart, dimension aLineWidth) -> dimension
                    {
                        return columnOffsetX + indentLeft - hang(aLineStart) + aLineWidth + indentRight;
                    };

                    // where paragraphs meet, the larger of their margins separates them (CSS margin collapsing)
                    yLine += std::max(previousMarginBottom, paragraphMargin.top);
                    marginBottom = std::max(marginBottom, paragraphMargin.bottom);

                    if (paragraphStyle.paragraph().padding())
                        yLine += paragraphStyle.paragraph().padding().value().top;

                    bool first = true;

                    for (auto const& paragraphLine : paragraphLines)
                    {
                        auto const paragraphLineStart = std::next(glyphs().begin(), paragraphLine.first);
                        auto const paragraphLineEnd = std::next(glyphs().begin(), paragraphLine.second);

                        if (!first)
                        {
                            if (paragraphStyle.paragraph().line_spacing())
                                yLine += paragraphStyle.paragraph().line_spacing().value();
                        }
                        else
                            first = false;

                        if (paragraphLineStart == paragraphLineEnd || is_line_breaking_whitespace(*paragraphLineStart))
                        {
                            auto lineStart = paragraphLineStart;
                            auto lineEnd = (paragraphLineStart == paragraphLineEnd || !is_line_breaking_whitespace(*paragraphLineStart)) ? 
                                paragraphLineEnd : paragraphLineStart;

                            auto const alignBaselinesResult = glyphs().align_baselines(lineStart, lineEnd, true);

                            glyph_char::cluster_range clusters{
                                static_cast<glyph_char::cluster_index>(from_glyph(lineStart).first),
                                static_cast<glyph_char::cluster_index>(from_glyph(lineEnd).second) };
                            if (alignBaselinesResult.clusters)
                            {
                                clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                            }
                            auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                            auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                            if (lineStart != lineEnd && std::prev(lineEnd)->clusters.first < lineStart->clusters.first) // RTL
                                textLineStart = std::prev(lineEnd)->clusters.first;

                            document_span const span{
                                textLineStart,
                                textLineEnd,
                                lineStart - glyphs().begin() - paragraph.span.glyphsFirst,
                                lineEnd - glyphs().begin() - paragraph.span.glyphsFirst };

                            size const lineExtents{
                                    lineEnd != lineStart ? (lineEnd - 1)->cell[1].x - (lineStart)->cell[0].x : 0.0f,
                                    line_height(lineStart, lineEnd, alignBaselinesResult) };

                            dimension const xLine = line_x(lineStart, lineEnd, lineExtents.cx);

                            lines.emplace_back(
                                this,
                                std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                column.index(),
                                span,
                                point{ xLine, yLine },
                                lineExtents,
                                alignBaselinesResult.majorFont,
                                alignBaselinesResult.baseline);

                            yLine += lines.back().extents.cy;
                            iTextExtents->cx = std::max(iTextExtents->cx, line_extent_x(lineStart, lines.back().extents.cx));
                        }
                        else if (WordWrap && static_cast<coordinate>((paragraphLineEnd - 1)->cell[0].x) + static_cast<coordinate>((paragraphLineEnd - 1)->cell_extents().x) - 
                            static_cast<coordinate>(paragraphLineStart->cell[0].x) > lineAvailableWidth + hang(paragraphLineStart))
                        {
                            auto add_line = [&](auto first, auto last)
                            {
                                if (last != first && is_line_breaking_whitespace(*(last - 1)))
                                    --last;

                                auto const alignBaselinesResult = glyphs().align_baselines(first, last, true);

                                glyph_char::cluster_range clusters{
                                    static_cast<glyph_char::cluster_index>(from_glyph(first).first),
                                    static_cast<glyph_char::cluster_index>(from_glyph(last).second) };
                                if (alignBaselinesResult.clusters)
                                {
                                    clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                    clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                }
                                auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                                auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                                if (first != last && std::prev(last)->clusters.first < first->clusters.first) // RTL
                                    textLineStart = std::prev(last)->clusters.first;

                                document_span const span{
                                    textLineStart,
                                    textLineEnd,
                                    first - glyphs().begin() - paragraph.span.glyphsFirst,
                                    last - glyphs().begin() - paragraph.span.glyphsFirst };

                                size const lineExtents{
                                    last != first ? (last - 1)->cell[1].x - (first)->cell[0].x : 0.0f,
                                    line_height(first, last, alignBaselinesResult) };

                                dimension const xLine = line_x(first, last, lineExtents.cx);

                                lines.emplace_back(
                                    this,
                                    std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                    column.index(),
                                    span,
                                    point{ xLine, yLine },
                                    lineExtents,
                                    alignBaselinesResult.majorFont,
                                    alignBaselinesResult.baseline);

                                yLine += lines.back().extents.cy;
                                iTextExtents->cx = std::max(iTextExtents->cx, line_extent_x(first, lines.back().extents.cx));
                            };

                            if (glyph_text_direction(paragraphLineStart, paragraphLineEnd) == text_direction::LTR)
                            {
                                auto next = paragraphLineStart;
                                auto lineStart = next;
                                auto lineEnd = paragraphLineEnd;
                                coordinate offset = (lineEnd != lineStart ? lineStart->cell[0].x + hang(lineStart) : 0.0);
                                while (next != paragraphLineEnd)
                                {
                                    glyph_char const key{ {}, {}, {}, {}, {}, quadf_2d{ vec2{ offset + lineAvailableWidth, 0.0 }.as<float>(), vec2{offset + lineAvailableWidth, 0.0}.as<float>()}, {}};
                                    auto split = std::lower_bound(next, paragraphLineEnd, key, [](auto const& lhs, auto const& rhs) { return lhs.cell[0].x < rhs.cell[0].x; });
                                    if (split != next)
                                    {
                                        if (split != paragraphLineEnd)
                                            --split;
                                        else
                                        {
                                            auto const& previousChar = *(split - 1);
                                            auto const xPrevious = static_cast<coordinate>(previousChar.cell[0].x);
                                            auto const cxPrevious = static_cast<coordinate>(previousChar.cell_extents().x);
                                            if (xPrevious + cxPrevious >= offset + lineAvailableWidth)
                                                --split;
                                        }
                                    }
                                    if (split == next)
                                        ++split;
                                    if (split != paragraphLineEnd)
                                    {
                                        auto wordBreak = word_break(lineStart, split, paragraphLineEnd);
                                        if (wordBreak.first != wordBreak.second && is_inline_object(*wordBreak.first))
                                            wordBreak.second = wordBreak.first; // an object starts the next line rather than being the break's space
                                        if (wordBreak.first != lineStart)
                                        {
                                            lineEnd = wordBreak.first;
                                            next = wordBreak.second;
                                        }
                                        else
                                            next = lineEnd = split;
                                    }
                                    else
                                        next = paragraphLineEnd;
                                    add_line(lineStart, lineEnd);
                                    lineStart = next;
                                    if (lineStart != paragraphLineEnd)
                                        offset = lineStart->cell[0].x;
                                    lineEnd = paragraphLineEnd;
                                }
                            }
                            else // RTL
                            {
                                auto next = std::reverse_iterator{ paragraphLineEnd };
                                auto lineStart = next;
                                auto lineEnd = std::reverse_iterator{ paragraphLineStart };
                                coordinate const rightmost = (lineEnd != lineStart ? lineStart->cell[1].x : 0.0);
                                coordinate offset = rightmost;
                                while (next != std::reverse_iterator{ paragraphLineStart })
                                {
                                    glyph_char const key{ {}, {}, {}, {}, {}, quadf_2d{ vec2{ offset - lineAvailableWidth, 0.0 }.as<float>() }, {}};
                                    auto split = std::lower_bound(next, std::reverse_iterator{ paragraphLineStart }, key, [=](auto const& lhs, auto const& rhs) { return offset - lhs.cell[0].x < offset - rhs.cell[0].x; });
                                    if (split != next && (split != std::reverse_iterator{ paragraphLineStart } || static_cast<coordinate>((split - 1)->cell[0].x) + static_cast<coordinate>((split - 1)->cell_extents().x) >= rightmost - offset + lineAvailableWidth))
                                        --split;
                                    if (split == next)
                                        ++split;
                                    if (split != std::reverse_iterator{ paragraphLineStart })
                                    {
                                        auto wordBreak = word_break(lineStart, split, std::reverse_iterator{ paragraphLineStart });
                                        if (wordBreak.first != wordBreak.second && is_inline_object(*wordBreak.first))
                                            wordBreak.second = wordBreak.first; // an object starts the next line rather than being the break's space
                                        if (wordBreak.first != lineStart)
                                        {
                                            lineEnd = wordBreak.first;
                                            next = wordBreak.second;
                                        }
                                        else
                                            next = lineEnd = split;
                                    }
                                    else
                                        next = std::reverse_iterator{ paragraphLineStart };
                                    add_line(lineEnd.base(), lineStart.base());
                                    lineStart = next;
                                    if (lineStart != std::reverse_iterator{ paragraphLineStart })
                                        offset = lineStart->cell[1].x;
                                    lineEnd = std::reverse_iterator{ paragraphLineStart };
                                }
                            }
                        }
                        else
                        {
                            auto lineStart = paragraphLineStart;
                            auto lineEnd = paragraphLineEnd;
                            if (lineEnd != lineStart && is_line_breaking_whitespace(*(lineEnd - 1)))
                                --lineEnd;

                            auto const alignBaselinesResult = glyphs().align_baselines(lineStart, lineEnd, true);

                            glyph_char::cluster_range clusters{
                                static_cast<glyph_char::cluster_index>(from_glyph(lineStart).first),
                                static_cast<glyph_char::cluster_index>(from_glyph(lineEnd).second) };
                            if (alignBaselinesResult.clusters)
                            {
                                clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                            }
                            auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                            auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                            if (lineEnd != lineStart && std::prev(lineEnd)->clusters.first < lineStart->clusters.first) // RTL
                                textLineStart = std::prev(lineEnd)->clusters.first;
                            
                            document_span const span{
                                textLineStart,
                                textLineEnd,
                                lineStart - glyphs().begin() - paragraph.span.glyphsFirst,
                                lineEnd - glyphs().begin() - paragraph.span.glyphsFirst };

                            size const lineExtents{
                                    lineEnd != lineStart ? (lineEnd - 1)->cell[1].x - (lineStart)->cell[0].x : 0.0f,
                                    line_height(lineStart, lineEnd, alignBaselinesResult) };

                            dimension const xLine = line_x(lineStart, lineEnd, lineExtents.cx);

                            lines.emplace_back(
                                this,
                                std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                column.index(),
                                span,
                                point{ xLine, yLine },
                                lineExtents,
                                alignBaselinesResult.majorFont,
                                alignBaselinesResult.baseline);
                            
                            yLine += lines.back().extents.cy;
                            iTextExtents->cx = std::max(iTextExtents->cx, line_extent_x(lineStart, lines.back().extents.cx));
                        }
                    }

                    if (paragraphStyle.paragraph().padding())
                        yLine += paragraphStyle.paragraph().padding().value().bottom;
                    if (std::next(iterParagraph) == iGlyphParagraphs.end())
                        yLine += paragraphMargin.bottom; // nothing after it to collapse with

                    yColumn = std::max(yColumn, yLine);
                }

                previousMarginBottom = marginBottom;
                yposParagraph += yColumn;
                iTextExtents->cy = std::max(iTextExtents->cy, yposParagraph);

                auto next_pass = [&]()
                    {
                        if (pass <= 3)
                        {
                            for (auto& c : iGlyphColumns)
                                c.lines.clear();
                            yposParagraph = 0.0;
                            previousMarginBottom = 0.0;
                            iTextExtents = size{};
                            iterParagraph = iGlyphParagraphs.begin();
                            ++pass;
                        }
                    };
                switch (pass)
                {
                case 1:
                case 3:
                    if (!vertical_scrollbar().visible() && !showVerticalScrollbar && iTextExtents->cy > availableHeight)
                    {
                        showVerticalScrollbar = true;
                        availableWidth -= vertical_scrollbar().width();
                        next_pass();
                    }
                    else if (++iterParagraph == iGlyphParagraphs.end() && pass == 1)
                        next_pass();
                    break;
                case 2:
                    if (!horizontal_scrollbar().visible() && !showHorizontalScrollbar && iTextExtents->cx > availableWidth)
                    {
                        showHorizontalScrollbar = true;
                        availableHeight -= horizontal_scrollbar().width();
                        next_pass();
                    }
                    else if (++iterParagraph == iGlyphParagraphs.end())
                        next_pass();
                    break;
                case 4:
                    ++iterParagraph;
                    break;
                }
            }

            if (iTextExtents->cy < client_rect(false).cy)
            {
                auto const space = client_rect(false).cy - iTextExtents->cy;
                auto const defaultAlignment = default_style().paragraph().alignment().as_std_optional().value_or(alignment());
                auto const adjust =
                    ((defaultAlignment & neogfx::alignment::Vertical) == neogfx::alignment::Bottom) ? space :
                    ((defaultAlignment & neogfx::alignment::Vertical) == neogfx::alignment::VCenter) ? std::floor(space / 2.0) : 0.0;
                if (adjust != 0.0)
                    for (auto& paragraph : iGlyphParagraphs)
                        paragraph.ypos += adjust;
            }
        }
        catch (std::bad_alloc)
        {
            for (auto& paragraph : iGlyphParagraphs)
            {
                paragraph.ypos = 0.0;
                for (auto& column : iGlyphColumns)
                    column.lines.clear();
            }
            iOutOfMemory = true;
        }
    }

    void text_edit::animate()
    {
        if (neolib::service<neolib::i_power>().green_mode_active() && !iHasAnimations)
            return;
        if (iHasAnimations)
            update();
        if (has_focus())
            update_cursor();
    }

    void text_edit::update_cursor()
    {
        update(cursor_rect());
    }

    void text_edit::make_cursor_visible(bool aForcePreviewScroll)
    {
        scoped_units su{ *this, units::Pixels };
        auto const cgp = cursor_glyph_position();
        auto const gp = glyph_position(cgp, true);
        make_visible(gp, aForcePreviewScroll ? 
            point{ std::ceil(std::min(client_rect(false).width() / 3.0, 200.0)), 0.0 } : 
            point{});
    }

    void text_edit::make_visible(position_info const& aGlyphPosition, point const& aPreview)
    {
        scoped_units su{ *this, units::Pixels };
        auto extents = (aGlyphPosition.paragraph != iGlyphParagraphs.end() && 
            aGlyphPosition.column != iGlyphColumns.end() && 
            aGlyphPosition.line != aGlyphPosition.column.value()->lines.end() ?
                size{ aGlyphPosition.glyph != aGlyphPosition.lineEnd.value() ? aGlyphPosition.glyph.value()->cell_extents().x : 0.0, aGlyphPosition.line.value()->extents.cy } :
                size{ 0.0, font().height() });
        auto position = aGlyphPosition.pos;
        position.x += column_rect(aGlyphPosition.column_index()).x - column_rect(0).x;
        auto const& internalPadding = padding();
        auto const& colulmnPadding = column(0).padding; ///< todo: other columns?
        auto const& totalPadding = internalPadding + colulmnPadding;
        extents.cy = std::min(extents.cy, vertical_scrollbar().page());
        if (position.y < vertical_scrollbar().position())
            vertical_scrollbar().set_position(position.y);
        else if (position.y + extents.cy + totalPadding.size().cy > vertical_scrollbar().position() + vertical_scrollbar().page())
            vertical_scrollbar().set_position(position.y + extents.cy - vertical_scrollbar().page() + totalPadding.size().cy );
        if (position.x < horizontal_scrollbar().position() + aPreview.x)
            horizontal_scrollbar().set_position(position.x - aPreview.x);
        else if (position.x + extents.cx + totalPadding.size().cx > horizontal_scrollbar().position() + horizontal_scrollbar().page() - aPreview.x)
            horizontal_scrollbar().set_position(position.x + extents.cx + aPreview.x - horizontal_scrollbar().page() + totalPadding.size().cx);
    }

    text_edit::style text_edit::glyph_style(document_glyphs::const_iterator aGlyph, const document_column& aColumn) const
    {
        style result = iDefaultStyle;
        result.merge(column_style(aColumn.info));
        result.character().set_paper_color();
        if (iText.empty())
            return result;
        auto characterIndex = from_glyph(aGlyph).first;
        if (characterIndex == iText.size())
            --characterIndex;
        auto const existingCharacterStyleCookie = iText[characterIndex].style;
        auto const existingCharacterStyle = iStyleMap.find(existingCharacterStyleCookie);
        if (existingCharacterStyle != iStyleMap.end())
            result.merge(**existingCharacterStyle);
        return result;
    }

    void text_edit::draw_glyphs(i_graphics_context& aGc, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const
    {
        auto lineStart = aLine->glyph_begin();
        auto lineEnd = aLine->glyph_end();

        if (lineStart == lineEnd)
            return;

        if (is_line_breaking_whitespace(*(lineEnd - 1)))
            --lineEnd;

        auto documentGlyph = lineStart;

        glyph_text lineGlyphs{ font(), lineStart, lineEnd };
        lineGlyphs.content().align_baselines();

        auto const columnRect = column_rect(aColumn.index());
        auto const clientRect = client_rect();
        optional_text_format textAppearance;
        point const lineOrigin = lineStart->cell[0];
        point const textPos = aPosition - lineOrigin;
        auto segmentStart = lineGlyphs.cbegin();

        bool hasAnimations = false;

        for (auto segmentGlyph = segmentStart; segmentGlyph != lineGlyphs.cend(); ++segmentGlyph, ++documentGlyph)
        {
            bool selected = false;
            if (cursor().position() != cursor().anchor())
            {
                auto const gp = from_glyph(documentGlyph);
                selected = (static_cast<cursor::position_type>(gp.second) > std::min(cursor().position(), cursor().anchor()) && 
                    static_cast<cursor::position_type>(gp.first) < std::max(cursor().position(), cursor().anchor()));
            }
            auto const& style = glyph_style(documentGlyph, iColumns[aColumn.index()]);
            auto const& glyphColor = with_bounding_box(style.character().glyph_color() == neolib::none ?
                style.character().text_color() != neolib::none ?
                    with_bounding_box(style.character().text_color(), columnRect) :
                    default_text_color() : 
                style.character().glyph_color(), clientRect, true);
            auto const& nextTextAppearance = !selected ?
                text_format{
                    glyphColor,
                    style.character().paper_color() != neolib::none ? optional_text_color{ neogfx::text_color{ style.character().paper_color() } } : optional_text_color{},
                    style.character().text_effect(),
                    style.character().text_animation() }.with_smart_underline(style.character().smart_underline()).with_emoji_ignored(style.character().ignore_emoji()) :
                text_format{
                    has_focus() ? service<i_app>().current_style().palette().color(color_role::SelectedText) : glyphColor,
                    has_focus() ? service<i_app>().current_style().palette().color(color_role::Selection) : service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(64) };
            if (textAppearance && textAppearance.value() != nextTextAppearance)
            {
                hasAnimations = hasAnimations || textAppearance.value().animation().has_value();
                aGc.draw_glyph_text(textPos, lineGlyphs, segmentStart, segmentGlyph, textAppearance.value());
                segmentStart = segmentGlyph;
            }
            textAppearance = nextTextAppearance;
        }
        if (segmentStart != lineGlyphs.cend())
        {
            hasAnimations = hasAnimations || textAppearance.value().animation().has_value();
            aGc.draw_glyph_text(textPos, lineGlyphs, segmentStart, lineGlyphs.cend(), textAppearance.value());
        }

        if (iInlineObjects)
        {
            // inline objects (math, images) are painted over their glyphs' cells, on the line's baseline
            auto objectGlyph = lineStart;
            for (auto g = lineGlyphs.cbegin(); g != lineGlyphs.cend(); ++g, ++objectGlyph)
            {
                if (!is_inline_object(*g))
                    continue;
                auto const textPosition = from_glyph(objectGlyph);
                auto const* object = inline_objects::object_at(*this, textPosition.first);
                if (!object)
                    continue;
                bool const selected = cursor().position() != cursor().anchor() && has_focus() &&
                    static_cast<cursor::position_type>(textPosition.second) > std::min(cursor().position(), cursor().anchor()) &&
                    static_cast<cursor::position_type>(textPosition.first) < std::max(cursor().position(), cursor().anchor());
                auto const& style = glyph_style(objectGlyph, iColumns[aColumn.index()]);
                color_or_gradient const ink = selected ?
                    color_or_gradient{ service<i_app>().current_style().palette().color(color_role::SelectedText) } :
                    style.character().text_color() != neolib::none ? style.character().text_color() : color_or_gradient{ default_text_color() };
                // the paragraph's line area: aPosition is the line's position, which includes its indent and alignment
                auto const& paragraph = style.paragraph();
                auto const indentLeft = (paragraph.margin() ? paragraph.margin()->left : 0.0) + (paragraph.padding() ? paragraph.padding()->left : 0.0);
                auto const indentRight = (paragraph.margin() ? paragraph.margin()->right : 0.0) + (paragraph.padding() ? paragraph.padding()->right : 0.0);
                auto const columnLeft = aPosition.x - aLine->xpos();
                iInlineObjects->draw(aGc, *object, glyphs().glyph_font(*objectGlyph),
                    point{ textPos.x + g->cell[0].x, aPosition.y + aLine->baseline }, ink,
                    columnLeft + indentLeft, columnLeft + column_rect(aColumn.index()).width() - indentRight);
            }
        }

        if (hasAnimations)
            iHasAnimations = true;
    }

    void text_edit::draw_cursor(i_graphics_context& aGc) const
    {
        auto elapsedTime_ms = (neolib::this_process::elapsed_ms() - iCursorAnimationStartTime);
        auto const flashInterval_ms = cursor().flash_interval().count();
        auto const normalizedFrameTime = (elapsedTime_ms % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
        auto const cursorAlpha = neolib::service<neolib::i_power>().green_mode_active() ? 1.0 : partitioned_ease(easing::InvertedInOutQuint, normalizedFrameTime);
        auto cursorColor = cursor().color();
        if (cursorColor == neolib::none)
            cursorColor = service<i_app>().current_style().palette().default_text_color_for_widget(*this);
        auto const cursorRect = cursor_rect();
        if (cursor().style() == cursor_style::Xor)
        {
            aGc.push_logical_operation(logical_operation::Xor);
            aGc.fill_rect(cursorRect, (cursorAlpha >= 0.5 ? color::White : color::Black));
            aGc.pop_logical_operation();
        }
        else if (std::holds_alternative<color>(cursorColor))
        {
            aGc.fill_rect(cursorRect, static_variant_cast<const color&>(cursorColor).with_combined_alpha(cursorAlpha));
        }
        else if (std::holds_alternative<gradient>(cursorColor))
        {
            aGc.fill_rect(cursorRect, static_variant_cast<const gradient&>(cursorColor).with_combined_alpha(cursorAlpha));
        }
    }

    void text_edit::draw_bookmarks(i_graphics_context& aGc) const
    {
        if (iBookmarks.empty() || iGlyphColumns.empty())
            return;

        auto const textColor = default_text_color();
        auto const clientRect = client_rect(false);
        auto const documentTop = column_rect(0).top() - vertical_scrollbar().position();

        auto const iconExtent = iBookmarkIcon.extents();

        for (auto const bookmark : iBookmarks)
        {
            // a bookmark is drawn below the line containing the character preceding it
            coordinate y = 0.0;
            bool afterLastLine = false;
            if (bookmark > 0)
            {
                glyph_line const* match = nullptr;
                glyph_line const* following = nullptr;
                for (auto const& column : iGlyphColumns)
                {
                    auto line = std::upper_bound(column.lines.begin(), column.lines.end(), bookmark - 1,
                        [](position_type cp, glyph_line const& l) { return cp < l.text_begin_index(); });
                    if (line == column.lines.begin())
                        continue;
                    auto const next = line;
                    --line;
                    if (match == nullptr || match->text_begin_index() < line->text_begin_index())
                    {
                        match = &*line;
                        following = (next != column.lines.end() ? &*next : nullptr);
                    }
                }
                if (match == nullptr)
                    continue;
                y = match->ypos() + match->extents.cy;
                // centre between this line and the next (paragraph spacing may separate them)
                if (following != nullptr && following->ypos() > y)
                    y = (y + following->ypos()) / 2.0;
                afterLastLine = (following == nullptr);
            }
            else if (!iGlyphColumns[0].lines.empty())
                y = iGlyphColumns[0].lines.front().ypos(); // honour vertical alignment
            y = std::round(documentTop + y);
            // after the last line (only when the end of the document is in view; if it is scrolled off the bottom leave it where it is):
            // bottom alignment: dotted line on the visible bottom edge (the paint clip rect, which extends into the padding);
            // otherwise: drop half an icon so the icon clears the line, or if there is not that much room centre in the gap above the visible bottom edge
            if (afterLastLine)
            {
                rect visibleRect = default_clip_rect().intersection(clientRect);
                visibleRect.inflate(size{ padding_adjust() });
                auto const visibleBottom = visibleRect.bottom() - 1.0;
                if (y <= visibleBottom)
                {
                    auto const defaultAlignment = default_style().paragraph().alignment().as_std_optional().value_or(alignment());
                    if ((defaultAlignment & neogfx::alignment::Vertical) == neogfx::alignment::Bottom)
                        y = visibleBottom;
                    else
                        y = std::min(y + std::ceil(iconExtent.cy / 2.0), std::round((y + visibleBottom) / 2.0));
                }
            }
            if (y + iconExtent.cy < clientRect.top() || y - iconExtent.cy > clientRect.bottom())
                continue;

            aGc.draw_line(point{ clientRect.left() + iconExtent.cx + 1.0_dip, y }, point{ clientRect.right(), y }, pen{ textColor, line_style::Dot });
            aGc.draw_texture(rect{ point{ clientRect.left(), std::round(y - iconExtent.cy / 2.0) }, iconExtent }, iBookmarkIcon, textColor, shader_effect::ColorizeAlpha);
        }
    }

    rect text_edit::cursor_rect() const
    {
        auto cursorGlyphIndex = cursor_glyph_position();
        auto cursorPos = glyph_position(cursorGlyphIndex, true);
        dimension yHeight = 0.0;
        std::optional<scalar> xWidth;
        scalar xOffset = 0.0;
        scalar yOffset = 0.0;
        if (cursorPos.glyph && cursorPos.glyph.value() != glyphs().end())
        {
            auto iterGlyph = (cursorPos.glyph > cursorPos.lineStart.value() ? cursorPos.glyph.value() - 1 : cursorPos.glyph.value());
            auto const& glyph = *iterGlyph;
            yHeight = cursorPos.line.value()->extents.cy;
            yOffset = glyph.cell[0].as<scalar>().y;
            xWidth = cursorPos.glyph.value()->cell_extents().as<scalar>().x;
            // Cursor within a ligature (or other multi-character glyph): the cursor is placed at the
            // character boundary assuming the characters are evenly spaced across the glyph.
            auto const& cursorGlyph = *cursorPos.glyph.value();
            auto const cursorGlyphChars = from_glyph(cursorPos.glyph.value());
            auto const clusterSize = cursorGlyphChars.second - cursorGlyphChars.first;
            if (clusterSize > 1 && cursor().position() > cursorGlyphChars.first && cursor().position() < cursorGlyphChars.second)
            {
                auto const characterWidth = xWidth.value() / static_cast<scalar>(clusterSize);
                auto const characterOffset = static_cast<scalar>(cursor().position() - cursorGlyphChars.first);
                xOffset = (direction(cursorGlyph) != text_direction::RTL ? characterWidth * characterOffset : -characterWidth * characterOffset);
                xWidth = characterWidth;
            }
        }
        else if (cursorPos.lineStart && cursorPos.lineStart.value() != cursorPos.lineEnd.value())
            yHeight = cursorPos.line.value()->extents.cy;
        else
        {
            auto const& currentStyle = current_style();
            yHeight = currentStyle.character().font().value().height();
            if (currentStyle.paragraph().padding() || currentStyle.paragraph().margin())
            {
                auto padding_of = [](paragraph_style const& aStyle) { return aStyle.padding() ? aStyle.padding().value() : neogfx::padding{}; };
                auto margin_of = [](paragraph_style const& aStyle) { return aStyle.margin() ? aStyle.margin().value() : neogfx::margin{}; };
                auto const& current = currentStyle.paragraph();
                if (iText.empty())
                    yOffset = margin_of(current).top + padding_of(current).top;
                else
                {
                    auto previousPos = glyph_position(to_glyph(std::prev(iText.begin() + cursor().position())) - glyphs().begin(), true);
                    auto const previousStyle = (previousPos.glyph && previousPos.column) ?
                        glyph_style(previousPos.glyph.value(), iColumns.at(previousPos.column_index())) : currentStyle;
                    auto const& previous = previousStyle.paragraph();
                    yOffset = std::max(margin_of(previous).bottom, margin_of(current).top) + padding_of(previous).bottom + padding_of(current).top;
                }
            }
        }
        auto const columnRectSansPadding = column_rect(cursorPos.column_index());
        rect cursorRect{ 
            point{ cursorPos.pos - 
                point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + columnRectSansPadding.top_left() + point{ xOffset, yOffset },
            size{ static_cast<scalar>(cursor().width(*this, xWidth)), yHeight } };
        auto const rightmost = column_rect(columns() - 1).right();
        if (cursorRect.right() > rightmost)
            cursorRect.x += (rightmost - cursorRect.right());
        return cursorRect;
    }

    double text_edit::calc_padding_adjust(style const& aStyle) const
    {
        if (!aStyle.character().text_effect())
            return 0.0;
        else
        {
            scalar scale = 1.0;
            switch (aStyle.character().text_effect()->type())
            {
            case text_effect_type::None:
            default:
                scale = 1.0;
                break;
            case text_effect_type::Outline:
                scale = 1.0;
                break;
            case text_effect_type::Glow:
            case text_effect_type::Shadow:
                scale = 0.5;
                break;
            }
            return std::floor(aStyle.character().text_effect()->width() * scale);
        }
    }

    double text_edit::padding_adjust() const
    {
        auto paddingAdjust = calc_padding_adjust(default_style());
        for (auto const& s : iStyles)
            paddingAdjust = std::max(paddingAdjust, calc_padding_adjust(*s));
        return paddingAdjust;
    }
}
