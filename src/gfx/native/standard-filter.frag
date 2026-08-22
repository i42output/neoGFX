void standard_filter_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (uFilterEnabled)
    {
        switch(uFilterType)
        {
        case FILTER_None:
            break;
        case FILTER_GaussianBlur:
            {
                int d = uFilterKernelSize / 2;
                vec2 axis = uFilterPass == 0 ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
                vec4 sum = vec4(0.0);
                float weightSum = 0.0;
                for (int f = -d; f <= d; ++f)
                {
                    float w = texelFetch(uFilterKernel, ivec2(f + d, 0)).r;
                    vec4 texel = texel_at(TexCoord + (axis * float(f)) / uTextureExtents);
                    sum += texel * w;
                    weightSum += w;
                }
                color = sum / weightSum;
            }
            break;
        case FILTER_GaussianBlur2D:
            {
                int d = uFilterKernelSize / 2;
                vec4 sum = vec4(0.0);
                float weightSum = 0.0;
                for (int y = -d; y <= d; ++y)
                {
                    for (int x = -d; x <= d; ++x)
                    {
                        float w = texelFetch(uFilterKernel, ivec2(x + d, y + d)).r;
                        vec4 texel = texel_at(TexCoord + vec2(float(x), float(y)) / uTextureExtents);
                        sum += texel * w;
                        weightSum += w;
                    }
                }
                color = sum / weightSum;
            }
            break;
        case FILTER_DilateOctagon:
            {
                int taps = int(uFilterArguments.x);
                vec2 stepUv = vec2(uFilterArguments.y, uFilterArguments.z) / uTextureExtents;
                vec4 best = combined_texel_at(TexCoord);
                if (best.a != 1.0)
                {
                    for (int f = 1; f <= taps; ++f)
                    {
                        vec2 o = stepUv * float(f);
                        vec4 s0 = combined_texel_at(TexCoord - o);
                        vec4 s1 = combined_texel_at(TexCoord + o);
                        if (s0.a > best.a)
                            best = s0;
                        if (s1.a > best.a)
                            best = s1;
                    }
                }
                color = best;
            }
            break;
        case FILTER_DilateDisk:
            {
                vec4 best = combined_texel_at(TexCoord);
                if (best.a != 1.0)
                {
                    float r = uFilterArguments.x;
                    vec2 stepUv = vec2(1.0) / uTextureExtents;
                    for (float x = -r; x <= r; x += 1.0)
                    {
                        for (float y = -r; y <= r; y += 1.0)
                        {
                            if (x * x + y * y > r * r)
                                continue;
                            vec4 s = combined_texel_at(TexCoord + vec2(x, y) * stepUv);
                            if (s.a > best.a)
                                best = s;
                        }
                    }
                }
                color = best;
            }
            break;
        }
    }
}
