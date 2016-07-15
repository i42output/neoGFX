#include <neolib/neolib.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/default_item_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/tab_page_container.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/gui/dialog/colour_dialog.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>

namespace ng = neogfx;

class my_item_model : public ng::basic_default_item_model<void*, 5>, public ng::item_presentation_model
{
public:
	my_item_model()
	{
		set_column_heading_text(0, "TOne");
		set_column_heading_text(1, "Two");
		set_column_heading_text(2, "Three");
		set_column_heading_text(3, "Four");
		set_column_heading_text(4, "Five");
	}
public:
	virtual ng::optional_colour cell_colour(const ng::item_model_index& aIndex, colour_type_e aColourType) const
	{
		std::srand(aIndex.row() ^ aIndex.column()); // use seed to make random colour based on row/index
		switch (aColourType)
		{
		case ForegroundColour:
			return ng::colour(32 + rand() % (256 - 64), 32 + rand() % (256 - 64), 32 + rand() % (256 - 64));
		default:
			return ng::optional_colour();
		}
	}
};

class keypad_button : public ng::push_button
{
public:
	keypad_button(ng::text_edit& aTextEdit, uint32_t aNumber) :
		ng::push_button(boost::lexical_cast<std::string>(aNumber)), iTextEdit(aTextEdit)
	{
		clicked([this, aNumber]()
		{
			ng::app::instance().change_style("Keypad").
				set_colour(aNumber != 9 ? ng::colour(aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0) : ng::colour::LightGoldenrod);
			if (aNumber == 9)
				iTextEdit.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
			else if (aNumber == 8)
				iTextEdit.set_default_style(ng::text_edit::style(ng::font("SnareDrum One NBP", "Regular", 60.0), ng::colour::Black, ng::text_edit::style::colour_type(), ng::colour::White));
			else if (aNumber == 0)
				iTextEdit.set_default_style(ng::text_edit::style(ng::font("SnareDrum Two NBP", "Regular", 60.0), ng::colour::White));
			else
				iTextEdit.set_default_style(
					ng::text_edit::style(
						ng::optional_font(),
						ng::gradient(
							ng::colour(aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0).lighter(0x40),
							ng::colour(aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0).lighter(0xC0),
							ng::gradient::Horizontal),
						ng::text_edit::style::colour_type()));
		});
	}
private:
	ng::text_edit& iTextEdit;
};

void create_game(ng::i_layout& aLayout);

int main(int argc, char* argv[])
{
	ng::app app("neoGFX Test App");
	try
	{
		app.change_style("Default").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
		app.change_style("Slate").set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
		app.register_style(ng::style("Keypad")).set_font_info(ng::font_info("Segoe UI", std::string("Regular"), 9));
		app.change_style("Keypad");
		app.current_style().set_colour(ng::colour::Black);
		app.change_style("Default");

		ng::window window(ng::size{ 675, 675 });

		ng::vertical_layout layout0(window);

		app.add_action("Goldenrod Style").set_shortcut("Ctrl+Alt+Shift+G").triggered([]()
		{
			ng::app::instance().change_style("Keypad").set_colour(ng::colour::LightGoldenrod);
		});

		auto& contactsAction = app.add_action("&Contacts...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#contacts.png").set_shortcut("Alt+C");
		contactsAction.triggered([]()
		{
			ng::app::instance().change_style("Keypad").set_colour(ng::colour::White);
		});
		auto& muteAction = app.add_action("Mute/&Unmute Sound", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#mute.png");
		muteAction.set_checkable(true);
		muteAction.set_checked_image("file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#unmute.png");

		auto& cutAction = app.add_action("Cut", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#cut.png").disable().set_shortcut("Ctrl+X");
		auto& copyAction = app.add_action("Copy", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#copy.png").disable().set_shortcut("Ctrl+C");
		auto& pasteAction = app.add_action("Paste", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#paste.png").set_shortcut("Ctrl+V");
		auto& pasteAndGoAction = app.add_action("Paste and Go", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#paste_and_go.png").set_shortcut("Ctrl+Shift+V");
		auto& deleteAction = app.add_action("Delete").set_shortcut("Del");
		auto& selectAllAction = app.add_action("Select All").set_shortcut("Ctrl+A");

		neolib::callback_timer ct{ app, [&app, &cutAction, &copyAction, &pasteAction, &pasteAndGoAction](neolib::callback_timer& aTimer) 
		{
			aTimer.again();
			if (app.clipboard().sink_active())
			{
				auto& sink = app.clipboard().active_sink();
				if (sink.can_cut())
					cutAction.enable();
				else
					cutAction.disable();
				if (sink.can_copy())
					copyAction.enable();
				else
					copyAction.disable();
				if (sink.can_paste())
				{
					pasteAction.enable();
					pasteAndGoAction.enable();
				}
				else
				{
					pasteAction.disable();
					pasteAndGoAction.disable();
				}
			}
		}, 100 };

		cutAction.triggered([&app]()
		{
			app.clipboard().cut();
		});

		copyAction.triggered([&app]()
		{
			app.clipboard().copy();
		});

		pasteAction.triggered([&app]()
		{
			app.clipboard().paste();
		});

		pasteAndGoAction.triggered([&app]()
		{
			app.clipboard().paste();
		});

		deleteAction.triggered([&app]()
		{
			app.clipboard().delete_selected();
		});

		selectAllAction.triggered([&app]()
		{
			app.clipboard().select_all();
		});

		ng::menu_bar menu(layout0);
		auto& exitAction = app.add_action("Exit").set_shortcut("Alt+F4");
		exitAction.triggered([]() 
		{
			ng::app::instance().quit(0);
		});
		auto& fileMenu = menu.add_sub_menu("&File");
		fileMenu.add_action(exitAction);
		auto& editMenu = menu.add_sub_menu("&Edit");
		editMenu.add_action(app.add_action("Undo").set_shortcut("Ctrl+Z"));
		editMenu.add_action(app.add_action("Redo").set_shortcut("Ctrl+Y"));
		editMenu.add_separator();
		editMenu.add_action(cutAction);
		editMenu.add_action(copyAction);
		editMenu.add_action(pasteAction);
		editMenu.add_action(pasteAndGoAction);
		editMenu.add_action(deleteAction);
		editMenu.add_separator();
		editMenu.add_action(selectAllAction);
		auto& viewMenu = menu.add_sub_menu("&View");
		auto& addFavouriteAction = app.add_action("Add Favourite...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#add_favourite.png");
		auto& organizeFavouritesAction = app.add_action("Organize Favourites...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#organize_favourites.png");
		auto& favouritesMenu = menu.add_sub_menu("F&avourites");
		favouritesMenu.add_action(addFavouriteAction);
		favouritesMenu.add_action(organizeFavouritesAction);
		favouritesMenu.add_separator();
		favouritesMenu.add_action(app.add_action("Alice", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		favouritesMenu.add_action(app.add_action("Bob", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		favouritesMenu.add_action(app.add_action("Carlos", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		favouritesMenu.add_action(app.add_action("Dave", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		auto& menuDrones = favouritesMenu.add_sub_menu("Silent Running Drones");
		menuDrones.set_image("file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#folder.png");
		menuDrones.add_action(app.add_action("Dewey", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		menuDrones.add_action(app.add_action("Huey", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		menuDrones.add_action(app.add_action("Louie", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
		auto& subMenu2 = favouritesMenu.add_sub_menu("DC Characters");
		subMenu2.set_image("file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#folder.png");
		auto& subMenu3 = favouritesMenu.add_sub_menu("Marvel Characters");
		subMenu3.set_image("file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#folder.png");
		favouritesMenu.add_separator();
		for (int i = 1; i <= 5; ++i)
		{
			auto& sm = favouritesMenu.add_sub_menu("More" + boost::lexical_cast<std::string>(i));
			for (int j = 1; j <= 5; ++j)
			{
				auto& sm2 = sm.add_sub_menu("More" + boost::lexical_cast<std::string>(j));
				int n = rand() % 100;
				for (int k = 1; k < n; ++k)
				{
					sm2.add_action(app.add_action("More" + boost::lexical_cast<std::string>(k), "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#favourite.png"));
				}
			}
		}
		menu.add_action(contactsAction);
		menu.add_action(muteAction);
		auto& testMenu = menu.add_sub_menu("&Test");
		testMenu.add_action(contactsAction);
		testMenu.add_action(muteAction);
		testMenu.add_action(muteAction);
		testMenu.add_action(muteAction);
		testMenu.add_action(muteAction);
		ng::i_action& colourAction = app.add_action("Colour Dialog...");
		colourAction.triggered([&window]()
		{
			ng::color_dialog cd(window);
			cd.exec();
		});
		testMenu.add_action(colourAction);
		auto& windowMenu = menu.add_sub_menu("&Window");
		auto& helpMenu = menu.add_sub_menu("&Help");

		ng::toolbar toolbar(layout0);
		toolbar.add_action(contactsAction);
		toolbar.add_action(addFavouriteAction);
		toolbar.add_action(organizeFavouritesAction);
		toolbar.add_action(app.add_action("Keywords...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#keyword.png"));
		toolbar.add_action(app.add_action("Settings...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#settings.png"));
		toolbar.add_action(app.add_action("Manage Plugins...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#manage_plugins.png"));
		toolbar.add_action(muteAction);
		toolbar.add_separator();
		toolbar.add_action(cutAction);
		toolbar.add_action(copyAction);
		toolbar.add_action(pasteAction);
		toolbar.add_action(pasteAndGoAction);
		toolbar.add_separator();
		toolbar.add_action(app.add_action("Check for Updates...", "file://" + boost::filesystem::current_path().string() + "/caw_toolbar.naa#setup.png"));

		ng::tab_page_container tabContainer(layout0);

		// Buttons

		ng::i_widget& buttonsPage = tabContainer.add_tab_page("Buttons").widget();
		ng::vertical_layout layoutButtons(buttonsPage);
		layoutButtons.set_margins(ng::margins(8));
		ng::push_button button0(layoutButtons, "This is the neoGFX test application.");
		button0.label().set_placement(ng::label_placement::ImageTextVertical);
		button0.image().set_image(ng::image{ "file://" + boost::filesystem::current_path().string() + "/../../../../../../neoGFX.png" });
		button0.image().set_minimum_size(ng::size{ 32, 32 });
		button0.image().set_maximum_size(ng::size{ 160, std::numeric_limits<ng::dimension>::max() });
		button0.set_size_policy(ng::size_policy::Expanding);
		button0.set_foreground_colour(ng::colour::LightGoldenrodYellow);
		ng::push_button button1(layoutButtons, "the,,, quick brown fox jumps over the lazy dog");
		button1.set_foreground_colour(ng::colour::LightGoldenrod);
		ng::push_button button2(layoutButtons, u8"ويقفز الثعلب البني السريع فوق الكلب الكسول");
		button2.set_foreground_colour(ng::colour::Goldenrod);
		ng::push_button button3(layoutButtons, u8"שועל חום קפיצות מעל הכלב העצלן");
		button3.set_foreground_colour(ng::colour::DarkGoldenrod);
		button3.set_minimum_size(ng::size(288, 64));
		ng::push_button button4(layoutButtons, u8"请停止食用犬");
		button4.set_foreground_colour(ng::colour::CadetBlue);
		button4.set_maximum_size(ng::size(128, 64));
		ng::push_button button5(layoutButtons, u8"sample te&xt نص عينة sample text טקסט לדוגמא 示例文本 sample text\nKerning test: Tr. WAVAVAW. zzz zoz ozo ooo");
		ng::text_edit textEdit(layoutButtons);
		textEdit.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
		ng::horizontal_layout layoutLineEdits(layoutButtons);
		ng::line_edit lineEdit(layoutLineEdits);
		lineEdit.set_text("Line edit");
		lineEdit.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
		ng::line_edit lineEditPassword(layoutLineEdits);
		lineEditPassword.set_text("Password");
		lineEditPassword.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
		ng::spin_box spinBox(layoutLineEdits);
		spinBox.set_minimum(20);
		spinBox.set_maximum(100);
		spinBox.set_step(5);
		ng::slider slider(layoutLineEdits);
		slider.set_minimum(20);
		slider.set_maximum(100);
		slider.set_step(5);
		spinBox.value_changed([&slider, &spinBox]() {slider.set_value(spinBox.value()); });
		slider.value_changed([&slider, &spinBox]() {spinBox.set_value(slider.value()); });
		slider.set_value(spinBox.value());
		ng::double_spin_box doubleSpinBox(layoutLineEdits);
		doubleSpinBox.set_minimum(-10);
		doubleSpinBox.set_maximum(20);
		doubleSpinBox.set_step(0.5);
		ng::horizontal_layout layout2(layoutButtons);
		ng::label label1(layout2, "Label 1:");
		ng::push_button button6(layout2, "RGB <-> HSV\ncolour space\nconversion test");
		layout2.add_spacer().set_weight(ng::size(2.0f));
		ng::push_button button7(layout2, "Toggle\n&mute.");
		button7.set_foreground_colour(ng::colour::LightCoral);
		button7.set_maximum_size(ng::size(128, 64));
		button7.clicked([&muteAction]() { muteAction.toggle(); });
		layout2.add_spacer().set_weight(ng::size(1.0));
		ng::push_button button8(layout2, "Enable/disable\ncontacts action.");
		button8.set_foreground_colour(ng::colour(255, 235, 160));
		button8.clicked([&contactsAction]() { if (contactsAction.is_enabled()) contactsAction.disable(); else contactsAction.enable(); });
		ng::horizontal_layout layout3(layoutButtons);
		std::srand(4242);
		for (uint32_t i = 0; i < 10; ++i)
		{
			auto button = std::make_shared<ng::push_button>(std::string(1, 'A' + i));
			layout3.add_item(button);
			ng::colour randomColour = ng::colour(std::rand() % 256, std::rand() % 256, std::rand() % 256);
			button->set_foreground_colour(randomColour);
		}
		ng::vertical_layout layoutRadiosAndChecks(layout2);
		ng::check_box triState(layoutRadiosAndChecks, "Tristate checkbox", ng::check_box::TriState);
		triState.checked([&triState]()
		{
			static uint32_t n;
			if ((n++)%2 == 1)
				triState.set_indeterminate();
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
		password.checked([&lineEditPassword]()
		{
			lineEditPassword.set_password(true);
		});
		password.unchecked([&lineEditPassword]()
		{
			lineEditPassword.set_password(false);
		});
		ng::gradient_widget gw(layoutRadiosAndChecks);
		ng::vertical_spacer spacerCheckboxes(layoutRadiosAndChecks);
		ng::vertical_layout layout4(layout2);
		ng::push_button button9(layout4, "Default/Slate\nStyle");
		button9.clicked([&app]()
		{
			if (app.current_style().name() == "Default")
				app.change_style("Slate");
			else
				app.change_style("Default");
		});
		button9.set_foreground_colour(ng::colour::Aquamarine);
		ng::horizontal_layout layout6(layout4);
		ng::push_button buttonMinus(layout6, "-");
		ng::push_button buttonPlus(layout6, "+");
		ng::horizontal_layout layout7(layout4);
		ng::push_button buttonKerning(layout7, "kern");
		ng::push_button buttonSubpixel(layout7, "subpix");
		buttonMinus.clicked([&app]()
		{
			app.current_style().set_font_info(app.current_style().font_info().with_size(app.current_style().font_info().size() - 0.1f));
		});
		buttonPlus.clicked([&app]()
		{
			app.current_style().set_font_info(app.current_style().font_info().with_size(app.current_style().font_info().size() + 0.1f));
		});
		buttonKerning.clicked([&app]()
		{
			auto fi = app.current_style().font_info();
			if (fi.kerning())
				fi.disable_kerning();
			else
				fi.enable_kerning();
			app.current_style().set_font_info(fi);
		});
		buttonSubpixel.clicked([&app]()
		{
			if (app.rendering_engine().is_subpixel_rendering_on())
				app.rendering_engine().subpixel_rendering_off();
			else
				app.rendering_engine().subpixel_rendering_on();
		});
		ng::push_button buttonColourPicker(layout4, "Colour Picker");
		buttonColourPicker.clicked([&window]()
		{
			ng::colour_dialog colourPicker(window, ng::app::instance().change_style("Keypad").colour());
			if (colourPicker.exec() == ng::dialog::Accepted)
				ng::app::instance().change_style("Keypad").set_colour(colourPicker.selected_colour());

		});
		ng::vertical_spacer spacer1(layout4);
		ng::grid_layout keypad(4, 3, layout2);
		keypad.set_minimum_size(ng::size(100.0, 0.0));
		keypad.set_spacing(0.0);
		for (uint32_t row = 0; row < 3; ++row)
			for (uint32_t col = 0; col < 3; ++col)
				keypad.add_item(row, col, std::make_shared<keypad_button>(textEdit, row * 3 + col + 1));
		keypad.add_item(3, 1, std::make_shared<keypad_button>(textEdit, 0));
		keypad.add_span(3, 1, 1, 2);

		neolib::callback_timer animation(app, [&](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			std::srand(static_cast<unsigned int>(app.program_elapsed_ms() / 5000));
			const double PI = 2.0 * std::acos(0.0);
			double brightness = ::sin((app.program_elapsed_ms() / 16 % 360) * (PI / 180.0)) / 2.0 + 0.5;
			ng::colour randomColour = ng::colour(std::rand() % 256, std::rand() % 256, std::rand() % 256);
			randomColour = randomColour.to_hsv().with_brightness(brightness).to_rgb();
			button6.set_foreground_colour(randomColour);
		}, 16);

		// Item Views

		app.surface_manager().surface(0).save_mouse_cursor();
		app.surface_manager().surface(0).set_mouse_cursor(ng::mouse_system_cursor::Wait);

		ng::i_widget& itemViewsPage = tabContainer.add_tab_page("Item Views").widget();
		ng::vertical_layout layoutItemViews(itemViewsPage);
		ng::table_view tableView(layoutItemViews);
		tableView.set_minimum_size(ng::size(128, 128));
		ng::push_button button10(layoutItemViews, "Toggle List\nHeader View");
		button10.clicked([&tableView]()
		{
			if (tableView.column_header().visible())
				tableView.column_header().hide();
			else
				tableView.column_header().show();
		});
		ng::radio_button noSelection(layoutItemViews, "No selection");
		ng::radio_button singleSelection(layoutItemViews, "Single selection");
		ng::radio_button multipleSelection(layoutItemViews, "Multiple selection");
		ng::radio_button extendedSelection(layoutItemViews, "Extended selection");

		my_item_model itemModel;
		#ifdef NDEBUG
		itemModel.reserve(10000);
		#else
		itemModel.reserve(100);
		#endif
		ng::app::event_processing_context epc(app);
		for (uint32_t row = 0; row < itemModel.capacity(); ++row)
		{
			if (row % 1000 == 0)
				app.process_events(epc);
			for (uint32_t col = 0; col < 5; ++col)
			{
				if (col == 0)
					itemModel.insert_item(ng::item_model_index(row), row + 1);
				else
				{
					std::string randomString;
					for (uint32_t j = std::rand() % 16; j-- > 0;)
						randomString += static_cast<char>('A' + std::rand() % ('z' - 'A' + 1));
					itemModel.insert_cell_data(ng::item_model_index(row, col), randomString);
				}
			}
		} 
	
		tableView.set_model(itemModel);
		tableView.set_presentation_model(itemModel);

		app.surface_manager().surface(0).restore_mouse_cursor();

		auto& w = tabContainer.add_tab_page("Lots").widget();
		ng::vertical_layout l(w);
		for (int i = 0; i < 10000; ++i)
			l.add_item(std::make_shared<ng::push_button>(boost::lexical_cast<std::string>(i)));
		auto& w2 = tabContainer.add_tab_page("Images").widget();
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
			l4.add_item(i / 3, i % 3, hashWidget);
		}
		ng::image smallHash(":/test/resources/channel.png");

		auto& gamePage = tabContainer.add_tab_page("Game").widget();
		ng::vertical_layout gl(gamePage);
		create_game(gl);

		auto& tabDrawing = tabContainer.add_tab_page("Drawing").widget();
		tabDrawing.painting([&tabDrawing](ng::graphics_context& aGc)
		{
			ng::texture logo{ ng::image{ "file://" + boost::filesystem::current_path().string() + "/../../../../../../neoGFX.png" } };
			aGc.draw_texture(ng::point{ (tabDrawing.extents() - logo.extents()) / 2.0 }, logo);
			aGc.fill_rounded_rect(ng::rect{ 100, 100, 100, 100 }, 10.0, ng::colour::Goldenrod);
			aGc.fill_rect(ng::rect{ 300, 250, 200, 100 },
				ng::gradient{
					{{ 0.0, ng::colour::Black },{ 0.33, ng::colour::Yellow },{ 0.66, ng::colour::Red },{ 1.0, ng::colour::White }}, ng::gradient::Horizontal });
			aGc.fill_rounded_rect(ng::rect{ 300, 400, 200, 100 }, 10.0,
				ng::gradient{
					{{0.0, ng::colour::Black}, {0.33, ng::colour::Yellow}, { 0.66, ng::colour::Red}, {1.0, ng::colour::White}}, ng::gradient::Horizontal });
			aGc.draw_rounded_rect(ng::rect{ 300, 400, 200, 100 }, 10.0, ng::pen{ ng::colour::Blue4, 2.0 });
			aGc.draw_rounded_rect(ng::rect{ 150, 150, 300, 300 }, 10.0, ng::pen{ ng::colour::Red4, 2.0 });
			aGc.fill_rounded_rect(ng::rect{ 500, 500, 200, 200 }, 10.0, 
				ng::gradient{ { { 0.0, ng::colour::Black },{ 0.33, ng::colour::Yellow },{ 0.66, ng::colour::Red },{ 1.0, ng::colour::White } }, ng::gradient::Radial });
			aGc.draw_rounded_rect(ng::rect{ 500, 500, 200, 200 }, 10.0, ng::pen{ ng::colour::Black, 1.0 });
			aGc.fill_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::colour::Chocolate);
			aGc.draw_arc(ng::point{ 500, 50 }, 75, 0.0, ng::to_rad(45.0), ng::pen{ ng::colour::White, 3.0 });
			aGc.draw_arc(ng::point{ 500, 50 }, 50, ng::to_rad(5.0), ng::to_rad(40.0), ng::pen{ ng::colour::Yellow, 3.0 });
		});
		
		auto& tabEditor = tabContainer.add_tab_page("Editor").widget();
		ng::vertical_layout layoutEditor(tabEditor);
		ng::text_edit textEdit2(layoutEditor);
		textEdit2.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
		ng::push_button editorStyle1(layoutEditor, "Style 1");
		editorStyle1.clicked([&textEdit2]()
		{
			textEdit2.set_default_style(ng::text_edit::style(ng::optional_font(), ng::gradient(ng::colour::DarkGoldenrod, ng::colour::LightGoldenrodYellow, ng::gradient::Horizontal), ng::text_edit::style::colour_type()));
		});
		ng::push_button editorStyle2(layoutEditor, "Style 2");
		editorStyle2.clicked([&textEdit2]()
		{
			textEdit2.set_default_style(ng::text_edit::style(ng::font("SnareDrum One NBP", "Regular", 60.0), ng::colour::White));
		});

		tabContainer.add_tab_page("Foo").tab().set_image(smallHash);
		tabContainer.add_tab_page("Bar").tab().set_image(smallHash);
		tabContainer.add_tab_page("Baz").tab().set_image(smallHash);
		
		return app.exec();
	}
	catch (std::exception& e)
	{
		app.halt();
		std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), std::string("main: terminating with exception: ") + e.what());
		std::exit(EXIT_FAILURE);
	}
	catch (...)
	{
		app.halt();
		std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
		app.surface_manager().display_error_message(app.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + app.name(), "main: terminating with unknown exception");
		std::exit(EXIT_FAILURE);
	}
}

