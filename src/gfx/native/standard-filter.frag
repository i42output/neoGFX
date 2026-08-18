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
        case 2: // effect: GaussianBlur2D
            {
                int d = uFilterKernelSize / 2;
                vec4 sum = vec4(0.0);
                float weightSum = 0.0;
                for (int y = -d; y <= d; ++y)
                {
                    for (int x = -d; x <= d; ++x)
                    {
                        float w = texelFetch(uFilterKernel, ivec2(x + d, y + d)).r;
                        sum += texel_at(TexCoord + vec2(float(x), float(y)) / uTextureExtents) * w;
                        weightSum += w;
                    }
                }
                color = sum / weightSum;
            }
            break;
        case 3: // filter: Dilate (separable morphological max)
            {
                int taps = int(uFilterArguments.x);
                vec2 stepUv = vec2(uFilterArguments.y, uFilterArguments.z) / uTextureExtents;
                // No bounds test: dilate_filter::outset() is the exact per-axis
                // reach of the whole pass sequence, and scoped_filter clears the
                // buffers to outset() + 1, so every texel a tap can reach is
                // already cleared.
                // combined_texel_at, not texel_at: texel_at resolves to
                // texelFetch(.., gl_SampleID) and max() of a single arbitrary
                // sample varies per invocation at a partially covered edge. A
                // Gaussian tolerates that because it averages many taps; max
                // does not.
                vec4 best = combined_texel_at(TexCoord);
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
                color = best;
            }
            break;
        }
    }
}
