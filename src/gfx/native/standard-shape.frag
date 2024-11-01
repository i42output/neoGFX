// standard-shape.frag
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Parts hereof...
// The MIT License
// Copyright © 2019 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the Software), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

float spot_alpha(float a, float aliasThreshold)
{
    if (a != 0.0 && aliasThreshold == 0.0)
        a = 1.0;
    return a;
}

vec4 shape_color(float d0, vec4 color, float outlineCount, float outlineWidth, float aliasThreshold, vec4 outerColor, vec4 innerColor)
{
    float a = spot_alpha(1.0 - smoothstep(-0.5, 0.5, d0), aliasThreshold);
    color = vec4(color.xyz, color.a * a);
    if (outlineCount == 1.0)
    {
        if (aliasThreshold != 0.0)
        {
            float a = spot_alpha(1.0 - smoothstep(outlineWidth / 2.0, outlineWidth / 2.0 + aliasThreshold, abs(d0)), aliasThreshold);
            if (d0 < 0.0)
                color = mix(color, outerColor, a);
            else
                color = mix(vec4(outerColor.xyz, 0.0), outerColor, a);
        }
        else
        {
            if (d0 >= -outlineWidth / 2.0 && d0 <= outlineWidth / 2.0)
                color = outerColor;
            else if (d0 > outlineWidth / 2.0)
                discard;
        }
    }
    else if (outlineCount == 2.0)
    {
        if (aliasThreshold != 0.0)
        {
            float a = spot_alpha(1.0 - smoothstep(-(outlineWidth / 2.0 + aliasThreshold), outlineWidth / 2.0 + aliasThreshold, d0), aliasThreshold);
            color = mix4(vec4(outerColor.xyz, 0.0), outerColor, innerColor, color, a);           
        }
        else
        {
            if (d0 < 0.0 && d0 >= -outlineWidth / 2.0)
                color = innerColor;
            else if (d0 >= 0.0 && d0 <= outlineWidth / 2.0)
                color = outerColor;
            else if (d0 > outlineWidth / 2.0)
                discard;
        }
    }
    return color;
}

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

void draw_line(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdSegment(fragPos, function1.xy, function1.zw);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
    {
        float a = spot_alpha(1.0 - smoothstep(function3.y == 0.0 ? 0.5 : 0.0, function3.w / 2.0, abs(d0)), function3.y);
        color = vec4(color.xyz, color.a * a);
    }
}

float bezier_sdSegmentSq(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa,ba) / dot(ba,ba), 0.0, 1.0);
    return dot(pa - ba * h, pa - ba * h);
}
                
float bezier_sdSegment(vec2 p, vec2 a, vec2 b)
{
    return sqrt(bezier_sdSegmentSq(p,a,b));
}
                
// todo: research a non-iterative approach
float bezier_udBezier(vec2 p0, vec2 p1, vec2 p2, vec2 p3, vec2 pos)
{
    const int kNum = 50;
    float res = 1e10;
    vec2 a = p0;
    for (int i = 1; i < kNum; i++)
    {
        float t = float(i) / float(kNum - 1);
        float s = 1.0 - t;
        vec2 b = p0 * s * s * s + p1 * 3.0 * s * s * t + p2 * 3.0 * s * t * t + p3 * t * t * t;
        float d = bezier_sdSegmentSq(pos, a, b);
        if (d < res) res = d;
        a = b;
    }
    return sqrt(res);
}

void draw_cubic_bezier(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = bezier_udBezier(function1.xy, function1.zw, function2.xy, function2.zw, fragPos.xy);
    float a = spot_alpha(1.0 - smoothstep(function3.y == 0.0 ? 0.5 : 0.0, function3.w / 2.0, abs(d0)), function3.y);
    color = vec4(color.xyz, color.a * a);
}

float sdTriangle(vec2 p, vec2 p0, vec2 p1, vec2 p2)
{
    vec2 e0 = p1 - p0, e1 = p2 - p1, e2 = p0 - p2;
    vec2 v0 = p - p0, v1 = p - p1, v2 = p - p2;
    vec2 pq0 = v0 - e0 * clamp(dot(v0,e0) / dot(e0,e0), 0.0, 1.0);
    vec2 pq1 = v1 - e1 * clamp(dot(v1,e1) / dot(e1,e1), 0.0, 1.0);
    vec2 pq2 = v2 - e2 * clamp(dot(v2,e2) / dot(e2,e2), 0.0, 1.0);
    float s = sign(e0.x * e2.y - e0.y * e2.x);
    vec2 d = min(min(vec2(dot(pq0,pq0), s * (v0.x * e0.y - v0.y * e0.x)),
                        vec2(dot(pq1,pq1), s * (v1.x * e1.y - v1.y * e1.x))),
                        vec2(dot(pq2,pq2), s * (v2.x * e2.y - v2.y * e2.x)));
    return -sqrt(d.x) * sign(d.y);
}

void draw_triangle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdTriangle(fragPos.xy, function1.xy, function1.zw, function2.xy);
    color = vec4(color.xyz, color.a * (1.0 - smoothstep(-0.5, 0.5, d0)));
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdBox(vec2 p, vec2 b)
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

void draw_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdBox(fragPos.xy - function1.xy, function1.zw * 0.5);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}

void draw_circle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdCircle(fragPos.xy - function1.xy, function1.z);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdEllipse(vec2 p, vec2 ab)
{
    if (ab.x == ab.y)
        return sdCircle(p, ab.x);

    p = abs(p); if (p.x > p.y) { p = p.yx; ab = ab.yx; }
    float l = ab.y * ab.y - ab.x * ab.x;
    float m = ab.x * p.x / l;      float m2 = m * m;
    float n = ab.y * p.y / l;      float n2 = n * n;
    float c = (m2 + n2 - 1.0) / 3.0; float c3 = c * c * c;
    float q = c3 + m2 * n2 * 2.0;
    float d = c3 + m2 * n2;
    float g = m + m * n2;
    float co;
    if (d < 0.0)
    {
        float h = acos(q / c3) / 3.0;
        float s = cos(h);
        float t = sin(h) * sqrt(3.0);
        float rx = sqrt(-c * (s + t + 2.0) + m2);
        float ry = sqrt(-c * (s - t + 2.0) + m2);
        co = (ry + sign(l) * rx + abs(g) / (rx * ry) - m) / 2.0;
    }
    else
    {
        float h = 2.0 * m * n * sqrt(d);
        float s = sign(q + h) * pow(abs(q + h), 1.0 / 3.0);
        float u = sign(q - h) * pow(abs(q - h), 1.0 / 3.0);
        float rx = -s - u - c * 4.0 + 2.0 * m2;
        float ry = (s - u) * sqrt(3.0);
        float rm = sqrt(rx * rx + ry * ry);
        co = (ry / sqrt(rm - rx) + 2.0 * g / rm - m) / 2.0;
    }
    vec2 r = ab * vec2(co, sqrt(1.0 - co * co));
    return length(r - p) * sign(p.y - r.y);
}

void draw_ellipse(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdEllipse(fragPos.xy - function1.xy, function1.zw);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdPie(vec2 p, vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(l,m * sign(c.y * p.x - c.x * p.y));
}

void draw_pie(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (fragPos.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdArc(vec2 p, vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(-l,m * sign(c.y * p.x - c.x * p.y));
}

void draw_arc(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    vec4 noOutline = vec4(0.0);
    draw_pie(color, function1, function2, noOutline, function4, function5);

    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (fragPos.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    float d0 = sdArc(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else if (smoothstep(function3.w / 2.0, function3.w / 2.0 + function3.y, abs(d0)) != 1.0)
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}
                
float sdRoundedBox(vec2 p, vec2 b, vec4 r)
{
    r.xy = (p.x>0.0)?r.yz : r.xw;
    r.x  = (p.y>0.0)?r.y  : r.x;
    r.x  = min(r.x, min(b.x, b.y));
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}
                
void draw_rounded_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdRoundedBox(fragPos.xy - function1.xy, function1.zw * 0.5, function2);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdEllipseBox(vec2 f, vec2 c, vec2 b, vec4 rx, vec4 ry)
{
    vec2 p = f - c;
    if (rx.x + rx.y > b.x * 2.0)
    {
        rx.x *= (b.x * 2.0 / (rx.x + rx.y));
        rx.y *= (b.x * 2.0 / (rx.x + rx.y));
    }
    if (rx.w + rx.z > b.x * 2.0)
    {
        rx.w *= (b.x * 2.0 / (rx.w + rx.z));
        rx.z *= (b.x * 2.0 / (rx.w + rx.z));
    }
    if (ry.x + ry.w > b.y * 2.0)
    {
        ry.x *= (b.y * 2.0 / (ry.x + ry.w));
        ry.w *= (b.y * 2.0 / (ry.x + ry.w));
    }
    if (ry.y + ry.z > b.y * 2.0)
    {
        ry.y *= (b.y * 2.0 / (ry.y + ry.z));
        ry.z *= (b.y * 2.0 / (ry.y + ry.z));
    }
    vec2 tl = c + vec2(-b.x, -b.y) + vec2(rx.x, ry.x);
    vec2 tr = c + vec2(b.x, -b.y) + vec2(-rx.y, ry.y);
    vec2 br = c + vec2(b.x, b.y) + vec2(-rx.z, -ry.z);
    vec2 bl = c + vec2(-b.x, b.y) + vec2(rx.w, -ry.w);
    float d1 = (f.x < tl.x && f.y < tl.y) ? sdEllipse(f - tl, vec2(rx.x, ry.x)) : sdBox(p, b);
    float d2 = (f.x > tr.x && f.y < tr.y) ? sdEllipse(f - tr, vec2(rx.y, ry.y)) : sdBox(p, b);
    float d3 = (f.x > br.x && f.y > br.y) ? sdEllipse(f - br, vec2(rx.z, ry.z)) : sdBox(p, b);
    float d4 = (f.x < bl.x && f.y > bl.y) ? sdEllipse(f - bl, vec2(rx.w, ry.w)) : sdBox(p, b);
    return max(d1, max(d2, max(d3, d4)));
}

void draw_ellipse_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdEllipseBox(fragPos.xy, function1.xy, function1.zw * 0.5, function2, function6);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

float sdPolygon(in vec2 p, in vec2 meta)
{
    const uint num = uint(meta.y) - uint(meta.x);
    const uint offset = uint(meta.x);
    float d = dot(p-bShapeVertices[0 + offset].xy,p-bShapeVertices[0 + offset].xy);
    float s = 1.0;
    for( uint i=0, j=num-1; i<num; j=i, i++ )
    {
        // distance
        vec2 e = bShapeVertices[j + offset].xy - bShapeVertices[i + offset].xy;
        vec2 w =    p - bShapeVertices[i + offset].xy;
        vec2 b = w - e*clamp( dot(w,e)/dot(e,e), 0.0, 1.0 );
        d = min( d, dot(b,b) );

        // winding number from http://geomalgorithms.com/a03-_inclusion.html
        bvec3 cond = bvec3( p.y>=bShapeVertices[i + offset].y, 
                            p.y <bShapeVertices[j + offset].y, 
                            e.x*w.y>e.y*w.x );
        if( all(cond) || all(not(cond)) ) s=-s;  
    }
    
    return s*sqrt(d);
}

void draw_polygon(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));

    float d0 = sdPolygon(fragPos, function1.xy);
    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

void draw_checker_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5)
{
    vec2 fragPos = Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5));
    vec2 shapePos = fragPos.xy - function1.xy;
    
    float d0 = sdBox(shapePos, function1.zw * 0.5);

    if (d0 < function3.w / 2.0)
    {
        ivec2 checkerPos = ivec2(shapePos) + ivec2(function1.zw * 0.5);
        ivec2 checkerSize = ivec2(function2.xy);
        int row = checkerPos.y / checkerSize.y;
        int col = checkerPos.x / checkerSize.x;
        if ((row + col + 1) % 2 == int(function2.z))
            discard;
    }

    if (function3.y == 0.0 && function3.w != 0.0 && (d0 > function3.w / 2.0 || (color.a == 0.0 && abs(d0) > function3.w / 2.0)))
        discard;
    else
        color = shape_color(d0, color, function3.x, function3.w, function3.y, function4, function5);
}

void standard_shape_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (uShapeEnabled)
    {
        switch(uShape)
        {
        case SHAPE_Line:
            draw_line(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_CubicBezier:
            draw_cubic_bezier(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Triangle:
            draw_triangle(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Rect:
            draw_rect(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Circle:
            draw_circle(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Ellipse:
            draw_ellipse(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Pie:
            draw_pie(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_Arc:
            draw_arc(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_RoundedRect:
            draw_rounded_rect(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_EllipseRect:
            draw_ellipse_rect(color, function1, function2, function3, function4, function5, function6);
            break;
        case SHAPE_Polygon:
            draw_polygon(color, function1, function2, function3, function4, function5);
            break;
        case SHAPE_CheckerRect:
            draw_checker_rect(color, function1, function2, function3, function4, function5);
            break;
        }
    }
}
