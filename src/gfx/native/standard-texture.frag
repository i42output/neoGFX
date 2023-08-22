vec4 texel_at(vec2 texCoord, int sampleId)
{
    vec4 texel = vec4(0.0);
    if (uTextureMultisample < 5) // Scaled
    {
        texel = texture(tex, texCoord).rgba;
    }
    else
    {
        texel = texelFetch(texMS, ivec2(texCoord * uTextureExtents), sampleId).rgba;
    }
    switch(uTextureDataFormat)
    {
    case 1: // RGBA
    default:
        break;
    case 2: // Red
        texel = vec4(1.0, 1.0, 1.0, texel.r);
        break;
    case 3: // SubPixel
        texel = vec4(1.0, 1.0, 1.0, (texel.r + texel.g + texel.b) / 3.0);
        break;
    }
    return texel;
}
vec4 texel_at(vec2 texCoord)
{
    return texel_at(texCoord, gl_SampleID);
}
vec4 combined_texel_at(vec2 texCoord)
{
    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < gl_NumSamples; ++i)
    {
        sum += texel_at(texCoord, i);
    }
    return sum / float(gl_NumSamples);
}
void standard_texture_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4)
{
    if (uTextureEnabled)
    {
        vec4 texel = texel_at(TexCoord);
        switch(uTextureEffect)
        {
        case 0: // effect: None
            color = texel.rgba * color;
            break;
        case 1: // effect: Colorize, ColorizeAverage
            {
                float avg = (texel.r + texel.g + texel.b) / 3.0;
                color = vec4(avg, avg, avg, texel.a) * color;
            }
            break;
        case 2: // effect: ColorizeMaximum
            {
                float maxChannel = max(texel.r, max(texel.g, texel.b));
                color = vec4(maxChannel, maxChannel, maxChannel, texel.a) * color;
            }
            break;
        case 3: // effect: ColorizeSpot
            color = vec4(1.0, 1.0, 1.0, texel.a) * color;
            break;
        case 4: // effect: ColorizeAlpha
            {
                float avg = (texel.r + texel.g + texel.b) / 3.0;
                color = vec4(1.0, 1.0, 1.0, texel.a * avg) * color;
            }
            break;
        case 5: // effect: Monochrome
            {
                float gray = dot(color.rgb * texel.rgb, vec3(0.299, 0.587, 0.114));
                color = vec4(gray, gray, gray, texel.a) * color;
            }
            break;
        case 10: // effect: Filter
            break;
        case 99: // effect: Ignore
            break;
        }
    }
}
