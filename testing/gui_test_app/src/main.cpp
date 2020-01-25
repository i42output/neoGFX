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
#include <neogfx/gui/dialog/colour_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>

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
    const ng::item_cell_info& cell_info(const ng::item_model_index& aIndex) const override
    {
        if (aIndex.column() == 4)
        {
            if (aIndex.row() == 4)
            {
                static const ng::item_cell_info sReadOnly = { ng::item_cell_flags::Default & ~ng::item_cell_flags::Editable };
                return sReadOnly;
            }
            if (aIndex.row() == 5)
            {
                static const ng::item_cell_info sUnselectable = { ng::item_cell_flags::Default & ~ng::item_cell_flags::Selectable };
                return sUnselectable;
            }
        }
        return ng::basic_item_model<void*, 9u>::cell_info(aIndex);
    }
};

class my_item_presentation_model : public ng::basic_item_presentation_model<my_item_model>
{
    typedef ng::basic_item_presentation_model<my_item_model> base_type;
public:
    my_item_presentation_model(my_item_model& aModel) : 
        base_type{ aModel }, 
        iCellImages{ {
            ng::image{ ":/test/resources/icon.png" }, 
            ng::image{ ":/closed/resources/caw_toolbar.naa#contacts.png" },
            ng::image{ ":/closed/resources/caw_toolbar.naa#favourite.png" },
            ng::image{ ":/closed/resources/caw_toolbar.naa#folder.png" }
        } }
    {
    }
public:
    void set_colour_type(const std::optional<ng::item_cell_colour_type>& aColourType)
    {
        iColourType = aColourType;
    }
    ng::optional_colour cell_colour(const ng::item_presentation_model_index& aIndex, ng::item_cell_colour_type aColourType) const override
    {
        neolib::basic_random<double> prng{ (to_item_model_index(aIndex).row() << 16) + to_item_model_index(aIndex).column() }; // use seed to make random colour based on row/index
        if (aColourType == iColourType)
            return ng::hsv_colour{ prng(0.0, 360.0), prng(0.0, 1.0), prng(0.75, 1.0) }.to_rgb();
        else if (aColourType == ng::item_cell_colour_type::Foreground && iColourType)
            return ng::colour::Black;
        return {};
    }
    ng::optional_texture cell_image(const ng::item_presentation_model_index& aIndex) const override
    {
        if (column_image_size(aIndex.column()))
            return iCellImages[(std::hash<uint32_t>{}(to_item_model_index(aIndex).row()) + std::hash<uint32_t>{}(to_item_model_index(aIndex).column())) % 5];
        else
            return ng::optional_texture{};
    }
private:
    std::optional<ng::item_cell_colour_type> iColourType;
    std::array<ng::optional_texture, 4> iCellImages;
};

class easing_item_presentation_model : public ng::basic_item_presentation_model<ng::basic_item_model<ng::easing>>
{
    typedef ng::basic_item_presentation_model<ng::basic_item_model<ng::easing>> base_type;
public:
    easing_item_presentation_model(ng::basic_item_model<ng::easing>& aModel) : base_type{ aModel }
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
            ng::texture newTexture{ ng::size{48.0, 48.0}, 1.0, ng::texture_sampling::Multisample };
            ng::graphics_context gc{ newTexture };
            auto const textColour = ng::service<ng::i_app>().current_style().palette().text_colour();
            gc.draw_rect(ng::rect{ ng::point{}, ng::size{48.0, 48.0} }, ng::pen{ textColour, 1.0 });
            ng::optional_point lastPos;
            ng::pen pen{ textColour, 2.0 };
            for (double x = 0.0; x <= 40.0; x += 2.0)
            {
                ng::point pos{ x + 4.0, ng::ease(easingFunction, x / 40.0) * 40.0 + 4.0 };
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
                palette().set_colour(aNumber != 9 ? ng::colour{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 } : ng::colour::LightGoldenrod);
            if (aNumber == 9)
                iTextEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient_direction::Horizontal }, ng::colour_or_gradient{} });
            else if (aNumber == 8)
                iTextEdit.set_default_style(ng::text_edit::style{ ng::font{"SnareDrum One NBP", "Regular", 60.0}, ng::colour::White });
            else if (aNumber == 0)
                iTextEdit.set_default_style(ng::text_edit::style{ ng::font{"SnareDrum Two NBP", "Regular", 60.0}, ng::colour::White });
            else
                iTextEdit.set_default_style(
                    ng::text_edit::style{
                        ng::optional_font{},
                        ng::gradient{
                            ng::colour{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 }.lighter(0x40),
                            ng::colour{ aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0 }.lighter(0xC0),
                            ng::gradient_direction::Horizontal},
                        ng::colour_or_gradient{} });
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
        app.change_style("Default").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Slate").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.register_style(ng::style("Keypad")).set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Keypad");
        app.current_style().palette().set_colour(ng::colour::Black);
        app.change_style("Slate");

        ng::window& window = ui.mainWindow;

        bool showFps = false;
        bool fullRefresh = false;
        auto fpsFont = window.font().with_size(18);
        window.PaintOverlay([&showFps, &window, fpsFont](ng::i_graphics_context& aGc)
        {
            if (showFps)
            {
                auto const numbers = (boost::format("%1$.2f/%2$.2f") % window.fps() % window.potential_fps()).str();
                aGc.draw_text(ng::point{ 100, 120 }, (boost::format(" %1% FPS/PFPS ") % numbers).str(), fpsFont, ng::text_appearance{ ng::colour::White, ng::colour::DarkBlue.darker(0x40), ng::text_effect{ ng::text_effect_type::Outline, ng::colour::Black } });
            }
        });

        window.surface().rendering_finished([&fullRefresh, &window]()
        {
            if (fullRefresh)
                window.update();
        });
        
        app.add_action("Goldenrod Style").set_shortcut("Ctrl+Alt+Shift+G").triggered([]()
        {
            ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(ng::colour::LightGoldenrod);
        });

        ui.actionContacts.triggered([]()
        {
            ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(ng::colour::White);
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

        ui.actionColourDialog.triggered([&window]()
        {
            ng::colour_dialog cd(window);
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
            s.set_glyph_colour(ui.gradientWidget.gradient());
            s2.set_glyph_colour(ui.gradientWidget.gradient());
            s2.set_background_colour(ng::colour_or_gradient{});
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
                ui.buttonChina.set_maximum_size(ng::size{ 128_spx, 64_spx });
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
        bool colourCycle = false;
        ui.button6.clicked([&colourCycle]() { colourCycle = !colourCycle; });
        ui.buttonArcadeMode.clicked([&ui]() { ui.actionArcadeMode.toggle(); });
        ui.button7.clicked([&ui]() { ui.actionMute.toggle(); });
        ui.button8.clicked([&ui]() { if (ui.actionContacts.is_enabled()) ui.actionContacts.disable(); else ui.actionContacts.enable(); });
        prng.seed(3);
        auto transitionPrng = prng;
        std::vector<ng::transition_id> transitions;
        for (uint32_t i = 0; i < 10; ++i)
        {
            auto& button = ui.layout3.emplace<ng::push_button>(std::string(1, 'A' + i));
            ng::colour randomColour = ng::colour{ prng(255), prng(255), prng(255) };
            button.set_foreground_colour(randomColour);
            button.clicked([&, randomColour]() { ui.textEdit.BackgroundColour = randomColour.same_lightness_as(app.current_style().palette().background_colour()); });
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
                cs.set_style(ng::text_edit::style{ ng::optional_font{}, ng::colour_or_gradient{}, ng::colour_or_gradient{}, ng::text_effect{ ng::text_effect_type::Outline, ng::colour::White } });
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
            s.set_text_colour(app.current_style().palette().text_colour().light() ? ng::colour::Black : ng::colour::White);
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Outline, app.current_style().palette().text_colour() });
            ui.textEdit.set_default_style(s);
            auto s2 = ui.textEditEditor.default_style();
            s2.set_text_effect(ng::text_effect{ ng::text_effect_type::Outline, ng::colour::White });
            ui.textEditEditor.set_default_style(s2);
        });
        ui.editGlow.checked([&]()
        {
            ui.effectWidthSlider.set_value(5);
            auto s = ui.textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, ng::colour::Orange.with_lightness(0.9) });
            ui.textEdit.set_default_style(s);
        });
        ui.effectWidthSlider.ValueChanged([&]()
        {
            auto s = ui.textEdit.default_style();
            auto& textEffect = s.text_effect();
            if (textEffect == std::nullopt)
                return;
            s.set_text_effect(ng::text_effect{ ui.editGlow.is_checked() ? ng::text_effect_type::Glow : ng::text_effect_type::Outline, textEffect->colour(), ui.effectWidthSlider.value(), textEffect->aux1() });
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
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, textEffect->colour(), textEffect->width(), ui.effectAux1Slider.value() });
            ui.textEdit.set_default_style(s);
            std::ostringstream oss;
            oss << ui.effectWidthSlider.value() << std::endl << ui.effectAux1Slider.value() << std::endl;
            ui.textEditSmall.set_text(oss.str());
        });
        ui.editShadow.checked([&]()
        {
            auto s = ui.textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Shadow, ng::colour::Black });
            ui.textEdit.set_default_style(s);
        });
        ui.radioSliderFont.checked([&ui, &app]()
        {
            app.current_style().set_font_info(app.current_style().font_info().with_size(ui.slider1.normalized_value() * 18.0 + 4));
        });
        auto update_theme_colour = [&ui]()
        {
            auto themeColour = ng::service<ng::i_app>().current_style().palette().colour().to_hsv();
            themeColour.set_hue(ui.slider1.normalized_value() * 360.0);
            ng::service<ng::i_app>().current_style().palette().set_colour(themeColour.to_rgb());
        };
        ui.slider1.ValueChanged([update_theme_colour, &ui, &app]()
        {
            ui.spinBox1.set_value(ui.slider1.value());
            if (ui.radioSliderFont.is_checked())
                app.current_style().set_font_info(app.current_style().font_info().with_size(ui.slider1.normalized_value() * 18.0 + 4));
            else if (ui.radioThemeColour.is_checked())
                update_theme_colour();
        });
        ui.slider1.set_normalized_value((app.current_style().font_info().size() - 4) / 18.0);
        ui.radioThemeColour.checked([update_theme_colour, &ui, &app]()
        {
            ui.slider1.set_normalized_value(ng::service<ng::i_app>().current_style().palette().colour().to_hsv().hue() / 360.0);
            update_theme_colour();
        });

        ui.themeColour.clicked([&window]()
        {
            static std::optional<ng::colour_dialog::custom_colour_list> sCustomColours;
            if (sCustomColours == std::nullopt)
            {
                sCustomColours = ng::colour_dialog::custom_colour_list{};
                std::fill(sCustomColours->begin(), sCustomColours->end(), ng::colour::White);
            }
            auto oldColour = ng::service<ng::i_app>().change_style("Keypad").palette().colour();
            ng::colour_dialog colourPicker(window, ng::service<ng::i_app>().change_style("Keypad").palette().colour());
            colourPicker.set_custom_colours(*sCustomColours);
            colourPicker.SelectionChanged([&]()
            {
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(colourPicker.selected_colour());
            });
            if (colourPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(colourPicker.selected_colour());
            else
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(oldColour);
            *sCustomColours = colourPicker.custom_colours();
        });

        ui.themeFont.clicked([&window]()
        {
            ng::font_dialog fontPicker(window, ng::service<ng::i_app>().current_style().font_info());
            if (fontPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().current_style().set_font_info(fontPicker.selected_font());
        });

        ui.editColour.clicked([&]()
        {
            static std::optional<ng::colour_dialog::custom_colour_list> sCustomColours;
            static ng::colour sInk = ng::service<ng::i_app>().current_style().palette().text_colour();
            ng::colour_dialog colourPicker(window, sInk);
            if (sCustomColours != std::nullopt)
                colourPicker.set_custom_colours(*sCustomColours);
            if (colourPicker.exec() == ng::dialog_result::Accepted)
            {
                sInk = colourPicker.selected_colour();
                ui.textEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient_direction::Horizontal }, ng::colour_or_gradient{} }, true);
                ui.textField1.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient_direction::Horizontal }, ng::colour_or_gradient{} }, true);
                ui.textField2.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient_direction::Horizontal }, ng::colour_or_gradient{} }, true);
            }
            sCustomColours = colourPicker.custom_colours();
        });

        ng::vertical_spacer spacer1{ ui.layout4 };
        ui.button9.clicked([&app]()
        {
            if (app.current_style().name() == "Default")
                app.change_style("Slate");
            else
                app.change_style("Default");
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
            if (colourCycle)
            {
                const double PI = 2.0 * std::acos(0.0);
                double brightness = ::sin((app.program_elapsed_ms() / 16 % 360) * (PI / 180.0)) / 2.0 + 0.5;
                neolib::random prng{ app.program_elapsed_ms() / 5000 };
                ng::colour randomColour = ng::colour{ prng(255), prng(255), prng(255) };
                randomColour = randomColour.to_hsv().with_brightness(brightness).to_rgb();
                ui.button6.set_foreground_colour(randomColour);
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

        auto update_column5_heading = [&]()
        {
            std::string heading;
            if (itemModel.column_read_only(5))
                heading = "Read only";
            if (!itemModel.column_selectable(5))
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

        ui.checkColumn5ReadOnly.checked([&]() { itemModel.set_column_read_only(5, true); update_column5_heading(); });
        ui.checkColumn5ReadOnly.Unchecked([&]() { itemModel.set_column_read_only(5, false); update_column5_heading(); });
        ui.checkColumn5Unselectable.checked([&]() { itemModel.set_column_selectable(5, false); update_column5_heading(); });
        ui.checkColumn5Unselectable.Unchecked([&]() { itemModel.set_column_selectable(5, true); update_column5_heading(); });

        itemModel.set_column_min_value(0, 0u);
        itemModel.set_column_max_value(0, 9999u);
        itemModel.set_column_min_value(1, 0u);
        itemModel.set_column_max_value(1, 9999u);
        itemModel.set_column_step_value(1, 1u);
        tableView1.set_model(itemModel);
        my_item_presentation_model ipm1{ itemModel };
        tableView1.set_presentation_model(ipm1);
        ipm1.set_column_editable(4, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(5, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(6, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(7, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(8, ng::item_cell_editable::WhenFocused);
        tableView2.set_model(itemModel);
        my_item_presentation_model ipm2{ itemModel };
        ipm2.set_column_editable(0, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(1, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(2, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(3, ng::item_cell_editable::WhenFocused);
        tableView2.set_presentation_model(ipm2);
        tableView2.column_header().set_expand_last_column(true);
        tableView1.Keyboard([&tableView1](const ng::keyboard_event& ke)
        {
            if (ke.type() == ng::keyboard_event_type::KeyPressed && ke.scan_code() == ng::ScanCode_DELETE && tableView1.model().rows() > 0 && tableView1.selection_model().has_current_index())
                tableView1.model().erase(tableView1.model().begin() + tableView1.presentation_model().to_item_model_index(tableView1.selection_model().current_index()).row());
        });

        ui.checkUpperTableViewImages.checked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm1.set_column_image_size(c, ng::size{ 16_spx }); });
        ui.checkUpperTableViewImages.unchecked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm1.set_column_image_size(c, ng::optional_size{}); });
        ui.radioUpperTableViewMonochrome.checked([&] { ipm1.set_colour_type({}); ui.tableView1.update(); });
        ui.radioUpperTableViewColouredText.checked([&] { ipm1.set_colour_type(ng::item_cell_colour_type::Foreground); ui.tableView1.update(); });
        ui.radioUpperTableViewColouredCells.checked([&] { ipm1.set_colour_type(ng::item_cell_colour_type::Background); ui.tableView1.update(); });
        ui.checkLowerTableViewImages.checked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm2.set_column_image_size(c, ng::size{ 16_spx }); });
        ui.checkLowerTableViewImages.unchecked([&] { for (uint32_t c = 0u; c <= 6u; c += 2u) ipm2.set_column_image_size(c, ng::optional_size{}); });
        ui.radioLowerTableViewMonochrome.checked([&] { ipm2.set_colour_type({}); ui.tableView2.update(); });
        ui.radioLowerTableViewColouredText.checked([&] { ipm2.set_colour_type(ng::item_cell_colour_type::Foreground); ui.tableView2.update(); });
        ui.radioLowerTableViewColouredCells.checked([&] { ipm2.set_colour_type(ng::item_cell_colour_type::Background); ui.tableView2.update(); });

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
            hashWidget->set_background_colour(i % 2 == 0 ? ng::colour::Black : ng::colour::White);
            ui.gridLayoutImages.add_item_at_position(i / 3, i % 3, hashWidget);
        }
        ng::image smallHash(":/test/resources/channel.png");

        create_game(ui.layoutGame);

        neolib::basic_random<uint8_t> rngColour;
        auto random_colour = [&]()
        {
            return ng::colour{ rngColour(255), rngColour(255), rngColour(255) };
        };

        ng::basic_item_model<ng::easing> easingItemModel;
        for (auto i = 0; i < ng::standard_easings().size(); ++i)
            easingItemModel.insert_item(easingItemModel.end(), ng::standard_easings()[i], ng::to_string(ng::standard_easings()[i]));
        easing_item_presentation_model easingPresentationModel{ easingItemModel };
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
        std::array<ng::colour, 4> texColour =
        {
            ng::colour::Red,
            ng::colour::Green,
            ng::colour::Blue,
            ng::colour::White
        };
        ng::font renderToTextureFont{ "Exo 2", ng::font_style::Bold, 11.0 };
        auto test_pattern = [renderToTextureFont](ng::i_graphics_context& aGc, const ng::point& aOrigin, double aDpiScale, const ng::colour& aColour, const std::string& aText)
        {
            aGc.draw_circle(aOrigin + ng::point{ 32.0, 32.0 }, 32.0, ng::pen{ aColour, aDpiScale * 2.0 });
            aGc.draw_rect(ng::rect{ aOrigin + ng::point{ 0.0, 0.0 }, ng::size{ 64.0, 64.0 } }, ng::pen{ aColour, 1.0 });
            aGc.draw_line(aOrigin + ng::point{ 0.0, 0.0 }, aOrigin + ng::point{ 64.0, 64.0 }, ng::pen{ aColour, aDpiScale * 4.0 });
            aGc.draw_line(aOrigin + ng::point{ 64.0, 0.0 }, aOrigin + ng::point{ 0.0, 64.0 }, ng::pen{ aColour, aDpiScale * 4.0 });
            aGc.draw_multiline_text(aOrigin + ng::point{ 4.0, 4.0 }, aText, renderToTextureFont, ng::text_appearance{ ng::colour::White, ng::text_effect{ ng::text_effect_type::Outline, ng::colour::Black, 2.0 } });
            aGc.draw_pixel(aOrigin + ng::point{ 2.0, 2.0 }, ng::colour{ 0xFF, 0x01, 0x01, 0xFF });
            aGc.draw_pixel(aOrigin + ng::point{ 3.0, 2.0 }, ng::colour{ 0x02, 0xFF, 0x02, 0xFF });
            aGc.draw_pixel(aOrigin + ng::point{ 4.0, 2.0 }, ng::colour{ 0x03, 0x03, 0xFF, 0xFF });
        };

        // render to texture demo
        for (std::size_t i = 0; i < 4; ++i)
        {
            ng::graphics_context texGc{ tex[i] };
            test_pattern(texGc, ng::point{}, 1.0, texColour[i], "Render\nTo\nTexture");
        }

        ui.pageDrawing.painting([&](ng::i_graphics_context& aGc)
        {
            ng::service<ng::i_rendering_engine>().want_game_mode();
            aGc.draw_rect(ng::rect{ ng::point{ 5, 5 }, ng::size{ 2, 2 } }, ng::colour::White);
            aGc.draw_pixel(ng::point{ 7, 7 }, ng::colour::Blue);
            aGc.draw_focus_rect(ng::rect{ ng::point{ 8, 8 }, ng::size{ 16, 16 } });
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 100, 100 }, ng::size{ 100, 100 } }, 10.0, ng::colour::Goldenrod);
            aGc.fill_rect(ng::rect{ ng::point{ 300, 250 }, ng::size{ 200, 100 } }, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Horizontal));
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Horizontal));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, ng::pen{ ng::colour::Blue4, 2.0 });
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 150, 150 }, ng::size{ 300, 300 } }, 10.0, ng::pen{ ng::colour::Red4, 2.0 });
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, ui.gradientWidget.gradient().with_direction(ng::gradient_direction::Radial));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, ng::pen{ ng::colour::Black, 1.0 });
            for (int x = 0; x < 3; ++x)
            {
                aGc.fill_rect(ng::rect{ ng::point{ 600.0 + x * 17, 600.0 }, ng::size{ 16, 16 } }, ng::colour::Green);
                aGc.draw_rect(ng::rect{ ng::point{ 600.0 + x * 17, 600.0 }, ng::size{ 16, 16 } }, ng::pen{ ng::colour::White, 1.0 });
            }
            aGc.fill_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::colour::Chocolate);
            aGc.draw_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::pen{ ng::colour::White, 3.0 });
            aGc.draw_arc(ng::point{ 500, 50 }, 50, ng::to_rad(5.0), ng::to_rad(40.0), ng::pen{ ng::colour::Yellow, 3.0 });

            for (int x = 0; x < 10; ++x)
                for (int y = 0; y < 10; ++y)
                    if ((x + y % 2) % 2 == 0)
                        aGc.draw_pixel(ng::point{ 32.0 + x, 32.0 + y }, ng::colour::Black);
                    else
                        aGc.set_pixel(ng::point{ 32.0 + x, 32.0 + y }, ng::colour::Goldenrod);

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
            test_pattern(aGc, texLocation + ng::point{ 0.0, 0.0 }, 1.0_spx, texColour[0], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 0.0, 65.0 }, 1.0_spx, texColour[1], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 0.0 }, 1.0_spx, texColour[2], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 65.0 }, 1.0_spx, texColour[3], "Render\nTo\nScreen");
        });

        neolib::callback_timer animator{ app, [&](neolib::callback_timer& aTimer)
        {
            aTimer.set_duration(ui.pageDrawing.can_update() ? 0 : 100);
            aTimer.again();
            ui.pageDrawing.update();
        }, 100 };

        ui.buttonStyle1.clicked([&ui]()
        {
            ui.textEditEditor.set_default_style(ng::text_edit::style{ ng::optional_font(), ng::gradient(ng::colour::Red, ng::colour::White, ng::gradient_direction::Horizontal), ng::colour_or_gradient() });
        });
        ui.buttonStyle2.clicked([&ui]()
        {
            ui.textEditEditor.set_default_style(ng::text_edit::style{ ng::font("SnareDrum One NBP", "Regular", 60.0), ng::colour::White });
        });

        ui.pageCircles.painting([&ui, &random_colour](ng::i_graphics_context& aGc)
        {
            neolib::basic_random<ng::coordinate> prng;
            for (int i = 0; i < 100; ++i)
            {
                switch (static_cast<int>(prng(2)))
                {
                case 0:
                    aGc.draw_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().extents().cy - 1) }, prng(255),
                        ng::pen{ random_colour(), prng(1, 3) });
                    break;
                case 1:
                    aGc.draw_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().cy - 1) }, prng(255),
                        ng::pen{ random_colour(), prng(1, 3) },
                        random_colour().with_alpha(random_colour().red()));
                    break;
                case 2:
                    aGc.fill_circle(
                        ng::point{ prng(ui.pageCircles.client_rect().cx - 1), prng(ui.pageCircles.client_rect().cy - 1) }, prng(255),
                        random_colour().with_alpha(random_colour().red()));
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

