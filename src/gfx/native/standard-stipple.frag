void standard_stipple_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uStippleEnabled)
    {
        float d = 0.0;
        if (!uShapeEnabled)
            d = distance(uStippleVertex, Coord);
        else if (uShape == SHAPE_Circle)
        {
            float y = Coord.y - function1.y;
            float x = Coord.x - function1.x;
            d = (atan(y, x) + PI) * function1.z;
        }
        uint patternBit = uint((d + uStipplePosition) / uStippleFactor) % 16;
        if ((uStipplePattern & (1 << patternBit)) == 0)
            discard;
    }
}
