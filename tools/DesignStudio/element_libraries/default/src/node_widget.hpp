// sticky_note.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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
#include <random>
#include <neogfx/app/action.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/node.hpp>

namespace neogfx::DesignStudio
{
    template <typename PinType>
    class node_pin_widget : public widget<>, public i_pin_widget
    {
        typedef widget<> base_type;
    public:
        node_pin_widget(PinType& aPin) :
            iPin{ aPin },
            iLayout{ *this, alignment::Left | alignment::VCenter },
            iPinIcon{ iLayout, image{ ":/neogfx/DesignStudio/default_nel/resources/circle-outline.png" } }
        {
            iPinIcon.set_fixed_size(size{ 12.0_dip });
            iPin.set_widget(*this);
            iSink += iPin.get().connection_added([&](i_node_connection&)
            {
                iPinIcon.set_image(image{ iPin.connected() ? ":/neogfx/DesignStudio/default_nel/resources/circle.png" : ":/neogfx/DesignStudio/default_nel/resources/circle-outline.png" });
            });
            iSink += iPin.get().connection_removed([&](i_node_connection&)
            {
                iPinIcon.set_image(image{ iPin.connected() ? ":/neogfx/DesignStudio/default_nel/resources/circle.png" : ":/neogfx/DesignStudio/default_nel/resources/circle-outline.png" });
            });
        }
    public:
        color_or_gradient color() const override
        {
            return iPinIcon.image_color() != none ? iPinIcon.image_color() : palette_color(color_role::Text);
        }
        i_widget const& icon() const override
        {
            return iPinIcon;
        }
        i_widget& icon() override
        {
            return iPinIcon;
        }
    private:
        PinType& iPin;
        horizontal_layout iLayout;
        image_widget iPinIcon;
        sink iSink;
    };

    class node_input_pin_widget : public node_pin_widget<i_node_input_pin>
    {
        typedef node_pin_widget<i_node_input_pin> base_type;
    public:
        using base_type::base_type;
    };

    class node_output_pin_widget : public node_pin_widget<i_node_output_pin>
    {
        typedef node_pin_widget<i_node_output_pin> base_type;
    public:
        using base_type::base_type;
    };

    class node_widget : public widget<>
    {
        typedef widget<> base_type;
    public:
        node_widget(i_element& aElement) :
            iNode{ static_cast<i_node&>(aElement) },
            iLayout{ *this, alignment::Left | alignment::Top },
            iTitleLayout{ iLayout, alignment::Left | alignment::VCenter },
            iPanelLayout{ iLayout, alignment::Top },
            iInputPanel{ iPanelLayout, alignment::Left | alignment::Top },
            iPanelSpacer{ iPanelLayout },
            iOutputPanel{ iPanelLayout, alignment::Right | alignment::Top },
            iTitle{ iTitleLayout, "Node"_t }
        {
            set_padding(neogfx::padding{ 2.0_dip, 2.0_dip, 2.0_dip, 2.0_dip });
            set_minimum_size(size{ 128.0_dip, 128.0_dip });
            iLayout.set_padding(neogfx::padding{ 0.0 });
            iTitleLayout.set_padding(neogfx::padding{ 4.0_dip, 2.0_dip, 4.0_dip, 2.0_dip });
            iTitle.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
            iPanelLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding });
            iInputPanel.set_size_policy(neogfx::size_policy{ size_constraint::Expanding });
            iOutputPanel.set_size_policy(neogfx::size_policy{ size_constraint::Expanding });
            iPanelLayout.set_weight(neogfx::size{ 1.0 });
            iInputPanel.set_weight(neogfx::size{ 1.0 });
            iOutputPanel.set_weight(neogfx::size{ 1.0 });
            
            iSink += aElement.context_menu([&](i_menu& aMenu)
            {
            });
            iSink += iNode.input_added([&](i_node_input_pin& aPin) 
            {
                iInputPanel.emplace<node_input_pin_widget>(aPin);
            });
            iSink += iNode.output_added([&](i_node_output_pin& aPin) 
            {
                iOutputPanel.emplace<node_output_pin_widget>(aPin);
            });

            iNode.add_input(*make_ref<node_input_pin>(iNode));
            iNode.add_input(*make_ref<node_input_pin>(iNode));
            iNode.add_input(*make_ref<node_input_pin>(iNode));
            iNode.add_output(*make_ref<node_output_pin>(iNode));

            for (auto& e : iNode.parent())
            {
                auto n = dynamic_cast<i_node*>(&*e);
                if (n && n != &iNode && n->connected_inputs() == 0)
                {
                    connect_nodes(*iNode.outputs()[0], *n->inputs()[0]);
                    break;
                }
            }
        }
    protected:
        widget_part part(const point& aPosition) const override
        {
            return base_type::part(aPosition);
        }
    protected:
        void layout_items(bool aDefer = false) override
        {
            base_type::layout_items(aDefer);
        }
    protected:
        void paint(i_graphics_context& aGc) const override
        {
            base_type::paint(aGc);
            auto const nodeColor = palette_color(color_role::Void);
            aGc.draw_rounded_rect(client_rect(), 8.0_dip, nodeColor, nodeColor.with_alpha(0.5));
            scoped_scissor ss{ aGc, node_title_rect() };
            aGc.fill_rounded_rect(node_title_rect(), 8.0_dip, nodeColor.shaded(0x30));
        }
    protected:
        double background_opacity() const override
        {
            return 0.0;
        }
        color palette_color(color_role aColorRole) const override
        {
            return base_type::palette_color(aColorRole);
        }
    protected:
        const i_widget& get_widget_at(const point& aPosition) const override
        {
            auto& result = base_type::get_widget_at(aPosition);
            if (&result == this)
                return base_type::parent();
            return result;
        }
    private:
        rect node_title_rect() const
        {
            return rect{ iTitleLayout.position(), iTitleLayout.extents() };
        }
    private:
        i_node& iNode;
        sink iSink;
        vertical_layout iLayout;
        vertical_layout iTitleLayout;
        horizontal_layout iPanelLayout;
        vertical_layout iInputPanel;
        horizontal_spacer iPanelSpacer;
        vertical_layout iOutputPanel;
        label iTitle;
    };

    template <>
    struct element_base<node_widget>
    {
        typedef node base;
    };
}
