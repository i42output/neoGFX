void standard_vertex_shader(inout vec3 coord, inout vec4 color)
{
    gl_Position = vec4((uProjectionMatrix * (uTransformationMatrix * vec4(coord, 1.0))).xyz, 1.0);
    color.a *= uOpacity;
}
