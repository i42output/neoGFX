#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/sprite.hpp>

namespace ng = neogfx;

const uint8_t sSpaceShip[8][8]
{
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 1, 0, 0, 0, 0, 1, 0 },
};

int main()
{
	ng::app app("neopede - neoGFX Sample Application");
	ng::window window(800, 800);
	ng::vertical_layout layout0(window);
	return app.exec();
}