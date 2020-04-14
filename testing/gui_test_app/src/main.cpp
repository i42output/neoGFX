#include <neolib/neolib.hpp>
#include <csignal>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <neolib/random.hpp>
#include <neogfx/core/easing.hpp>
#include <neogfx/core/i_animator.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/app/file_dialog.hpp>

#include "test.ui.hpp"

namespace ng = neogfx;
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
            ng::image{ ":/closed/resources/caw_toolbar.naa#contacts.png" },
            ng::image{ ":/closed/resources/caw_toolbar.naa#favourite.png" },
            ng::image{ ":/closed/resources/caw_toolbar.naa#folder.png" },
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
        auto const backgroundColor = ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Background);
        if (aColorRole == iColorRole)
            return ng::color{ prng(0.0, 1.0), prng(0.0, 1.0), prng(0.0, 1.0) }.with_lightness((aColorRole == ng::color_role::Text ? textColor.light() : backgroundColor.light()) ? 0.85 : 0.15);
        else if (aColorRole == ng::color_role::Foreground && iColorRole)
            return ng::color::Black;
        else if (aColorRole == ng::color_role::Background)
            return backgroundColor.shade(aIndex.row() % 2 == 0 ? 0x00 : 0x08);
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
                iTextEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ ng::color::DarkGoldenrod, ng::color::LightGoldenrodYellow, ng::gradient_direction::Horizontal }, ng::color_or_gradient{} });
            else if (aNumber == 8)
                iTextEdit.set_default_style(ng::text_edit::style{ ng::font{"SnareDrum One NBP", "Regular", 60.0}, ng::color::White });
            else if (aNumber == 0)
                iTextEdit.set_default_style(ng::text_edit::style{ ng::font{"SnareDrum Two NBP", "Regular", 60.0}, ng::color::White });
            else
                iTextEdit.set_default_style(
                    ng::text_edit::style{
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

void create_game(ng::i_layout& aLayout);

void signal_handler(int signal)
{
       if (signal == SIGABRT) {
        std::cerr << "SIGABRT received\n";
    }
    else {
        std::cerr << "Unexpected signal " << signal << " received\n";
    }
    std::_Exit(EXIT_FAILURE);
}

inline ng::quad line_to_quad(const ng::vec3& aStart, const ng::vec3& aEnd, double aLineWidth)
{
    auto const vecLine = aEnd - aStart;
    auto const length = vecLine.magnitude();
    auto const halfWidth = aLineWidth / 2.0;
    auto const v1 = ng::vec3{ -halfWidth, -halfWidth, 0.0 };
    auto const v2 = ng::vec3{ -halfWidth, halfWidth, 0.0 };
    auto const v3 = ng::vec3{ length + halfWidth, halfWidth, 0.0 };
    auto const v4 = ng::vec3{ length + halfWidth, -halfWidth, 0.0 };
    auto const r = ng::rotation_matrix(ng::vec3{ 1.0, 0.0, 0.0 }, vecLine);
    return ng::quad{ aStart + r * v1, aStart + r * v2, aStart + r * v3, aStart + r * v4 };
}

int main(int argc, char* argv[])
{
    auto previous_handler = std::signal(SIGABRT, signal_handler);
    if (previous_handler == SIG_ERR) 
    {
        std::cerr << "SIGABRT handler setup failed\n";
        return EXIT_FAILURE;
    }

    /* Yes this is an 800 line (and counting) function and whilst in general such long functions are
    egregious this function is a special case: it is test code which mostly just creates widgets. 
    Most of this code is about to disappear into code auto-generated by the neoGFX resource compiler! */

    test::ui ui{ argc, argv };
    auto& app = ui.appTest;

    try
    {
        app.change_style("Light").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Dark").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.register_style(ng::style("Keypad")).set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Keypad");
        app.current_style().palette().set_color(ng::color_role::Theme, ng::color::Black);
        app.change_style("Dark");

        ng::window& window = ui.mainWindow;

        bool showFps = false;
        bool fullRefresh = false;
        auto fpsFont = window.font().with_size(18);
        window.PaintOverlay([&showFps, &window, fpsFont](ng::i_graphics_context& aGc)
        {
            if (showFps)
            {
                auto const numbers = (boost::format("%1$.2f/%2$.2f") % window.fps() % window.potential_fps()).str();
                aGc.draw_text(ng::point{ 100, 120 }, (boost::format(" %1% FPS/PFPS ") % numbers).str(), fpsFont, ng::text_appearance{ ng::color::White, ng::color::DarkBlue.darker(0x40), ng::text_effect{ ng::text_effect_type::Outline, ng::color::Black } });
            }
        });

        window.surface().rendering_finished([&fullRefresh, &window]()
        {
            if (fullRefresh)
                window.update();
        });

        app.actionFileOpen.triggered([&]()
        {
            auto textFile = ng::open_file_dialog(window, ng::file_dialog_spec{ "Edit Text File", {}, { "*.txt" }, "Text Files" });
            if (textFile)
            {
                std::ifstream file{ (*textFile)[0] };
                ui.textEdit.set_plain_text(std::string{ std::istreambuf_iterator<char>{file}, {} });
            }
        });
        
        app.add_action("Goldenrod Style").set_shortcut("Ctrl+Alt+Shift+G").triggered([]()
        {
            ng::service<ng::i_app>().change_style("Keypad").palette().set_color(ng::color_role::Theme, ng::color::LightGoldenrod);
        });

        ui.actionContacts.triggered([]()
        {
            ng::service<ng::i_app>().change_style("Keypad").palette().set_color(ng::color_role::Theme, ng::color::White);
        });
        
        neolib::callback_timer ct{ app, [&app, &ui](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (ng::service<ng::i_clipboard>().sink_active())
            {
                auto& sink = ng::service<ng::i_clipboard>().active_sink();
                if (sink.can_paste())
                {
                    ui.actionPasteAndGo.enable();
                }
                else
                {
                    ui.actionPasteAndGo.disable();
                }
            }
            if (ui.actionArcadeMode.is_checked())
                ng::service<ng::i_rendering_engine>().want_game_mode();
        }, 100 };

        ui.actionPasteAndGo.triggered([&app]()
        {
            ng::service<ng::i_clipboard>().paste();
        });

        neolib::random menuPrng{ 0 };
        for (int i = 1; i <= 5; ++i)
        {
            auto& sm = ui.menuFavourites.add_sub_menu("More_" + boost::lexical_cast<std::string>(i));
            for (int j = 1; j <= 5; ++j)
            {
                auto& sm2 = sm.add_sub_menu("More_" + boost::lexical_cast<std::string>(i) + "_" + boost::lexical_cast<std::string>(j));
                int n = menuPrng(100);
                for (int k = 1; k < n; ++k)
                {
                    sm2.add_action(app.add_action("More_" + boost::lexical_cast<std::string>(i) + "_" + boost::lexical_cast<std::string>(j) + "_" + boost::lexical_cast<std::string>(k), ":/closed/resources/caw_toolbar.naa#favourite.png"));
                }
            }
        }

        ui.actionColorDialog.triggered([&window]()
        {
            ng::color_dialog cd(window);
            cd.exec();
        });

        ui.actionManagePlugins.triggered([&]()
            {
                ng::service<ng::i_rendering_engine>().enable_frame_rate_limiter(!ng::service<ng::i_rendering_engine>().frame_rate_limited());
            });

        ui.actionNextTab.triggered([&]() { ui.tabPages.select_next_tab(); });
        ui.actionPreviousTab.triggered([&]() { ui.tabPages.select_previous_tab(); });

        ui.gradientWidget.gradient_changed([&]()
        {
            auto s = ui.textEdit.default_style();
            auto s2 = ui.textEditEditor.default_style();
            s.set_glyph_color(ui.gradientWidget.gradient());
            s2.set_glyph_color(ui.gradientWidget.gradient());
            s2.set_background_color(ng::color_or_gradient{});
            ui.textEdit.set_default_style(s);
            ui.textEditEditor.set_default_style(s2);
        });

        ui.button1.clicked([&ui]()
        {
            if (ui.tabPages.style() == ng::tab_container_style::TabAlignmentTop)
                ui.tabPages.set_style(ng::tab_container_style::TabAlignmentBottom);
            else if (ui.tabPages.style() == ng::tab_container_style::TabAlignmentBottom)
                ui.tabPages.set_style(ng::tab_container_style::TabAlignmentLeft);
            else if (ui.tabPages.style() == ng::tab_container_style::TabAlignmentLeft)
                ui.tabPages.set_style(ng::tab_container_style::TabAlignmentRight);
            else if (ui.tabPages.style() == ng::tab_container_style::TabAlignmentRight)
                ui.tabPages.set_style(ng::tab_container_style::TabAlignmentTop);
        });
        ui.buttonChina.clicked([&window, &ui]() 
        { 
            window.set_title_text(u8"请停止食用犬"); 
            if (ui.buttonChina.has_maximum_size())
                ui.buttonChina.set_maximum_size(ng::optional_size{});
            else 
                ui.buttonChina.set_maximum_size(ng::size{ 128_dip, 64_dip });
        });
        ui.dropList.model().insert_item(ui.dropList.model().end(), "Red");
        ui.dropList.model().insert_item(ui.dropList.model().end(), "Green");
        ui.dropList.model().insert_item(ui.dropList.model().end(), "Blue");
        ui.dropList2.model().insert_item(ui.dropList2.model().end(), "Square");
        ui.dropList2.model().insert_item(ui.dropList2.model().end(), "Triangle");
        ui.dropList2.model().insert_item(ui.dropList2.model().end(), "Circle");
        for (int32_t i = 1; i <= 100; ++i)
            ui.dropList3.model().insert_item(ui.dropList3.model().end(), "Example_" + boost::lexical_cast<std::string>(i));
        neolib::random prng;
        for (int32_t i = 1; i <= 250; ++i)
        {
            std::string randomString;
            for (uint32_t j = prng(12); j-- > 0;)
                randomString += static_cast<char>('A' + prng('z' - 'A'));
            ui.dropList4.model().insert_item(ui.dropList4.model().end(), randomString);
        }
        ui.toggleEditable.Toggled([&]()
        {
            ui.dropList.set_editable(!ui.dropList.editable());
            ui.dropList2.set_editable(!ui.dropList2.editable());
            ui.dropList3.set_editable(!ui.dropList3.editable());
            ui.dropList4.set_editable(!ui.dropList4.editable());
        });
        ui.buttonGenerateUuid.clicked([&]() { ui.textEdit.set_text(to_string(neolib::generate_uuid())); });
        ui.dropList.SelectionChanged([&](const ng::optional_item_model_index& aIndex) { ui.textEdit.set_text(aIndex != std::nullopt ? ui.dropList.model().cell_data(*aIndex).to_string() : std::string{}); });
        ui.dropList2.SelectionChanged([&](const ng::optional_item_model_index& aIndex) { ui.textEdit.set_text(aIndex != std::nullopt ? ui.dropList2.model().cell_data(*aIndex).to_string() : std::string{}); });
        ui.dropList3.SelectionChanged([&](const ng::optional_item_model_index& aIndex) { ui.textEdit.set_text(aIndex != std::nullopt ? ui.dropList3.model().cell_data(*aIndex).to_string() : std::string{}); });
        ui.dropList4.SelectionChanged([&](const ng::optional_item_model_index& aIndex) { ui.textEdit.set_text(aIndex != std::nullopt ? ui.dropList4.model().cell_data(*aIndex).to_string() : std::string{}); });
        ng::layout_as_same_size(ui.textField1.label(), ui.textField2.label());
        ui.textField1.input_box().TextChanged([&ui]()
        {
            ui.button1.set_text(ui.textField1.input_box().text());
        });
        ui.spinBox1.ValueChanged([&ui]() { ui.slider1.set_value(ui.spinBox1.value()); });
        bool colorCycle = false;
        ui.button6.clicked([&colorCycle]() { colorCycle = !colorCycle; });
        ui.buttonArcadeMode.clicked([&ui]() { ui.actionArcadeMode.toggle(); });
        ui.button7.clicked([&ui]() { ui.actionMute.toggle(); });
        ui.button8.clicked([&ui]() { if (ui.actionContacts.is_enabled()) ui.actionContacts.disable(); else ui.actionContacts.enable(); });
        prng.seed(3);
        auto transitionPrng = prng;
        std::vector<ng::transition_id> transitions;
        for (uint32_t i = 0; i < 10; ++i)
        {
            auto& button = ui.layout3.emplace<ng::push_button>(std::string(1, 'A' + i));
            ng::color randomColor = ng::color{ prng(255), prng(255), prng(255) };
            button.set_foreground_color(randomColor);
            button.clicked([&, randomColor]() { ui.textEdit.BackgroundColor = randomColor.same_lightness_as(app.current_style().palette().color(ng::color_role::Background)); });
            transitions.push_back(ng::service<ng::i_animator>().add_transition(button.Position, ng::easing::OutBounce, transitionPrng.get(1.0, 2.0), false));
        }
        ng::event<> startAnimation;
        startAnimation([&ui, &transitions, &transitionPrng]()
        {
            for (auto t : transitions)
                ng::service<ng::i_animator>().transition(t).reset(true, true);
            for (auto i = 0u; i < ui.layout3.count(); ++i)
            {
                auto& button = ui.layout3.get_widget_at(i);
                auto finalPosition = button.position();
                button.set_position(ng::point{ finalPosition.x, finalPosition.y - transitionPrng.get(600.0, 800.0) }.ceil());
                button.set_position(finalPosition);
            }
        });
        ui.mainWindow.Window([&startAnimation](const ng::window_event& aEvent)
        { 
            if (aEvent.type() == ng::window_event_type::Resized)
                startAnimation.async_trigger(); 
        });
        auto showHideTabs = [&ui]()
        {
            if (ui.checkTriState.is_checked())
                ui.tabPages.hide_tab(8);
            else
                ui.tabPages.show_tab(8);
            if (ui.checkTriState.is_indeterminate())
                ui.tabPages.hide_tab(9);
            else
                ui.tabPages.show_tab(9);
        };
        ui.checkTriState.checked([&ui, showHideTabs]()
        {
            static uint32_t n;
            if ((n++)%2 == 1)
                ui.checkTriState.set_indeterminate();
            showHideTabs();
        });
        ui.checkTriState.Unchecked([&ui, showHideTabs]()
        {
            showHideTabs();
        });
        ui.checkWordWrap.check();
        ui.checkWordWrap.checked([&]()
        {
            ui.textEdit.set_word_wrap(true);
        });
        ui.checkWordWrap.Unchecked([&]()
        {
            ui.textEdit.set_word_wrap(false);
        });
        ui.checkPassword.checked([&]()
        {
            ui.textField2.hint().set_text("Enter password");
            ui.textField2.input_box().set_password(true);
        });
        ui.checkPassword.Unchecked([&]()
        {
            ui.textField2.hint().set_text("Enter text");
            ui.textField2.input_box().set_password(false);
        });
        ui.checkGroupBoxCheckable.checked([&showFps, &fullRefresh, &ui]()
        {
            showFps = true;
            ui.groupBox.set_checkable(true);
            ui.groupBox.check_box().checked([&fullRefresh]() { fullRefresh = true; });
            ui.groupBox.check_box().Unchecked([&fullRefresh]() { fullRefresh = false; });
        });
        ui.checkGroupBoxCheckable.Unchecked([&showFps, &ui]()
        {
            showFps = false;
            ui.groupBox.set_checkable(false);
        });
        ui.checkColumns.checked([&]()
        {
            ui.checkPassword.disable();
            ui.textEdit.set_columns(3);
            ui.gradientWidget.GradientChanged([&]()
            {
                auto cs = ui.textEdit.column(2);
                cs.set_style(ng::text_edit::style{ ng::optional_font{}, ng::color_or_gradient{}, ng::color_or_gradient{}, ng::text_effect{ ng::text_effect_type::Outline, ng::color::White } });
                ui.textEdit.set_column(2, cs);
            }, ui.textEdit);
        });
        ui.checkColumns.Unchecked([&]()
        {
            ui.checkPassword.enable();
            ui.textEdit.remove_columns();
            ui.gradientWidget.GradientChanged.unsubscribe(ui.textEdit);
        });
        ui.checkKerning.Toggled([&app, &ui]()
        {
            auto fi = app.current_style().font_info();
            if (ui.checkKerning.is_checked())
                fi.enable_kerning();
            else
                fi.disable_kerning();
            app.current_style().set_font_info(fi);
        });
        ui.checkSubpixel.Toggled([&app, &ui]()
        {
            if (ui.checkSubpixel.is_checked())
                ng::service<ng::i_rendering_engine>().subpixel_rendering_on();
            else
                ng::service<ng::i_rendering_engine>().subpixel_rendering_off();
        });
        ui.editNormal.checked([&]()
        {
            auto s = ui.textEdit.default_style();
            s.set_text_effect(ng::optional_text_effect{});
            ui.textEdit.set_default_style(s);
        });
        ui.editOutline.checked([&]()
        {
            ui.effectWidthSlider.set_value(1);
            auto s = ui.textEdit.default_style();
            s.set_text_color(app.current_style().palette().color(ng::color_role::Text).light() ? ng::color::Black : ng::color::White);
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Outline, app.current_style().palette().color(ng::color_role::Text) });
            ui.textEdit.set_default_style(s);
            auto s2 = ui.textEditEditor.default_style();
            s2.set_text_effect(ng::text_effect{ ng::text_effect_type::Outline, ng::color::White });
            ui.textEditEditor.set_default_style(s2);
        });
        ui.editGlow.checked([&]()
        {
            ui.effectWidthSlider.set_value(5);
            auto s = ui.textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, ng::color::Orange.with_lightness(0.9) });
            ui.textEdit.set_default_style(s);
        });
        ui.effectWidthSlider.ValueChanged([&]()
        {
            auto s = ui.textEdit.default_style();
            auto& textEffect = s.text_effect();
            if (textEffect == std::nullopt)
                return;
            s.set_text_effect(ng::text_effect{ ui.editGlow.is_checked() ? ng::text_effect_type::Glow : ng::text_effect_type::Outline, textEffect->color(), ui.effectWidthSlider.value(), textEffect->aux1() });
            ui.textEdit.set_default_style(s);
            std::ostringstream oss;
            oss << ui.effectWidthSlider.value() << std::endl << ui.effectAux1Slider.value() << std::endl;
            auto column = ui.textEdit.column(0);
            column.set_margins(ui.effectWidthSlider.value());
            ui.textEdit.set_column(0, column);
            ui.textEditSmall.set_text(oss.str());
        });
        ui.effectAux1Slider.ValueChanged([&]()
        {
            ui.editGlow.check();
            auto s = ui.textEdit.default_style();
            auto& textEffect = s.text_effect();
            if (textEffect == std::nullopt)
                return;
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, textEffect->color(), textEffect->width(), ui.effectAux1Slider.value() });
            ui.textEdit.set_default_style(s);
            std::ostringstream oss;
            oss << ui.effectWidthSlider.value() << std::endl << ui.effectAux1Slider.value() << std::endl;
            ui.textEditSmall.set_text(oss.str());
        });
        ui.editShadow.checked([&]()
        {
            auto s = ui.textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Shadow, ng::color::Black });
            ui.textEdit.set_default_style(s);
        });
        ui.radioSliderFont.checked([&ui, &app]()
        {
            app.current_style().set_font_info(app.current_style().font_info().with_size(ui.slider1.normalized_value() * 18.0 + 4));
        });
        auto update_theme_color = [&ui]()
        {
            auto themeColor = ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Theme).to_hsv();
            themeColor.set_hue(ui.slider1.normalized_value() * 360.0);
            ng::service<ng::i_app>().current_style().palette().set_color(ng::color_role::Theme, themeColor.to_rgb());
        };
        ui.slider1.ValueChanged([update_theme_color, &ui, &app]()
        {
            ui.spinBox1.set_value(ui.slider1.value());
            if (ui.radioSliderFont.is_checked())
                app.current_style().set_font_info(app.current_style().font_info().with_size(ui.slider1.normalized_value() * 18.0 + 4));
            else if (ui.radioThemeColor.is_checked())
                update_theme_color();
        });
        ui.slider1.set_normalized_value((app.current_style().font_info().size() - 4) / 18.0);
        ui.radioThemeColor.checked([update_theme_color, &ui, &app]()
        {
            ui.slider1.set_normalized_value(ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Theme).to_hsv().hue() / 360.0);
            update_theme_color();
        });

        ui.themeColor.clicked([&window]()
        {
            static std::optional<ng::color_dialog::custom_color_list> sCustomColors;
            if (sCustomColors == std::nullopt)
            {
                sCustomColors = ng::color_dialog::custom_color_list{};
                std::fill(sCustomColors->begin(), sCustomColors->end(), ng::color::White);
            }
            auto oldColor = ng::service<ng::i_app>().change_style("Keypad").palette().color(ng::color_role::Theme);
            ng::color_dialog colorPicker(window, ng::service<ng::i_app>().change_style("Keypad").palette().color(ng::color_role::Theme));
            colorPicker.set_custom_colors(*sCustomColors);
            colorPicker.SelectionChanged([&]()
            {
                ng::service<ng::i_app>().change_style("Keypad").palette().set_color(ng::color_role::Theme, colorPicker.selected_color());
            });
            if (colorPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().change_style("Keypad").palette().set_color(ng::color_role::Theme, colorPicker.selected_color());
            else
                ng::service<ng::i_app>().change_style("Keypad").palette().set_color(ng::color_role::Theme, oldColor);
            *sCustomColors = colorPicker.custom_colors();
        });

        ui.themeFont.clicked([&window]()
        {
            ng::font_dialog fontPicker(window, ng::service<ng::i_app>().current_style().font_info());
            if (fontPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().current_style().set_font_info(fontPicker.selected_font());
        });

        ui.editColor.clicked([&]()
        {
            static std::optional<ng::color_dialog::custom_color_list> sCustomColors;
            static ng::color sInk = ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Text);
            ng::color_dialog colorPicker(window, sInk);
            if (sCustomColors != std::nullopt)
                colorPicker.set_custom_colors(*sCustomColors);
            if (colorPicker.exec() == ng::dialog_result::Accepted)
            {
                sInk = colorPicker.selected_color();
                ui.textEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::color::White, ng::gradient_direction::Horizontal }, ng::color_or_gradient{} }, true);
                ui.textField1.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::color::White, ng::gradient_direction::Horizontal }, ng::color_or_gradient{} }, true);
                ui.textField2.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::color::White, ng::gradient_direction::Horizontal }, ng::color_or_gradient{} }, true);
            }
            sCustomColors = colorPicker.custom_colors();
        });

        ng::vertical_spacer spacer1{ ui.layout4 };
        ui.button9.clicked([&app]()
        {
            if (app.current_style().name() == "Light")
                app.change_style("Dark");
            else
                app.change_style("Light");
        });
        for (uint32_t row = 0; row < 3; ++row)
            for (uint32_t col = 0; col < 3; ++col)
                ui.keypad.add_item_at_position(row, col, std::make_shared<keypad_button>(ui.textEdit, row * 3 + col + 1));
        ui.keypad.add_item_at_position(3, 1, std::make_shared<keypad_button>(ui.textEdit, 0));
        ui.keypad.add_span(3, 1, 1, 2);

        neolib::callback_timer animation(app, [&](neolib::callback_timer& aTimer)
        {
            if (ui.button6.is_singular())
                return;
            aTimer.again();
            if (colorCycle)
            {
                const double PI = 2.0 * std::acos(0.0);
                double brightness = ::sin((app.program_elapsed_ms() / 16 % 360) * (PI / 180.0)) / 2.0 + 0.5;
                neolib::random prng{ app.program_elapsed_ms() / 5000 };
                ng::color randomColor = ng::color{ prng(255), prng(255), prng(255) };
                randomColor = randomColor.to_hsv().with_brightness(brightness).to_rgb();
                ui.button6.set_foreground_color(randomColor);
            }
        }, 16);

        app.action_file_new().triggered([&]()
        {
        });

        uint32_t standardButtons = 0;
        uint32_t standardButton = 1;
        while (standardButton != 0)
        {
            auto bd = ng::dialog_button_box::standard_button_details(static_cast<ng::standard_button>(standardButton));
            if (bd.first != ng::button_role::Invalid)
            {
                auto& button = ui.groupMessageBoxButtons.item_layout().emplace<ng::check_box>(bd.second);
                button.checked([&standardButtons, standardButton]() { standardButtons |= standardButton; });
                button.Unchecked([&standardButtons, standardButton]() { standardButtons &= ~standardButton; });
            }
            standardButton <<= 1;
        }
        ui.buttonOpenMessageBox.clicked([&]()
        {
            ng::standard_button result;
            if (ui.radioMessageBoxIconInformation.is_checked())
                result = ng::message_box::information(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (ui.radioMessageBoxIconQuestion.is_checked())
                result = ng::message_box::question(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (ui.radioMessageBoxIconWarning.is_checked())
                result = ng::message_box::warning(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (ui.radioMessageBoxIconStop.is_checked())
                result = ng::message_box::stop(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (ui.radioMessageBoxIconError.is_checked())
                result = ng::message_box::error(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (ui.radioMessageBoxIconCritical.is_checked())
                result = ng::message_box::critical(window, ui.lineEditMessageBoxTitle.text(), ui.textEditMessageBoxText.text(), ui.textEditMessageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            ui.labelMessageBoxResult.set_text("Result = " + ng::dialog_button_box::standard_button_details(result).second);
        });

        // Item Views

        ng::service<ng::i_window_manager>().save_mouse_cursor();
        ng::service<ng::i_window_manager>().set_mouse_cursor(ng::mouse_system_cursor::Wait);

        ng::table_view& tableView1 = ui.tableView1;
        ng::table_view& tableView2 = ui.tableView2;
        tableView1.set_minimum_size(ng::size(128, 128));
        tableView2.set_minimum_size(ng::size(128, 128));
        ui.button10.clicked([&tableView1, &tableView2]()
        {
            if (tableView1.column_header().visible())
                tableView1.column_header().hide();
            else
                tableView1.column_header().show();
            if (tableView2.column_header().visible())
                tableView2.column_header().hide();
            else
                tableView2.column_header().show();
        });

        my_item_model itemModel;
        #ifdef NDEBUG
        itemModel.reserve(500);
        #else
        itemModel.reserve(100);
        #endif
        ng::event_processing_context epc{ app };
        for (uint32_t row = 0; row < itemModel.capacity(); ++row)
        {
            #ifdef NDEBUG
            if (row % 100 == 0)
                app.process_events(epc);
            #else
            if (row % 10 == 0)
                app.process_events(epc);
            #endif
            neolib::random prng;
            for (uint32_t col = 0; col < 9; ++col)
            {
                if (col == 0)
                    itemModel.insert_item(ng::item_model_index(row), row + 1);
                else if (col == 1)
                    itemModel.insert_cell_data(ng::item_model_index(row, col), row + 1);
                else if (col != 7)
                {
                    std::string randomString;
                    for (uint32_t j = prng(12); j-- > 0;)
                        randomString += static_cast<char>('A' + prng('z' - 'A'));
                    itemModel.insert_cell_data(ng::item_model_index(row, col), randomString);
                }
            }
        } 

        itemModel.set_column_min_value(0, 0u);
        itemModel.set_column_max_value(0, 9999u);
        itemModel.set_column_min_value(1, 0u);
        itemModel.set_column_max_value(1, 9999u);
        itemModel.set_column_step_value(1, 1u);
        tableView1.set_model(itemModel);
        my_item_presentation_model ipm1{ itemModel };
        tableView1.set_presentation_model(ipm1);
        ipm1.set_column_editable_when_focused(4);
        ipm1.set_column_editable_when_focused(5);
        ipm1.set_column_editable_when_focused(6);
        ipm1.set_column_editable_when_focused(7);
        ipm1.set_column_editable_when_focused(8);
        tableView2.set_model(itemModel);
        my_item_presentation_model ipm2{ itemModel };
        ipm2.set_column_editable_when_focused(0);
        ipm2.set_column_editable_when_focused(1);
        ipm2.set_column_editable_when_focused(2);
        ipm2.set_column_editable_when_focused(3);
        tableView2.set_presentation_model(ipm2);
        tableView2.column_header().set_expand_last_column(true);
        tableView1.Keyboard([&tableView1](const ng::keyboard_event& ke)
        {
            if (ke.type() == ng::keyboard_event_type::KeyPressed && ke.scan_code() == ng::ScanCode_DELETE && tableView1.model().rows() > 0 && tableView1.selection_model().has_current_index())
                tableView1.model().erase(tableView1.model().begin() + tableView1.presentation_model().to_item_model_index(tableView1.selection_model().current_index()).row());
        });

        ipm1.ItemChecked([&](const ng::item_presentation_model_index& aIndex) { ipm2.check(ipm2.from_item_model_index(ipm1.to_item_model_index(aIndex))); });
        ipm1.ItemUnchecked([&](const ng::item_presentation_model_index& aIndex) { ipm2.uncheck(ipm2.from_item_model_index(ipm1.to_item_model_index(aIndex))); });

        auto update_column5_heading = [&](bool aReadOnly, bool aUnselectable)
        {
            std::string heading;
            if (aReadOnly)
                heading = "Read only";
            if (aUnselectable)
            {
                if (!heading.empty())
                    heading += "/\n";
                heading += "Unselectable";
            }
            if (heading.empty())
                heading = "Five";
            else
                heading = "*****" + heading + "*****";
            itemModel.set_column_name(5, heading);
        };

        ng::item_tree_model treeModel;
        auto entities = treeModel.insert_item(treeModel.send(), "Entity");
        auto components = treeModel.insert_item(treeModel.send(), "Component");
        auto systems = treeModel.insert_item(treeModel.send(), "System");
        auto animals = treeModel.append_item(entities, "Animals");
        treeModel.append_item(animals, "Kitten");
        treeModel.append_item(animals, "Hedgehog");
        auto dolphins = treeModel.append_item(animals, "Dolphins");
        treeModel.append_item(dolphins, "T. truncatus");
        treeModel.append_item(dolphins, "O. orca");
        auto people = treeModel.append_item(entities, "People");
        auto athletes = treeModel.append_item(people, "Athletes (London 2012 Gold Medalists, Running)");
        treeModel.append_item(athletes, "Usain Bolt");
        treeModel.append_item(athletes, "Usain Bolt");
        treeModel.append_item(athletes, "Kirani James");
        treeModel.append_item(athletes, "David Rudisha");
        treeModel.append_item(athletes, "Taoufik Makhloufi");
        treeModel.append_item(athletes, "Mo Farah");
        treeModel.append_item(athletes, "Mo Farah");
        treeModel.append_item(athletes, "Shelly-Ann Fraser-Pryce");
        treeModel.append_item(athletes, "Allyson Felix");
        treeModel.append_item(athletes, "Sanya Richards-Ross");
        treeModel.append_item(athletes, "Caster Semenya");
        treeModel.append_item(athletes, "Maryam Yusuf Jamal");
        treeModel.append_item(athletes, "Meseret Defar Tola");
        treeModel.append_item(athletes, "Tirunesh Dibaba Kenene");

        my_item_tree_presentation_model itpm1{ treeModel };
        my_item_tree_presentation_model itpm2{ treeModel, true };

        ui.treeView1.set_model(treeModel);
        ui.treeView1.set_presentation_model(itpm1);
        ui.treeView2.set_model(treeModel);
        ui.treeView2.set_presentation_model(itpm2);

        ui.checkTableViewsCheckable.checked([&]() { ipm1.set_column_checkable(0, true); ipm2.set_column_checkable(0, true); ipm1.set_column_checkable(1, true); ipm2.set_column_checkable(1, true); itpm1.set_column_checkable(0, true); itpm2.set_column_checkable(0, true); });
        ui.checkTableViewsCheckable.unchecked([&]() { ipm1.set_column_checkable(0, false); ipm2.set_column_checkable(0, false); ipm1.set_column_checkable(1, false); ipm2.set_column_checkable(1, false); itpm1.set_column_checkable(0, false); itpm2.set_column_checkable(0, false); });
        ui.checkTableViewsCheckableTriState.checked([&]() { ipm1.set_column_tri_state_checkable(0, true); ipm1.set_column_tri_state_checkable(1, true); ipm2.set_column_tri_state_checkable(0, true); ipm2.set_column_tri_state_checkable(1, true); itpm1.set_column_tri_state_checkable(0, true); itpm2.set_column_tri_state_checkable(0, true); });
        ui.checkTableViewsCheckableTriState.unchecked([&]() { ipm1.set_column_tri_state_checkable(0, false); ipm1.set_column_tri_state_checkable(1, false); ipm2.set_column_tri_state_checkable(0, false); ipm2.set_column_tri_state_checkable(1, false); itpm1.set_column_tri_state_checkable(0, false); itpm2.set_column_tri_state_checkable(0, false); });
        ui.checkColumn5ReadOnly.checked([&]() { ipm1.set_column_read_only(5, true); ipm2.set_column_read_only(5, true); update_column5_heading(true, ui.checkColumn5Unselectable.is_checked()); });
        ui.checkColumn5ReadOnly.unchecked([&]() { ipm1.set_column_read_only(5, false); ipm2.set_column_read_only(5, false); update_column5_heading(false, ui.checkColumn5Unselectable.is_checked()); });
        ui.checkColumn5Unselectable.checked([&]() { ipm1.set_column_selectable(5, false); ipm2.set_column_selectable(5, false); update_column5_heading(ui.checkColumn5ReadOnly.is_checked(), true); });
        ui.checkColumn5Unselectable.unchecked([&]() { ipm1.set_column_selectable(5, true); ipm2.set_column_selectable(5, true); update_column5_heading(ui.checkColumn5ReadOnly.is_checked(), false); });

        ui.checkUpperTableViewImages.checked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm1.set_column_image_size(c, ng::size{ 16_dip }); itpm1.set_column_image_size(0, ng::size{ 16_dip }); });
        ui.checkUpperTableViewImages.unchecked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm1.set_column_image_size(c, ng::optional_size{}); itpm1.set_column_image_size(0, ng::optional_size{}); });
        ui.radioUpperTableViewMonochrome.checked([&] { ipm1.set_color_role({}); ui.tableView1.update(); itpm1.set_color_role({}); ui.treeView1.update(); });
        ui.radioUpperTableViewColoredText.checked([&] { ipm1.set_color_role(ng::color_role::Text); ui.tableView1.update(); itpm1.set_color_role(ng::color_role::Text); ui.treeView1.update(); });
        ui.radioUpperTableViewColoredCells.checked([&] { ipm1.set_color_role(ng::color_role::Background); ui.tableView1.update(); itpm1.set_color_role(ng::color_role::Background); ui.treeView1.update(); });
        ui.checkLowerTableViewImages.checked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm2.set_column_image_size(c, ng::size{ 16_dip }); itpm2.set_column_image_size(0, ng::size{ 16_dip }); });
        ui.checkLowerTableViewImages.unchecked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm2.set_column_image_size(c, ng::optional_size{}); itpm2.set_column_image_size(0, ng::optional_size{}); });
        ui.radioLowerTableViewMonochrome.checked([&] { ipm2.set_color_role({}); ui.tableView2.update(); itpm2.set_color_role({}); ui.treeView2.update(); });
        ui.radioLowerTableViewColoredText.checked([&] { ipm2.set_color_role(ng::color_role::Text); ui.tableView2.update(); itpm2.set_color_role(ng::color_role::Text); ui.treeView2.update(); });
        ui.radioLowerTableViewColoredCells.checked([&] { ipm2.set_color_role(ng::color_role::Background); ui.tableView2.update(); itpm2.set_color_role(ng::color_role::Background); ui.treeView2.update(); });

        ng::basic_item_model<ng::easing> easingItemModelUpperTableView;
        ui.dropListEasingUpperTableView.SelectionChanged([&](const ng::optional_item_model_index& aIndex) 
        { 
            tableView1.set_default_transition(easingItemModelUpperTableView.item(*aIndex), 0.75); 
        });
        for (auto i = 0; i < ng::standard_easings().size(); ++i)
            easingItemModelUpperTableView.insert_item(easingItemModelUpperTableView.end(), ng::standard_easings()[i], ng::to_string(ng::standard_easings()[i]));
        easing_item_presentation_model easingPresentationModelUpperTableView{ ui.dropListEasingUpperTableView, easingItemModelUpperTableView, false };
        ui.dropListEasingUpperTableView.set_size_policy(ng::size_constraint::Minimum);
        ui.dropListEasingUpperTableView.set_model(easingItemModelUpperTableView);
        ui.dropListEasingUpperTableView.set_presentation_model(easingPresentationModelUpperTableView);
        ui.dropListEasingUpperTableView.selection_model().set_current_index(ng::item_presentation_model_index{ ng::standard_easing_index(ng::easing::One) });
        ui.dropListEasingUpperTableView.accept_selection();

        ng::basic_item_model<ng::easing> easingItemModelLowerTableView;
        ui.dropListEasingLowerTableView.SelectionChanged([&](const ng::optional_item_model_index& aIndex)
        {
            tableView2.set_default_transition(easingItemModelLowerTableView.item(*aIndex), 0.75);
        });
        for (auto i = 0; i < ng::standard_easings().size(); ++i)
            easingItemModelLowerTableView.insert_item(easingItemModelLowerTableView.end(), ng::standard_easings()[i], ng::to_string(ng::standard_easings()[i]));
        easing_item_presentation_model easingPresentationModelLowerTableView{ ui.dropListEasingLowerTableView, easingItemModelLowerTableView, false };
        ui.dropListEasingLowerTableView.set_size_policy(ng::size_constraint::Minimum);
        ui.dropListEasingLowerTableView.set_model(easingItemModelLowerTableView);
        ui.dropListEasingLowerTableView.set_presentation_model(easingPresentationModelLowerTableView);
        ui.dropListEasingLowerTableView.selection_model().set_current_index(ng::item_presentation_model_index{ ng::standard_easing_index(ng::easing::One) });
        ui.dropListEasingLowerTableView.accept_selection();

        ui.radioNoTableViewSelection.checked([&]() { ui.tableView1.selection_model().set_mode(ng::item_selection_mode::NoSelection); ui.tableView2.selection_model().set_mode(ng::item_selection_mode::NoSelection); });
        ui.radioSingleTableViewSelection.checked([&]() { ui.tableView1.selection_model().set_mode(ng::item_selection_mode::SingleSelection); ui.tableView2.selection_model().set_mode(ng::item_selection_mode::SingleSelection); });
        ui.radioMultipleTableViewSelection.checked([&]() { ui.tableView1.selection_model().set_mode(ng::item_selection_mode::MultipleSelection); ui.tableView2.selection_model().set_mode(ng::item_selection_mode::MultipleSelection); });
        ui.radioExtendedTableViewSelection.checked([&]() { ui.tableView1.selection_model().set_mode(ng::item_selection_mode::ExtendedSelection); ui.tableView2.selection_model().set_mode(ng::item_selection_mode::ExtendedSelection); });

        ui.radioSingleTableViewSelection.check();

        tableView1.selection_model().current_index_changed([&](const ng::optional_item_presentation_model_index& aCurrentIndex, const ng::optional_item_presentation_model_index&)
        {
            if (aCurrentIndex)
            {
                auto const modelIndex = tableView1.presentation_model().to_item_model_index(*aCurrentIndex);
                tableView2.selection_model().set_current_index(tableView2.presentation_model().from_item_model_index(modelIndex));
            }
        });

        tableView2.selection_model().current_index_changed([&](const ng::optional_item_presentation_model_index& aCurrentIndex, const ng::optional_item_presentation_model_index&)
        {
            if (aCurrentIndex)
            {
                auto const modelIndex = tableView2.presentation_model().to_item_model_index(*aCurrentIndex);
                tableView1.selection_model().set_current_index(tableView1.presentation_model().from_item_model_index(modelIndex));
            }
        });

        ng::service<ng::i_window_manager>().restore_mouse_cursor(window);

        #ifdef NDEBUG
        for (int i = 0; i < 1000; ++i)
        #else
        for (int i = 0; i < 100; ++i)
        #endif
            ui.layoutLots.emplace<ng::push_button>(boost::lexical_cast<std::string>(i));

        ng::image hash(":/test/resources/channel_32.png");
        for (uint32_t i = 0; i < 9; ++i)
        {
            auto hashWidget = std::make_shared<ng::image_widget>(hash, ng::aspect_ratio::Keep, static_cast<ng::cardinal>(i));
            hashWidget->set_size_policy(ng::size_constraint::Expanding);
            hashWidget->set_background_color(i % 2 == 0 ? ng::color::Black : ng::color::White);
            ui.gridLayoutImages.add_item_at_position(i / 3, i % 3, hashWidget);
        }
        ng::image smallHash(":/test/resources/channel.png");

        create_game(ui.layoutGame);

        neolib::basic_random<uint8_t> rngColor;
        auto random_color = [&]()
        {
            return ng::color{ rngColor(255), rngColor(255), rngColor(255) };
        };

        ng::basic_item_model<ng::easing> easingItemModel;
        for (auto i = 0; i < ng::standard_easings().size(); ++i)
            easingItemModel.insert_item(easingItemModel.end(), ng::standard_easings()[i], ng::to_string(ng::standard_easings()[i]));
        easing_item_presentation_model easingPresentationModel{ ui.dropListEasing, easingItemModel };
        ui.dropListEasing.set_size_policy(ng::size_constraint::Minimum);
        ui.dropListEasing.set_model(easingItemModel);
        ui.dropListEasing.set_presentation_model(easingPresentationModel);
        ui.dropListEasing.selection_model().set_current_index(ng::item_presentation_model_index{ 0 });
        ui.dropListEasing.accept_selection();
        ng::texture logo{ ng::image{ ":/test/resources/neoGFX.png" } };

        std::array<ng::texture, 4> tex = 
        {
            ng::texture{ ng::size{64.0, 64.0}, 1.0, ng::texture_sampling::Multisample },
            ng::texture{ ng::size{64.0, 64.0}, 1.0, ng::texture_sampling::Multisample },
            ng::texture{ ng::size{64.0, 64.0}, 1.0, ng::texture_sampling::Multisample },
            ng::texture{ ng::size{64.0, 64.0}, 1.0, ng::texture_sampling::Multisample }
        };
        std::array<ng::color, 4> texColor =
        {
            ng::color::Red,
            ng::color::Green,
            ng::color::Blue,
            ng::color::White
        };
        ng::font renderToTextureFont{ "Exo 2", ng::font_style::Bold, 11.0 };
        auto test_pattern = [renderToTextureFont](ng::i_graphics_context& aGc, const ng::point& aOrigin, double aDpiScale, const ng::color& aColor, const std::string& aText)
        {
            aGc.draw_circle(aOrigin + ng::point{ 32.0, 32.0 }, 32.0, ng::pen{ aColor, aDpiScale * 2.0 });
            aGc.draw_rect(ng::rect{ aOrigin + ng::point{ 0.0, 0.0 }, ng::size{ 64.0, 64.0 } }, ng::pen{ aColor, 1.0 });
            aGc.draw_line(aOrigin + ng::point{ 0.0, 0.0 }, aOrigin + ng::point{ 64.0, 64.0 }, ng::pen{ aColor, aDpiScale * 4.0 });
            aGc.draw_line(aOrigin + ng::point{ 64.0, 0.0 }, aOrigin + ng::point{ 0.0, 64.0 }, ng::pen{ aColor, aDpiScale * 4.0 });
            aGc.draw_multiline_text(aOrigin + ng::point{ 4.0, 4.0 }, aText, renderToTextureFont, ng::text_appearance{ ng::color::White, ng::text_effect{ ng::text_effect_type::Outline, ng::color::Black, 2.0 } });
            aGc.draw_pixel(aOrigin + ng::point{ 2.0, 2.0 }, ng::color{ 0xFF, 0x01, 0x01, 0xFF });
            aGc.draw_pixel(aOrigin + ng::point{ 3.0, 2.0 }, ng::color{ 0x02, 0xFF, 0x02, 0xFF });
            aGc.draw_pixel(aOrigin + ng::point{ 4.0, 2.0 }, ng::color{ 0x03, 0x03, 0xFF, 0xFF });
        };

        // render to texture demo
        for (std::size_t i = 0; i < 4; ++i)
        {
            ng::graphics_context texGc{ tex[i] };
            ng::scoped_snap_to_pixel snap{ texGc };
            test_pattern(texGc, ng::point{}, 1.0, texColor[i], "Render\nTo\nTexture");
        }

        ui.pageDrawing.painting([&](ng::i_graphics_context& aGc)
        {
            ng::service<ng::i_rendering_engine>().want_game_mode();
            aGc.draw_rect(ng::rect{ ng::point{ 5, 5 }, ng::size{ 2, 2 } }, ng::color::White);
            aGc.draw_pixel(ng::point{ 7, 7 }, ng::color::Blue);
            aGc.draw_focus_rect(ng::rect{ ng::point{ 8, 8 }, ng::size{ 16, 16 } });
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 100, 100 }, ng::size{ 100, 100 } }, 10.0, ng::color::Goldenrod);
            aGc.fill_rect(ng::rect{ ng::point{ 300, 250 }, ng::size{ 200, 100 } }, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Horizontal));
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Horizontal));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, ng::pen{ ng::color::Blue4, 2.0 });
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 150, 150 }, ng::size{ 300, 300 } }, 10.0, ng::pen{ ng::color::Red4, 2.0 });
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Radial));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, ng::pen{ ng::color::Black, 1.0 });
            for (int x = 0; x < 3; ++x)
            {
                aGc.fill_rect(ng::rect{ ng::point{ 600.0 + x * 17, 600.0 }, ng::size{ 16, 16 } }, ng::color::Green);
                aGc.draw_rect(ng::rect{ ng::point{ 600.0 + x * 17, 600.0 }, ng::size{ 16, 16 } }, ng::pen{ ng::color::White, 1.0 });
            }
            aGc.fill_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::color::Chocolate);
            aGc.draw_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::pen{ ng::color::White, 3.0 });
            aGc.draw_arc(ng::point{ 500, 50 }, 50, ng::to_rad(5.0), ng::to_rad(40.0), ng::pen{ ng::color::Yellow, 3.0 });

            for (int x = 0; x < 10; ++x)
                for (int y = 0; y < 10; ++y)
                    if ((x + y % 2) % 2 == 0)
                        aGc.draw_pixel(ng::point{ 32.0 + x, 32.0 + y }, ng::color::Black);
                    else
                        aGc.set_pixel(ng::point{ 32.0 + x, 32.0 + y }, ng::color::Goldenrod);

            // easing function demo
            ng::scalar t = static_cast<ng::scalar>(app.program_elapsed_us());
            auto const d = 1000000.0;
            auto const x = ng::ease(easingItemModel.item(ui.dropListEasing.selection()), int(t / d) % 2 == 0 ? std::fmod(t, d) / d : 1.0 - std::fmod(t, d) / d) * (ui.pageDrawing.extents().cx - logo.extents().cx);
            aGc.draw_texture(ng::point{ x, (ui.pageDrawing.extents().cy - logo.extents().cy) / 2.0 }, logo);

            auto texLocation = ng::point{ (ui.pageDrawing.extents().cx - 64.0) / 2.0, (ui.pageDrawing.extents().cy - logo.extents().cy) / 4.0 }.ceil();
            aGc.draw_texture(texLocation + ng::point{ 0.0, 0.0 }, tex[0]);
            aGc.draw_texture(texLocation + ng::point{ 0.0, 65.0 }, tex[1]);
            aGc.draw_texture(texLocation + ng::point{ 65.0, 0.0 }, tex[2]);
            aGc.draw_texture(texLocation + ng::point{ 65.0, 65.0 }, tex[3]);

            texLocation.x += 140.0;
            test_pattern(aGc, texLocation + ng::point{ 0.0, 0.0 }, 1.0_dip, texColor[0], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 0.0, 65.0 }, 1.0_dip, texColor[1], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 0.0 }, 1.0_dip, texColor[2], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 65.0 }, 1.0_dip, texColor[3], "Render\nTo\nScreen");
        });

        neolib::callback_timer animator{ app, [&](neolib::callback_timer& aTimer)
        {
            aTimer.set_duration(ui.pageDrawing.can_update() ? 0 : 100);
            aTimer.again();
            ui.pageDrawing.update();
        }, 100 };

        ui.buttonStyle1.clicked([&ui]()
        {
            ui.textEditEditor.set_default_style(ng::text_edit::style{ ng::optional_font(), ng::gradient(ng::color::Red, ng::color::White, ng::gradient_direction::Horizontal), ng::color_or_gradient() });
        });
        ui.buttonStyle2.clicked([&ui]()
        {
            ui.textEditEditor.set_default_style(ng::text_edit::style{ ng::font("SnareDrum One NBP", "Regular", 60.0), ng::color::White });
        });

        ui.pageCircles.painting([&ui, &random_color](ng::i_graphics_context& aGc)
        {
            neolib::basic_random<ng::coordinate> prng;
            for (int i = 0; i < 100; ++i)
            {
                switch (static_cast<int>(prng(2)))
                {
                case 0:
                    aGc.draw_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().extents().cy - 1) }, prng(255),
                        ng::pen{ random_color(), prng(1, 3) });
                    break;
                case 1:
                    aGc.draw_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().cy - 1) }, prng(255),
                        ng::pen{ random_color(), prng(1, 3) },
                        random_color().with_alpha(random_color().red()));
                    break;
                case 2:
                    aGc.fill_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().cy - 1) }, prng(255),
                        random_color().with_alpha(random_color().red()));
                    break;
                }
            }
        });

        return app.exec();
    }
    catch (std::exception& e)
    {
        app.halt();
        std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
        std::exit(EXIT_FAILURE);
    }
    catch (...)
    {
        app.halt();
        std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
        ng::service<ng::i_surface_manager>().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
        std::exit(EXIT_FAILURE);
    }
}

