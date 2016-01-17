namespace glsl
{

	const char* const SUBPIXEL_RGB_HORIZONTAL_FRAG =
	"#version 130\n"
	"\n"
	"uniform sampler2D glyphTexture;\n"
	"uniform vec2 glyphTextureExtents;\n"
	"in vec4 Color;\n"
	"out vec4 FragColor;\n"
	"varying vec2 vGlyphTexCoord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	FragColor = vec4(Color.xyz, Color.a * texture(glyphTexture, vec2(vGlyphTexCoord.x, vGlyphTexCoord.y)).a);\n"
	"}\n";

}
