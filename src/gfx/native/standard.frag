#define PI 3.1415926538
#define SHAPE_None 0x00
#define SHAPE_Line 0x01
#define SHAPE_CubicBezier 0x02
#define SHAPE_Triangle 0x03
#define SHAPE_Rect 0x04
#define SHAPE_Circle 0x05
#define SHAPE_Ellipse 0x06
#define SHAPE_Pie 0x07
#define SHAPE_Arc 0x08
#define SHAPE_RoundedRect 0x09
#define SHAPE_EllipseRect 0x0A
#define SHAPE_Polygon 0x0B

vec4 mix3(vec4 a, vec4 b, vec4 c, float t)
{
    float t1 = smoothstep(0.0, 1.0 / 3.0, t);
    float t2 = smoothstep(1.0 / 3.0, 2.0 / 3.0, t);
    float t3 = smoothstep(2.0 / 3.0, 1.0, t);

    vec4 ab = mix(a, b, t1);
    vec4 bc = mix(b, c, t3);
    
    return mix(ab, bc, t2);
}

vec4 mix4(vec4 a, vec4 b, vec4 c, vec4 d, float t)
{
    float t1 = smoothstep(0.0, 1.0 / 3.0, t);
    float t2 = smoothstep(1.0 / 3.0, 2.0 / 3.0, t);
    float t3 = smoothstep(2.0 / 3.0, 1.0, t);

    vec4 ab = mix(a, b, t1);
    vec4 bc = mix(b, c, t2);
    vec4 cd = mix(c, d, t3);
    
    float tFinal1 = smoothstep(0.0, 2.0 / 3.0, t);
    float tFinal2 = smoothstep(1.0 / 3.0, 1.0, t);

    vec4 abc = mix(ab, bc, tFinal1);
    return mix(abc, cd, tFinal2);
}
                        
void standard_fragment_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
}
