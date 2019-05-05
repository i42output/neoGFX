#include <neolib/neolib.hpp>
#include <csignal>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <neolib/random.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gui/dialog/colour_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>
#include <neogfx/gui/widgets.hpp>
#include <neogfx/core/easing.hpp>
#include <neogfx/core/i_animator.hpp>

namespace ng = neogfx;

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
    const ng::item_cell_data_info& cell_data_info(const ng::item_model_index& aIndex) const override
    {
        if (aIndex.column() == 4)
        {
            if (aIndex.row() == 4)
            {
                static const ng::item_cell_data_info sReadOnly = { false, true };
                return sReadOnly;
            }
            if (aIndex.row() == 5)
            {
                static const ng::item_cell_data_info sUnselectable = { true, false };
                return sUnselectable;
            }
        }
        return ng::basic_item_model<void*, 9u>::cell_data_info(aIndex);
    }
};

class my_item_presentation_model : public ng::basic_item_presentation_model<my_item_model>
{
private:
    typedef ng::basic_item_presentation_model<my_item_model> base_type;
public:
    my_item_presentation_model(my_item_model& aModel, ng::item_cell_colour_type aColourType) : base_type{ aModel }, iColourType{ aColourType }
    {
    }
public:
    ng::optional_colour cell_colour(const ng::item_presentation_model_index& aIndex, ng::item_cell_colour_type aColourType) const override
    {
        neolib::basic_random<double> prng{ (to_item_model_index(aIndex).row() << 16) + to_item_model_index(aIndex).column() }; // use seed to make random colour based on row/index
        if (aColourType == iColourType)
            return ng::hsv_colour{prng(0.0, 360.0), prng(0.0, 1.0), prng(0.75, 1.0) }.to_rgb();
        else
            return iColourType == ng::item_cell_colour_type::Foreground ? ng::optional_colour{} : ng::colour::Black;
    }
private:
    ng::item_cell_colour_type iColourType;
};

class easing_item_presentation_model : public ng::basic_item_presentation_model<ng::basic_item_model<ng::easing>>
{
private:
    typedef ng::basic_item_presentation_model<ng::basic_item_model<ng::easing>> base_type;
public:
    easing_item_presentation_model(ng::basic_item_model<ng::easing>& aModel) : base_type{ aModel }
    {
        iSink += ng::service<ng::i_app>().current_style_changed([this](ng::style_aspect)
        {
            iTextures.clear();
            visual_appearance_changed.async_trigger();
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
                iTextEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal }, ng::colour_or_gradient{} });
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
                            ng::gradient::Horizontal},
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
    Will probably be broken up at a later date when a proper test suite is created. */

    ng::app app(argc, argv, "neoGFX Test App");

    try
    {
        app.set_default_window_icon(ng::image(":/test/resources/icon.png"));
        app.change_style("Default").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Slate").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.register_style(ng::style("Keypad")).set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
        app.change_style("Keypad");
        app.current_style().palette().set_colour(ng::colour::Black);
        app.change_style("Slate");

        std::optional<ng::window> windowObject;
        auto fullscreenResolution = app.program_options().full_screen();
        if (fullscreenResolution != std::nullopt)
        {
            if (fullscreenResolution->first == 0)
                windowObject.emplace(
                    ng::video_mode{ 
                        static_cast<uint32_t>(ng::service<ng::i_basic_services>().display().desktop_rect().width()), 
                        static_cast<uint32_t>(ng::service<ng::i_basic_services>().display().desktop_rect().height()) });
            else
                windowObject.emplace(ng::video_mode{ fullscreenResolution->first, fullscreenResolution->second });
        }
        else
            windowObject.emplace(ng::size{ 800, 800 } * ng::service<ng::i_surface_manager>().display().metrics().horizontal_dpi() / 96.0);
        ng::window& window = *windowObject;

        ng::status_bar statusBar{ window.status_bar_layout() };
        auto& layout0 = window.client_layout();

        bool showFps = false;
        bool fullRefresh = false;
        auto fpsFont = window.font().with_size(18);
        window.paint_overlay([&showFps, &window, fpsFont](ng::graphics_context& aGc)
        {
            if (showFps)
                aGc.draw_text(ng::point{ 100, 120 }, (boost::format(" %1$6.2f FPS ") % window.fps()).str(), fpsFont, ng::text_appearance{ ng::colour::White, ng::colour::DarkBlue.darker(0x40), ng::text_effect{ ng::text_effect_type::Outline, ng::colour::Black } });
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

        auto& contactsAction = app.add_action("&Contacts...", ":/closed/resources/caw_toolbar.naa#contacts.png").set_shortcut("Alt+C");
        contactsAction.triggered([]()
        {
            ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(ng::colour::White);
        });
        auto& muteAction = app.add_action("Mute/&Unmute Sound", ":/closed/resources/caw_toolbar.naa#mute.png");
        muteAction.set_checkable(true);
        muteAction.set_checked_image(":/closed/resources/caw_toolbar.naa#unmute.png");

        auto& pasteAndGoAction = app.add_action("Paste and Go", ":/closed/resources/caw_toolbar.naa#paste_and_go.png").set_shortcut("Ctrl+Shift+V");

        neolib::callback_timer ct{ app, [&app, &pasteAndGoAction](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (ng::service<ng::i_clipboard>().sink_active())
            {
                auto& sink = ng::service<ng::i_clipboard>().active_sink();
                if (sink.can_paste())
                {
                    pasteAndGoAction.enable();
                }
                else
                {
                    pasteAndGoAction.disable();
                }
            }
        }, 100 };

        pasteAndGoAction.triggered([&app]()
        {
            ng::service<ng::i_clipboard>().paste();
        });

        ng::menu_bar menu(window.menu_layout());

        auto& fileMenu = menu.add_sub_menu("&File");
        fileMenu.add_action(app.action_file_new());
        fileMenu.add_action(app.action_file_open());
        fileMenu.add_separator();
        fileMenu.add_action(app.action_file_close());
        fileMenu.add_separator();
        fileMenu.add_action(app.action_file_save());
        fileMenu.add_separator();
        fileMenu.add_action(app.action_file_exit());

        auto& editMenu = menu.add_sub_menu("&Edit");
        editMenu.add_action(app.action_undo());
        editMenu.add_action(app.action_redo());
        editMenu.add_separator();
        editMenu.add_action(app.action_cut());
        editMenu.add_action(app.action_copy());
        editMenu.add_action(app.action_paste());
        editMenu.add_action(pasteAndGoAction);
        editMenu.add_action(app.action_delete());
        editMenu.add_separator();
        editMenu.add_action(app.action_select_all());
        auto& viewMenu = menu.add_sub_menu("&View");
        auto& addFavouriteAction = app.add_action("Add Favourite...", ":/closed/resources/caw_toolbar.naa#add_favourite.png");
        auto& organizeFavouritesAction = app.add_action("Organize Favourites...", ":/closed/resources/caw_toolbar.naa#organize_favourites.png");

        auto& favouritesMenu = menu.add_sub_menu("F&avourites");
        favouritesMenu.add_action(addFavouriteAction);
        favouritesMenu.add_action(organizeFavouritesAction);
        favouritesMenu.add_separator();
        favouritesMenu.add_action(app.add_action("Alice", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        favouritesMenu.add_action(app.add_action("Bob", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        favouritesMenu.add_action(app.add_action("Carlos", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        favouritesMenu.add_action(app.add_action("Dave", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        auto& menuDrones = favouritesMenu.add_sub_menu("Silent Running Drones");
        menuDrones.set_image(":/closed/resources/caw_toolbar.naa#folder.png");
        menuDrones.add_action(app.add_action("Dewey", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        menuDrones.add_action(app.add_action("Huey", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        menuDrones.add_action(app.add_action("Louie", ":/closed/resources/caw_toolbar.naa#favourite.png"));
        auto& subMenu2 = favouritesMenu.add_sub_menu("DC Characters");
        subMenu2.set_image(":/closed/resources/caw_toolbar.naa#folder.png");
        auto& subMenu3 = favouritesMenu.add_sub_menu("Marvel Characters");
        subMenu3.set_image(":/closed/resources/caw_toolbar.naa#folder.png");
        favouritesMenu.add_separator();
        neolib::random menuPrng{ 0 };
        for (int i = 1; i <= 5; ++i)
        {
            auto& sm = favouritesMenu.add_sub_menu("More_" + boost::lexical_cast<std::string>(i));
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
        menu.add_action(contactsAction);
        menu.add_action(muteAction);
        menu.add_action(app.add_action("&Xyzzy...", ":/neogfx/resources/icons.naa#eyedropper.png"));

        auto& testMenu = menu.add_sub_menu("&Test");
        testMenu.add_action(app.add_action("Emacs style sequence #1")).set_shortcut("Ctrl+K, Ctrl+B").triggered([&]()
            {
                ng::message_box::information(window, "Emacs style sequence #1", "Ctrl+K, Ctrl+B");
            });
        testMenu.add_action(app.add_action("Emacs style sequence #2")).set_shortcut("Ctrl+K, Ctrl+K").triggered([&]()
            {
                ng::message_box::information(window, "Emacs style sequence #2", "Ctrl+K, Ctrl+K");
            });
        testMenu.add_action(contactsAction);
        testMenu.add_action(muteAction);
        testMenu.add_action(muteAction);
        testMenu.add_action(muteAction);
        testMenu.add_action(muteAction);
        ng::i_action& colourAction = app.add_action("Colour Dialog...");
        colourAction.triggered([&window]()
        {
            ng::colour_dialog cd(window);
            cd.exec();
        });
        testMenu.add_action(colourAction);

        auto& windowMenu = menu.add_sub_menu("&Window");
        auto& nextTab = windowMenu.add_action(app.add_action("Next Tab").set_shortcut("Ctrl+Tab"));
        auto& previousTab = windowMenu.add_action(app.add_action("Previous Tab").set_shortcut("Shift+Ctrl+Tab"));

        auto& helpMenu = menu.add_sub_menu("&Help");

        ng::toolbar toolbar(window.toolbar_layout());
        toolbar.add_action(app.action_file_new());
        toolbar.add_action(app.action_file_open());
        toolbar.add_action(app.action_file_save());
        toolbar.add_separator();
        toolbar.add_action(contactsAction);
        toolbar.add_action(addFavouriteAction);
        toolbar.add_action(organizeFavouritesAction);
        toolbar.add_action(app.add_action("Keywords...", ":/closed/resources/caw_toolbar.naa#keyword.png"));
        toolbar.add_action(app.add_action("Settings...", ":/closed/resources/caw_toolbar.naa#settings.png"));
        toolbar.add_action(app.add_action("Manage Plugins...", ":/closed/resources/caw_toolbar.naa#manage_plugins.png")).triggered([&]()
            {
                ng::service<ng::i_rendering_engine>().enable_frame_rate_limiter(!ng::service<ng::i_rendering_engine>().frame_rate_limited());
            });
        toolbar.add_action(muteAction);
        toolbar.add_separator();
        toolbar.add_action(app.action_cut());
        toolbar.add_action(app.action_copy());
        toolbar.add_action(app.action_paste());
        toolbar.add_action(pasteAndGoAction);
        toolbar.add_separator();
        toolbar.add_action(app.add_action("Check for Updates...", ":/closed/resources/caw_toolbar.naa#setup.png"));

        ng::tab_page_container tabContainer(layout0, true);

        nextTab.triggered([&]() { tabContainer.select_next_tab(); });
        previousTab.triggered([&]() { tabContainer.select_previous_tab(); });

        // Buttons

        ng::i_widget& buttonsPage = tabContainer.add_tab_page("Buttons").as_widget();
        ng::vertical_layout layoutButtons(buttonsPage);
        layoutButtons.set_margins(ng::margins(8));
        ng::horizontal_layout topButtons(layoutButtons);
        ng::push_button button0(topButtons, "This is the neoGFX test application.");
        button0.label().set_placement(ng::label_placement::ImageTextVertical);
        button0.image().set_image(ng::image{ ":/test/resources/neoGFX.png" });
        button0.image().set_minimum_size(window.dpi_scale(ng::size{ 32, 32 }));
        button0.image().set_maximum_size(window.dpi_scale(ng::size{ 160, std::numeric_limits<ng::dimension>::max() }));
        button0.set_size_policy(ng::size_policy::Expanding);
        button0.set_foreground_colour(ng::colour::LightGoldenrodYellow);
        ng::push_button button1(topButtons, "the,,, quick brown fox jumps over the lazy dog.\nChange tab bar placement.");
        button1.clicked([&tabContainer]()
        {
            if (tabContainer.style() == ng::tab_container_style::TabAlignmentTop)
                tabContainer.set_style(ng::tab_container_style::TabAlignmentBottom);
            else if (tabContainer.style() == ng::tab_container_style::TabAlignmentBottom)
                tabContainer.set_style(ng::tab_container_style::TabAlignmentLeft);
            else if (tabContainer.style() == ng::tab_container_style::TabAlignmentLeft)
                tabContainer.set_style(ng::tab_container_style::TabAlignmentRight);
            else if (tabContainer.style() == ng::tab_container_style::TabAlignmentRight)
                tabContainer.set_style(ng::tab_container_style::TabAlignmentTop);
        });
        ng::push_button buttonGenerateUuid(topButtons, "&Generate UUID");
        ng::horizontal_layout international(layoutButtons);
        button1.set_foreground_colour(ng::colour::LightGoldenrod);
        ng::push_button button2(international, u8"ويقفز الثعلب البني السريع فوق الكلب الكسول");
        button2.set_foreground_colour(ng::colour::Goldenrod);
        ng::push_button button3(international, u8"שועל חום קפיצות מעל הכלב העצלן");
        button3.set_foreground_colour(ng::colour::DarkGoldenrod);
        ng::push_button buttonJapan(international, u8"クジラを食べないでください。");
        ng::push_button button4(international, u8"请停止食用犬");
        button4.clicked([&window, &button4]() { window.set_title_text(u8"请停止食用犬"); if (button4.has_maximum_size()) button4.set_maximum_size(ng::optional_size{}); else button4.set_maximum_size(window.dpi_scale(ng::size{ 128, 64 })); });
        button4.set_foreground_colour(ng::colour::CadetBlue);
        ng::push_button button5(layoutButtons, u8"sample text نص عينة sample text טקסט לדוגמא 示例文本 sample text\nKerning test: Tr. WAVAVAW. zzz zoz ozo ooo");
        ng::horizontal_layout dropListLayout(layoutButtons);
        ng::drop_list dropList(dropListLayout);
        dropList.model().insert_item(dropList.model().end(), "Red");
        dropList.model().insert_item(dropList.model().end(), "Green");
        dropList.model().insert_item(dropList.model().end(), "Blue");
        ng::drop_list dropList2(dropListLayout);
        dropList2.model().insert_item(dropList2.model().end(), "Square");
        dropList2.model().insert_item(dropList2.model().end(), "Triangle");
        dropList2.model().insert_item(dropList2.model().end(), "Circle");
        ng::drop_list dropList3(dropListLayout);
        for (int32_t i = 1; i <= 100; ++i)
            dropList3.model().insert_item(dropList3.model().end(), "Example_" + boost::lexical_cast<std::string>(i));
        ng::drop_list dropList4(dropListLayout);
        neolib::random prng;
        for (int32_t i = 1; i <= 250; ++i)
        {
            std::string randomString;
            for (uint32_t j = prng(12); j-- > 0;)
                randomString += static_cast<char>('A' + prng('z' - 'A'));
            dropList4.model().insert_item(dropList4.model().end(), randomString);
        }
        ng::check_box toggleEditable{ dropListLayout, "Toggle Editable" };
        toggleEditable.toggled([&]()
        {
            dropList.set_editable(!dropList.editable());
            dropList2.set_editable(!dropList2.editable());
            dropList3.set_editable(!dropList3.editable());
            dropList4.set_editable(!dropList4.editable());
        });
        ng::horizontal_layout editLayout(layoutButtons);
        ng::text_edit textEdit(editLayout);
        buttonGenerateUuid.clicked([&]() { textEdit.set_text(neolib::to_string(neolib::generate_uuid())); });
        dropList.selection_changed([&](ng::optional_item_model_index& aIndex) { textEdit.set_text(aIndex != std::nullopt ? dropList.model().cell_data(*aIndex).to_string() : std::string{}); });
        dropList2.selection_changed([&](ng::optional_item_model_index& aIndex) { textEdit.set_text(aIndex != std::nullopt ? dropList2.model().cell_data(*aIndex).to_string() : std::string{}); });
        dropList3.selection_changed([&](ng::optional_item_model_index& aIndex) { textEdit.set_text(aIndex != std::nullopt ? dropList3.model().cell_data(*aIndex).to_string() : std::string{}); });
        dropList4.selection_changed([&](ng::optional_item_model_index& aIndex) { textEdit.set_text(aIndex != std::nullopt ? dropList4.model().cell_data(*aIndex).to_string() : std::string{}); });
        textEdit.set_focus_policy(textEdit.focus_policy() | neogfx::focus_policy::ConsumeTabKey);
        textEdit.set_tab_stop_hint("00000000");
        ng::slider effectWidthSlider{ editLayout, ng::slider::Vertical };
        effectWidthSlider.set_minimum(1);
        effectWidthSlider.set_maximum(10);
        effectWidthSlider.set_step(1);
        effectWidthSlider.set_value(5);
        ng::double_slider effectAux1Slider{ editLayout, ng::slider::Vertical };
        effectAux1Slider.set_minimum(0.1);
        effectAux1Slider.set_maximum(10.0);
        effectAux1Slider.set_step(0.1);
        effectAux1Slider.set_value(1.0);
        ng::text_edit smallTextEdit(editLayout);
        smallTextEdit.set_maximum_width(100);
        smallTextEdit.set_alignment(ng::alignment::Right);
        ng::horizontal_layout layoutLineEdits(layoutButtons);
        ng::vertical_layout layoutLineEdits2(layoutLineEdits);
        ng::vertical_layout layoutLineEdits3(layoutLineEdits);
        ng::text_field textField1(layoutLineEdits2, "Field 1:", "Enter text", ng::text_field_placement::LabelLeft);
        ng::text_field textField2(layoutLineEdits2, "Field 2:", "Enter text", ng::text_field_placement::LabelLeft);
        ng::text_field textField3(layoutLineEdits3, "Field 3", "Enter text", ng::text_field_placement::LabelAbove);
        ng::text_field textField4(layoutLineEdits3, "Field 4", "Enter text", ng::text_field_placement::LabelAbove);
        textField1.input_box().text_changed([&button1, &textField1]()
        {
            button1.text().set_text(textField1.input_box().text());
        });
        textEdit.Size.changed([&textField1](const ng::size& size)
        {
            std::ostringstream oss;
            oss << "textEdit size: " << size;
            textField1.input_box().set_text(oss.str());
        });
        ng::vertical_layout layoutSpinners(layoutLineEdits);
        ng::horizontal_layout layoutSpinners2(layoutSpinners);
        ng::vertical_spacer spacerSpinners(layoutSpinners);
        ng::double_spin_box spinBox(layoutSpinners2);
        spinBox.set_minimum(-100.0);
        spinBox.set_maximum(100.0);
        spinBox.set_step(0.1);
        ng::double_slider slider(layoutSpinners2);
        slider.set_minimum(-100.0);
        slider.set_maximum(100.0);
        slider.set_step(0.1);
        spinBox.value_changed([&slider, &spinBox]() {slider.set_value(spinBox.value()); });
        ng::double_spin_box doubleSpinBox(layoutSpinners2);
        doubleSpinBox.set_minimum(-10);
        doubleSpinBox.set_maximum(20);
        doubleSpinBox.set_step(0.5);
        ng::horizontal_layout layout2(layoutButtons);
        ng::label label1(layout2, "Label 1:");
        bool colourCycle = false;
        ng::push_button button6(layout2, "RGB <-> HSV\ncolour space\nconversion test");
        button6.clicked([&colourCycle]() { colourCycle = !colourCycle; });
        layout2.add_spacer().set_weight(ng::size(2.0f));
        ng::push_button button7(layout2, "Toggle\n&mute.");
        button7.set_foreground_colour(ng::colour::LightCoral);
        button7.set_maximum_size(window.dpi_scale(ng::size{ 128, 64 }));
        button7.clicked([&muteAction]() { muteAction.toggle(); });
        layout2.add_spacer().set_weight(ng::size(1.0));
        ng::push_button button8(layout2, "Enable/disable\ncontacts action.");
        button8.set_foreground_colour(ng::colour(255, 235, 160));
        button8.clicked([&contactsAction]() { if (contactsAction.is_enabled()) contactsAction.disable(); else contactsAction.enable(); });
        ng::horizontal_layout layout3(layoutButtons);
        prng.seed(3);
        auto transitionPrng = prng;
        std::vector<ng::transition_id> transitions;
        for (uint32_t i = 0; i < 10; ++i)
        {
            auto& button = layout3.emplace<ng::push_button>(std::string(1, 'A' + i));
            ng::colour randomColour = ng::colour{ prng(255), prng(255), prng(255) };
            button.set_foreground_colour(randomColour);
            button.clicked([&app, &textEdit, randomColour]() { textEdit.BackgroundColour = randomColour.same_lightness_as(app.current_style().palette().background_colour()); });
            transitions.push_back(ng::service<ng::i_animator>().add_transition(button.Position, ng::easing::OutBounce, transitionPrng.get(1.0, 2.0), false));
        }
        layout3.layout_completed([&layout3, &transitions, &transitionPrng]()
        {
            for (auto t : transitions)
                ng::service<ng::i_animator>().transition(t).enable(true);
            for (auto i = 0u; i < layout3.count(); ++i)
            {
                auto& button = layout3.get_widget_at(i);
                auto finalPosition = button.position();
                button.set_position(ng::point{ finalPosition.x, finalPosition.y - transitionPrng.get(600.0, 800.0) }.ceil());
                button.set_position(finalPosition);
            }
        });
        ng::group_box groupBox{ layout2, "Group Box" };
        ng::vertical_layout& layoutRadiosAndChecks = static_cast<ng::vertical_layout&>(groupBox.item_layout());
        ng::check_box triState(layoutRadiosAndChecks, "Tristate checkbo&x", ng::button_checkable::TriState);
        auto showHideTabs = [&triState, &tabContainer]()
        {
            if (triState.is_checked())
                tabContainer.hide_tab(8);
            else
                tabContainer.show_tab(8);
            if (triState.is_indeterminate())
                tabContainer.hide_tab(9);
            else
                tabContainer.show_tab(9);
        };
        triState.checked([&triState, showHideTabs]()
        {
            static uint32_t n;
            if ((n++)%2 == 1)
                triState.set_indeterminate();
            showHideTabs();
        });
        triState.unchecked([&triState, showHideTabs]()
        {
            showHideTabs();
        });
        ng::check_box wordWrap(layoutRadiosAndChecks, "Editor word wrap");
        wordWrap.check();
        wordWrap.checked([&textEdit]()
        {
            textEdit.set_word_wrap(true);
        });
        wordWrap.unchecked([&textEdit]()
        {
            textEdit.set_word_wrap(false);
        });
        ng::check_box password(layoutRadiosAndChecks, "Password");
        password.checked([&textField2]()
        {
            textField2.hint().set_text("Enter password");
            textField2.input_box().set_password(true);
        });
        password.unchecked([&textField2]()
        {
            textField2.hint().set_text("Enter text");
            textField2.input_box().set_password(false);
        });
        ng::check_box columns(layoutRadiosAndChecks, "Columns");
        ng::check_box groupBoxCheckable(layoutRadiosAndChecks, "Group Box Checkable");
        groupBoxCheckable.checked([&showFps, &fullRefresh, &groupBox]()
        {
            showFps = true;
            groupBox.set_checkable(true);
            groupBox.check_box().checked([&fullRefresh]() { fullRefresh = true; });
            groupBox.check_box().unchecked([&fullRefresh]() { fullRefresh = false; });
        });
        groupBoxCheckable.unchecked([&showFps, &groupBox]()
        {
            showFps = false;
            groupBox.set_checkable(false);
        });
        ng::gradient_widget gw(layoutRadiosAndChecks);
        columns.checked([&textEdit, &gw, &password]()
        {
            password.disable();
            textEdit.set_columns(3);
            gw.gradient_changed([&gw, &textEdit]()
            {
                auto cs = textEdit.column(2);
                cs.set_style(ng::text_edit::style{ ng::optional_font{}, ng::colour_or_gradient{}, ng::colour_or_gradient{}, ng::text_effect{ ng::text_effect_type::Outline, gw.gradient() } });
                textEdit.set_column(2, cs);
            }, textEdit);
        });
        columns.unchecked([&textEdit, &gw, &password]()
        {
            password.enable();
            textEdit.remove_columns();
            gw.gradient_changed.unsubscribe(textEdit);
        });
        ng::vertical_spacer spacerCheckboxes(layoutRadiosAndChecks);
        ng::vertical_layout layout4(layout2);
        ng::horizontal_layout layout7(layout4);
        ng::check_box buttonKerning(layout7, "kern");
        ng::check_box buttonSubpixel(layout7, "subpix");
        buttonKerning.toggled([&app, &buttonKerning]()
        {
            auto fi = app.current_style().font_info();
            if (buttonKerning.is_checked())
                fi.enable_kerning();
            else
                fi.disable_kerning();
            app.current_style().set_font_info(fi);
        });
        buttonSubpixel.toggled([&app, &buttonSubpixel]()
        {
            if (buttonSubpixel.is_checked())
                ng::service<ng::i_rendering_engine>().subpixel_rendering_on();
            else
                ng::service<ng::i_rendering_engine>().subpixel_rendering_off();
        });
        buttonSubpixel.check();
        ng::horizontal_layout layoutColourPickers{ layout4 };
        ng::push_button themeColour(layoutColourPickers, "theme"); themeColour.image().set_image(ng::image{ ":/closed/resources/caw_toolbar.naa#colour.png" });
        ng::push_button themeFont(layoutColourPickers, "font"); themeFont.image().set_image(ng::image{ ":/closed/resources/caw_toolbar.naa#font.png" });
        ng::push_button editColour(layoutColourPickers, "edit"); editColour.image().set_image(ng::image{ ":/closed/resources/caw_toolbar.naa#colour.png" });
        ng::widget lw{ layout4 };
        lw.set_margins(ng::margins{});
        ng::grid_layout layoutEffects{ lw, 2, 2, ng::alignment::Left | ng::alignment::VCentre };
        layoutEffects.set_margins(ng::margins{});
        ng::radio_button editNormal{ layoutEffects, "normal" };
        ng::radio_button editOutline{ layoutEffects, "outline" };
        ng::radio_button editGlow{ layoutEffects, "glow" };
        ng::radio_button editShadow{ layoutEffects, "shadow" };
        editNormal.checked([&]()
        {
            auto s = textEdit.default_style();
            s.set_text_effect(ng::optional_text_effect{});
            textEdit.set_default_style(s);
        });
        editOutline.checked([&]()
        {
            effectWidthSlider.set_value(1);
            auto s = textEdit.default_style();
            s.set_text_colour(app.current_style().palette().text_colour().light() ? ng::colour::Black : ng::colour::White);
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Outline, app.current_style().palette().text_colour() });
            textEdit.set_default_style(s);
        });
        editGlow.checked([&]()
        {
            effectWidthSlider.set_value(5);
            auto s = textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, ng::colour::Orange.with_lightness(0.9) });
            textEdit.set_default_style(s);
        });
        effectWidthSlider.value_changed([&]()
        {
            auto s = textEdit.default_style();
            s.set_text_effect(ng::text_effect{ editGlow.is_checked() ? ng::text_effect_type::Glow : ng::text_effect_type::Outline, s.text_effect()->colour(), effectWidthSlider.value(), s.text_effect()->aux1() });
            textEdit.set_default_style(s);
            std::ostringstream oss;
            oss << effectWidthSlider.value() << std::endl << effectAux1Slider.value() << std::endl;
            auto column = textEdit.column(0);
            column.set_margins(effectWidthSlider.value());
            textEdit.set_column(0, column);
            smallTextEdit.set_text(oss.str());
        });
        effectAux1Slider.value_changed([&]()
        {
            editGlow.check();
            auto s = textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Glow, s.text_effect()->colour(), s.text_effect()->width(), effectAux1Slider.value() });
            textEdit.set_default_style(s);
            std::ostringstream oss;
            oss << effectWidthSlider.value() << std::endl << effectAux1Slider.value() << std::endl;
            smallTextEdit.set_text(oss.str());
        });
        editShadow.checked([&]()
        {
            auto s = textEdit.default_style();
            s.set_text_effect(ng::text_effect{ ng::text_effect_type::Shadow, ng::colour::Black });
            textEdit.set_default_style(s);
        });
        ng::radio_button radio1(layout4, "Radio 1");
        ng::radio_button radioSliderFont(layout4, "Slider changes\nfont size");
        radioSliderFont.checked([&slider, &app]()
        {
            app.current_style().set_font_info(app.current_style().font_info().with_size(slider.normalized_value() * 18.0 + 4));
        });
        ng::radio_button radio3(layout4, "Radio 3");
        radio3.disable();
        ng::radio_button radioThemeColour(layout4, "Slider changes\ntheme colour");
        auto update_theme_colour = [&slider]()
        {
            auto themeColour = ng::service<ng::i_app>().current_style().palette().colour().to_hsv();
            themeColour.set_hue(slider.normalized_value() * 360.0);
            ng::service<ng::i_app>().current_style().palette().set_colour(themeColour.to_rgb());
        };
        slider.value_changed([update_theme_colour, &slider, &radioSliderFont, &radioThemeColour, &spinBox, &app]()
        {
            spinBox.set_value(slider.value());
            if (radioSliderFont.is_checked())
                app.current_style().set_font_info(app.current_style().font_info().with_size(slider.normalized_value() * 18.0 + 4));
            else if (radioThemeColour.is_checked())
                update_theme_colour();
        });
        slider.set_normalized_value((app.current_style().font_info().size() - 4) / 18.0);
        radioThemeColour.checked([update_theme_colour, &slider, &app]()
        {
            slider.set_normalized_value(ng::service<ng::i_app>().current_style().palette().colour().to_hsv().hue() / 360.0);
            update_theme_colour();
        });

        themeColour.clicked([&window]()
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
            colourPicker.selection_changed([&]()
            {
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(colourPicker.selected_colour());
            });
            if (colourPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(colourPicker.selected_colour());
            else
                ng::service<ng::i_app>().change_style("Keypad").palette().set_colour(oldColour);
            *sCustomColours = colourPicker.custom_colours();
        });

        themeFont.clicked([&window]()
        {
            ng::font_dialog fontPicker(window, ng::service<ng::i_app>().current_style().font_info());
            if (fontPicker.exec() == ng::dialog_result::Accepted)
                ng::service<ng::i_app>().current_style().set_font_info(fontPicker.selected_font());
        });

        editColour.clicked([&]()
        {
            static std::optional<ng::colour_dialog::custom_colour_list> sCustomColours;
            static ng::colour sInk = ng::service<ng::i_app>().current_style().palette().text_colour();
            ng::colour_dialog colourPicker(window, sInk);
            if (sCustomColours != std::nullopt)
                colourPicker.set_custom_colours(*sCustomColours);
            if (colourPicker.exec() == ng::dialog_result::Accepted)
            {
                sInk = colourPicker.selected_colour();
                textEdit.set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient::Horizontal }, ng::colour_or_gradient{} }, true);
                textField1.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient::Horizontal }, ng::colour_or_gradient{} }, true);
                textField2.input_box().set_default_style(ng::text_edit::style{ ng::optional_font{}, ng::gradient{ sInk, ng::colour::White, ng::gradient::Horizontal }, ng::colour_or_gradient{} }, true);
            }
            sCustomColours = colourPicker.custom_colours();
        });

        ng::vertical_spacer spacer1{ layout4 };
        ng::vertical_layout keypadLayout{ layout2 };
        ng::push_button button9(keypadLayout, "Default/Slate\nStyle");
        button9.clicked([&app]()
        {
            if (app.current_style().name() == "Default")
                app.change_style("Slate");
            else
                app.change_style("Default");
        });
        button9.set_foreground_colour(ng::colour::Aquamarine);
        ng::grid_layout keypad{ keypadLayout, 4, 3 };
        keypad.set_minimum_size(ng::size{ 100.0, 0.0 });
        keypad.set_spacing(ng::size{});
        for (uint32_t row = 0; row < 3; ++row)
            for (uint32_t col = 0; col < 3; ++col)
                keypad.add_item_at_position(row, col, std::make_shared<keypad_button>(textEdit, row * 3 + col + 1));
        keypad.add_item_at_position(3, 1, std::make_shared<keypad_button>(textEdit, 0));
        keypad.add_span(3, 1, 1, 2);

        neolib::callback_timer animation(app, [&](neolib::callback_timer& aTimer)
        {
            if (button6.is_singular())
                return;
            aTimer.again();
            if (colourCycle)
            {
                const double PI = 2.0 * std::acos(0.0);
                double brightness = ::sin((app.program_elapsed_ms() / 16 % 360) * (PI / 180.0)) / 2.0 + 0.5;
                neolib::random prng{ app.program_elapsed_ms() / 5000 };
                ng::colour randomColour = ng::colour{ prng(255), prng(255), prng(255) };
                randomColour = randomColour.to_hsv().with_brightness(brightness).to_rgb();
                button6.set_foreground_colour(randomColour);
            }
        }, 16);

        ng::i_widget& mdiPage = tabContainer.add_tab_page("MDI").as_widget();
        app.action_file_new().triggered([&]()
        {

        });

        ng::i_widget& messageBoxesPage = tabContainer.add_tab_page("Message Boxes").as_widget();
        ng::horizontal_layout messageBoxesPageLayout1{ messageBoxesPage };
        ng::group_box messageBoxIconsGroup{ messageBoxesPageLayout1, "Icons" };
        ng::radio_button messageBoxIconInformation{ messageBoxIconsGroup.item_layout(), "Information" };
        ng::radio_button messageBoxIconQuestion{ messageBoxIconsGroup.item_layout(), "Question" };
        ng::radio_button messageBoxIconWarning{ messageBoxIconsGroup.item_layout(), "Warning" };
        ng::radio_button messageBoxIconStop{ messageBoxIconsGroup.item_layout(), "Stop" };
        ng::radio_button messageBoxIconError{ messageBoxIconsGroup.item_layout(), "Error" };
        ng::radio_button messageBoxIconCritical{ messageBoxIconsGroup.item_layout(), "Critical" };
        messageBoxIconInformation.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#information.png" }); 
        messageBoxIconQuestion.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#question.png" });
        messageBoxIconWarning.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#warning.png" });
        messageBoxIconStop.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#stop.png" });
        messageBoxIconError.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#error.png" });
        messageBoxIconCritical.label().image().set_image(ng::image{ ":/neogfx/resources/icons.naa#critical.png" });
        messageBoxIconInformation.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        messageBoxIconQuestion.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        messageBoxIconWarning.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        messageBoxIconStop.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        messageBoxIconError.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        messageBoxIconCritical.label().image().set_fixed_size(window.dpi_scale(ng::size{ 24.0 }));
        ng::group_box messageBoxButtonsGroup{ messageBoxesPageLayout1, "Buttons" };
        uint32_t standardButtons = 0;
        uint32_t standardButton = 1;
        while (standardButton != 0)
        {
            auto bd = ng::dialog_button_box::standard_button_details(static_cast<ng::standard_button>(standardButton));
            if (bd.first != ng::button_role::Invalid)
            {
                auto& button = messageBoxButtonsGroup.item_layout().emplace<ng::check_box>(bd.second);
                button.checked([&standardButtons, standardButton]() { standardButtons |= standardButton; });
                button.unchecked([&standardButtons, standardButton]() { standardButtons &= ~standardButton; });
            }
            standardButton <<= 1;
        }
        ng::group_box messageBoxTextGroup{ messageBoxesPageLayout1, "Text" };
        ng::label messageBoxTitleLabel{ messageBoxTextGroup.item_layout(), "Title:" };
        ng::line_edit messageBoxTitle{ messageBoxTextGroup.item_layout() };
        messageBoxTitle.set_text("neoGFX Message Box Test");
        ng::label messageBoxTextLabel{ messageBoxTextGroup.item_layout(), "Text:" };
        ng::text_edit messageBoxText{ messageBoxTextGroup.item_layout() };
        messageBoxText.set_text("This is a test of the neoGFX message box.\nThis is a line of text.\n\nThis is a line of text after a blank line");
        messageBoxText.set_minimum_size(window.dpi_scale(ng::size{ 256, 128 }));
        ng::label messageBoxDetailedTextLabel{ messageBoxTextGroup.item_layout(), "Detailed Text:" };
        ng::text_edit messageBoxDetailedText{ messageBoxTextGroup.item_layout() };
        messageBoxDetailedText.set_text("This is where optional informative text usually goes.\nThis is a line of text.\n\nThe previous line was intentionally left blank.");
        messageBoxDetailedText.set_minimum_size(window.dpi_scale(ng::size{ 256, 128 }));
        ng::push_button openMessageBox{ messageBoxesPageLayout1, "Open Message Box" };
        openMessageBox.set_size_policy(ng::size_policy::Minimum);
        ng::label messageBoxResult{ messageBoxesPageLayout1 };
        openMessageBox.clicked([&]()
        {
            ng::standard_button result;
            if (messageBoxIconInformation.is_checked())
                result = ng::message_box::information(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (messageBoxIconQuestion.is_checked())
                result = ng::message_box::question(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (messageBoxIconWarning.is_checked())
                result = ng::message_box::warning(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (messageBoxIconStop.is_checked())
                result = ng::message_box::stop(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (messageBoxIconError.is_checked())
                result = ng::message_box::error(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            else if (messageBoxIconCritical.is_checked())
                result = ng::message_box::critical(window, messageBoxTitle.text(), messageBoxText.text(), messageBoxDetailedText.text(), static_cast<ng::standard_button>(standardButtons));
            messageBoxResult.text().set_text("Result = " + ng::dialog_button_box::standard_button_details(result).second);
        });

        // Item Views

        ng::service<ng::i_window_manager>().save_mouse_cursor();
        ng::service<ng::i_window_manager>().set_mouse_cursor(ng::mouse_system_cursor::Wait);

        ng::i_widget& itemViewsPage = tabContainer.add_tab_page("Item Views").as_widget();
        ng::vertical_layout layoutItemViews(itemViewsPage);
        ng::table_view tableView1(layoutItemViews);
        ng::table_view tableView2(layoutItemViews);
        tableView1.set_minimum_size(ng::size(128, 128));
        tableView2.set_minimum_size(ng::size(128, 128));
        ng::push_button button10(layoutItemViews, "Toggle List\nHeader View");
        button10.clicked([&tableView1, &tableView2]()
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
        ng::horizontal_layout tableViewTweaks(layoutItemViews);
        tableViewTweaks.set_alignment(ng::alignment::Top);
        tableViewTweaks.add_spacer();
        ng::vertical_layout tableViewSelection(tableViewTweaks);
        tableViewSelection.set_weight(ng::size{});
        ng::radio_button noSelection(tableViewSelection, "No selection");
        ng::radio_button singleSelection(tableViewSelection, "Single selection");
        ng::radio_button multipleSelection(tableViewSelection, "Multiple selection");
        ng::radio_button extendedSelection(tableViewSelection, "Extended selection");
        ng::group_box column5(tableViewTweaks, "Column 5");
        ng::check_box column5ReadOnly(column5.item_layout(), "Read only");
        ng::check_box column5Unselectable(column5.item_layout(), "Unselectable");
        ng::vertical_spacer column5Spacer(column5.item_layout());
        tableViewTweaks.add_spacer();

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

        column5ReadOnly.checked([&]() {    itemModel.set_column_read_only(5, true); update_column5_heading(); });
        column5ReadOnly.unchecked([&]() { itemModel.set_column_read_only(5, false); update_column5_heading(); });
        column5Unselectable.checked([&]() { itemModel.set_column_selectable(5, false); update_column5_heading(); });
        column5Unselectable.unchecked([&]() { itemModel.set_column_selectable(5, true); update_column5_heading(); });

        itemModel.set_column_min_value(0, 0u);
        itemModel.set_column_max_value(0, 9999u);
        itemModel.set_column_min_value(1, 0u);
        itemModel.set_column_max_value(1, 9999u);
        itemModel.set_column_step_value(1, 1u);
        tableView1.set_model(itemModel);
        my_item_presentation_model ipm1{ itemModel, ng::item_cell_colour_type::Foreground };
        tableView1.set_presentation_model(ipm1);
        ipm1.set_column_editable(4, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(5, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(6, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(7, ng::item_cell_editable::WhenFocused);
        ipm1.set_column_editable(8, ng::item_cell_editable::WhenFocused);
        tableView2.set_model(itemModel);
        my_item_presentation_model ipm2{ itemModel, ng::item_cell_colour_type::Background };
        ipm2.set_column_editable(0, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(1, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(2, ng::item_cell_editable::WhenFocused);
        ipm2.set_column_editable(3, ng::item_cell_editable::WhenFocused);
        tableView2.set_presentation_model(ipm2);
        tableView2.column_header().set_expand_last_column(true);
        tableView1.keyboard_event([&tableView1](const ng::keyboard_event& ke)
        {
            if (ke.type() == ng::keyboard_event_type::KeyPressed && ke.scan_code() == ng::ScanCode_DELETE && tableView1.model().rows() > 0 && tableView1.selection_model().has_current_index())
                tableView1.model().erase(tableView1.model().begin() + tableView1.presentation_model().to_item_model_index(tableView1.selection_model().current_index()).row());
        });

        ng::service<ng::i_window_manager>().restore_mouse_cursor(window);

        auto& w = tabContainer.add_tab_page("Lots").as_widget();
        ng::vertical_layout l(w);
        #ifdef NDEBUG
        for (int i = 0; i < 1000; ++i)
        #else
        for (int i = 0; i < 100; ++i)
        #endif
            l.emplace<ng::push_button>(boost::lexical_cast<std::string>(i));
        auto& w2 = tabContainer.add_tab_page("Images").as_widget();
        ng::horizontal_layout l2(w2);
        ng::vertical_layout l3(l2);
        ng::image_widget iw(l3, ng::image(":/test/resources/channel_256.png"), ng::aspect_ratio::Ignore);
        iw.set_background_colour(ng::colour::Red.lighter(0x80));
        iw.set_size_policy(ng::size_policy::Expanding);
        iw.set_minimum_size(ng::size{});
        ng::image_widget iw2(l3, ng::image(":/test/resources/channel_256.png"), ng::aspect_ratio::Keep);
        iw2.set_background_colour(ng::colour::Green.lighter(0x80));
        iw2.set_size_policy(ng::size_policy::Expanding);
        iw2.set_minimum_size(ng::size{});
        ng::image_widget iw3(l3, ng::image(":/test/resources/channel_256.png"), ng::aspect_ratio::KeepExpanding);
        iw3.set_background_colour(ng::colour::Blue.lighter(0x80));
        iw3.set_size_policy(ng::size_policy::Expanding);
        iw3.set_minimum_size(ng::size{});
        ng::image_widget iw4(l3, ng::image(":/test/resources/channel_256.png"), ng::aspect_ratio::Stretch);
        iw4.set_background_colour(ng::colour::Magenta.lighter(0x80));
        iw4.set_size_policy(ng::size_policy::Expanding);
        iw4.set_minimum_size(ng::size{});
        ng::image_widget iw5(l2, ng::image(":/test/resources/orca.png"));
        ng::grid_layout l4(l2);
        l4.set_spacing(ng::size{});
        ng::image hash(":/test/resources/channel_32.png");
        for (uint32_t i = 0; i < 9; ++i)
        {
            auto hashWidget = std::make_shared<ng::image_widget>(hash, ng::aspect_ratio::Keep, static_cast<ng::cardinal_placement>(i));
            hashWidget->set_size_policy(ng::size_policy::Expanding);
            hashWidget->set_background_colour(i % 2 == 0 ? ng::colour::Black : ng::colour::White);
            l4.add_item_at_position(i / 3, i % 3, hashWidget);
        }
        ng::image smallHash(":/test/resources/channel.png");

        auto& gamePage = tabContainer.add_tab_page("Game").as_widget();
        ng::vertical_layout gl(gamePage);
        create_game(gl);

        neolib::basic_random<uint8_t> rngColour;
        auto random_colour = [&]()
        {
            return ng::colour{ rngColour(255), rngColour(255), rngColour(255) };
        };

        auto& tabDrawing = tabContainer.add_tab_page("Drawing").as_widget();
        ng::vertical_layout tabDrawingLayout1{ tabDrawing };
        ng::horizontal_layout tabDrawingLayout2{ tabDrawing };
        tabDrawingLayout2.set_size_policy(ng::size_policy::Minimum);
        tabDrawingLayout2.emplace<ng::label>("Easing:");
        ng::basic_item_model<ng::easing> easingItemModel;
        for (auto i = 0; i < ng::standard_easings().size(); ++i)
            easingItemModel.insert_item(easingItemModel.end(), ng::standard_easings()[i], ng::to_string(ng::standard_easings()[i]));
        easing_item_presentation_model easingPresentationModel{ easingItemModel };
        ng::drop_list easingDropDown{ tabDrawingLayout2 };
        easingDropDown.set_size_policy(ng::size_policy::Minimum);
        easingDropDown.set_model(easingItemModel);
        easingDropDown.set_presentation_model(easingPresentationModel);
        easingDropDown.selection_model().set_current_index(ng::item_model_index{ 0 });
        easingDropDown.accept_selection();
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
        auto test_pattern = [renderToTextureFont](ng::graphics_context& aGc, const ng::point& aOrigin, double aDpiScale, const ng::colour& aColour, const std::string& aText)
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

        tabDrawing.painting([&](ng::graphics_context& aGc)
        {
            ng::service<ng::i_rendering_engine>().want_game_mode();
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 100, 100 }, ng::size{ 100, 100 } }, 10.0, ng::colour::Goldenrod);
            aGc.fill_rect(ng::rect{ ng::point{ 300, 250 }, ng::size{ 200, 100 } }, gw.gradient().with_direction(ng::gradient::Horizontal));
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, gw.gradient().with_direction(ng::gradient::Horizontal));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 300, 400 }, ng::size{ 200, 100 } }, 10.0, ng::pen{ ng::colour::Blue4, 2.0 });
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 150, 150 }, ng::size{ 300, 300 } }, 10.0, ng::pen{ ng::colour::Red4, 2.0 });
            aGc.fill_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, gw.gradient().with_direction(ng::gradient::Radial));
            aGc.draw_rounded_rect(ng::rect{ ng::point{ 500, 500 }, ng::size{ 200, 200 } }, 10.0, ng::pen{ ng::colour::Black, 1.0 });
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
            ng::scalar t = app.program_elapsed_us();
            auto d = 1000000.0;
            auto x = ng::ease(easingItemModel.item(easingDropDown.selection()), int(t / d) % 2 == 0 ? std::fmod(t, d) / d : 1.0 - std::fmod(t, d) / d) * (tabDrawing.extents().cx - logo.extents().cx);
//            auto x = ng::ease(ng::easing_class::Linear, ng::easing_class::Bounce, int(t / d) % 2 == 0 ? std::fmod(t, d) / d : 1.0 - std::fmod(t, d) / d) * (tabDrawing.extents().cx - logo.extents().cx);
            aGc.draw_texture(ng::point{ x, (tabDrawing.extents().cy - logo.extents().cy) / 2.0 }, logo);

            auto texLocation = ng::point{ (tabDrawing.extents().cx - 64.0) / 2.0, (tabDrawing.extents().cy - logo.extents().cy) / 4.0 }.ceil();
            aGc.draw_texture(texLocation + ng::point{ 0.0, 0.0 }, tex[0]);
            aGc.draw_texture(texLocation + ng::point{ 0.0, 65.0 }, tex[1]);
            aGc.draw_texture(texLocation + ng::point{ 65.0, 0.0 }, tex[2]);
            aGc.draw_texture(texLocation + ng::point{ 65.0, 65.0 }, tex[3]);

            texLocation.x += 140.0;
            test_pattern(aGc, texLocation + ng::point{ 0.0, 0.0 }, aGc.dpi_scale(1.0), texColour[0], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 0.0, 65.0 }, aGc.dpi_scale(1.0), texColour[1], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 0.0 }, aGc.dpi_scale(1.0), texColour[2], "Render\nTo\nScreen");
            test_pattern(aGc, texLocation + ng::point{ 65.0, 65.0 }, aGc.dpi_scale(1.0), texColour[3], "Render\nTo\nScreen");
        });

        neolib::callback_timer animator{ app, [&](neolib::callback_timer& aTimer)
        {
            aTimer.set_duration(tabDrawing.can_update() ? 0 : 100);
            aTimer.again();
            tabDrawing.update();
        }, 100 };

        auto& tabEditor = tabContainer.add_tab_page("Editor").as_widget();
        ng::vertical_layout layoutEditor(tabEditor);
        ng::text_edit textEdit2(layoutEditor);
        textEdit2.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::Red, ng::colour::White, ng::gradient::Horizontal), ng::colour_or_gradient()));
        ng::push_button editorStyle1(layoutEditor, "Style 1");
        editorStyle1.clicked([&textEdit2]()
        {
            textEdit2.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::Red, ng::colour::White, ng::gradient::Horizontal), ng::colour_or_gradient()));
        });
        ng::push_button editorStyle2(layoutEditor, "Style 2");
        editorStyle2.clicked([&textEdit2]()
        {
            textEdit2.set_default_style(ng::text_edit::style(ng::font("SnareDrum One NBP", "Regular", 60.0), ng::colour::White));
        });

        auto& circlesWidget = tabContainer.add_tab_page("Circles").as_widget();
        circlesWidget.painting([&circlesWidget, &random_colour](ng::graphics_context& aGc)
        {
            neolib::basic_random<ng::coordinate> prng;
            for (int i = 0; i < 100; ++i)
            {
                switch (static_cast<int>(prng(2)))
                {
                case 0:
                    aGc.draw_circle(
                        ng::point{ prng(circlesWidget.client_rect().cx - 1), prng(circlesWidget.client_rect().extents().cy - 1) }, prng(255),
                        ng::pen{ random_colour(), prng(1, 3) });
                    break;
                case 1:
                    aGc.draw_circle(
                        ng::point{ prng(circlesWidget.client_rect().cx - 1), prng(circlesWidget.client_rect().cy - 1) }, prng(255),
                        ng::pen{ random_colour(), prng(1, 3) },
                        random_colour().with_alpha(random_colour().red()));
                    break;
                case 2:
                    aGc.fill_circle(
                        ng::point{ prng(circlesWidget.client_rect().cx - 1), prng(circlesWidget.client_rect().cy - 1) }, prng(255),
                        random_colour().with_alpha(random_colour().red()));
                    break;
                }
            }
        });
        tabContainer.add_tab_page("Foo").tab().set_image(smallHash);
        tabContainer.add_tab_page("Bar").tab().set_image(smallHash);

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

