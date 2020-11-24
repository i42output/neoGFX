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
#include <neolib/core/i_enum.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/settings_dialog.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/slider_box.hpp>
#include <neogfx/gui/widget/color_widget.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>
#include <neogfx/gui/widget/font_widget.hpp>

namespace neogfx
{
    template <typename Base>
    struct setting_widget : public Base
    {
        bool updating = false;

        typedef Base base_type;
        using base_type::base_type;
    };

    template <typename T>
    struct create_slider_box
    {
        ref_ptr<i_widget> operator()(neolib::i_setting& aSetting, i_layout& aLayout, sink& aSink)
        {
            auto settingWidget = make_ref<setting_widget<basic_slider_box<T>>>(aLayout);
            settingWidget->set_minimum(aSetting.constraints().minimum_value<T>());
            settingWidget->set_maximum(aSetting.constraints().maximum_value<T>());
            settingWidget->set_step(aSetting.constraints().step_value<T>());
            settingWidget->set_value(aSetting.value().get<T>());
            aSink += settingWidget->value_changed([&, settingWidget]()
            {
                if (!settingWidget->updating)
                    aSetting.set_value(settingWidget->value());
            });
            aSink += aSetting.changing([&, settingWidget]()
            {
                neolib::scoped_flag sf{ settingWidget->updating };
                settingWidget->set_value(aSetting.value<T>(true));
            });
            aSink += aSetting.changed([&, settingWidget]()
            {
                neolib::scoped_flag sf{ settingWidget->updating };
                settingWidget->set_value(aSetting.value<T>(true));
            });
            return settingWidget;
        }
    };

    class default_setting_widget_factory : public reference_counted<i_setting_widget_factory>
    {
    public:
        typedef i_setting_widget_factory abstract_type;
    public:
        default_setting_widget_factory(neolib::i_settings const& aSettings, ref_ptr<i_setting_widget_factory> aUserFactory) :
            iSettings{ aSettings },
            iUserFactory{ aUserFactory }
        {
        }
    public:
        void create_widget(neolib::i_setting& aSetting, i_layout& aLayout, sink& aSink, i_ref_ptr<i_widget>& aResult) const override
        {
            ref_ptr<i_widget> result;
            if (iUserFactory)
                result = iUserFactory->create_widget(aSetting, aLayout, aSink);
            if (!result)
            {
                switch (aSetting.value().type())
                {
                case neolib::setting_type::Boolean:
                    {
                        auto settingWidget = make_ref<setting_widget<check_box>>(aLayout);
                        settingWidget->set_checked(aSetting.value().get<bool>());
                        aSink += settingWidget->Checked([&, settingWidget]()
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(true);
                        });
                        aSink += settingWidget->Unchecked([&, settingWidget]()
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(false);
                        });
                        aSink += aSetting.changing([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_checked(aSetting.value<bool>(true));
                        });
                        aSink += aSetting.changed([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_checked(aSetting.value<bool>(true));
                        });
                        result = settingWidget;
                    }
                    break;
                case neolib::setting_type::Int8:
                    result = create_slider_box<int8_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Int16:
                    result = create_slider_box<int16_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Int32:
                    result = create_slider_box<int32_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Int64:
                    result = create_slider_box<int64_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Uint8:
                    result = create_slider_box<uint8_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Uint16:
                    result = create_slider_box<uint16_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Uint32:
                    result = create_slider_box<uint32_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Uint64:
                    result = create_slider_box<uint64_t>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Float32:
                    result = create_slider_box<float>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::Float64:
                    result = create_slider_box<double>{}(aSetting, aLayout, aSink);
                    break;
                case neolib::setting_type::String:
                    // todo
                    break;
                case neolib::setting_type::Enum:
                    {
                        auto settingWidget = make_ref<setting_widget<drop_list>>(aLayout);
                        auto const& e = aSetting.value().get<neolib::i_enum>();
                        auto enumModel = make_ref<basic_item_model<neolib::i_enum::underlying_type>>();
                        for (auto& ee : e.enumerators())
                            enumModel->insert_item(enumModel->end(), ee.first(), iSettings.friendly_text(aSetting, ee.second()).to_std_string());
                        settingWidget->set_model(enumModel);
                        aSink += settingWidget->selection_changed([&, settingWidget, enumModel](const optional_item_model_index& aCurrentIndex)
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(enumModel->item(*aCurrentIndex));
                        });
                        auto update_widget = [&, settingWidget, enumModel]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            auto const modelIndex = enumModel->find_item(aSetting.value(true).get<neolib::i_enum>().value());
                            settingWidget->selection_model().set_current_index(settingWidget->presentation_model().from_item_model_index(modelIndex));
                            settingWidget->accept_selection();
                        };
                        aSink += aSetting.changing(update_widget);
                        aSink += aSetting.changed(update_widget);
                        update_widget();
                        result = settingWidget;
                    }
                    break;
                case neolib::setting_type::Custom:
                    if (aSetting.value().type_name() == "neogfx::color")
                    {
                        auto settingWidget = make_ref<setting_widget<color_widget>>(aLayout, aSetting.value().get<color>());
                        aSink += settingWidget->ColorChanged([&, settingWidget]()
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(settingWidget->color());
                        });
                        aSink += aSetting.changing([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_color(aSetting.value<color>(true));
                        });
                        aSink += aSetting.changed([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_color(aSetting.value<color>(true));
                        });
                        result = settingWidget;
                    }
                    else if (aSetting.value().type_name() == "neogfx::gradient")
                    {
                        auto settingWidget = make_ref<setting_widget<gradient_widget>>(aLayout, aSetting.value().get<unique_gradient>());
                        settingWidget->set_size_policy(size_constraint::Minimum, size_constraint::Minimum);
                        aSink += settingWidget->GradientChanged([&, settingWidget]()
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(unique_gradient{ settingWidget->gradient() });
                        });
                        aSink += aSetting.changing([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_gradient(unique_gradient{ aSetting.value<unique_gradient>(true) });
                        });
                        aSink += aSetting.changed([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->set_gradient(unique_gradient{ aSetting.value<unique_gradient>(true) });
                        });
                        result = settingWidget;
                    }
                    else if (aSetting.value().type_name() == "neogfx::font")
                    {
                        auto settingWidget = make_ref<setting_widget<font_widget>>(aLayout, font{ aSetting.value().get<font_info>() });
                        settingWidget->set_size_policy(size_constraint::Minimum, size_constraint::Minimum);
                        aSink += settingWidget->SelectionChanged([&, settingWidget]()
                        {
                            if (!settingWidget->updating)
                                aSetting.set_value(static_cast<const font_info&>(settingWidget->selected_font()));
                        });
                        aSink += aSetting.changing([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->select_font(aSetting.value<font_info>(true));
                        });
                        aSink += aSetting.changed([&, settingWidget]()
                        {
                            neolib::scoped_flag sf{ settingWidget->updating };
                            settingWidget->select_font(aSetting.value<font_info>(true));
                        });
                        result = settingWidget;
                    }
                    break;
                default:
                    break;
                }
            }
            if (!result)
                throw unsupported_setting_type();
            aResult = result;
        }
    private:
        neolib::i_settings const& iSettings;
        ref_ptr<i_setting_widget_factory> iUserFactory;
    };

    settings_dialog::settings_dialog(neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory, ref_ptr<i_setting_icons> aIcons) :
        dialog{ "Settings", window_style::DefaultDialog },
        iSettings{ aSettings },
        iWidgetFactory{ make_ref<default_setting_widget_factory>(aSettings, aWidgetFactory) },
        iIcons{ aIcons },
        iLayout{ client_layout() },
        iTree{ iLayout },
        iDetails{ iLayout },
        iDetailLayout{ iDetails },
        iBackground{ aIcons != nullptr ? texture{ aIcons->default_icon() } : texture{ image{ ":/neogfx/resources/images/settings.png" } } }
    {
        init();
    }

    settings_dialog::settings_dialog(i_widget& aParent, neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory, ref_ptr<i_setting_icons> aIcons) :
        dialog{ aParent, "Settings", window_style::DefaultDialog },
        iSettings{ aSettings },
        iWidgetFactory{ make_ref<default_setting_widget_factory>(aSettings, aWidgetFactory) },
        iIcons{ aIcons },
        iLayout{ client_layout() },
        iTree{ iLayout },
        iDetails{ iLayout },
        iDetailLayout{ iDetails },
        iBackground{ aIcons != nullptr ? texture{ aIcons->default_icon()  } : texture{ image{ ":/neogfx/resources/images/settings.png" } } }
    {
        init();
    }

    settings_dialog::~settings_dialog()
    {
    }

    typedef ref_ptr<neolib::vector<ref_ptr<i_widget>>> setting_group_widget_list;
    typedef basic_item_tree_model<setting_group_widget_list> settings_tree_item_model;
    class settings_tree_presentation_model : public basic_item_presentation_model<settings_tree_item_model>
    {
    public:
        settings_tree_presentation_model(ref_ptr<i_setting_icons> aIcons) :
            iIcons{ aIcons },
            iDefaultIcon{ image{ ":/neogfx/resources/images/settings.png" } }
        {
        }
    public:
        optional_font cell_font(item_presentation_model_index const& aIndex) const override
        {
            if (!item_model().has_parent(to_item_model_index(aIndex)))
                return default_font().with_style(font_style::Bold);
            return default_font().with_size(10);
        }
        optional_texture cell_image(item_presentation_model_index const& aIndex) const override
        {
            if (iIcons == nullptr)
                return iDefaultIcon;
            return iIcons->default_icon();
        }
        optional_size cell_image_size(item_presentation_model_index const& aIndex) const override
        {
            if (!item_model().has_parent(to_item_model_index(aIndex)))
                return size{ 32.0_dip, 32.0_dip };
            return size{ 16.0_dip, 16.0_dip };
        }
    private:
        ref_ptr<i_setting_icons> iIcons;
        texture iDefaultIcon;
    };

    class setting_group_widget : public widget<>
    {
    public:
        setting_group_widget(std::string const& aTitle) :
            iLayout{ *this },
            iTitle{ iLayout, translate(aTitle) }
        {
            set_padding({});
            set_size_policy(size_constraint::Expanding, size_constraint::Minimum);
            iLayout.set_padding({});
            auto reset_font = [&]()
            {
                iTitle.text_widget().set_font(service<i_app>().current_style().font().with_size(service<i_app>().current_style().font().size() * 1.25).with_underline(true));
            };
            iSink += service<i_app>().current_style_changed([this, reset_font](style_aspect aAspect)
            {
                if ((aAspect & style_aspect::Font) == style_aspect::Font)
                    reset_font();
            });
            reset_font();
        }
    public:
        vertical_layout iLayout;
        label iTitle;
        sink iSink;
    };

    void settings_dialog::init()
    {
        set_minimum_size(size{ 672_dip, 446_dip });
        iLayout.set_size_policy(size_constraint::Expanding);
        iTree.set_weight(size{ 1.0, 1.0 });
        iDetails.set_minimum_size(size{});
        iDetails.set_size_policy(size_constraint::Expanding);
        iDetails.set_weight(size{ 2.0, 1.0 });
        iDetailLayout.set_size_policy(size_constraint::Expanding);

        auto treeModel = make_ref<settings_tree_item_model>();
        auto treePresentationModel = make_ref<settings_tree_presentation_model>(iIcons);
        iTree.set_model(treeModel);
        iTree.set_presentation_model(treePresentationModel);

        std::map<std::string, ref_ptr<setting_group_widget>> groupWidgets;
        for (auto const& category : iSettings.all_categories())
        {
            auto c = treeModel->insert_item(treeModel->send(), make_ref<setting_group_widget_list::element_type>(), category.second().to_std_string());
            auto existingGroups = iSettings.all_groups().find(category.first());
            if (existingGroups != iSettings.all_groups().end())
                for (auto const& group : existingGroups->second())
                {
                    auto g = treeModel->append_item(c, make_ref<setting_group_widget_list::element_type>(), group.second().to_std_string());
                    auto settingGroupWidget = make_ref<setting_group_widget>(group.second().to_std_string());
                    iDetailLayout.add(settingGroupWidget);
                    treeModel->item(c)->push_back(settingGroupWidget);
                    treeModel->item(g)->push_back(settingGroupWidget);
                    groupWidgets[group.first().to_std_string()] = settingGroupWidget;
                }
        }

        for (auto const& setting : iSettings.all_settings_ordered())
        {
            if (setting->format().empty())
                continue;
            thread_local std::vector<std::string> keyBits;
            keyBits.clear();
            keyBits = neolib::tokens(setting->key().to_std_string(), "."s);
            keyBits.resize(2);
            auto groupWidget = groupWidgets.find(keyBits[0] + "." + keyBits[1]);
            if (groupWidget == groupWidgets.end())
                continue;
            i_layout* itemLayout = nullptr;
            auto new_layout = [&]()
            {
                if (itemLayout != nullptr)
                    itemLayout->add_spacer();
                itemLayout = &groupWidget->second->layout().add<horizontal_layout>();
                itemLayout->set_padding({});
                itemLayout->set_size_policy(size_constraint::Minimum, size_constraint::Minimum);
            };
            
            new_layout();
            
            std::string nextLabel;
            std::optional<std::string> nextArgument;

            auto emit_label = [&]()
            {
                if (!nextLabel.empty())
                {
                    itemLayout->add(make_ref<label>(translate(nextLabel)));
                    nextLabel.clear();
                }
            };

            auto emit_optional_check_box = [&]()
            {
                if (!nextLabel.empty())
                {
                    auto& optionalCheckBox = itemLayout->add(make_ref<check_box>(translate(nextLabel)));
                    iSink += optionalCheckBox.Checked([&]()
                    { 
                        if (setting->is_default(true))
                            setting->set_value(setting->default_value()); 
                    });
                    iSink += optionalCheckBox.Unchecked([&]()
                    { 
                        setting->clear(); 
                    });
                    auto update_check_box = [&]()
                    {
                        optionalCheckBox.set_checked(!setting->is_default(true));
                    };
                    iSink += setting->changing(update_check_box);
                    iSink += setting->changed(update_check_box);
                    update_check_box();
                    nextLabel.clear();
                }
            };

            for (auto ch : setting->format().to_std_string())
            {
                switch (ch)
                {
                case '%':
                    if (!nextArgument)
                        nextArgument.emplace();
                    else if (nextArgument->empty())
                    {
                        nextLabel += ch;
                        nextArgument = {};
                    }
                    else
                    {
                        thread_local std::vector<std::string> bits;
                        bits.clear();
                        bits = neolib::tokens(*nextArgument, ":"s);
                        if (bits.size() == 1 && bits[0] == "?")
                            iWidgetFactory->create_widget(*setting, *itemLayout, iSink);
                        else if (bits.size() == 2 && bits[1] == "?")
                        {
                            auto existing = iSettings.all_settings().find(string{ bits[0] });
                            if (existing != iSettings.all_settings().end())
                                iWidgetFactory->create_widget(*existing->second(), *itemLayout, iSink);
                        }
                        nextArgument = {};
                    }
                    break;
                case '\n':
                    emit_label();
                    new_layout();
                    break;
                default:
                    if (nextArgument)
                    {
                        if (setting->constraints().optional())
                            emit_optional_check_box();
                        else
                            emit_label();
                        *nextArgument += ch;
                    }
                    else
                        nextLabel += ch;
                    break;
                }
            }
            emit_label();
            if (itemLayout != nullptr)
                itemLayout->add_spacer();
        }

        iDetailLayout.add_spacer();

        auto updateTreeFont = [=]()
        {
            treePresentationModel->set_default_font(service<i_app>().current_style().font().with_size(14));
        };
        iSink += service<i_app>().current_style_changed([=](style_aspect aAspect)
        {
            if ((aAspect & (style_aspect::Font)) != style_aspect::None)
                updateTreeFont();
        });
        updateTreeFont();

        treePresentationModel->set_column_read_only(0);
        iTree.selection_model().set_mode(item_selection_mode::SingleSelection);
        auto update_details = [&, treeModel](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& /* aPreviousIndex */)
        {
            for (auto c = treeModel->sbegin(); c != treeModel->send(); ++c)
                for (auto& w : *treeModel->item(c))
                    aCurrentIndex ? w->hide() : w->show();
            if (aCurrentIndex)
                for (auto& w : *treeModel->item(iTree.presentation_model().to_item_model_index(*aCurrentIndex)))
                    w->show();
        };
        iTree.selection_model().current_index_changed(update_details);
        update_details(iTree.selection_model().current_index_maybe(), {});

        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);
        button_box().add_button(standard_button::Discard);
        button_box().add_button(standard_button::Apply);

        TryAccept([&](bool& aCanAccept, bool)
        {
            aCanAccept = iSettings.modified();
        });

        auto update_buttons = [&]()
        {
            if (iSettings.modified())
            {
                button_box().enable_role(button_role::Apply);
                button_box().enable_role(button_role::Destructive);
            }
            else
            {
                button_box().disable_role(button_role::Apply);
                button_box().disable_role(button_role::Destructive);
            }
        };
        update_buttons();

        iSink += iSettings.setting_changing([update_buttons](const neolib::i_setting&)
        {
            update_buttons();
        });

        iSink += iSettings.setting_changed([update_buttons](const neolib::i_setting&)
        {
            update_buttons();
        });

        HaveResult([&](dialog_result aResult)
        {
            switch (aResult)
            {
            case dialog_result::Accepted:
                iSettings.apply_changes();
                break;
            case dialog_result::Rejected:
                iSettings.discard_changes();
                break;
            }
        });

        button_box().Clicked([&](standard_button aButton)
        {
            switch (aButton)
            {
            case standard_button::Apply:
                iSettings.apply_changes();
                break;
            case standard_button::Discard:
                iSettings.discard_changes();
                break;
            }
        });

        center_on_parent();

        iTree.set_frame_style(frame_style::NoFrame);
        iDetails.set_frame_color();

        iDetails.Painting([&](i_graphics_context& aGc)
        {
            rect const candyRect{ iDetails.client_rect().bottom_right() - size{ 128.0_dip, 128.0_dip }, size{ 256.0_dip, 256.0_dip } };
            aGc.draw_texture(candyRect, iBackground, color::White.with_alpha(0.25));
        });

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