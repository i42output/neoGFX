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
#include <map>
#include <optional>
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
    private:
        static constexpr std::uint32_t NoNode = ~std::uint32_t{};
        static constexpr std::size_t MaxHeaderLength = 64u;
        static constexpr double MinZoom = 0.1;
        static constexpr double MaxZoom = 4.0;
        static constexpr double MaxFitZoom = 1.5;
        static constexpr double GridSubdivisions = 8.0;     ///< (minor grid lines to a major one (as the workspace's grid's default))
        static constexpr dimension MinTextHeight = 5.0;     ///< (text smaller than this (in pixels) is not drawn: it cannot be read)
        // (a run of a node's header in one syntax highlighting category: its text and its offset from the header's start (graph coordinates))
        struct run
        {
            string text;
            std::optional<category> kind;
            dimension offset = 0.0;
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
            string label;                                   ///< (its kind)
            dimension labelWidth = 0.0;
            dimension headerWidth = 0.0;
            std::u32string key;                             ///< (which node it is, as the script is edited: where it was dragged to is kept by it)
            point offset;                                   ///< (where it has been dragged to: from its place (graph coordinates))
            dimension parametersWidth = 0.0;
            dimension parametersTop = 0.0;                  ///< (its parameters' top: below its body's statements (from its body's top))
            neogfx::size laneExtents;
            std::vector<std::uint32_t> uses;                ///< (a parameter: the nodes whose headers use it)
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
            update();
        }
        // (the grid in the workspace's grid colour (a setting): its minor lines in it, its major lines (every GridSubdivisions'th) in the 
        // style's void colour, as the workspace's are)
        void set_grid_colour(neogfx::gradient const& aGridColour)
        {
            iGridColour = aGridColour;
            update();
        }
        // (laid out again: e.g. its font has changed)
        void refresh_layout()
        {
            iLayoutPending = true;
            iZoomedFont = std::nullopt;
            iZoomedLabelFont = std::nullopt;
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
        }
        void zoom_to_fit()
        {
            iUserView = false;
            iFitPending = true;
            update();
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
            auto const paint_node = [&](std::uint32_t i)
            {
                auto const& n = iNodes[i];
                auto const r = to_screen(n.bounds);
                if (!r.intersects(cr))
                    return;
                auto const accent = accent_colour(n);
                auto const radius = 6.0 * iZoom;
                auto const header = rect{ r.top_left(), size{ r.cx, header_height() * iZoom } };
                aGc.fill_rounded_rect(r, radius, node_colour(n));
                {
                    scoped_scissor ss{ aGc, header.intersection(cr) };
                    aGc.fill_rounded_rect(r, radius, accent.with_alpha(n.outline.kind == outline_kind::Module ? 0.06 : 0.22));
                }
                bool const selected = (i == iSelected);
                aGc.draw_rounded_rect(r, radius,
                    pen{ selected ? accent : accent.with_alpha(0.55), selected ? 2.0 : 1.0, n.outline.kind == outline_kind::Module ? line_style::Dash : line_style::Solid });
                // (a renamed parameter or variable: its button that propagates its rename to its uses)
                bool const renamed = (i == iRenameNode && !iScriptStale);
                if (zoomedFont.height() < MinTextHeight)
                {
                    if (renamed)
                        draw_propagate(aGc, propagate_rect(n), accent);
                    return;
                }
                // (its code, in the colours of its syntax highlighting (its runs as measured when laid out))
                scoped_scissor ss{ aGc, header.deflated(size{ 2.0 }).intersection(cr) };
                point const textPos{ header.x + padding_x() * iZoom, header.y + (header.cy - zoomedFont.height()) / 2.0 };
                for (auto const& run : n.runs)
                    aGc.draw_text(point{ textPos.x + run.offset * iZoom, textPos.y }, run.text, zoomedFont, text_format{ category_colour(run.kind) });
                if (zoomedLabelFont.height() >= MinTextHeight)
                    aGc.draw_text(point{ header.right() - (padding_x() + n.labelWidth) * iZoom, header.y + (header.cy - zoomedLabelFont.height()) / 2.0 },
                        n.label, zoomedLabelFont, text_format{ text_colour().with_alpha(0.55) });
                if (renamed)
                    draw_propagate(aGc, propagate_rect(n), accent);
            };
            // (a node being dragged (and its contents) over the others)
            auto const dragged = iDrag && iDrag->dragging ? iDrag->node : NoNode;
            for (std::uint32_t i = 0u; i < iNodes.size(); ++i)
                if (dragged == NoNode || (i != dragged && !is_descendant(i, dragged)))
                    paint_node(i);
            if (dragged != NoNode)
                for (auto i = dragged; i < iNodes.size() && (i == dragged || is_descendant(i, dragged)); ++i)
                    paint_node(i);
            // (the flow of each body (and a loop's back to its start), and the parameters' wires)
            auto const flowColour = mix(text_colour(), color::White, 0.5);
            for (auto const& n : iNodes)
            {
                // (each alternative entered on its own lane (the flow does not pass from the body to it))
                for (auto const a : n.alternatives)
                {
                    auto const& alternative = iNodes[a];
                    auto const entry = to_screen(point{ alternative.bounds.x - 18.0, flow_y(alternative) });
                    aGc.fill_circle(entry, 3.0 * iZoom, flowColour);
                    draw_flow(aGc, entry, to_screen(point{ alternative.bounds.x, flow_y(alternative) }), flowColour);
                }
                if (n.children.empty())
                    continue;
                auto const& first = iNodes[n.children.front()];
                auto const entry = to_screen(point{ n.lane.x - 18.0, flow_y(first) });
                aGc.fill_circle(entry, 3.0 * iZoom, flowColour);
                draw_flow(aGc, entry, to_screen(point{ first.bounds.x, flow_y(first) }), flowColour);
                for (std::size_t c = 1u; c < n.children.size(); ++c)
                {
                    auto const& from = iNodes[n.children[c - 1u]];
                    auto const& to = iNodes[n.children[c]];
                    draw_flow(aGc, to_screen(point{ from.bounds.right(), flow_y(from) }), to_screen(point{ to.bounds.x, flow_y(to) }), flowColour);
                }
                if (n.outline.kind == outline_kind::Loop)
                {
                    auto const& last = iNodes[n.children.back()];
                    auto const bottom = n.lane.bottom() + 6.0;
                    pen const loopPen{ flowColour.with_alpha(0.6), 1.5 * iZoom, line_style::Dash };
                    point const p0 = to_screen(point{ last.bounds.right(), flow_y(last) });
                    point const p1 = to_screen(point{ last.bounds.right() + 8.0, flow_y(last) });
                    point const p2 = to_screen(point{ last.bounds.right() + 8.0, bottom });
                    point const p3 = to_screen(point{ n.lane.x - 10.0, bottom });
                    point const p4 = to_screen(point{ n.lane.x - 10.0, flow_y(first) });
                    aGc.draw_line(p0, p1, loopPen);
                    aGc.draw_line(p1, p2, loopPen);
                    aGc.draw_line(p2, p3, loopPen);
                    aGc.draw_line(p3, p4, loopPen);
                }
            }
            for (auto const& n : iNodes)
                for (auto const use : n.uses)
                {
                    auto const& user = iNodes[use];
                    auto const wireColour = category_colour(category::Parameter);
                    point const from = to_screen(point{ n.bounds.right(), n.bounds.y + n.bounds.cy / 2.0 });
                    point const to = to_screen(point{ user.bounds.x, data_y(user) });
                    pen const wirePen{ wireColour, 2.0 * iZoom };
                    auto const dx = std::max(std::abs(to.x - from.x) / 2.0, 24.0 * iZoom);
                    aGc.draw_cubic_bezier(from, point{ from.x + dx, from.y }, point{ to.x - dx, to.y }, to, wirePen);
                    aGc.fill_circle(from, 3.5 * iZoom, wireColour);
                    aGc.fill_circle(to, 3.5 * iZoom, wireColour);
                }
            // (where a statement being dragged would be moved to)
            if (iDrag && iDrag->dragging && iDrag->dropIndex)
            {
                auto const& n = iNodes[iDrag->node];
                auto const& siblings = iNodes[n.outline.parent].children;
                auto const x = *iDrag->dropIndex < siblings.size() ?
                    iNodes[siblings[*iDrag->dropIndex]].bounds.x - gap() / 2.0 : iNodes[siblings.back()].bounds.right() + gap() / 2.0;
                auto const& lane = iNodes[n.outline.parent].lane;
                aGc.draw_line(to_screen(point{ x, lane.y - 4.0 }), to_screen(point{ x, lane.bottom() + 4.0 }), pen{ accent_colour(n), 3.0 });
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
                    // (moved past a sibling: the statement moved there in the script (and laid out in its place); else left where it was 
                    // dropped)
                    auto const& n = iNodes[drag.node];
                    if (drag.dropIndex)
                    {
                        iMoved.erase(n.key);
                        arrange_nodes();
                        move_node(drag.node, *drag.dropIndex);
                    }
                    else
                        iMoved[n.key] = n.offset;
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
                    auto const& user = iNodes[i];
                    if (user.outline.kind == outline_kind::Parameter)
                        continue;
                    auto const end = std::max(user.outline.first, user.outline.headerLast);
                    for (auto p = user.outline.first; p + name.size() <= end; ++p)
                        if (iCategories[p] == category::Parameter && iText.compare(p, name.size(), name) == 0 &&
                            (p == user.outline.first || !is_name(iText[p - 1u])) && (p + name.size() == iText.size() || !is_name(iText[p + name.size()])))
                        {
                            n.uses.push_back(i);
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
        static bool is_name(char32_t aCh)
        {
            return aCh == U'_' || aCh == U'$' || aCh > 0x7Fu || (aCh < 0x80u && std::isalnum(static_cast<int>(aCh)));
        }
    private:
        dimension padding_x() const { return 10.0; }
        dimension gap() const { return 36.0; }
        dimension header_height() const { return std::ceil(font().height()) + 12.0; }
        dimension flow_y(node const& aNode) const { return aNode.bounds.y + header_height() / 2.0; }  // (its header's vertical middle)
        dimension data_y(node const& aNode) const { return aNode.bounds.y + header_height() - 7.0; }
        // (the nodes' headers measured, then the nodes arranged (see arrange_nodes))
        void layout_nodes(i_graphics_context const& aGc)
        {
            if (iNodes.empty())
                return;
            auto const labelFont = font().with_size(font().size() * 0.8);
            auto const padding = padding_x();
            // (bottom up: a node is before its descendants)
            for (auto i = iNodes.size(); i-- > 0u;)
            {
                auto& n = iNodes[i];
                // (its header's runs and its kind measured (once: painting only draws them))
                n.runs.clear();
                dimension textWidth = 0.0;
                for (std::size_t first = 0u; first < n.header.size();)
                {
                    auto last = first + 1u;
                    while (last < n.header.size() && n.headerCategories[last] == n.headerCategories[first])
                        ++last;
                    auto& r = n.runs.emplace_back(run{ string{ neolib::utf32_to_utf8(n.header.substr(first, last - first)) }, n.headerCategories[first], textWidth });
                    textWidth += aGc.text_extent(r.text, font()).cx;
                    first = last;
                }
                n.label = kind_label(n);
                n.labelWidth = aGc.text_extent(n.label, labelFont).cx;
                n.headerWidth = textWidth + n.labelWidth + padding * 3.0;
            }
            arrange_nodes();
        }
        // (the nodes' extents and positions, in graph coordinates, from their headers as measured: a body's statements left to right, a 
        // function's parameters before them; a node that has been dragged is where it was dragged to, 
        // from its place (but not before its container's first), its container enclosing it)
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
                dimension slotX = 0.0;
                for (auto c : n.children)
                {
                    auto& child = iNodes[c];
                    child.offset = moved_offset(child, point{ slotX, 0.0 });
                    lane.cx = std::max(lane.cx, slotX + child.offset.x + child.extents.cx);
                    lane.cy = std::max(lane.cy, child.offset.y + child.extents.cy);
                    slotX += child.extents.cx + gap();
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
                n.extents = size{ std::max(n.headerWidth, 80.0), header_height() };
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
                auto const top = n.bounds.y + header_height() + padding;
                point p{ n.bounds.x + padding, top + n.parametersTop };
                for (auto i : n.parameters)
                {
                    iNodes[i].bounds = rect{ p + iNodes[i].offset, iNodes[i].extents };
                    p.y += iNodes[i].extents.cy + 8.0;
                }
                auto const laneStart = n.bounds.x + padding + (n.parameters.empty() ? 24.0 : n.parametersWidth + 48.0);
                p = point{ laneStart, top };
                for (auto i : n.children)
                {
                    iNodes[i].bounds = rect{ p + iNodes[i].offset, iNodes[i].extents };
                    p.x += iNodes[i].extents.cx + gap();
                }
                n.lane = rect{ point{ laneStart, top }, n.laneExtents };
                p = point{ laneStart, top + (n.children.empty() ? 0.0 : n.laneExtents.cy + gap() / 2.0) };
                for (auto i : n.alternatives)
                {
                    iNodes[i].bounds = rect{ p + iNodes[i].offset, iNodes[i].extents };
                    p.y += iNodes[i].extents.cy + gap() / 2.0;
                }
            }
        }
        // (where a node has been dragged to, from its place (its offset from its container's first): not before its container's first)
        point moved_offset(node const& aNode, point const& aPlace) const
        {
            auto const moved = iMoved.find(aNode.key);
            if (moved == iMoved.end())
                return point{};
            return point{ std::max(moved->second.x, -aPlace.x), std::max(moved->second.y, -aPlace.y) };
        }
        // (the place among its siblings that a statement being dragged would be moved to: before the first sibling whose centre is to 
        // the right of its centre (none: its own place, or a parameter))
        std::optional<std::size_t> drop_index(std::uint32_t aNode) const
        {
            auto const& n = iNodes[aNode];
            if (n.outline.kind == outline_kind::Parameter || n.outline.kind == outline_kind::Alternative)
                return std::nullopt;
            auto const& siblings = iNodes[n.outline.parent].children;
            auto const current = static_cast<std::size_t>(std::find(siblings.begin(), siblings.end(), aNode) - siblings.begin());
            std::size_t before = 0u;
            for (auto s : siblings)
                if (s != aNode && iNodes[s].bounds.center().x < n.bounds.center().x)
                    ++before;
            auto const index = before <= current ? before : before + 1u;
            return (index == current || index == current + 1u) ? std::nullopt : std::optional<std::size_t>{ index };
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
                iZoomedFont = font().with_size(std::max(font().size() * iZoom, 1.0));
            return *iZoomedFont;
        }
        neogfx::font const& zoomed_label_font() const
        {
            if (!iZoomedLabelFont)
                iZoomedLabelFont = font().with_size(std::max(font().size() * 0.8 * iZoom, 1.0));
            return *iZoomedLabelFont;
        }
        void draw_flow(i_graphics_context& aGc, point const& aFrom, point const& aTo, color const& aColour) const
        {
            // (its arrow head in the direction of its line (nodes that have been dragged may be anywhere))
            auto const arrow = 6.0 * iZoom;
            auto const dx = aTo.x - aFrom.x;
            auto const dy = aTo.y - aFrom.y;
            auto const length = std::hypot(dx, dy);
            point const direction = length > 0.0 ? point{ dx / length, dy / length } : point{ 1.0, 0.0 };
            point const normal{ -direction.y, direction.x };
            point const base{ aTo.x - direction.x * arrow, aTo.y - direction.y * arrow };
            aGc.draw_line(aFrom, base, pen{ aColour, 2.0 * iZoom });
            aGc.draw_triangle(
                point{ base.x + normal.x * arrow * 0.6, base.y + normal.y * arrow * 0.6 }, aTo, 
                point{ base.x - normal.x * arrow * 0.6, base.y - normal.y * arrow * 0.6 }, pen{ aColour, 1.0 }, aColour);
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
        color node_colour(node const& aNode) const
        {
            if (aNode.outline.kind == outline_kind::Module)
                return mix(background_colour(), text_colour(), 0.02);
            return mix(background_colour(), text_colour(), 0.05);
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
            editRect.cy = header_height() * iZoom;
            if (aInsert)
                editRect.y += aNode == 0u ? header_height() * iZoom : editRect.cy + 4.0;
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
        // (where a rename's propagate button is: in its node's header, before its label)
        rect propagate_rect(node const& aNode) const
        {
            auto const r = to_screen(aNode.bounds);
            auto const headerHeight = header_height() * iZoom;
            auto const extent = std::max(std::floor(headerHeight * 0.75), 8.0);
            return rect{ point{ r.right() - (padding_x() * 2.0 + aNode.labelWidth) * iZoom - extent, r.y + (headerHeight - extent) / 2.0 }, 
                neogfx::size{ extent } };
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
        std::map<std::u32string, point> iMoved;     ///< (where nodes have been dragged to (by key))
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
