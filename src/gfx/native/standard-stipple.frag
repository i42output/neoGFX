void standard_stipple_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (uStippleEnabled)
    {
        vec2 pixelPos = Coord.xy; // note: pixel position not fragment position
        float d = 0.0;
        if (!uShapeEnabled)
            d = distance(uStippleVertex.xy, pixelPos);
        else
        {
            if (function3.w == 0.0)
                return;
            if (uShape == SHAPE_Rect)
            {
                vec2 p0 = pixelPos - function1.xy;
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
                float y = pixelPos.y - function1.y;
                float x = pixelPos.x - function1.x;
                d = (atan(y, x) + PI) * function1.z;
            }
            else if (uShape == SHAPE_Polygon)
            {
                const uint num = uint(function1.y) - uint(function1.x);
                const uint offset = uint(function1.x);
                uint closest = 0;
                float dd = dot(pixelPos-bShapeVertices[0 + offset].xy,pixelPos-bShapeVertices[0 + offset].xy);
                float s = 1.0;
                for( uint i=0, j=num-1; i<num; j=i, i++ )
                {
                    // distance
                    vec2 e = bShapeVertices[j + offset].xy - bShapeVertices[i + offset].xy;
                    vec2 w =    pixelPos - bShapeVertices[i + offset].xy;
                    vec2 b = w - e*clamp( dot(w,e)/dot(e,e), 0.0, 1.0 );
                    if (dot(b,b) < dd)
                        closest = i;
                    dd = min(dd, dot(b,b));
                    // winding number from http://geomalgorithms.com/a03-_inclusion.html
                    bvec3 cond = bvec3( pixelPos.y>=bShapeVertices[i + offset].y, 
                                        pixelPos.y <bShapeVertices[j + offset].y, 
                                        e.x*w.y>e.y*w.x );
                    if( all(cond) || all(not(cond)) ) s=-s;  
                }
                if (abs(s*sqrt(dd)) > function3.w / 2.0)
                    return;
                d = 0.0;
                if (closest > 0)
                    for (uint i = 0; i != closest - 1; ++i)
                        d += distance(bShapeVertices[i + offset].xy, bShapeVertices[i + 1 + offset].xy);
                if (closest != 0)
                    d += distance(bShapeVertices[closest - 1 + offset].xy, pixelPos);
                else
                    d += distance(bShapeVertices[closest + offset].xy, pixelPos);
            }
        }
        d = mod(d + uStipplePosition, uStipplePatternLength);
        bool draw = true;
        float offset = 0.0;
        for (uint i = 0; i != uStipplePatternSize; ++i)
        {
            if (d < offset + uStipplePattern[i])
            {
                if (!draw)
                    discard;
                break;
            }
            offset += uStipplePattern[i];
            draw = !draw;
        }
    }
}
