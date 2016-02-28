/*#include <neogfx/neogfx.hpp>
#include <neogfx/app.hpp>
#include <neogfx/window.hpp>
#include <neogfx/vertical_layout.hpp>
#include <neogfx/sprite.hpp>*/

#include <neogfx/numerical.hpp>

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

template class ng::basic_vector<double, 3>;

int main()
{
//	ng::app app("neopede - neoGFX Sample Application");
//	ng::window window(800, 800);
//	ng::vertical_layout layout0(window);

	ng::vector3 v1(1.0, 2.0, 3.0);
	ng::vector3 v2 = v1.yzy;
	ng::vector2 v3 = v2.xx;
	v2.yz = ng::vector2(42.0, 43.0);

//	return app.exec();
}