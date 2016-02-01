#include <neolib/neolib.hpp>
#include <boost/lexical_cast.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/push_button.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/horizontal_layout.hpp>
#include <neogfx/grid_layout.hpp>
#include <neogfx/spacer.hpp>
#include <neogfx/table_view.hpp>
#include <neogfx/check_box.hpp>
#include <neogfx/default_item_model.hpp>
#include <neogfx/item_presentation_model.hpp>
#include <neogfx/i_surface.hpp>

namespace ng = neogfx;

class my_item_model : public ng::basic_default_item_model<void*, 5>, public ng::item_presentation_model
{
public:
	my_item_model()
	{
		set_column_heading_text(0, "One");
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
	keypad_button(uint32_t aNumber) : 
		ng::push_button(boost::lexical_cast<std::string>(aNumber))
	{
		pressed.subscribe([aNumber]()
		{
			ng::app::instance().change_style("Keypad").set_colour(ng::colour(aNumber & 1 ? 64 : 0, aNumber & 2 ? 64 : 0, aNumber & 4 ? 64 : 0));
		});
	}
};

int main(int argc, char* argv[])
{
	ng::app app("neoGFX Test App");
	app.change_style("Default").set_font_info(ng::font_info("Segoe UI", std::string("Semibold"), 12));
	app.change_style("Slate").set_font_info(ng::font_info("Segoe UI", std::string("Semibold"), 12));
	app.register_style(ng::style("Keypad")).set_font_info(ng::font_info("Segoe UI", std::string("Semibold"), 12));
	app.change_style("Default");
	ng::window window(800, 800);
	ng::vertical_layout layout1(window); 
	layout1.set_margins(ng::margins(8));
	ng::push_button button0(layout1, "This is the neoGFX test application.");
	button0.set_foreground_colour(ng::colour::LightGoldenrodYellow);
	ng::push_button button1(layout1, "the,,, quick brown fox jumps over the lazy dog");
	button1.set_foreground_colour(ng::colour::LightGoldenrod);
	ng::push_button button2(layout1, u8"ويقفز الثعلب البني السريع فوق الكلب الكسول");
	button2.set_foreground_colour(ng::colour::Goldenrod);
	ng::push_button button3(layout1, u8"שועל חום קפיצות מעל הכלב העצלן");
	button3.set_foreground_colour(ng::colour::DarkGoldenrod);
	button3.set_minimum_size(ng::size(128, 64));
	ng::push_button button4(layout1, u8"请停止食用犬");
	button4.set_foreground_colour(ng::colour::CadetBlue);
	button4.set_maximum_size(ng::size(128, 64));
	ng::push_button button5(layout1, u8"sample text نص عينة sample text טקסט לדוגמא 示例文本 sample text");
	ng::table_view tableView(layout1);
	tableView.set_minimum_size(ng::size(128, 128));
	ng::horizontal_layout layout2(layout1);
	ng::label label1(layout2, "Label 1:");
	ng::push_button button6(layout2, "RGB <-> HSL\ncolour space\nconversion test");
	button6.set_minimum_size(ng::size(128, 64));
	layout2.add_spacer().set_weight(ng::size(2.0f));
	ng::push_button button7(layout2, "button7");
	button7.set_foreground_colour(ng::colour::LightCoral);
	button7.set_maximum_size(ng::size(128, 64));
	layout2.add_spacer().set_weight(ng::size(1.0));
	ng::push_button button8(layout2, "Multi-line\ntext.");
	button8.set_foreground_colour(ng::colour(255, 235, 160));
	ng::horizontal_layout layout3(layout1);
	std::srand(4242);
	for (uint32_t i = 0; i < 10; ++i)
	{
		layout3.add_widget(std::make_shared<ng::push_button>(std::string(1, 'A' + i)));
		ng::colour randomColour = ng::colour(std::rand() % 256, std::rand() % 256, std::rand() % 256);
		layout3.get_widget(i).set_foreground_colour(randomColour);
	}
	ng::vertical_layout layoutCheckboxes(layout2);
	ng::check_box noSelection(layoutCheckboxes, "No selection");
	ng::check_box singleSelection(layoutCheckboxes, "Single selection");
	ng::check_box multipleSelection(layoutCheckboxes, "Multiple selection");
	ng::check_box extendedSelection(layoutCheckboxes, "Extended selection");
	ng::check_box triState(layoutCheckboxes, "Tristate checkbox", ng::check_box::TriState);
	triState.checked.subscribe([&triState]()
	{
		static uint32_t n;
		if ((n++)%2 == 1)
			triState.set_indeterminate();
	});
	ng::vertical_spacer spacerCheckboxes(layoutCheckboxes);
	ng::vertical_layout layout4(layout2);
	ng::push_button button9(layout4, "Default/Slate\nStyle");
	button9.pressed.subscribe([&app]()
	{
		if (app.current_style().name() == "Default")
			app.change_style("Slate");
		else
			app.change_style("Default");
	});
	button9.set_foreground_colour(ng::colour::Aquamarine);
	ng::push_button button10(layout4, "Toggle List\nHeader View");
	button10.pressed.subscribe([&tableView]()
	{
		if (tableView.column_header().visible())
			tableView.column_header().hide();
		else
			tableView.column_header().show();
	});
	ng::horizontal_layout layout5(layout4);
	ng::push_button buttonMinus(layout5, "-");
	ng::push_button buttonPlus(layout5, "+");
	buttonMinus.pressed.subscribe([&app]()
	{
		app.current_style().set_font_info(app.current_style().font_info().with_size(app.current_style().font_info().size() - 0.1f));
	});
	buttonPlus.pressed.subscribe([&app]()
	{
		app.current_style().set_font_info(app.current_style().font_info().with_size(app.current_style().font_info().size() + 0.1f));
	});
	ng::vertical_spacer spacer1(layout4);
	ng::grid_layout keypad(4, 3, layout2);
	keypad.set_minimum_size(ng::size(100.0, 0.0));
	keypad.set_spacing(0.0);
	for (uint32_t row = 0; row < 3; ++row)
		for (uint32_t col = 0; col < 3; ++col)
			keypad.add_widget(row, col, std::make_shared<keypad_button>(row * 3 + col + 1));
	keypad.add_widget(3, 1, std::make_shared<keypad_button>(0));

	app.surface_manager().surface(0).save_mouse_cursor();
	app.surface_manager().surface(0).set_mouse_cursor(ng::mouse_system_cursor::Wait);

	my_item_model itemModel;
	#ifdef NDEBUG
	itemModel.reserve(10000);
	#else
	itemModel.reserve(100);
	#endif
	for (uint32_t row = 0; row < itemModel.capacity(); ++row)
	{
		if (row % 1000 == 0)
			app.process_events();
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

	neolib::callback_timer animation(app, [&](neolib::callback_timer& aTimer)
	{
		aTimer.again();
		std::srand(static_cast<unsigned int>(app.program_elapsed_ms() / 5000));
		const double PI = 2.0 * std::acos(0.0);
		double lightness = ::sin((app.program_elapsed_ms() / 16 % 360) * (PI / 180.0)) / 2.0 + 0.5;
		ng::colour randomColour = ng::colour(std::rand() % 256, std::rand() % 256, std::rand() % 256);
		randomColour = randomColour.to_hsl().with_lightness(lightness).to_rgb();
		button6.set_foreground_colour(randomColour);
	}, 16);

	return app.exec();
}

