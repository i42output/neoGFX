ivec2 render_position()
{
    if (uGlyphGuiCoordinates)
        return ivec2(Coord.x, uGlyphRenderTargetExtents.y - Coord.y);
    else
        return ivec2(Coord.xy);
}
                
vec3 output_pixel()
{
    return texelFetch(uGlyphRenderOutput, render_position(), 0).rgb;
}
                
void standard_glyph_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uGlyphEnabled)
    {
        float a = 0.0;
        if (uGlyphSubpixel)
        {
            vec4 aaaAlpha = texture(tex, TexCoord);
            if (aaaAlpha.rgb == vec3(1.0, 1.0, 1.0))
                return;
            else if (aaaAlpha.rgb == vec3(0.0, 0.0, 0.0))
                discard;
            else
            {
                switch(uGlyphSubpixelFormat)
                {
                default:
                    a = (aaaAlpha.r + aaaAlpha.g + aaaAlpha.b) / 3.0;
                    color = vec4(color.xyz, color.a * a);
                    break;
                case 1: // RGBHorizontal
                    color = vec4(color.rgb * aaaAlpha.rgb * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.rgb * color.a), 1.0);
                    break;
                case 2: // BGRHorizontal
                    color = vec4(color.rgb * aaaAlpha.bgr * color.a + output_pixel() * (vec3(1.0, 1.0, 1.0) - aaaAlpha.bgr * color.a), 1.0);
                    break;
                }
            }
        }
        else
        {
            a = texture(tex, TexCoord).r;
            if (a == 0)
                discard;
            color = vec4(color.xyz, color.a * a);
        }
    }
}
