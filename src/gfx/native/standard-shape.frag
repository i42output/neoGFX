// Parts hereof...
// The MIT License
// Copyright © 2019 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the Software), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

void draw_line(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float dy = function1.y - function1.w;
    float dx = function1.x - function1.z;
    float m = dy / dx; // GLSL allows divide-by-zero, we won't use the Inf
    float c = function1.y - m * function1.x;
    float d = dx != 0.0 ? (abs(m) < 1.0 ? distance(vec2(Coord.x, m * Coord.x + c), Coord.xy) : distance(vec2((Coord.y - c) / m, Coord.y), Coord.xy)) : abs(Coord.x - function1.x);
    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, d)));
}

// https://www.shadertoy.com/view/XdVBWd
// The MIT License
// Copyright © 2018 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \Software\), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED \AS IS\, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
                
float bezier_sdSegmentSq(in vec2 p, in vec2 a, in vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa,ba) / dot(ba,ba), 0.0, 1.0);
    return dot(pa - ba * h, pa - ba * h);
}
                
float bezier_sdSegment(in vec2 p, in vec2 a, in vec2 b)
{
    return sqrt(bezier_sdSegmentSq(p,a,b));
}
                
// todo: research a non-iterative approach
float bezier_udBezier(vec2 p0, vec2 p1, vec2 p2, in vec2 p3, vec2 pos)
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
void draw_cubic_bezier(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = bezier_udBezier(function1.xy, function1.zw, function2.xy, function2.zw, Coord.xy + (gl_SamplePosition - vec2(0.5, 0.5)));
    color = vec4(color.xyz, color.a * (1.0 - smoothstep(function3.x / 2.0 - 0.5, function3.x / 2.0 + 0.5, d)));
}
float sdTriangle(in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2)
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
void draw_triangle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d0 = sdTriangle(Coord.xy, function1.xy, function1.zw, function2.xy);
    if (function2.w == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.z / 2.0 - 0.5, function2.z / 2.0 + 0.5, abs(d0))));
}
void draw_circle(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = distance(function1.xy, Coord.xy) - function1.z;
    if (function2.x == 1.0)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, d)));
    else if (abs(d) >= function1.w / 2.0 - 0.5)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function1.w / 2.0 - 0.5, function1.w / 2.0 + 0.5, abs(d))));
}
float sdEllipse(in vec2 p, in vec2 ab)
{
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
void draw_ellipse(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float d = sdEllipse(Coord.xy - function1.xy, function1.zw);
    if (function2.y == 1.0)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, d)));
    else if (abs(d) >= function2.x / 2.0 - 0.5)
        color = vec4(color.xyz, color.a * (1.0 - smoothstep(function2.x / 2.0 - 0.5, function2.x / 2.0 + 0.5, abs(d))));
}
float sdPie(in vec2 p, in vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(l,m * sign(c.y * p.x - c.x * p.y));
}
void draw_pie(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
    if (function2.z == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));
}
float sdArc(in vec2 p, in vec2 c, in float r)
{
    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p - c * clamp(dot(p,c),0.0,r));
    return max(-l,m * sign(c.y * p.x - c.x * p.y));
}
void draw_arc(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    float a0 = PI * 0.5 + function2.x * 0.5;
    vec2 p0 = (Coord.xy - function1.xy) * mat2(cos(a0), -sin(a0), sin(a0), cos(a0));
    if (function2.z == 1.0)
    {
        float d0 = sdPie(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    }
    else
    {
        float d0 = sdArc(p0, vec2(sin((function2.x - function1.w) * 0.5), cos((function2.x - function1.w) * 0.5)), function1.z);
        color = vec4(color.xyz, color.a * (1 - smoothstep(function2.y / 2.0 - 0.5, function2.y / 2.0 + 0.5, abs(d0))));
    }
}
                
float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r )
{
    r.xy = (p.x>0.0)?r.xy : r.zw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}
                
void draw_rounded_rect(inout vec4 color, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    vec2 p0 = (Coord.xy - function1.xy);
    float d0 = sdRoundedBox(p0, function1.zw * 0.5, function2);
    if (function3.y == 1.0)
        color = vec4(color.xyz, color.a * (1 - smoothstep(-0.5, 0.5, d0)));
    else
        color = vec4(color.xyz, color.a * (1 - smoothstep(function3.x / 2.0 - 0.5, function3.x / 2.0 + 0.5, abs(d0))));
}
void standard_shape_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3)
{
    if (uShapeEnabled)
    {
        switch(uShape)
        {
        case SHAPE_Line:
            draw_line(color, function1, function2, function3);
            break;
        case SHAPE_CubicBezier:
            draw_cubic_bezier(color, function1, function2, function3);
            break;
        case SHAPE_Triangle:
            draw_triangle(color, function1, function2, function3);
            break;
        case SHAPE_Circle:
            draw_circle(color, function1, function2, function3);
            break;
        case SHAPE_Ellipse:
            draw_ellipse(color, function1, function2, function3);
            break;
        case SHAPE_Pie:
            draw_pie(color, function1, function2, function3);
            break;
        case SHAPE_Arc:
            draw_arc(color, function1, function2, function3);
            break;
        case SHAPE_RoundedRect:
            draw_rounded_rect(color, function1, function2, function3);
            break;
        }
    }
}
