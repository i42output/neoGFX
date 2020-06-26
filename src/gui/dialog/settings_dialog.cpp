// settings_dialog.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>

namespace neogfx
{
    class default_setting_widget_factory : public reference_counted<i_setting_widget_factory>
    {
    public:
        default_setting_widget_factory(ref_ptr<i_setting_widget_factory> aUserFactory) :
            iUserFactory{ aUserFactory }
        {
        }
    public:
        std::shared_ptr<i_widget> create_widget(const neolib::i_setting& aSetting, i_layout& aLayout) const
        {
            std::shared_ptr<i_widget> settingWidget;
            if (iUserFactory)
                settingWidget = iUserFactory->create_widget(aSetting, aLayout);
            if (!settingWidget)
            {
                switch (aSetting.type())
                {
                case neolib::simple_variant_type::Boolean:
                    // todo
                    break;
                case neolib::simple_variant_type::Integer:
                    // todo
                    break;
                case neolib::simple_variant_type::Real:
                    // todo
                    break;
                case neolib::simple_variant_type::String:
                    // todo
                    break;
                case neolib::simple_variant_type::Enum:
                    // todo
                    break;
                case neolib::simple_variant_type::CustomType:
                    {
                        auto const& customTypeName = aSetting.value().get<i_ref_ptr<neolib::i_custom_type>>()->name();
                        // todo
                        throw unsupported_setting_type(customTypeName);
                    }
                    break;
                }
            }
            return settingWidget;
        }
    private:
        ref_ptr<i_setting_widget_factory> iUserFactory;
    };

    settings_dialog::settings_dialog(neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory) :
        dialog{ "Settings", window_style::DefaultDialog },
        iSettings{ aSettings },
        iLayout{ client_layout() },
        iTree{ iLayout },
        iDetails{ iLayout },
        iDetailLayout{ iDetails }
    {
        init();
    }

    settings_dialog::settings_dialog(i_widget& aParent, neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory) :
        dialog{ aParent, "Settings", window_style::DefaultDialog },
        iSettings{ aSettings },
        iLayout{ client_layout() },
        iTree{ iLayout },
        iDetails{ iLayout },
        iDetailLayout{ iDetails }
    {
        init();
    }

    settings_dialog::~settings_dialog()
    {
    }

    void settings_dialog::init()
    {
        set_minimum_size(size{ 760_dip, 440_dip });
        iLayout.set_size_policy(size_constraint::Expanding);
        iTree.set_weight(size{ 1.0, 1.0 });
        iDetails.set_size_policy(size_constraint::Expanding);
        iDetails.set_weight(size{ 2.0, 1.0 });
        iDetailLayout.set_size_policy(size_constraint::Expanding);

        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);
        button_box().add_button(standard_button::Apply);

        center_on_parent();

        iTree.set_frame_style(frame_style::NoFrame);
        iDetails.set_frame_color();

        auto update_colors = [&](style_aspect aspect)
        {
            if (aspect == style_aspect::Color)
            {
                iTree.set_background_color(container_background_color());
                iDetails.set_background_color(container_background_color());
                iDetails.set_frame_color(container_background_color().dark() ? color::Black : color::White);
            }
        };
        update_colors(style_aspect::Color);
        iSink += service<i_app>().current_style_changed(update_colors);

        set_ready_to_render(true);
    }
}