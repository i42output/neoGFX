// script_graph.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2026 Leigh Johnston

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

#pragma once

#include <neogfx/neogfx.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <optional>
#include <functional>
#include <queue>
#include <utility>
#include <string>
#include <vector>

#include <neolib/core/string_utf.hpp>
#include <neolib/task/timer.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/tools/DesignStudio/i_script_highlighter.hpp>
#include <neogfx/tools/DesignStudio/i_script_debugger.hpp>

namespace neogfx::DesignStudio
{
    // a script's node graph: the outline of its structure (see i_script_highlighter::outline) as nodes, flowing left to right (as Unreal's
    // Blueprints): a statement whose body has statements (a function, a loop, a branch) contains them, its parameters (a function's) before
    // them, a branch's alternatives (e.g. its else) below them, each on a lane of its own; the statements of a body are joined, in order, 
    // by its flow (white arrows), and a function's parameters to the statements that use them (wires in the parameters' colour); nodes 
    // are in the colours of their syntax highlighting categories. The graph is zoomed by
    // the mouse wheel (initially to fit) and panned by dragging it. A node is dragged (with its contents) to where it is dropped (within its
    // container, which encloses it). It edits the script: a node's code is edited in place (double click or F2; Return accepts, Escape 
    // cancels), a statement is inserted after a node (Insert), a node is deleted (Delete) or duplicated (Ctrl+D), and a statement dragged 
    // past another statement of the same body is moved before or after it; a parameter or a variable renamed in it has a button (in 
    // its node's header) that renames its uses too; each edit is of the script's text (see ScriptEdit), whose outline the graph then shows (the script's 
    // changes are told it (see script_changed)).
    class script_graph : public widget<>
    {
        typedef widget<> base_type;
    public:
        using category = i_script_highlighter::category;
        using span = i_script_highlighter::span;
        using outline_node = i_script_highlighter::outline_node;
        using outline_kind = i_script_highlighter::outline_kind;
        using position_type = std::uint32_t; ///< (a code point of the script, as the editor's positions are)
    public:
        // (a node selected: its text [first, last))
        define_event(NodeSelected, node_selected, position_type, position_type)
        // (an edit of the script: its text [first, last) replaced (edits of one change are in the order that they are made))
        define_event(ScriptEdit, script_edit, position_type, position_type, i_string const&)
        // (its nodes have been dragged (or put back): where they are (see node_layout) is to be kept with the script)
        define_event(LayoutChanged, layout_changed)
    private:
        static constexpr std::uint32_t NoNode = ~std::uint32_t{};
        static constexpr std::size_t MaxHeaderLength = 64u;
        static constexpr double MinZoom = 0.1;
        static constexpr double MaxZoom = 4.0;
        static constexpr double MaxFitZoom = 1.5;
        static constexpr double GridSubdivisions = 8.0;     ///< (minor grid lines to a major one (as the workspace's grid's default))
        static constexpr dimension MinTextHeight = 5.0;     ///< (text smaller than this (in pixels) is not drawn: it cannot be read)
        static constexpr std::size_t WrapLength = 28u;      ///< (a node's code is wrapped to lines of at most this many characters (see wrap))
        static constexpr dimension IconSpace = 18.0;        ///< (before a node's kind, in its title bar: its kind's icon)
        static constexpr dimension CodeInset = 20.0;        ///< (a node's code from its sides: its pins are beside it)
        static constexpr dimension PinInset = 10.0;         ///< (a node's pins' centres from its sides)
        static constexpr dimension PinSize = 5.0;           ///< (half a pin's height)
        static constexpr dimension RouteMargin = 14.0;      ///< (how far a flow that is routed keeps from nodes)
        static constexpr dimension CornerRadius = 28.0;     ///< (a routed flow rounded where it turns)
        // (a run of a node's header in one syntax highlighting category: its text, its offset from its line's start (graph coordinates) 
        // and its line (the header's code is wrapped: see wrap))
        struct run
        {
            string text;
            std::optional<category> kind;
            dimension offset = 0.0;
            std::uint32_t line = 0u;
        };
        struct node
        {
            outline_node outline;
            std::vector<std::uint32_t> parameters;
            std::vector<std::uint32_t> children;            ///< (the statements of its body, in order)
            std::vector<std::uint32_t> alternatives;        ///< (a branch's alternatives (e.g. its else), in order: each on a lane of its own, below its body)
            std::u32string header;                          ///< (its header's code, on one line)
            std::vector<std::optional<category>> headerCategories;
            std::vector<run> runs;                          ///< (its header's runs, measured when laid out: painting only draws them)
            std::vector<dimension> zoomedOffsets;           ///< (its runs' offsets as measured in the zoomed font (see measure_zoomed))
            dimension textWidth = 0.0;                      ///< (its header's code's width (its widest line's), in the font (unzoomed))
            std::uint32_t lineCount = 0u;                   ///< (its header's code's lines, below its title row (its kind): none, a module's)
            string label;                                   ///< (its kind)
            dimension labelWidth = 0.0;                     ///< (graph coordinates: as measured in its font (see label_font))
            dimension zoomedLabelWidth = 0.0;
            dimension headerWidth = 0.0;                    ///< (graph coordinates: its code's, its label's and its inputs', as measured 
                                                            ///< unzoomed (see header_width))
            std::u32string key;                             ///< (which node it is, as the script is edited: where it was dragged to is kept by it)
            point offset;                                   ///< (where it has been dragged to: from its place (graph coordinates))
            dimension parametersWidth = 0.0;
            dimension parametersTop = 0.0;                  ///< (its parameters' top: below its body's statements (from its body's top))
            neogfx::size laneExtents;
            std::vector<std::uint32_t> uses;                ///< (a parameter: the nodes whose headers use it)
            std::vector<std::uint32_t> inputs;              ///< (the parameters that its header uses: each on a row of its own, below its code)
            std::vector<string> inputNames;
            dimension inputsWidth = 0.0;                    ///< (its inputs' names' width (graph coordinates))
            point place;                                    ///< (where it is (from its container's lane's start) if it has not been dragged: 
                                                            ///< a statement is after the one before it, wherever that is)
            bool flowsOut = false;                          ///< (the flow passes from it to another (its out pin is connected))
            std::size_t firstFlow = 0u;                     ///< (its flows (see flow): iFlows [firstFlow, firstFlow + flowCount))
            std::size_t flowCount = 0u;
            neogfx::size extents;                           ///< (graph coordinates)
            neogfx::rect bounds;
            neogfx::rect lane;                              ///< (its body's statements)
        };
        struct drag
        {
            point from;
            std::uint32_t node;
            point offset;                                   ///< (the node's offset when the drag began)
            bool dragging = false;
            std::optional<std::size_t> dropIndex;           ///< (the place among its siblings that it would be moved to)
        };
        struct pan
        {
            point from;
            point offset;
        };
        // (a parameter or a variable renamed in the graph (a header that starts with its name edited): the uses of its old name, which 
        // can be renamed too (propagated))
        struct pending_rename
        {
            position_type definition;                       ///< (where it is (as renamed))
            std::u32string from;
            std::u32string to;
            std::vector<position_type> uses;                ///< (where its old name is used (in the script as renamed), in order)
        };
        // (a flow in a node's body: its route (graph coordinates), from its start to its end: an S-curve between them if it has two 
        // points, else straight lines, rounded where they turn; an entry's start dotted; a loop's flow back to its start dashed)
        struct flow
        {
            std::vector<point> route;
            bool entry = false;
            bool loop = false;
        };
        struct inline_edit
        {
            std::uint32_t node;
            bool insert;                                    ///< (a statement inserted after the node (else its header replaced))
        };
    public:
        script_graph(i_layout& aLayout) :
            base_type{ aLayout }
        {
            set_focus_policy(neogfx::focus_policy::StrongFocus);
            set_size_policy(size_constraint::Expanding);
            set_padding(neogfx::padding{});
            // (its labels (translated) laid out again in the app's language when it changes)
            iSink += service<i_app>().language_changed([this]() { refresh_layout(); });
        }
        ~script_graph()
        {
            iInlineEditor = nullptr;
        }
    public:
        // the script (its code points), its syntax highlighting and its outline (nodes in the order of i_script_highlighter::outline)
        void set_script(std::u32string const& aText, std::vector<span> const& aSpans, std::vector<outline_node> const& aOutline)
        {
            std::optional<std::pair<position_type, outline_kind>> const selected = iSelected != NoNode ?
                std::optional<std::pair<position_type, outline_kind>>{ { iNodes[iSelected].outline.first, iNodes[iSelected].outline.kind } } : std::nullopt;
            iText = aText;
            iCategories.assign(iText.size(), std::nullopt);
            for (auto const& s : aSpans)
                for (auto i = std::min<std::size_t>(s.first, iText.size()); i < std::min<std::size_t>(s.last, iText.size()); ++i)
                    iCategories[i] = s.kind;
            iNodes.clear();
            iNodes.reserve(aOutline.size());
            std::map<std::u32string, std::uint32_t> occurrences;
            for (auto const& o : aOutline)
            {
                auto& n = iNodes.emplace_back();
                n.outline = o;
                n.outline.first = std::min<position_type>(n.outline.first, static_cast<position_type>(iText.size()));
                n.outline.last = std::clamp<position_type>(n.outline.last, n.outline.first, static_cast<position_type>(iText.size()));
                n.outline.headerLast = std::clamp<position_type>(n.outline.headerLast, n.outline.first, n.outline.last);
                if (o.parent != outline_node::NoParent && o.parent < iNodes.size() - 1u)
                {
                    if (o.kind == outline_kind::Parameter)
                        iNodes[o.parent].parameters.push_back(static_cast<std::uint32_t>(iNodes.size() - 1u));
                    else if (o.kind == outline_kind::Alternative)
                        iNodes[o.parent].alternatives.push_back(static_cast<std::uint32_t>(iNodes.size() - 1u));
                    else
                        iNodes[o.parent].children.push_back(static_cast<std::uint32_t>(iNodes.size() - 1u));
                }
                make_header(n);
                // (its key: its container's, its kind, its header and which of its container's nodes of that kind and header it is)
                auto key = (o.parent != outline_node::NoParent && o.parent < iNodes.size() - 1u ? iNodes[o.parent].key : std::u32string{}) +
                    U'/' + static_cast<char32_t>(U'0' + static_cast<std::uint32_t>(o.kind)) + n.header;
                auto const occurrence = std::to_string(occurrences[key]++);
                n.key = key + U'#' + std::u32string{ occurrence.begin(), occurrence.end() };
            }
            find_uses();
            // (the node that was selected, if it is still there (where it was))
            iSelected = NoNode;
            if (selected)
                for (std::uint32_t i = 0u; i < iNodes.size(); ++i)
                    if (iNodes[i].outline.first == selected->first && iNodes[i].outline.kind == selected->second)
                        iSelected = i;
            // (a parameter or a variable renamed in the graph: its node, if it is still where it was, as renamed, and its old name still is 
            // where it was used (else its uses are no longer known: its rename cannot be propagated))
            iRenameNode = NoNode;
            if (iRename)
            {
                for (std::uint32_t i = 0u; i < iNodes.size(); ++i)
                    if (iNodes[i].outline.first == iRename->definition && leading_name(iNodes[i].header) == iRename->to)
                        iRenameNode = i;
                bool const valid = iRenameNode != NoNode && std::all_of(iRename->uses.begin(), iRename->uses.end(), [&](position_type aUse)
                    { return aUse + iRename->from.size() <= iText.size() && iText.compare(aUse, iRename->from.size(), iRename->from) == 0; });
                if (!valid)
                {
                    iRename = std::nullopt;
                    iRenameNode = NoNode;
                }
            }
            iScriptStale = false;
            iDrag = std::nullopt;
            iLayoutPending = true;
            if (!iUserView)
                iFitPending = true;
            update();
        }
        bool has_script() const
        {
            return !iNodes.empty();
        }
        // the colours of the syntax highlighting categories (by category; none: the text colour), of text and of the background
        void set_colours(std::vector<std::optional<color>> const& aCategoryColours, color const& aText, color const& aBackground)
        {
            iCategoryColours = aCategoryColours;
            iTextColour = aText;
            iBackgroundColour = aBackground;
            iTitleGradients.clear();
            update();
        }
        // (the grid in the workspace's grid colour (a setting): its minor lines in it, its major lines (every GridSubdivisions'th) in the 
        // style's void colour, as the workspace's are)
        void set_grid_colour(neogfx::gradient const& aGridColour)
        {
            iGridColour = aGridColour;
            update();
        }
        // (where its nodes have been dragged to (by key: see set_script), as kept with the script between sessions (see LayoutChanged))
        std::map<std::u32string, point> node_layout() const
        {
            std::map<std::u32string, point> result;
            for (auto const& n : iNodes)
                if (auto const moved = iMoved.find(n.key); moved != iMoved.end())
                    result.insert(*moved);
            return result;
        }
        void set_node_layout(std::map<std::u32string, point> const& aLayout)
        {
            iMoved = aLayout;
            iLayoutPending = true;
            update();
        }
        // (laid out again: e.g. its font has changed)
        void refresh_layout()
        {
            iLayoutPending = true;
            iZoomedFont = std::nullopt;
            iZoomedLabelFont = std::nullopt;
            iLabelFont = std::nullopt;
            iGraphFont = std::nullopt;
            if (!iUserView)
                iFitPending = true;
            update();
        }
        // (the script has changed: its outline (see set_script) is out of date until it is outlined again)
        void script_changed()
        {
            if (!iScriptStale)
            {
                iScriptStale = true;
                update();
            }
        }
        // (the automatic layout: nodes that have been dragged put back in their places, and zoomed to fit (as when first shown))
        void reset_layout()
        {
            iMoved.clear();
            arrange_nodes();
            zoom_to_fit();
            LayoutChanged.trigger();
        }
        void zoom_to_fit()
        {
            iUserView = false;
            iFitPending = true;
            update();
        }
        // (debugging: the state of the breakpoint at a statement (its text's first position), as its node shows it; the statement that 
        // the program being debugged is at (its text [first, last); top: its innermost frame's, else a frame that called it), its node 
        // outlined; the statement of the node selected (none: none, or the module's), for a breakpoint)
        void set_breakpoint_query(std::function<i_script_debugger::breakpoint_state(position_type)> aQuery)
        {
            iBreakpointAt = std::move(aQuery);
            update();
        }
        void set_statement(std::optional<std::pair<position_type, position_type>> const& aStatement, bool aTop)
        {
            iStatement = aStatement;
            iStatementTop = aTop;
            update();
        }
        std::optional<position_type> selected_statement() const
        {
            if (iSelected == NoNode || iSelected >= iNodes.size() || iNodes[iSelected].outline.kind == outline_kind::Module)
                return {};
            return iNodes[iSelected].outline.first;
        }
        // the node that a position of the script is in (the innermost) selected (as the editor's cursor moves)
        void select_at(position_type aPosition)
        {
            std::uint32_t found = NoNode;
            for (std::uint32_t i = 1u; i < iNodes.size(); ++i)
                if (aPosition >= iNodes[i].outline.first && aPosition <= iNodes[i].outline.last)
                    found = i; // (pre-order: a node's descendants are after it)
            if (found != iSelected)
            {
                iSelected = found;
                update();
            }
        }
    protected:
        void resized() override
        {
            base_type::resized();
            if (!iUserView)
                iFitPending = true;
        }
        void paint(i_graphics_context& aGc) const override
        {
            if (iLayoutPending)
            {
                const_cast<script_graph&>(*this).layout_nodes(aGc);
                iLayoutPending = false;
            }
            if (iFitPending && !iNodes.empty())
            {
                const_cast<script_graph&>(*this).fit();
                iFitPending = false;
            }
            if (iMeasuredZoom != iZoom)
                const_cast<script_graph&>(*this).measure_zoomed(aGc);
            auto const cr = client_rect();
            aGc.fill_rect(cr, background_colour());
            // (the grid: its minor lines (if not too close together) in the workspace's grid colour, then its major lines)
            auto const gridStep = 32.0 * iZoom;
            auto const majorStep = gridStep * GridSubdivisions;
            if (gridStep >= 6.0)
            {
                if (iGridColour)
                    aGc.set_gradient(*iGridColour, cr);
                auto const gridColour = iGridColour ? color::White : mix(background_colour(), text_colour(), 0.07);
                for (auto x = std::fmod(iOffset.x, gridStep); x < cr.cx; x += gridStep)
                    aGc.draw_line(point{ x, 0.0 }, point{ x, cr.cy }, pen{ gridColour, 1.0 });
                for (auto y = std::fmod(iOffset.y, gridStep); y < cr.cy; y += gridStep)
                    aGc.draw_line(point{ 0.0, y }, point{ cr.cx, y }, pen{ gridColour, 1.0 });
                if (iGridColour)
                    aGc.clear_gradient();
            }
            if (majorStep >= 6.0)
            {
                auto const majorColour = service<i_app>().current_style().palette().color(color_role::Void);
                for (auto x = std::fmod(iOffset.x, majorStep); x < cr.cx; x += majorStep)
                    aGc.draw_line(point{ x, 0.0 }, point{ x, cr.cy }, pen{ majorColour, 1.0 });
                for (auto y = std::fmod(iOffset.y, majorStep); y < cr.cy; y += majorStep)
                    aGc.draw_line(point{ 0.0, y }, point{ cr.cx, y }, pen{ majorColour, 1.0 });
            }
            if (iNodes.empty())
            {
                string const message = "The script's structure is shown here once it parses."_t;
                auto const extents = aGc.text_extent(message, font());
                aGc.draw_text(point{ (cr.cx - extents.cx) / 2.0, (cr.cy - extents.cy) / 2.0 }.floor(), message, font(),
                    text_format{ text_colour().with_alpha(0.6) });
                return;
            }
            auto const& zoomedFont = zoomed_font();
            auto const& zoomedLabelFont = zoomed_label_font();
            // (the node of the statement that the program being debugged is at: the innermost that it is in)
            std::uint32_t current = NoNode;
            if (iStatement)
                for (std::uint32_t i = 1u; i < iNodes.size(); ++i)
                    if (iStatement->first >= iNodes[i].outline.first && iStatement->first < std::max(iNodes[i].outline.last, iNodes[i].outline.first + 1u))
                        current = i;
            auto const flowColour = mix(text_colour(), color::White, 0.5);
            auto const paint_node = [&](std::uint32_t i)
            {
                auto const& n = iNodes[i];
                auto const r = to_screen(n.bounds);
                if (!r.intersects(cr))
                    return;
                auto const accent = accent_colour(n);
                auto const radius = 6.0 * iZoom;
                bool const module = (n.outline.kind == outline_kind::Module);
                // (its shadow and its body; its title bar (its kind) textured (see title_gradient): lit along its top edge, finely grained 
                // and edged below; then its code (wrapped) and its inputs)
                auto const header = rect{ r.top_left(), size{ r.cx, header_height(n) * iZoom } };
                auto const title = rect{ r.top_left(), size{ r.cx, title_height() * iZoom } };
                if (!module)
                    for (int shadow = 3; shadow >= 1; --shadow)
                    {
                        auto const s = r.inflated(size{ shadow * iZoom });
                        aGc.fill_rounded_rect(rect{ s.top_left() + point{ 0.0, 2.0 * iZoom }, s.extents() }, radius + shadow * iZoom, color::Black.with_alpha(0.12));
                    }
                aGc.fill_rounded_rect(r, radius, node_colour(n));
                {
                    scoped_scissor ss{ aGc, title.intersection(cr) };
                    aGc.fill_rounded_rect(r, radius, title_gradient(accent).with_bounding_box(title));
                    aGc.fill_rounded_rect(r, radius, gloss_gradient().with_bounding_box(title));
                    if (title.cy >= 12.0)
                    {
                        pen const grain{ color::White.with_alpha(0.05), 1.0 };
                        auto x = title.x - title.cy;
                        if (cr.x - title.cy > x)
                            x += std::floor((cr.x - title.cy - x) / 5.0) * 5.0; // (from the first that is visible)
                        for (; x < std::min(title.right(), cr.right()); x += 5.0)
                            aGc.draw_line(point{ x, title.bottom() }, point{ x + title.cy, title.y }, grain);
                    }
                    aGc.draw_line(point{ title.x + radius, title.y + 0.5 }, point{ title.right() - radius, title.y + 0.5 }, pen{ color::White.with_alpha(0.3), 1.0 });
                }
                aGc.draw_line(point{ title.x, title.bottom() }, point{ title.right(), title.bottom() }, pen{ color::Black.with_alpha(0.6), 1.0 });
                bool const selected = (i == iSelected);
                if (module)
                    aGc.draw_rounded_rect(r, radius, pen{ selected ? accent : accent.with_alpha(0.55), selected ? 2.0 : 1.0, line_style::Dash });
                else
                    aGc.draw_rounded_rect(r, radius, selected ? pen{ color{ 0xF5, 0xA6, 0x23 }, 2.0 } : pen{ color::Black.with_alpha(0.7), 1.0 });
                // (debugging: the node that the program is at outlined (as the editor highlights its statement); a breakpoint's dot on 
                // its top left corner)
                if (i == current)
                    aGc.draw_rounded_rect(r.inflated(size{ 3.0 }), radius + 3.0, 
                        pen{ (iStatementTop ? color{ 0xFF, 0xE0, 0x66 } : color{ 0x8C, 0xD8, 0x8C }), 3.0 });
                if (iBreakpointAt && !module)
                {
                    auto const dotRadius = std::max(3.0, 5.0 * iZoom);
                    auto const state = iBreakpointAt(n.outline.first);
                    if (state != i_script_debugger::breakpoint_state::None)
                        aGc.fill_circle(r.top_left(), dotRadius + 1.5, background_colour());
                    if (state == i_script_debugger::breakpoint_state::Enabled)
                        aGc.fill_circle(r.top_left(), dotRadius, color{ 0xE5, 0x14, 0x00 });
                    else if (state == i_script_debugger::breakpoint_state::Disabled)
                        aGc.draw_circle(r.top_left(), dotRadius, pen{ color{ 0xE5, 0x14, 0x00 }, 1.5 });
                }
                // (its pins: where the flow passes into and out of it (filled if it does), a parameter's where it passes to its uses, 
                // and its inputs')
                auto const pinY = to_screen(point{ 0.0, flow_y(n) }).y;
                if (n.outline.kind == outline_kind::Parameter)
                    aGc.fill_circle(point{ r.right() - PinInset * iZoom, pinY }, 4.0 * iZoom, category_colour(category::Parameter));
                else if (!module)
                {
                    draw_pin(aGc, point{ r.x + PinInset * iZoom, pinY }, true, flowColour);
                    draw_pin(aGc, point{ r.right() - PinInset * iZoom, pinY }, n.flowsOut, flowColour);
                }
                for (std::size_t input = 0u; input < n.inputs.size(); ++input)
                    aGc.fill_circle(point{ r.x + PinInset * iZoom, to_screen(point{ 0.0, input_y(n, input) }).y }, 4.0 * iZoom, category_colour(category::Parameter));
                if (title.cy >= 8.0)
                    draw_icon(aGc, n, point{ title.x + (padding_x() + IconSpace / 2.0) * iZoom, title.center().y }, 5.0 * iZoom, mix(accent, color::White, 0.6));
                // (a renamed parameter or variable: its button that propagates its rename to its uses)
                bool const renamed = (i == iRenameNode && !iScriptStale);
                if (zoomedFont.height() < MinTextHeight)
                {
                    if (renamed)
                        draw_propagate(aGc, propagate_rect(n), accent);
                    return;
                }
                scoped_scissor ss{ aGc, header.deflated(size{ 2.0 }).intersection(cr) };
                if (zoomedLabelFont.height() >= MinTextHeight)
                    aGc.draw_text(point{ title.x + (padding_x() + IconSpace + 4.0) * iZoom, title.y + (title.cy - zoomedLabelFont.height()) / 2.0 },
                        n.label, zoomedLabelFont, text_format{ color::White.with_alpha(0.95) });
                auto const lineHeight = line_height() * iZoom;
                point const textPos{ header.x + CodeInset * iZoom, title.bottom() + CodePadding * iZoom + (lineHeight - zoomedFont.height()) / 2.0 };
                for (std::size_t run = 0u; run < n.runs.size() && run < n.zoomedOffsets.size(); ++run)
                    aGc.draw_text(point{ textPos.x + n.zoomedOffsets[run], textPos.y + n.runs[run].line * lineHeight }, n.runs[run].text, zoomedFont, 
                        text_format{ category_colour(n.runs[run].kind) });
                for (std::size_t input = 0u; input < n.inputNames.size(); ++input)
                    aGc.draw_text(point{ textPos.x, to_screen(point{ 0.0, input_y(n, input) }).y - zoomedFont.height() / 2.0 }, n.inputNames[input], zoomedFont, 
                        text_format{ text_colour().with_alpha(0.8) });
                if (renamed)
                    draw_propagate(aGc, propagate_rect(n), accent);
            };
            // (a node, then the flows in its body (under what is in it); a node being dragged (and its contents) over the others)
            auto const paint = [&](std::uint32_t i)
            {
                paint_node(i);
                auto const& n = iNodes[i];
                if (!to_screen(n.bounds).intersects(cr))
                    return;
                for (auto f = n.firstFlow; f < n.firstFlow + n.flowCount && f < iFlows.size(); ++f)
                {
                    auto const& flow = iFlows[f];
                    if (flow.entry)
                        aGc.fill_circle(to_screen(flow.route.front()), 3.0 * iZoom, flowColour);
                    draw_route(aGc, flow.route, flow.loop ? pen{ flowColour.with_alpha(0.6), 1.5 * iZoom, line_style::Dash } : pen{ flowColour, 2.0 * iZoom });
                }
            };
            auto const dragged = iDrag && iDrag->dragging ? iDrag->node : NoNode;
            for (std::uint32_t i = 0u; i < iNodes.size(); ++i)
                if (dragged == NoNode || (i != dragged && !is_descendant(i, dragged)))
                    paint(i);
            if (dragged != NoNode)
                for (auto i = dragged; i < iNodes.size() && (i == dragged || is_descendant(i, dragged)); ++i)
                    paint(i);
            // (the parameters' wires to their uses' inputs (routed: see route_flows), over the nodes (a use may be in a statement in 
            // another's body))
            for (auto const& wire : iWires)
                draw_route(aGc, wire.route, pen{ category_colour(category::Parameter), 2.0 * iZoom });
            // (where a statement being dragged would be moved to: after the sibling before that place, if it is in the row that the 
            // statement is level with, else before the sibling at it)
            if (iDrag && iDrag->dragging && iDrag->dropIndex)
            {
                auto const& n = iNodes[iDrag->node];
                auto const& siblings = iNodes[n.outline.parent].children;
                auto const index = *iDrag->dropIndex;
                bool const after = index > 0u && (index == siblings.size() || in_row(iNodes[siblings[index - 1u]], n.bounds.center()));
                auto const& sibling = iNodes[siblings[after ? index - 1u : index]];
                auto const x = after ? sibling.bounds.right() + gap() / 2.0 : sibling.bounds.x - gap() / 2.0;
                aGc.draw_line(to_screen(point{ x, sibling.bounds.y - 4.0 }), to_screen(point{ x, sibling.bounds.bottom() + 4.0 }), pen{ color{ 0xF5, 0xA6, 0x23 }, 3.0 });
            }
            // (the zoom)
            auto const zoomText = string{ std::to_string(static_cast<int>(std::round(iZoom * 100.0))) + "%" };
            auto const zoomExtents = aGc.text_extent(zoomText, font());
            auto const zoomRect = rect{ point{ cr.right() - zoomExtents.cx - 20.0, cr.bottom() - zoomExtents.cy - 14.0 }, zoomExtents + size{ 12.0, 6.0 } };
            aGc.fill_rounded_rect(zoomRect, 4.0, background_colour().with_alpha(0.85));
            aGc.draw_text(zoomRect.top_left() + point{ 6.0, 3.0 }, zoomText, font(), text_format{ text_colour().with_alpha(0.7) });
        }
    protected:
        bool mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier) override
        {
            if ((aWheel & mouse_wheel::Vertical) == mouse_wheel::None || aDelta.dy == 0.0)
                return base_type::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifier);
            // (zoomed about the point under the mouse)
            auto const zoom = std::clamp(iZoom * (aDelta.dy > 0.0 ? 1.15 : 1.0 / 1.15), MinZoom, MaxZoom);
            auto const graphPoint = to_graph(aPosition);
            iZoom = zoom;
            iOffset = aPosition - graphPoint * iZoom;
            iUserView = true;
            iZoomedFont = std::nullopt;
            iZoomedLabelFont = std::nullopt;
            update();
            return true;
        }
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override
        {
            if (aButton == mouse_button::Middle)
            {
                iPan = pan{ aPosition, iOffset };
                set_capture(capture_reason::MouseEvent, aPosition);
                return;
            }
            base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
            if (aButton != mouse_button::Left)
                return;
            set_focus();
            if (iRenameNode != NoNode && !iScriptStale && propagate_rect(iNodes[iRenameNode]).contains(aPosition))
            {
                propagate();
                return;
            }
            auto const n = node_at(aPosition);
            if (n != NoNode && n != 0u)
            {
                select(n);
                iDrag = drag{ aPosition, n, iNodes[n].offset };
            }
            else
                iPan = pan{ aPosition, iOffset };
        }
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override
        {
            base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifier);
            if (aButton != mouse_button::Left)
                return;
            auto const n = node_at(aPosition);
            if (n != NoNode && n != 0u)
            {
                select(n);
                begin_edit(n, false);
            }
        }
        void mouse_moved(const point& aPosition, key_modifier aKeyModifier) override
        {
            base_type::mouse_moved(aPosition, aKeyModifier);
            if (iPan)
            {
                iOffset = iPan->offset + (aPosition - iPan->from);
                iUserView = true;
                update();
            }
            else if (iDrag)
            {
                auto const& n = iNodes[iDrag->node];
                if (!iDrag->dragging && (aPosition - iDrag->from).to_delta().magnitude() > 6.0 && n.outline.parent != outline_node::NoParent)
                    iDrag->dragging = true;
                if (iDrag->dragging)
                {
                    // (the node (and its contents) where the mouse has moved it (its container enclosing it))
                    iMoved[n.key] = iDrag->offset + (to_graph(aPosition) - to_graph(iDrag->from));
                    arrange_nodes();
                    iDrag->dropIndex = drop_index(iDrag->node);
                    update();
                }
            }
        }
        void mouse_button_released(mouse_button aButton, const point& aPosition) override
        {
            base_type::mouse_button_released(aButton, aPosition);
            if (iPan)
            {
                iPan = std::nullopt;
                if (capturing())
                    release_capture(capture_reason::MouseEvent);
            }
            if (iDrag)
            {
                auto const drag = *iDrag;
                iDrag = std::nullopt;
                if (drag.dragging)
                {
                    // (moved among its siblings in a row: the statement moved there in the script (and laid out in its place); else left 
                    // where it was dropped (put back in its place if it was dropped near it), the statements after it following it)
                    auto const& n = iNodes[drag.node];
                    if (drag.dropIndex)
                    {
                        iMoved.erase(n.key);
                        arrange_nodes();
                        move_node(drag.node, *drag.dropIndex);
                    }
                    else if (std::hypot(n.offset.x, n.offset.y) < gap() / 3.0)
                        iMoved.erase(n.key);
                    else
                        iMoved[n.key] = n.offset;
                    arrange_nodes();
                    LayoutChanged.trigger();
                }
                update();
            }
        }
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override
        {
            bool const ctrl = (aKeyModifier & key_modifier::CTRL) != key_modifier::None;
            switch (aScanCode)
            {
            case ScanCode_DELETE:
                if (iSelected != NoNode)
                    delete_node(iSelected);
                return true;
            case ScanCode_F2:
            case ScanCode_RETURN:
            case ScanCode_KEYPAD_ENTER:
                if (iSelected != NoNode && iSelected != 0u)
                    begin_edit(iSelected, false);
                return true;
            case ScanCode_INSERT:
                begin_edit(iSelected != NoNode ? iSelected : 0u, true);
                return true;
            case ScanCode_D:
                if (!ctrl)
                    break;
                if (iSelected != NoNode)
                    duplicate_node(iSelected);
                return true;
            case ScanCode_0:
            case ScanCode_HOME:
                if (aScanCode == ScanCode_0 && !ctrl)
                    break;
                zoom_to_fit();
                return true;
            case ScanCode_ESCAPE:
                if (iSelected != NoNode)
                {
                    iSelected = NoNode;
                    update();
                }
                return true;
            default:
                break;
            }
            return base_type::key_pressed(aScanCode, aKeyCode, aKeyModifier);
        }
    private:
        // (a node's header: its code before its body (if it has one), on one line, in its syntax highlighting categories)
        void make_header(node& aNode) const
        {
            auto const& o = aNode.outline;
            if (o.kind == outline_kind::Module)
                return;
            auto end = std::max(o.first, o.headerLast);
            while (end > o.first && is_space(iText[end - 1u]))
                --end;
            for (auto p = o.first; p < end; ++p)
            {
                auto ch = iText[p];
                if (ch == U'\r')
                    continue;
                if (is_space(ch))
                {
                    if (!aNode.header.empty() && aNode.header.back() == U' ')
                        continue;
                    ch = U' ';
                }
                if (aNode.header.size() == MaxHeaderLength)
                {
                    aNode.header.back() = U'\x2026';
                    aNode.headerCategories.back() = std::nullopt;
                    break;
                }
                aNode.header.push_back(ch);
                aNode.headerCategories.push_back(iCategories[p]);
            }
        }
        // (the nodes whose headers use a function's parameters: their names there, highlighted as parameters)
        void find_uses()
        {
            for (auto& n : iNodes)
            {
                if (n.outline.kind != outline_kind::Parameter)
                    continue;
                std::u32string name;
                for (auto ch : n.header)
                {
                    if (!(ch == U'_' || ch == U'$' || ch > 0x7Fu || std::isalnum(static_cast<int>(ch))))
                        break;
                    name.push_back(ch);
                }
                if (name.empty())
                    continue;
                auto const function = n.outline.parent;
                for (std::uint32_t i = function + 1u; i < iNodes.size() && is_descendant(i, function); ++i)
                {
                    auto& user = iNodes[i];
                    if (user.outline.kind == outline_kind::Parameter)
                        continue;
                    auto const end = std::max(user.outline.first, user.outline.headerLast);
                    for (auto p = user.outline.first; p + name.size() <= end; ++p)
                        if (iCategories[p] == category::Parameter && iText.compare(p, name.size(), name) == 0 &&
                            (p == user.outline.first || !is_name(iText[p - 1u])) && (p + name.size() == iText.size() || !is_name(iText[p + name.size()])))
                        {
                            n.uses.push_back(i);
                            iNodes[i].inputs.push_back(static_cast<std::uint32_t>(&n - &iNodes[0]));
                            break;
                        }
                }
            }
        }
        bool is_descendant(std::uint32_t aNode, std::uint32_t aAncestor) const
        {
            for (auto n = iNodes[aNode].outline.parent; n != outline_node::NoParent; n = iNodes[n].outline.parent)
                if (n == aAncestor)
                    return true;
            return false;
        }
        static bool is_space(char32_t aCh)
        {
            return aCh == U' ' || aCh == U'\t' || aCh == U'\n' || aCh == U'\r';
        }
        static bool is_operator(char32_t aCh)
        {
            return std::u32string_view{ U"=+-*/%<>!&|^~:.@" }.find(aCh) != std::u32string_view::npos;
        }
        static bool is_name(char32_t aCh)
        {
            return aCh == U'_' || aCh == U'$' || aCh > 0x7Fu || (aCh < 0x80u && std::isalnum(static_cast<int>(aCh)));
        }
    private:
        dimension padding_x() const { return 10.0; }
        dimension gap() const { return 36.0; }
        // (a node's header: its title bar (its kind), then its code's lines (if it has code), above and below which is CodePadding, then 
        // a row for each parameter that its code uses (its inputs))
        static constexpr dimension CodePadding = 4.0;
        dimension title_height() const { return std::ceil(label_font().height()) + 10.0; }
        dimension line_height() const { return std::ceil(graph_font().height()) + 2.0; }
        dimension code_height(node const& aNode) const 
        { 
            return aNode.lineCount == 0u ? 0.0 : aNode.lineCount * line_height() + CodePadding * 2.0;
        }
        dimension header_height(node const& aNode) const 
        { 
            return title_height() + code_height(aNode) + (aNode.inputs.empty() ? 0.0 : aNode.inputs.size() * line_height() + CodePadding);
        }
        // (its code's (between its pins), its title's (its icon, its kind and room for a propagate button) and its inputs' (after their 
        // pins), whichever is widest)
        dimension header_width(dimension aTextWidth, dimension aLabelWidth, dimension aInputsWidth) const
        {
            return std::max({ aTextWidth + CodeInset * 2.0, padding_x() * 2.0 + IconSpace + 4.0 + aLabelWidth + title_height(), 
                CodeInset + aInputsWidth + padding_x() });
        }
        // (where the flow passes into and out of a node: its first line of code's middle (a module's title bar's))
        dimension flow_y(node const& aNode) const 
        { 
            return aNode.bounds.y + title_height() + (aNode.lineCount == 0u ? -title_height() / 2.0 : CodePadding + line_height() / 2.0);
        }
        point in_pin(node const& aNode) const { return point{ aNode.bounds.x + PinInset - PinSize * 0.8, flow_y(aNode) }; }
        point out_pin(node const& aNode) const { return point{ aNode.bounds.right() - PinInset + PinSize, flow_y(aNode) }; }
        // (an input's row's middle)
        dimension input_y(node const& aNode, std::size_t aInput) const 
        { 
            return aNode.bounds.y + title_height() + code_height(aNode) + (aInput + 0.5) * line_height();
        }
        // (a node's header's code wrapped: its lines (each [first, last) of it), of at most WrapLength characters (a continuation line, 
        // indented when drawn, two fewer), broken after the last space, opening bracket, comma or operator that fits (not in a string), 
        // else after the last space in a string that fits, else at the length)
        std::vector<std::pair<std::size_t, std::size_t>> wrap(node const& aNode) const
        {
            auto const& text = aNode.header;
            std::vector<std::pair<std::size_t, std::size_t>> lines;
            std::size_t start = 0u;
            while (start < text.size())
            {
                auto const limit = WrapLength - (lines.empty() ? 0u : 2u);
                if (text.size() - start <= limit)
                {
                    lines.emplace_back(start, text.size());
                    break;
                }
                // (else after a space in a string, else at the length)
                auto end = start + limit;
                bool found = false;
                for (bool const inStrings : { false, true })
                    for (auto p = start + limit; !found && p > start + 1u; --p)
                    {
                        auto const before = text[p - 1u];
                        bool const inString = aNode.headerCategories[p - 1u] == category::String && aNode.headerCategories[p] == category::String;
                        // (not within an operator of more than one character, e.g. ->, ==, **)
                        bool const withinOperator = is_operator(before) && is_operator(text[p]);
                        if (inStrings ? before == U' ' : 
                            !inString && !withinOperator && std::u32string_view{ U" ([{,=+-*/:" }.find(before) != std::u32string_view::npos)
                        {
                            end = p;
                            found = true;
                        }
                    }
                auto lineEnd = end;
                while (lineEnd > start && text[lineEnd - 1u] == U' ')
                    --lineEnd;
                lines.emplace_back(start, lineEnd);
                start = end;
                while (start < text.size() && text[start] == U' ')
                    ++start;
            }
            return lines;
        }
        // (the nodes' headers measured, then the nodes arranged (see arrange_nodes))
        void layout_nodes(i_graphics_context const& aGc)
        {
            if (iNodes.empty())
                return;
            auto const& labelFont = label_font();
            // (bottom up: a node is before its descendants)
            for (auto i = iNodes.size(); i-- > 0u;)
            {
                auto& n = iNodes[i];
                // (its header's code wrapped (its runs line by line: a continuation line indented) and its kind measured (once: painting 
                // only draws them))
                n.runs.clear();
                auto const lines = wrap(n);
                n.lineCount = static_cast<std::uint32_t>(lines.size());
                dimension textWidth = 0.0;
                for (std::uint32_t line = 0u; line < lines.size(); ++line)
                {
                    dimension lineWidth = 0.0;
                    if (line > 0u)
                        lineWidth += aGc.text_extent(n.runs.emplace_back(run{ string{ "  " }, std::nullopt, 0.0, line }).text, graph_font()).cx;
                    for (auto first = lines[line].first; first < lines[line].second;)
                    {
                        auto last = first + 1u;
                        while (last < lines[line].second && n.headerCategories[last] == n.headerCategories[first])
                            ++last;
                        auto& r = n.runs.emplace_back(run{ string{ neolib::utf32_to_utf8(n.header.substr(first, last - first)) }, n.headerCategories[first], lineWidth, line });
                        lineWidth += aGc.text_extent(r.text, graph_font()).cx;
                        first = last;
                    }
                    textWidth = std::max(textWidth, lineWidth);
                }
                n.label = kind_label(n);
                n.textWidth = textWidth;
                n.labelWidth = aGc.text_extent(n.label, labelFont).cx;
                n.inputNames.clear();
                n.inputsWidth = 0.0;
                for (auto input : n.inputs)
                    n.inputsWidth = std::max(n.inputsWidth, 
                        aGc.text_extent(n.inputNames.emplace_back(neolib::utf32_to_utf8(leading_name(iNodes[input].header))), graph_font()).cx);
                n.headerWidth = header_width(textWidth, n.labelWidth, n.inputsWidth);
            }
            arrange_nodes();
            iMeasuredZoom = 0.0; // (measured in the zoomed font when next painted)
        }
        // (the nodes' headers' runs' offsets measured in the zoomed font, as they are drawn (their boxes are as measured unzoomed: the 
        // graph's fonts are unhinted (see graph_font), so their text scales with them))
        void measure_zoomed(i_graphics_context const& aGc)
        {
            iMeasuredZoom = iZoom;
            for (auto& n : iNodes)
            {
                n.zoomedOffsets.clear();
                dimension offset = 0.0;
                for (std::size_t r = 0u; r < n.runs.size(); ++r)
                {
                    if (r > 0u && n.runs[r].line != n.runs[r - 1u].line)
                        offset = 0.0;
                    n.zoomedOffsets.push_back(offset);
                    offset += aGc.text_extent(n.runs[r].text, zoomed_font()).cx;
                }
                n.zoomedLabelWidth = aGc.text_extent(n.label, zoomed_label_font()).cx;
            }
        }
        // (the nodes' extents and positions, in graph coordinates, from their headers as measured: a body's statements left to right, a 
        // function's parameters before them; a node that has been dragged is where it was dragged to, 
        // from its place (but not before its container's first), its container enclosing it, and the statements after it follow it (as 
        // they would if it were not being dragged, while it is: see drop_index); then the flows routed (see route_flows))
        void arrange_nodes()
        {
            if (iNodes.empty())
                return;
            auto const padding = padding_x();
            // (bottom up: a node is before its descendants)
            for (auto i = iNodes.size(); i-- > 0u;)
            {
                auto& n = iNodes[i];
                size lane;
                point place;
                for (auto c : n.children)
                {
                    auto& child = iNodes[c];
                    child.place = place;
                    child.offset = moved_offset(child, place);
                    auto const at = place + child.offset;
                    lane.cx = std::max(lane.cx, at.x + child.extents.cx);
                    lane.cy = std::max(lane.cy, at.y + child.extents.cy);
                    auto const followed = (iDrag && iDrag->dragging && c == iDrag->node) ? place + iDrag->offset : at;
                    place = point{ followed.x + child.extents.cx + gap(), followed.y };
                }
                // (its alternatives below its body, each on a lane of its own)
                size content = lane;
                dimension alternativeY = n.children.empty() ? 0.0 : lane.cy + gap() / 2.0;
                for (auto a : n.alternatives)
                {
                    auto& alternative = iNodes[a];
                    alternative.offset = moved_offset(alternative, point{ 0.0, alternativeY });
                    content.cx = std::max(content.cx, alternative.offset.x + alternative.extents.cx);
                    content.cy = std::max(content.cy, alternativeY + alternative.offset.y + alternative.extents.cy);
                    alternativeY += alternative.extents.cy + gap() / 2.0;
                }
                // (its parameters below its body's statements: their wires pass beneath the statements before the ones that use them)
                n.parametersTop = n.children.empty() && n.alternatives.empty() ? 0.0 : content.cy + gap() / 2.0;
                size parameters;
                dimension slotY = n.parametersTop;
                for (auto p : n.parameters)
                {
                    auto& parameter = iNodes[p];
                    parameter.offset = moved_offset(parameter, point{ 0.0, slotY });
                    parameters.cx = std::max(parameters.cx, parameter.offset.x + parameter.extents.cx);
                    parameters.cy = std::max(parameters.cy, slotY + parameter.offset.y + parameter.extents.cy);
                    slotY += parameter.extents.cy + 8.0;
                }
                n.parametersWidth = parameters.cx;
                n.laneExtents = lane;
                n.extents = size{ std::max(n.headerWidth, 80.0), header_height(n) };
                if (!n.parameters.empty() || !n.children.empty() || !n.alternatives.empty())
                {
                    auto const laneStart = padding + (n.parameters.empty() ? 24.0 : parameters.cx + 48.0);
                    n.extents.cx = std::max(n.extents.cx, laneStart + content.cx + padding + (n.outline.kind == outline_kind::Loop ? 12.0 : 0.0));
                    n.extents.cy += padding + std::max(parameters.cy, content.cy) + padding + (n.outline.kind == outline_kind::Loop ? 10.0 : 0.0);
                }
            }
            // (top down: a node's position is its container's)
            iNodes[0].bounds = rect{ point{}, iNodes[0].extents };
            for (auto& n : iNodes)
            {
                auto const top = n.bounds.y + header_height(n) + padding;
                point p{ n.bounds.x + padding, top + n.parametersTop };
                for (auto i : n.parameters)
                {
                    iNodes[i].bounds = rect{ p + iNodes[i].offset, iNodes[i].extents };
                    p.y += iNodes[i].extents.cy + 8.0;
                }
                auto const laneStart = n.bounds.x + padding + (n.parameters.empty() ? 24.0 : n.parametersWidth + 48.0);
                for (auto i : n.children)
                    iNodes[i].bounds = rect{ point{ laneStart, top } + iNodes[i].place + iNodes[i].offset, iNodes[i].extents };
                n.lane = rect{ point{ laneStart, top }, n.laneExtents };
                p = point{ laneStart, top + (n.children.empty() ? 0.0 : n.laneExtents.cy + gap() / 2.0) };
                for (auto i : n.alternatives)
                {
                    iNodes[i].bounds = rect{ p + iNodes[i].offset, iNodes[i].extents };
                    p.y += iNodes[i].extents.cy + gap() / 2.0;
                }
            }
            route_flows();
        }
        // (the flows in each node's body: into its first statement (from its entry), from each statement to the next, back to a loop's 
        // start, and into each alternative of a branch; and a function's parameters' wires to their uses; each routed (see route) around 
        // what else is in the body)
        void route_flows()
        {
            iFlows.clear();
            iWires.clear();
            for (auto& n : iNodes)
                n.flowsOut = false;
            for (auto& n : iNodes)
            {
                n.firstFlow = iFlows.size();
                n.flowCount = 0u;
                if (n.children.empty() && n.alternatives.empty() && n.parameters.empty())
                    continue;
                auto const headerHeight = header_height(n);
                std::vector<rect> obstacles{ rect{ n.bounds.top_left(), size{ n.bounds.cx, headerHeight } } };
                for (auto const* members : { &n.children, &n.alternatives, &n.parameters })
                    for (auto m : *members)
                        obstacles.push_back(iNodes[m].bounds);
                rect const area{ point{ n.bounds.x, n.bounds.y + headerHeight }, size{ n.bounds.cx, n.bounds.cy - headerHeight } };
                // (its parameters' wires to their uses' inputs)
                for (auto p : n.parameters)
                    for (auto use : iNodes[p].uses)
                    {
                        auto const& user = iNodes[use];
                        auto const input = static_cast<std::size_t>(std::find(user.inputs.begin(), user.inputs.end(), p) - user.inputs.begin());
                        if (input < user.inputs.size())
                            iWires.push_back(flow{ route(point{ iNodes[p].bounds.right() - PinInset, flow_y(iNodes[p]) }, 
                                point{ user.bounds.x + PinInset, input_y(user, input) }, obstacles, area, true) });
                    }
                for (auto a : n.alternatives)
                {
                    auto const& alternative = iNodes[a];
                    iFlows.push_back(flow{ { point{ alternative.bounds.x - 14.0, flow_y(alternative) }, in_pin(alternative) }, true });
                }
                if (!n.children.empty())
                {
                    auto const& first = iNodes[n.children.front()];
                    point const entry{ n.lane.x - 18.0, flow_y(first) };
                    iFlows.push_back(flow{ route(entry, in_pin(first), obstacles, area, true), true });
                    for (std::size_t c = 1u; c < n.children.size(); ++c)
                    {
                        auto& from = iNodes[n.children[c - 1u]];
                        from.flowsOut = true;
                        iFlows.push_back(flow{ route(out_pin(from), in_pin(iNodes[n.children[c]]), obstacles, area, true) });
                    }
                    if (n.outline.kind == outline_kind::Loop)
                    {
                        auto& last = iNodes[n.children.back()];
                        last.flowsOut = true;
                        iFlows.push_back(flow{ route(out_pin(last), point{ n.lane.x - 10.0, entry.y }, obstacles, area, false), false, true });
                    }
                }
                n.flowCount = iFlows.size() - n.firstFlow;
            }
        }
        // (a flow's route from a point (going right) to a point (going right into it, else up into it), within an area: an S-curve (a 
        // straight line, if they are level) if that passes none of the obstacles (nodes), else the cheapest path along the lines that are a margin from them (a path's cost: 
        // its length, more where it is within the margin of an obstacle, much more where it crosses one, and more for each turn), so that 
        // it crosses nodes only if it must)
        std::vector<point> route(point const& aFrom, point const& aTo, std::vector<rect> const& aObstacles, rect const& aArea, bool aIntoRight) const
        {
            auto const margin = RouteMargin;
            auto const inside = [](point const& aPoint, rect const& aRect, dimension aInset)
            {
                return aPoint.x > aRect.x + aInset && aPoint.x < aRect.right() - aInset && aPoint.y > aRect.y + aInset && aPoint.y < aRect.bottom() - aInset;
            };
            // (the nodes that it starts and ends in (its pins are inside them): an S-curve leaves and enters them level with its pins)
            auto const containing = [&](point const& aPoint) -> rect const*
            {
                rect const* result = nullptr;
                for (auto const& o : aObstacles)
                    if (inside(aPoint, o, 0.0) && (!result || o.cx * o.cy < result->cx * result->cy))
                        result = &o;
                return result;
            };
            auto const fromNode = containing(aFrom);
            auto const toNode = containing(aTo);
            if (aIntoRight && aTo.x - aFrom.x > (std::abs(aTo.y - aFrom.y) < 0.5 ? 0.0 : margin * 2.0))
            {
                auto const dx = (aTo.x - aFrom.x) / 2.0;
                bool clear = true;
                for (int sample = 1; clear && sample < 24; ++sample)
                {
                    auto const t = sample / 24.0;
                    auto const u = 1.0 - t;
                    point const p{ u * u * u * aFrom.x + 3.0 * u * u * t * (aFrom.x + dx) + 3.0 * u * t * t * (aTo.x - dx) + t * t * t * aTo.x,
                        u * u * u * aFrom.y + 3.0 * u * u * t * aFrom.y + 3.0 * u * t * t * aTo.y + t * t * t * aTo.y };
                    clear = std::none_of(aObstacles.begin(), aObstacles.end(), 
                        [&](rect const& o) { return &o != fromNode && &o != toNode && inside(p, o, 3.0); });
                }
                if (clear)
                    return { aFrom, aTo };
            }
            point const start{ (fromNode ? fromNode->right() : aFrom.x) + margin, aFrom.y };
            point const end = aIntoRight ? point{ (toNode ? toNode->x : aTo.x) - margin, aTo.y } : point{ aTo.x, aTo.y + margin };
            // (the lines: through the start and the end, a margin from each obstacle and inside the area's edges)
            std::vector<coordinate> xs{ start.x, end.x, aArea.x + 4.0, aArea.right() - 4.0 };
            std::vector<coordinate> ys{ start.y, end.y, aArea.bottom() - 4.0 };
            for (auto const& o : aObstacles)
            {
                xs.push_back(o.x - margin);
                xs.push_back(o.right() + margin);
                ys.push_back(o.y - margin);
                ys.push_back(o.bottom() + margin);
            }
            auto const tidy = [](std::vector<coordinate>& aLines, coordinate aMin, coordinate aMax, coordinate aStart, coordinate aEnd)
            {
                std::erase_if(aLines, [&](coordinate c) { return (c < aMin || c > aMax) && c != aStart && c != aEnd; });
                std::sort(aLines.begin(), aLines.end());
                aLines.erase(std::unique(aLines.begin(), aLines.end(), [](coordinate a, coordinate b) { return b - a < 0.5; }), aLines.end());
            };
            tidy(xs, aArea.x + 1.0, aArea.right() - 1.0, start.x, end.x);
            tidy(ys, aArea.y + 1.0, aArea.bottom() - 1.0, start.y, end.y);
            auto const nearest = [](std::vector<coordinate> const& aLines, coordinate c)
            {
                return static_cast<std::size_t>(std::min_element(aLines.begin(), aLines.end(), 
                    [&](coordinate a, coordinate b) { return std::abs(a - c) < std::abs(b - c); }) - aLines.begin());
            };
            auto const nx = xs.size();
            auto const ny = ys.size();
            // (each step's cost (from a crossing to the next, right or down))
            auto const cost = [&](point const& p, point const& q)
            {
                bool const horizontal = (p.y == q.y);
                auto const lo = horizontal ? std::min(p.x, q.x) : std::min(p.y, q.y);
                auto const hi = horizontal ? std::max(p.x, q.x) : std::max(p.y, q.y);
                double result = hi - lo;
                for (auto const& o : aObstacles)
                    for (auto const& [inflation, weight] : { std::pair{ 0.0, 40.0 }, std::pair{ margin - 1.0, 1.5 } })
                    {
                        auto const r = o.inflated(size{ inflation });
                        if (horizontal ? (p.y <= r.y || p.y >= r.bottom()) : (p.x <= r.x || p.x >= r.right()))
                            continue;
                        auto const overlap = std::min(hi, horizontal ? r.right() : r.bottom()) - std::max(lo, horizontal ? r.x : r.y);
                        if (overlap > 0.0)
                            result += overlap * weight;
                    }
                return result;
            };
            std::vector<double> across(nx * ny, 0.0);
            std::vector<double> down(nx * ny, 0.0);
            for (std::size_t y = 0u; y < ny; ++y)
                for (std::size_t x = 0u; x < nx; ++x)
                {
                    if (x + 1u < nx)
                        across[y * nx + x] = cost(point{ xs[x], ys[y] }, point{ xs[x + 1u], ys[y] });
                    if (y + 1u < ny)
                        down[y * nx + x] = cost(point{ xs[x], ys[y] }, point{ xs[x], ys[y + 1u] });
                }
            // (the cheapest path: its states each a crossing and the direction that it was arrived at in (right, left, down, up))
            auto const turn = margin * 2.0;
            auto const state = [nx](std::size_t x, std::size_t y, std::size_t d) { return (y * nx + x) * 4u + d; };
            auto const none = std::numeric_limits<std::size_t>::max();
            std::vector<double> distance(nx * ny * 4u, std::numeric_limits<double>::infinity());
            std::vector<std::size_t> previous(nx * ny * 4u, none);
            using entry = std::pair<double, std::size_t>;
            std::priority_queue<entry, std::vector<entry>, std::greater<entry>> queue;
            auto const sx = nearest(xs, start.x);
            auto const sy = nearest(ys, start.y);
            auto const ex = nearest(xs, end.x);
            auto const ey = nearest(ys, end.y);
            distance[state(sx, sy, 0u)] = 0.0;
            queue.emplace(0.0, state(sx, sy, 0u));
            while (!queue.empty())
            {
                auto const [d, current] = queue.top();
                queue.pop();
                if (d > distance[current])
                    continue;
                auto const direction = current % 4u;
                auto const x = (current / 4u) % nx;
                auto const y = (current / 4u) / nx;
                for (std::size_t next = 0u; next < 4u; ++next)
                {
                    if ((next ^ 1u) == direction)
                        continue; // (not back the way it came)
                    std::size_t nextX = x;
                    std::size_t nextY = y;
                    double step = 0.0;
                    switch (next)
                    {
                    case 0u: if (x + 1u >= nx) continue; nextX = x + 1u; step = across[y * nx + x]; break;
                    case 1u: if (x == 0u) continue; nextX = x - 1u; step = across[y * nx + nextX]; break;
                    case 2u: if (y + 1u >= ny) continue; nextY = y + 1u; step = down[y * nx + x]; break;
                    default: if (y == 0u) continue; nextY = y - 1u; step = down[nextY * nx + x]; break;
                    }
                    auto const nextState = state(nextX, nextY, next);
                    auto const nextDistance = d + step + (next != direction ? turn : 0.0);
                    if (nextDistance < distance[nextState])
                    {
                        distance[nextState] = nextDistance;
                        previous[nextState] = current;
                        queue.emplace(nextDistance, nextState);
                    }
                }
            }
            // (arriving going right (into its end), else up)
            auto const arrival = aIntoRight ? 0u : 3u;
            std::size_t best = none;
            double bestDistance = std::numeric_limits<double>::infinity();
            for (std::size_t direction = 0u; direction < 4u; ++direction)
            {
                auto const d = distance[state(ex, ey, direction)] + (direction != arrival ? turn : 0.0);
                if (d < bestDistance)
                {
                    bestDistance = d;
                    best = state(ex, ey, direction);
                }
            }
            if (best == none || bestDistance == std::numeric_limits<double>::infinity())
                return { aFrom, aTo };
            std::vector<point> path{ aTo };
            for (auto s = best; s != none; s = previous[s])
                path.push_back(point{ xs[(s / 4u) % nx], ys[(s / 4u) / nx] });
            path.push_back(aFrom);
            std::reverse(path.begin(), path.end());
            // (its corners only: no point between two in line with them)
            auto const corners = [](std::vector<point> const& aPath)
            {
                std::vector<point> result;
                for (auto const& p : aPath)
                {
                    if (!result.empty() && result.back() == p)
                        continue;
                    if (result.size() >= 2u)
                    {
                        auto const& a = result[result.size() - 2u];
                        auto const& b = result.back();
                        if ((a.x == b.x && b.x == p.x) || (a.y == b.y && b.y == p.y))
                            result.pop_back();
                    }
                    result.push_back(p);
                }
                return result;
            };
            auto result = corners(path);
            // (each of its segments between corners (not its first or last) in the middle of the channel that it is in (between what 
            // is on either side of it, or the area's edge): so that it moves smoothly as the nodes do, not from one of the lines that it 
            // was routed along to another; but not past the far ends of the segments before and after it (it would turn back on itself), 
            // nor nearer its start or its end than the margin (its pins are there))
            for (std::size_t i = 1u; i + 2u < result.size(); ++i)
            {
                auto& p = result[i];
                auto& q = result[i + 1u];
                bool const horizontal = (p.y == q.y);
                auto lowest = -std::numeric_limits<coordinate>::infinity();
                auto highest = std::numeric_limits<coordinate>::infinity();
                for (std::size_t neighbour : { i - 1u, i + 2u })
                {
                    auto const c = horizontal ? result[neighbour].y : result[neighbour].x;
                    auto const old = horizontal ? p.y : p.x;
                    auto const gap = (neighbour == 0u || neighbour + 1u == result.size()) ? margin : 0.0;
                    if (old > c)
                        lowest = std::max(lowest, c + gap);
                    else if (old < c)
                        highest = std::min(highest, c - gap);
                }
                auto const position = horizontal ? p.y : p.x;
                auto const lo = horizontal ? std::min(p.x, q.x) : std::min(p.y, q.y);
                auto const hi = horizontal ? std::max(p.x, q.x) : std::max(p.y, q.y);
                auto before = horizontal ? aArea.y : aArea.x;
                auto after = horizontal ? aArea.bottom() : aArea.right();
                bool crosses = false;
                for (auto const& o : aObstacles)
                {
                    auto const oLo = horizontal ? o.x : o.y;
                    auto const oHi = horizontal ? o.right() : o.bottom();
                    if (oHi <= lo || oLo >= hi)
                        continue;
                    auto const oStart = horizontal ? o.y : o.x;
                    auto const oEnd = horizontal ? o.bottom() : o.right();
                    if (oEnd <= position + 0.5)
                        before = std::max(before, oEnd);
                    else if (oStart >= position - 0.5)
                        after = std::min(after, oStart);
                    else
                        crosses = true;
                }
                if (crosses || after <= before || highest < lowest)
                    continue;
                auto const middle = std::clamp((before + after) / 2.0, lowest, highest);
                (horizontal ? p.y : p.x) = middle;
                (horizontal ? q.y : q.x) = middle;
            }
            return corners(result);
        }
        // (where a node has been dragged to, from its place (its offset from its container's first): not before its container's first)
        point moved_offset(node const& aNode, point const& aPlace) const
        {
            auto const moved = iMoved.find(aNode.key);
            if (moved == iMoved.end())
                return point{};
            return point{ std::max(moved->second.x, -aPlace.x), std::max(moved->second.y, -aPlace.y) };
        }
        // (whether a node is in the row (of its siblings) that a point is level with)
        static bool in_row(node const& aNode, point const& aPoint)
        {
            return aPoint.y >= aNode.bounds.y && aPoint.y <= aNode.bounds.bottom();
        }
        // (the place among its siblings that a statement being dragged would be moved to: among those in the row that its centre is 
        // level with (statements dragged under others are in rows of their own), before the first whose centre is to the right of its 
        // centre, else after the last (none: its own place, a row with none of its siblings in it (it is only moved there), or a parameter))
        std::optional<std::size_t> drop_index(std::uint32_t aNode) const
        {
            auto const& n = iNodes[aNode];
            if (n.outline.kind == outline_kind::Parameter || n.outline.kind == outline_kind::Alternative)
                return std::nullopt;
            auto const& siblings = iNodes[n.outline.parent].children;
            auto const current = static_cast<std::size_t>(std::find(siblings.begin(), siblings.end(), aNode) - siblings.begin());
            auto const centre = n.bounds.center();
            std::optional<std::size_t> index;
            for (std::size_t s = 0u; s < siblings.size(); ++s)
            {
                auto const& sibling = iNodes[siblings[s]];
                if (siblings[s] == aNode || !in_row(sibling, centre))
                    continue;
                if (sibling.bounds.center().x > centre.x)
                {
                    index = s;
                    break;
                }
                index = s + 1u;
            }
            return (!index || *index == current || *index == current + 1u) ? std::nullopt : index;
        }
        void fit()
        {
            if (iNodes.empty())
                return;
            auto const cr = client_rect();
            auto const& extents = iNodes[0].extents;
            auto const margin = 16.0;
            iZoom = std::clamp(std::min((cr.cx - margin * 2.0) / extents.cx, (cr.cy - margin * 2.0) / extents.cy), MinZoom, MaxFitZoom);
            iOffset = point{ (cr.cx - extents.cx * iZoom) / 2.0, (cr.cy - extents.cy * iZoom) / 2.0 };
            iZoomedFont = std::nullopt;
            iZoomedLabelFont = std::nullopt;
        }
        point to_screen(point const& aPoint) const
        {
            return aPoint * iZoom + iOffset;
        }
        rect to_screen(rect const& aRect) const
        {
            return rect{ to_screen(aRect.top_left()), aRect.extents() * iZoom };
        }
        point to_graph(point const& aPoint) const
        {
            return (aPoint - iOffset) / iZoom;
        }
        // (the innermost node at a position (of the widget))
        std::uint32_t node_at(point const& aPosition) const
        {
            auto const p = to_graph(aPosition);
            std::uint32_t result = NoNode;
            for (std::uint32_t i = 0u; i < iNodes.size(); ++i)
                if (iNodes[i].bounds.contains(p))
                    result = i; // (pre-order: a node's descendants are after it, and within it)
            return result;
        }
        neogfx::font const& zoomed_font() const
        {
            if (!iZoomedFont)
                iZoomedFont = graph_font().with_size(std::max(font().size() * iZoom, 1.0));
            return *iZoomedFont;
        }
        // (its text: in its font (the coding font), unhinted, so that its glyphs are where they are exactly, at any zoom (a hinted font's 
        // are on whole pixels): its text, and so its nodes, scale smoothly as it is zoomed)
        neogfx::font const& graph_font() const
        {
            if (!iGraphFont)
                iGraphFont = font().with_hinting(false);
            return *iGraphFont;
        }
        // (a node's kind, in its title bar: in the app's font, bold, as large as the coding font, unhinted)
        neogfx::font const& label_font() const
        {
            if (!iLabelFont)
                iLabelFont = service<i_app>().current_style().font().with_style(font_style::Bold).with_size(std::max(font().size() * 0.9, 1.0)).with_hinting(false);
            return *iLabelFont;
        }
        neogfx::font const& zoomed_label_font() const
        {
            if (!iZoomedLabelFont)
                iZoomedLabelFont = label_font().with_size(std::max(font().size() * 0.9 * iZoom, 1.0));
            return *iZoomedLabelFont;
        }
        // (a title bar's texture: its kind's colour fading to the right, then glossy above and shaded below)
        neogfx::gradient const& title_gradient(color const& aAccent) const
        {
            auto existing = iTitleGradients.find(aAccent);
            if (existing == iTitleGradients.end())
                existing = iTitleGradients.emplace(aAccent, neogfx::gradient{ 
                    { mix(aAccent, color::Black, 0.35), mix(aAccent, color::Black, 0.65), mix(aAccent, color::Black, 0.8) }, gradient_direction::Horizontal }).first;
            return existing->second;
        }
        neogfx::gradient const& gloss_gradient() const
        {
            if (!iGloss)
                iGloss = neogfx::gradient{ { color::White.with_alpha(0.24), color::White.with_alpha(0.04), color::Black.with_alpha(0.0), color::Black.with_alpha(0.3) }, 
                    gradient_direction::Vertical };
            return *iGloss;
        }
        // (a flow's route (see flow) drawn: an S-curve, else its corners rounded)
        void draw_route(i_graphics_context& aGc, std::vector<point> const& aRoute, pen const& aPen) const
        {
            if (aRoute.size() < 2u)
                return;
            if (aRoute.size() == 2u)
            {
                auto const a = to_screen(aRoute[0]);
                auto const b = to_screen(aRoute[1]);
                auto const dx = (b.x - a.x) / 2.0;
                if (std::abs(a.y - b.y) < 0.5 || dx <= 0.0)
                    aGc.draw_line(a, b, aPen);
                else
                    aGc.draw_cubic_bezier(a, point{ a.x + dx, a.y }, point{ b.x - dx, b.y }, b, aPen);
                return;
            }
            point current = aRoute.front();
            for (std::size_t i = 1u; i + 1u < aRoute.size(); ++i)
            {
                auto const& corner = aRoute[i];
                auto const in = corner - aRoute[i - 1u];
                auto const out = aRoute[i + 1u] - corner;
                auto const inLength = std::hypot(in.x, in.y);
                auto const outLength = std::hypot(out.x, out.y);
                if (inLength == 0.0 || outLength == 0.0)
                    continue;
                auto const radius = std::min({ CornerRadius, inLength / 2.0, outLength / 2.0 });
                point const before = corner - in * (radius / inLength);
                point const after = corner + out * (radius / outLength);
                aGc.draw_line(to_screen(current), to_screen(before), aPen);
                aGc.draw_cubic_bezier(to_screen(before), to_screen(before + (corner - before) * 0.6), to_screen(after + (corner - after) * 0.6), 
                    to_screen(after), aPen);
                current = after;
            }
            aGc.draw_line(to_screen(current), to_screen(aRoute.back()), aPen);
        }
        // (a pin where the flow passes into or out of a node (pointing the way that it passes): its outline, filled if it does)
        void draw_pin(i_graphics_context& aGc, point const& aCentre, bool aConnected, color const& aColour) const
        {
            auto const s = PinSize * iZoom;
            path outline;
            outline.move_to(point{ aCentre.x - s * 0.8, aCentre.y - s });
            outline.line_to(point{ aCentre.x + s * 0.2, aCentre.y - s });
            outline.line_to(point{ aCentre.x + s, aCentre.y });
            outline.line_to(point{ aCentre.x + s * 0.2, aCentre.y + s });
            outline.line_to(point{ aCentre.x - s * 0.8, aCentre.y + s });
            outline.line_to(point{ aCentre.x - s * 0.8, aCentre.y - s });
            aGc.draw_path(outline, pen{ aColour, std::max(1.0, 1.3 * iZoom) }, aConnected ? brush{ aColour } : brush{});
        }
        // (a node's kind's icon, in its title bar, about a centre (h: its half size))
        void draw_icon(i_graphics_context& aGc, node const& aNode, point const& c, dimension h, color const& aColour) const
        {
            pen const stroke{ aColour, std::max(1.0, 1.5 * iZoom) };
            auto const line = [&](double x0, double y0, double x1, double y1) { aGc.draw_line(point{ c.x + x0 * h, c.y + y0 * h }, point{ c.x + x1 * h, c.y + y1 * h }, stroke); };
            switch (aNode.outline.kind)
            {
            case outline_kind::Module:
                line(0.0, -1.0, 1.0, 0.0);
                line(1.0, 0.0, 0.0, 1.0);
                line(0.0, 1.0, -1.0, 0.0);
                line(-1.0, 0.0, 0.0, -1.0);
                break;
            case outline_kind::Function:
                aGc.draw_cubic_bezier(point{ c.x + 0.8 * h, c.y - 0.85 * h }, point{ c.x + 0.3 * h, c.y - 1.15 * h }, point{ c.x, c.y - 0.85 * h }, 
                    point{ c.x - 0.05 * h, c.y - 0.3 * h }, stroke);
                line(-0.05, -0.3, -0.35, 1.0);
                line(-0.6, -0.25, 0.5, -0.25);
                break;
            case outline_kind::Class:
                aGc.draw_rect(rect{ point{ c.x - 0.8 * h, c.y - 0.8 * h }, size{ 1.6 * h, 1.6 * h } }, stroke);
                line(-0.8, -0.2, 0.8, -0.2);
                break;
            case outline_kind::Parameter:
                aGc.draw_circle(c, 0.6 * h, stroke);
                break;
            case outline_kind::Loop:
                for (int segment = 0; segment < 12; ++segment)
                {
                    auto const a0 = -0.7 + 5.1 * segment / 12.0;
                    auto const a1 = -0.7 + 5.1 * (segment + 1) / 12.0;
                    line(0.75 * std::cos(a0), 0.75 * std::sin(a0), 0.75 * std::cos(a1), 0.75 * std::sin(a1));
                }
                line(0.57, -0.48, 0.95, -0.95);
                line(0.57, -0.48, 0.05, -0.55);
                break;
            case outline_kind::Branch:
            case outline_kind::Alternative:
                line(-1.0, 0.0, -0.1, 0.0);
                line(-0.1, 0.0, 0.9, -0.8);
                line(-0.1, 0.0, 0.9, 0.8);
                break;
            case outline_kind::Return:
                line(0.8, -0.8, 0.8, 0.3);
                line(0.8, 0.3, -0.8, 0.3);
                line(-0.8, 0.3, -0.35, -0.15);
                line(-0.8, 0.3, -0.35, 0.75);
                break;
            case outline_kind::Jump:
                line(-1.0, 0.0, 1.0, 0.0);
                line(1.0, 0.0, 0.5, -0.5);
                line(1.0, 0.0, 0.5, 0.5);
                break;
            default:
                line(-0.8, -0.7, -0.1, 0.0);
                line(-0.1, 0.0, -0.8, 0.7);
                line(0.1, 0.7, 0.9, 0.7);
                break;
            }
        }
    private:
        color text_colour() const
        {
            return iTextColour ? *iTextColour : palette_color(color_role::Text);
        }
        color background_colour() const
        {
            return iBackgroundColour ? *iBackgroundColour : palette_color(color_role::Base);
        }
        color category_colour(std::optional<category> const& aCategory) const
        {
            if (aCategory && static_cast<std::size_t>(*aCategory) < iCategoryColours.size() && iCategoryColours[static_cast<std::size_t>(*aCategory)])
                return *iCategoryColours[static_cast<std::size_t>(*aCategory)];
            return text_colour();
        }
        // (a node's colour: its kind's syntax highlighting category's)
        color accent_colour(node const& aNode) const
        {
            switch (aNode.outline.kind)
            {
            case outline_kind::Function:
                return category_colour(category::Function);
            case outline_kind::Class:
                return category_colour(category::Type);
            case outline_kind::Parameter:
                return category_colour(category::Parameter);
            case outline_kind::Loop:
            case outline_kind::Branch:
            case outline_kind::Alternative:
            case outline_kind::Return:
            case outline_kind::Jump:
                return category_colour(category::Keyword);
            default:
                return text_colour();
            }
        }
        // (a node's body's colour: darker than the background (a module's lighter: it is the canvas that the others are on))
        color node_colour(node const& aNode) const
        {
            if (aNode.outline.kind == outline_kind::Module)
                return mix(background_colour(), text_colour(), 0.02);
            return mix(background_colour(), color::Black, 0.35);
        }
        // (a node's kind, as its label shows it (translated))
        string kind_label(node const& aNode) const
        {
            switch (aNode.outline.kind)
            {
            case outline_kind::Module:
                return "module"_t;
            case outline_kind::Function:
                return "function"_t;
            case outline_kind::Class:
                return "class"_t;
            case outline_kind::Parameter:
                return "parameter"_t;
            case outline_kind::Loop:
                return "loop"_t;
            case outline_kind::Branch:
                return "branch"_t;
            case outline_kind::Alternative:
                return "alternative"_t;
            case outline_kind::Return:
                return "return"_t;
            case outline_kind::Jump:
                return "jump"_t;
            default:
                return "statement"_t;
            }
        }
    private:
        void select(std::uint32_t aNode)
        {
            iSelected = aNode;
            update();
            NodeSelected(iNodes[aNode].outline.first, iNodes[aNode].outline.last);
        }
        // (a node's text extended to whole lines (if nothing else is on them): [first, last) and whether it is)
        std::pair<std::pair<position_type, position_type>, bool> line_span(std::uint32_t aNode) const
        {
            auto first = iNodes[aNode].outline.first;
            auto last = iNodes[aNode].outline.last;
            auto lineStart = first;
            while (lineStart > 0u && iText[lineStart - 1u] != U'\n' && (iText[lineStart - 1u] == U' ' || iText[lineStart - 1u] == U'\t'))
                --lineStart;
            bool const wholeStart = lineStart == 0u || iText[lineStart - 1u] == U'\n';
            auto lineEnd = last;
            while (lineEnd < iText.size() && iText[lineEnd] != U'\n' && is_space(iText[lineEnd]))
                ++lineEnd;
            bool const wholeEnd = lineEnd == iText.size() || iText[lineEnd] == U'\n';
            if (wholeStart && wholeEnd)
                return { { lineStart, static_cast<position_type>(std::min<std::size_t>(lineEnd + 1u, iText.size())) }, true };
            return { { first, last }, false };
        }
        std::u32string indentation_of(std::uint32_t aNode) const
        {
            auto const first = iNodes[aNode].outline.first;
            auto lineStart = first;
            while (lineStart > 0u && (iText[lineStart - 1u] == U' ' || iText[lineStart - 1u] == U'\t'))
                --lineStart;
            if (lineStart > 0u && iText[lineStart - 1u] != U'\n')
                return {};
            return iText.substr(lineStart, first - lineStart);
        }
        void edit(position_type aFirst, position_type aLast, std::u32string const& aText)
        {
            ScriptEdit(aFirst, aLast, string{ neolib::utf32_to_utf8(aText) });
        }
        void delete_node(std::uint32_t aNode)
        {
            if (aNode == 0u || iNodes[aNode].outline.kind == outline_kind::Parameter)
            {
                service<i_basic_services>().system_beep();
                return;
            }
            auto const [span, whole] = line_span(aNode);
            iSelected = NoNode;
            edit(span.first, span.second, {});
        }
        void duplicate_node(std::uint32_t aNode)
        {
            auto const [span, whole] = line_span(aNode);
            if (aNode == 0u || iNodes[aNode].outline.kind == outline_kind::Parameter || !whole)
            {
                service<i_basic_services>().system_beep();
                return;
            }
            auto text = iText.substr(span.first, span.second - span.first);
            if (text.empty() || text.back() != U'\n')
                text = U'\n' + text;
            edit(span.second, span.second, text);
        }
        // (a statement moved before its sibling at an index (or after the last): its lines (it and its siblings must be on lines of their
        // own) moved)
        void move_node(std::uint32_t aNode, std::size_t aIndex)
        {
            auto const& siblings = iNodes[iNodes[aNode].outline.parent].children;
            auto const [source, sourceWhole] = line_span(aNode);
            auto const target = aIndex < siblings.size() ? line_span(siblings[aIndex]) : line_span(siblings.back());
            if (!sourceWhole || !target.second)
            {
                service<i_basic_services>().system_beep();
                return;
            }
            auto const to = aIndex < siblings.size() ? target.first.first : target.first.second;
            auto text = iText.substr(source.first, source.second - source.first);
            if (text.empty() || text.back() != U'\n')
                text += U'\n';
            bool const needsNewLine = to == iText.size() && to > 0u && iText[to - 1u] != U'\n';
            if (to > source.second)
            {
                edit(to, to, needsNewLine ? U'\n' + text : text);
                edit(source.first, source.second, {});
            }
            else if (to < source.first)
            {
                edit(source.first, source.second, {});
                edit(to, to, text);
            }
        }
        // (a node's header edited in place (or a statement inserted after it))
        void begin_edit(std::uint32_t aNode, bool aInsert)
        {
            end_edit(false);
            if (aNode >= iNodes.size() || (!aInsert && aNode == 0u))
                return;
            iEditing = inline_edit{ aNode, aInsert };
            auto const& n = iNodes[aNode];
            auto editRect = to_screen(n.bounds);
            editRect.cy = header_height(n) * iZoom;
            if (aInsert)
                editRect.y += editRect.cy + (aNode == 0u ? 0.0 : 4.0);
            editRect.cx = std::max(editRect.cx, 280.0 * std::max(iZoom, 1.0));
            auto editor = make_ref<text_edit>(*this, text_edit_caps::SingleLine, frame_style::SolidFrame);
            // (in the font as zoomed: as the node's text is)
            editor->set_font(zoomed_font());
            editor->set_focus_policy(editor->focus_policy() | neogfx::focus_policy::ConsumeReturnKey | neogfx::focus_policy::ConsumeEscapeKey);
            editor->move(editRect.top_left());
            editor->resize(size{ editRect.cx, std::max(editRect.cy, std::ceil(zoomed_font().height()) + 8.0) });
            if (!aInsert)
            {
                auto end = std::max(n.outline.first, n.outline.headerLast);
                while (end > n.outline.first && is_space(iText[end - 1u]))
                    --end;
                editor->set_text(string{ neolib::utf32_to_utf8(iText.substr(n.outline.first, end - n.outline.first)) });
            }
            iInlineEditor = editor;
            editor->keyboard_event([this, &editorRef = *editor](const neogfx::keyboard_event& aEvent)
            {
                if (aEvent.type() != keyboard_event_type::KeyPressed)
                    return;
                if (aEvent.scan_code() == ScanCode_RETURN || aEvent.scan_code() == ScanCode_KEYPAD_ENTER)
                {
                    iEndEdit = true;
                    editorRef.keyboard_event().accept();
                }
                else if (aEvent.scan_code() == ScanCode_ESCAPE)
                {
                    iEndEdit = false;
                    editorRef.keyboard_event().accept();
                }
            });
            editor->focus_event([this](neogfx::focus_event aEvent, focus_reason)
            {
                if (aEvent == neogfx::focus_event::FocusLost && !iEndEdit)
                    iEndEdit = true;
            });
            // (the edit ends after the event that ends it (the editor is destroyed when it does))
            iEditEnder.emplace(neolib::service<neolib::i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                if (iEndEdit)
                    end_edit(*iEndEdit);
                else if (iEditing)
                    aTimer.again();
            }, std::chrono::milliseconds{ 20 });
            editor->set_focus();
            editor->select_all();
        }
        void end_edit(bool aAccept)
        {
            if (!iEditing || !iInlineEditor)
            {
                iEndEdit = std::nullopt;
                return;
            }
            auto const editing = *iEditing;
            auto const text = neolib::utf8_to_utf32(iInlineEditor->text().to_std_string());
            iEditing = std::nullopt;
            iEndEdit = std::nullopt;
            iInlineEditor = nullptr;
            update(); // (where the editor was)
            set_focus();
            if (!aAccept || editing.node >= iNodes.size())
                return;
            auto const& n = iNodes[editing.node];
            if (!editing.insert)
            {
                auto end = std::max(n.outline.first, n.outline.headerLast);
                while (end > n.outline.first && is_space(iText[end - 1u]))
                    --end;
                if (text != iText.substr(n.outline.first, end - n.outline.first))
                {
                    begin_rename(editing.node, end, text);
                    edit(n.outline.first, end, text);
                }
                return;
            }
            if (text.empty())
                return;
            // (inserted after the node (its lines), as indented as it; after the module: at the end of the script)
            if (editing.node == 0u)
            {
                auto const at = static_cast<position_type>(iText.size());
                edit(at, at, (at > 0u && iText.back() != U'\n' ? U"\n" : U"") + text + U"\n");
                return;
            }
            auto const [span, whole] = line_span(editing.node);
            if (whole)
                edit(span.second, span.second, (span.second == iText.size() && span.second > 0u && iText.back() != U'\n' ? U"\n" : U"") +
                    indentation_of(editing.node) + text + U"\n");
            else
                edit(n.outline.last, n.outline.last, U" " + text);
        }
        // (the name that a text starts with)
        static std::u32string leading_name(std::u32string const& aText)
        {
            std::size_t length = 0u;
            while (length < aText.size() && is_name(aText[length]))
                ++length;
            return aText.substr(0u, length);
        }
        // (a node's header [its first, aEnd) about to be edited (to aText): if it starts with a parameter's or a variable's name (as 
        // highlighted) and its name changes, the uses of its old name (the names so highlighted where it is: in its function (a 
        // parameter's, or a variable's if it is in one), else in the script; as they will be once it is edited) can be renamed too (see 
        // propagate))
        void begin_rename(std::uint32_t aNode, position_type aEnd, std::u32string const& aText)
        {
            iRename = std::nullopt;
            iRenameNode = NoNode;
            auto const& n = iNodes[aNode];
            if (n.outline.first >= iCategories.size() || 
                (iCategories[n.outline.first] != category::Parameter && iCategories[n.outline.first] != category::Variable))
                return;
            auto const kind = *iCategories[n.outline.first];
            auto const from = leading_name(iText.substr(n.outline.first, aEnd - n.outline.first));
            auto const to = leading_name(aText);
            if (from.empty() || to.empty() || from == to || n.outline.parent == outline_node::NoParent)
                return;
            auto const delta = static_cast<std::ptrdiff_t>(aText.size()) - static_cast<std::ptrdiff_t>(aEnd - n.outline.first);
            auto scope = n.outline.parent;
            if (kind == category::Variable)
                while (scope != 0u && iNodes[scope].outline.kind != outline_kind::Function)
                    scope = iNodes[scope].outline.parent;
            auto const& within = iNodes[scope];
            pending_rename rename{ n.outline.first, from, to, {} };
            for (auto p = within.outline.first; p + from.size() <= std::min<std::size_t>(within.outline.last, iText.size()); ++p)
                if ((p < n.outline.first || p >= aEnd) && iCategories[p] == kind && iText.compare(p, from.size(), from) == 0 &&
                    (p == 0u || !is_name(iText[p - 1u])) && (p + from.size() == iText.size() || !is_name(iText[p + from.size()])))
                    rename.uses.push_back(p >= aEnd ? static_cast<position_type>(static_cast<std::ptrdiff_t>(p) + delta) : p);
            if (!rename.uses.empty())
                iRename = rename;
        }
        // (a renamed parameter's or variable's uses renamed too (the last first: the positions of those before it are unchanged))
        void propagate()
        {
            if (!iRename || iScriptStale)
            {
                service<i_basic_services>().system_beep();
                return;
            }
            auto const rename = *iRename;
            iRename = std::nullopt;
            iRenameNode = NoNode;
            for (auto use = rename.uses.rbegin(); use != rename.uses.rend(); ++use)
                edit(*use, static_cast<position_type>(*use + rename.from.size()), rename.to);
            update();
        }
        // (where a rename's propagate button is: at the end of its node's title row)
        rect propagate_rect(node const& aNode) const
        {
            auto const r = to_screen(aNode.bounds);
            auto const titleHeight = title_height() * iZoom;
            auto const extent = std::max(std::floor(titleHeight * 0.8), 8.0);
            return rect{ point{ r.right() - padding_x() * iZoom - extent, r.y + (titleHeight - extent) / 2.0 }, neogfx::size{ extent } };
        }
        // (the propagate button: one name fanning out to its uses)
        void draw_propagate(i_graphics_context& aGc, rect const& aRect, color const& aColour) const
        {
            aGc.fill_rounded_rect(aRect, aRect.cx * 0.2, mix(background_colour(), aColour, 0.35));
            aGc.draw_rounded_rect(aRect, aRect.cx * 0.2, pen{ aColour, 1.0 });
            point const from{ aRect.x + aRect.cx * 0.28, aRect.center().y };
            aGc.fill_circle(from, std::max(aRect.cx * 0.1, 1.5), aColour);
            for (auto const dy : { -0.26, 0.0, 0.26 })
            {
                point const to{ aRect.x + aRect.cx * 0.76, aRect.center().y + aRect.cy * dy };
                aGc.draw_line(from, to, pen{ aColour, std::max(aRect.cx * 0.08, 1.0) });
                aGc.fill_circle(to, std::max(aRect.cx * 0.07, 1.0), aColour);
            }
        }
    private:
        std::u32string iText;
        std::vector<std::optional<category>> iCategories;
        std::vector<node> iNodes;
        std::uint32_t iSelected = NoNode;
        std::function<i_script_debugger::breakpoint_state(position_type)> iBreakpointAt;
        std::optional<std::pair<position_type, position_type>> iStatement;
        bool iStatementTop = true;
        std::vector<std::optional<color>> iCategoryColours;
        std::optional<color> iTextColour;
        std::optional<color> iBackgroundColour;
        std::optional<neogfx::gradient> iGridColour;
        double iZoom = 1.0;
        point iOffset;
        bool iUserView = false;                     ///< (zoomed or panned by the user: not fitted when resized)
        mutable bool iLayoutPending = false;
        mutable bool iFitPending = true;
        mutable std::optional<neogfx::font> iZoomedFont;
        mutable std::optional<neogfx::font> iZoomedLabelFont;
        mutable std::optional<neogfx::font> iLabelFont;
        mutable std::optional<neogfx::font> iGraphFont;
        mutable std::map<color, neogfx::gradient> iTitleGradients;    ///< (by colour (see title_gradient))
        mutable std::optional<neogfx::gradient> iGloss;
        double iMeasuredZoom = 0.0;     ///< (the zoom that the nodes' headers were last measured at (see measure_zoomed))
        std::map<std::u32string, point> iMoved;     ///< (where nodes have been dragged to (by key))
        std::vector<flow> iFlows;                   ///< (by the node whose body they are in (see route_flows))
        std::vector<flow> iWires;                   ///< (the parameters' wires to their uses (see route_flows))
        std::optional<drag> iDrag;
        std::optional<pan> iPan;
        std::optional<inline_edit> iEditing;
        std::optional<pending_rename> iRename;
        std::uint32_t iRenameNode = NoNode;         ///< (the node whose header was renamed (see pending_rename))
        bool iScriptStale = false;                  ///< (the script has changed since it was last outlined)
        std::optional<bool> iEndEdit;
        ref_ptr<text_edit> iInlineEditor;
        std::optional<neolib::callback_timer> iEditEnder;
        sink iSink;
    };
}
