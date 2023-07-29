vec4 gradient_color(in float n)
{
    n = max(min(n, 1.0), 0.0) * float(uGradientColorCount - 1);
    vec4 firstColor = texelFetch(uGradientColors, ivec2(floor(n), uGradientColorRow));
    vec4 secondColor = texelFetch(uGradientColors, ivec2(ceil(n), uGradientColorRow));
    return mix(firstColor, secondColor, n - floor(n));
}
                
float ellipse_radius(vec2 ab, vec2 center, vec2 pt)
{
    vec2 d = pt - center;
    float angle = 0;
    vec2 ratio = vec2(1.0, 1.0);
    if (ab.x >= ab.y)
        ratio.y = ab.x / ab.y;
    else
        ratio.x = ab.y / ab.x;
    angle = atan(d.y * ratio.y, d.x * ratio.x);
    float x = pow(abs(cos(angle)), 2.0 / uGradientExponents.x) * sign(cos(angle)) * ab.x;
    float y = pow(abs(sin(angle)), 2.0 / uGradientExponents.y) * sign(sin(angle)) * ab.y;
    return sqrt(x * x + y * y);
}
                
vec4 color_at(vec2 viewPos, vec4 boundingBox)
{
    vec2 s = boundingBox.zw - boundingBox.xy;
    vec2 pos = viewPos - boundingBox.xy;
    if (uGradientTile)
    {
        if (uGradientDirection != 1) /* vertical */
        {
            float adjust = 0;
            if (uGradientTileParams.z == 1)
                adjust = float(int(boundingBox.y) % uGradientTileParams.y);
            float frac = 1 / float(uGradientTileParams.y);
            pos.y = floor((float(int(pos.y + adjust) % uGradientTileParams.y) * frac + frac / 2) * s.y);
        }
        if (uGradientDirection != 0) /* horizontal */
        {
            float adjust = 0;
            if (uGradientTileParams.z == 1)
                adjust = float(int(boundingBox.x) % uGradientTileParams.x);
            float frac = 1 / float(uGradientTileParams.x);
            pos.x = floor((float(int(pos.x + adjust) % uGradientTileParams.x) * frac + frac / 2) * s.x);
        }
    }
    pos.x = max(min(pos.x, s.x - 1.0), 0.0);
    pos.y = max(min(pos.y, s.y - 1.0), 0.0);
    float gradientPos;
    if (uGradientDirection == 0) /* vertical */
    {
        gradientPos = pos.y / s.y;
        if (!uGradientGuiCoordinates)
            gradientPos = 1.0 - gradientPos;
    }
    else if (uGradientDirection == 1) /* horizontal */
        gradientPos = pos.x / s.x;
    else if (uGradientDirection == 2) /* diagonal */
    {
        vec2 center = s / 2.0;
        float angle;
        switch (uGradientStartFrom)
        {
        case 0:
            angle = atan(center.y, -center.x);
            break;
        case 1:
            angle = atan(-center.y, -center.x);
            break;
        case 2:
            angle = atan(-center.y, center.x);
            break;
        case 3:
            angle = atan(center.y, center.x);
            break;
        default:
            angle = uGradientAngle;
            break;
        }
        pos.y = s.y - pos.y;
        pos = pos - center;
        mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
        pos = rot * pos;
        pos = pos + center;
        gradientPos = pos.y / s.y;
    }
    else if (uGradientDirection == 3) /* rectangular */
    {
        float vert = pos.y / s.y;
        if (vert > 0.5)
            vert = 1.0 - vert;
        float horz = pos.x / s.x;
        if (horz > 0.5)
            horz = 1.0 - horz;
        gradientPos = min(vert, horz) * 2.0;
    }
    else /* radial */
    {
        vec2 ab = s / 2.0;
        pos -= ab;
        vec2 center = ab * uGradientCenter;
        float d = distance(center, pos);
        vec2 c1 = boundingBox.xy - boundingBox.xy - ab;
        vec2 c2 = boundingBox.xw - boundingBox.xy - ab;
        vec2 c3 = boundingBox.zw - boundingBox.xy - ab;
        vec2 c4 = boundingBox.zy - boundingBox.xy - ab;
        vec2 cc = c1;
        if (distance(center, c2) < distance(center, cc))
            cc = c2;
        if (distance(center, c3) < distance(center, cc))
            cc = c3;
        if (distance(center, c4) < distance(center, cc))
            cc = c4;
        vec2 fc = c1;
        if (distance(center, c2) > distance(center, fc))
            fc = c2;
        if (distance(center, c3) > distance(center, fc))
            fc = c3;
        if (distance(center, c4) > distance(center, fc))
            fc = c4;
        vec2 cs = vec2(min(abs(-ab.x + center.x), abs(ab.x + center.x)), min(abs(-ab.y + center.y), abs(ab.y + center.y)));
        vec2 fs = vec2(max(abs(-ab.x + center.x), abs(ab.x + center.x)), max(abs(-ab.y + center.y), abs(ab.y + center.y)));
        float r;
        if (uGradientShape == 0) // Ellipse
        {
            switch (uGradientSize)
            {
            default:
            case 0: // ClosestSide
                r = ellipse_radius(cs, center, pos);
                break;
            case 1: // FarthestSide
                r = ellipse_radius(fs, center, pos);
                break;
            case 2: // ClosestCorner
                r = ellipse_radius(abs(cc - center), center, pos);
                break;
            case 3: // FarthestCorner
                r = ellipse_radius(abs(fc - center), center, pos);
                break;
            }
        }
        else if (uGradientShape == 1) // Circle
        {
            switch (uGradientSize)
            {
            default:
            case 0:
                r = min(cs.x, cs.y);
                break;
            case 1:
                r = max(fs.x, fs.y);
                break;
            case 2:
                r = distance(cc, center);
                break;
            case 3:
                r = distance(fc, center);
                break;
            }
        }
        if (d < r)
            gradientPos = d / r;
        else
            gradientPos = 1.0;
    }
    return gradient_color(gradientPos);
}
                
void standard_gradient_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uGradientEnabled)
    {
        int d = uGradientFilterSize / 2;
        if (texelFetch(uGradientFilter, ivec2(d, d)).r == 1.0)
        {
            color = color_at(Coord.xy, function0) * vec4(1.0, 1.0, 1.0, color.a);  
        }
        else
        {
            vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
            for (int fy = -d; fy <= d; ++fy)
            {
                for (int fx = -d; fx <= d; ++fx)
                {
                    sum += (color_at(Coord.xy + vec2(fx, fy), function0) * texelFetch(uGradientFilter, ivec2(fx + d, fy + d)).r);
                }
            }
            color = sum * vec4(1.0, 1.0, 1.0, color.a); 
        }
    }
}
