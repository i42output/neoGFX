#version 130

uniform sampler2D glyphTexture;
uniform vec2 glyphTextureExtents;
in vec4 Color;
out vec4 FragColor;
varying vec2 vGlyphTexCoord;

void main()
{
	FragColor = vec4(Color.xyz, Color.a * texture(glyphTexture, vec2(vGlyphTexCoord.x, vGlyphTexCoord.y)).a);
}
