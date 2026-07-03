void standard_filter_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (uFilterEnabled)
    {
        switch(uFilterType)
        {
        case 0: // filter: None
            break;
        case 1: // effect: GaussianBlur
            {
                int d = uFilterKernelSize / 2;
                vec2 axis = uFilterPass == 0 ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
                vec4 sum = vec4(0.0);
                float weightSum = 0.0;
                for (int f = -d; f <= d; ++f)
                {
                    float w = texelFetch(uFilterKernel, ivec2(f + d, 0)).r;
                    sum += texel_at(TexCoord + (axis * float(f)) / uTextureExtents) * w;
                    weightSum += w;
                }
                color = sum / weightSum;
            }
            break;
        }
    }
}
