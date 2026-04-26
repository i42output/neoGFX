void standard_vertex_shader(inout vec3 coord, inout vec4 color)
{
    vec3 transformedVertex = (uProjectionMatrix * (uTransformationMatrix * vec4(coord, 1.0))).xyz;
    gl_Position = vec4(transformedVertex, 1.0);
    color.a *= uOpacity;
}
