#include <neogfx/neogfx.hpp>

#include <neolib/neolib.hpp>
#include <csignal>
#include <boost/lexical_cast.hpp>

#include <neolib/core/random.hpp>
#include <neolib/task/thread_pool.hpp>
#include <neolib/app/i_power.hpp>

#include <neogfx/core/easing.hpp>
#include <neogfx/core/i_transition_animator.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_game_controllers.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/gui/dialog/game_controller_dialog.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/game_world.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/collision_detector.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_render_cache.hpp>
#include <neogfx/game/animator.hpp>
#include <neogfx/game/time.hpp>

#include "test.ui.hpp"

namespace ng = neogfx;
using namespace ng::string_literals;
using namespace ng::unit_literals;

class my_item_model : public ng::basic_item_model<void*, 9u>
{
public:
    my_item_model()
    {
        set_column_name(0, "Zero");
        set_column_name(1, "One");
        set_column_name(2, "Two");
        set_column_name(3, "Three");
        set_column_name(4, "Four");
        set_column_name(5, "Five");
        set_column_name(6, "Six");
        set_column_name(7, "Empty");
        set_column_name(8, "Eight");
    }
public:
    const ng::item_cell_data& cell_data(const ng::item_model_index& aIndex) const override
    {
        if (aIndex.column() == 4)
        {
            if (aIndex.row() == 4)
            {
                static const ng::item_cell_data sReadOnly = { "** Read Only **" };
                return sReadOnly;
            }
            if (aIndex.row() == 5)
            {
                static const ng::item_cell_data sUnselectable = { "** Unselectable **" };
                return sUnselectable;
            }
        }
        return ng::basic_item_model<void*, 9u>::cell_data(aIndex);
    }
};

template <typename Model>
class my_basic_item_presentation_model : public ng::basic_item_presentation_model<Model>
{
    typedef ng::basic_item_presentation_model<Model> base_type;
public:
    typedef Model model_type;
public:
    my_basic_item_presentation_model(model_type& aModel, bool aSortable = false) :
        base_type{ aModel, aSortable },
        iCellImages{ {
            ng::image{ ":/test/resources/icon.png" }, 
            ng::image{ ":/closed/resources/caw_toolbar.zip#contacts.png" },
            ng::image{ ":/closed/resources/caw_toolbar.zip#favourite.png" },
            ng::image{ ":/closed/resources/caw_toolbar.zip#folder.png" },
            {}
        } }
    {
    }
public:
    ng::item_cell_flags cell_flags(const ng::item_presentation_model_index& aIndex) const override
    {
        if constexpr (model_type::container_traits::is_flat)
        {
            auto const modelIndex = base_type::to_item_model_index(aIndex);
            if (modelIndex.column() == 4)
            {
                if (modelIndex.row() == 4)
                    return base_type::cell_flags(aIndex) & ~ng::item_cell_flags::Editable;
                if (modelIndex.row() == 5)
                    return base_type::cell_flags(aIndex) & ~ng::item_cell_flags::Selectable;
            }
        }
        return base_type::cell_flags(aIndex);
    }
    void set_color_role(const std::optional<ng::color_role>& aColorRole)
    {
        iColorRole = aColorRole;
    }
    ng::optional_color cell_color(const ng::item_presentation_model_index& aIndex, ng::color_role aColorRole) const override
    {
        // use seed to make random color based on row/index ...
        neolib::basic_random<double> prng{ (base_type::to_item_model_index(aIndex).row() << 16) + base_type::to_item_model_index(aIndex).column() };
        auto const textColor = ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Text);
        auto const backgroundColor = ng::service<ng::i_app>().current_style().palette().color(aIndex.row() % 2 == 0 ? ng::color_role::Base : ng::color_role::AlternateBase);
        if (aColorRole == iColorRole)
            return ng::color{ prng(0.0, 1.0), prng(0.0, 1.0), prng(0.0, 1.0) }.with_lightness((aColorRole == ng::color_role::Text ? textColor.light() : backgroundColor.light()) ? 0.85 : 0.15);
        else if (aColorRole == ng::color_role::Base && iColorRole)
            return ng::color::Black;
        else if (aColorRole == ng::color_role::Background)
            return backgroundColor;
        return {};
    }
    ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
    {
        if (base_type::column_image_size(aIndex.column()))
        {
            auto const idx = (std::hash<uint32_t>{}(base_type::to_item_model_index(aIndex).row()) + std::hash<uint32_t>{}(base_type::to_item_model_index(aIndex).column())) % 5;
            return iCellImages[idx];
        }
        return ng::optional_texture{};
    }
private:
    std::optional<ng::color_role> iColorRole;
    std::array<ng::optional_texture, 5> iCellImages;
};

typedef my_basic_item_presentation_model<my_item_model> my_item_presentation_model;
typedef my_basic_item_presentation_model<ng::item_tree_model> my_item_tree_presentation_model;

class easing_item_presentation_model : public ng::default_drop_list_presentation_model<ng::basic_item_model<ng::easing>>
{
    typedef ng::default_drop_list_presentation_model<ng::basic_item_model<ng::easing>> base_type;
public:
    easing_item_presentation_model(ng::drop_list& aDropList, ng::basic_item_model<ng::easing>& aModel, bool aLarge = true) : base_type{ aDropList, aModel }, iLarge{ aLarge }
    {
        iSink += ng::service<ng::i_app>().current_style_changed([this](ng::style_aspect)
        {
            iTextures.clear();
            VisualAppearanceChanged.async_trigger();
        });
    }
public:
    ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
    {
        auto easingFunction = item_model().item(to_item_model_index(aIndex));
        auto iterTexture = iTextures.find(easingFunction);
        if (iterTexture == iTextures.end())
        {
            ng::dimension const d = iLarge ? 48.0 : 24.0;
            ng::texture newTexture{ ng::size{d, d}, 1.0, ng::texture_sampling::Multisample };
            ng::graphics_context gc{ newTexture };
            ng::scoped_snap_to_pixel snap{ gc };
            auto const textColor = ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Text);
            gc.draw_rect(ng::rect{ ng::point{}, ng::size{d, d} }, ng::pen{ textColor, 1.0 });
            ng::optional_point lastPos;
            ng::pen pen{ textColor, 2.0 };
            for (double x = 0.0; x <= d - 8.0; x += 2.0)
            {
                ng::point pos{ x + 4.0, ng::ease(easingFunction, x / (d - 8.0)) * (d - 8.0) + 4.0 };
                if (lastPos != std::nullopt)
                {
                    gc.draw_line(*lastPos, pos, pen);
                }
                lastPos = pos;
            }
            iterTexture = iTextures.emplace(easingFunction, newTexture).first;
        }
        return iterTexture->second;
    }
private:
    bool iLarge;
    mutable std::map<ng::easing, ng::texture> iTextures;
    ng::sink iSink;
};

class keypad_button : public ng::push_button
{
public:
    keypad_button(ng::text_edit& aTextEdit, uint32_t aNumber) :
        ng::push_button{ boost::lexical_cast<std::string>(aNumber) }, iTextEdit{ aTextEdit }
    {
        clicked([this, aNumber]()
        {
            ng::service<ng::i_app>().change_style("Keypad").
                palette().set_color(ng::color_role::Theme, aNumber != 9 ? ng::color{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 } : ng::color::LightGoldenrod);
            if (aNumber == 9)
                iTextEdit.set_default_style(ng::text_edit::character_style{ ng::optional_font{}, ng::gradient{ ng::color::DarkGoldenrod, ng::color::LightGoldenrodYellow, ng::gradient_direction::Horizontal }, ng::color_or_gradient{} });
            else if (aNumber == 8)
                iTextEdit.set_default_style(ng::text_edit::character_style{ ng::font{"SnareDrum One NBP", "Regular", 60.0}, ng::color::White });
            else if (aNumber == 0)
                iTextEdit.set_default_style(ng::text_edit::character_style{ ng::font{"SnareDrum Two NBP", "Regular", 60.0}, ng::color::White });
            else
                iTextEdit.set_default_style(
                    ng::text_edit::character_style{
                        ng::optional_font{},
                        ng::gradient{
                            ng::color{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 }.lighter(0x40),
                            ng::color{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 }.lighter(0xC0),
                            ng::gradient_direction::Horizontal},
                        ng::color_or_gradient{} });
        });
    }
private:
    ng::text_edit& iTextEdit;
};

ng::game::i_ecs& create_game(ng::i_layout& aLayout);

