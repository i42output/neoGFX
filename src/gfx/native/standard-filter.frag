void standard_filter_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
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
                if (texelFetch(uFilterKernel, ivec2(d, d)).r != 1.0)
                {
                    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
                    for (int fy = -d; fy <= d; ++fy)
                    {
                        for (int fx = -d; fx <= d; ++fx)
                        {
                            vec4 texel = texel_at(TexCoord + vec2(fx, fy) / uTextureExtents); 
                            sum += (texel * texelFetch(uFilterKernel, ivec2(fx + d, fy + d)).r);
                        }
                    }
                    color = sum;
                }
            }
            break;
        }
    }
}
