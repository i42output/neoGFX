void standard_stipple_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4)
{
    if (uStippleEnabled)
    {
        float d = 0.0;
        if (!uShapeEnabled)
            d = distance(uStippleVertex, Coord);
        else
        {
            if (function3.w == 0.0)
                return;
            if (uShape == SHAPE_Rect)
            {
                vec2 p0 = Coord.xy - function1.xy;
                vec2 b0 = function1.zw * 0.5;
                if (p0.y < 0.0 && abs(p0.y + b0.y) < function3.w / 2.0)
                    d = ceil(b0.x + p0.x);
                else if (p0.x > 0.0 && abs(p0.x - b0.x) <= function3.w / 2.0)
                    d = ceil(function1.z + b0.y + p0.y);
                else if (p0.y > 0.0 && abs(p0.y - b0.y) <= function3.w / 2.0)
                    d = floor(function1.z + function1.w + b0.x - p0.x);
                else if (p0.x < 0.0 && abs(p0.x + b0.x) <= function3.w / 2.0)
                    d = floor(function1.z + function1.w + function1.z + b0.y - p0.y); 
                else
                    discard;
            }
            else if (uShape == SHAPE_Circle)
            {
                float y = Coord.y - function1.y;
                float x = Coord.x - function1.x;
                d = (atan(y, x) + PI) * function1.z;
            }
        }
        uint patternBit = uint((d + uStipplePosition) / uStippleFactor) % 16;
        if ((uStipplePattern & (1 << patternBit)) == 0)
            discard;
    }
}
