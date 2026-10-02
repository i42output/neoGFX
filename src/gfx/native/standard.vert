void standard_vertex_shader(inout vec3 coord, inout vec4 color)
{
    vec4 position = vec4(coord, 1.0);
    // GPU model transformation (see game::model_transformation): the model table maps an entity id to the
    // index of its model matrix in the model matrices; any skin joint matrices follow it
    if (VertexModel > 0.0)
    {
        uint first = bModelTable[uModelTableBase + uint(VertexModel + 0.5)];
        if (any(greaterThan(VertexWeights, vec4(0.0))))
        {
            vec4 skinned = vec4(0.0);
            for (int i = 0; i < 4; ++i)
                if (VertexWeights[i] > 0.0)
                    skinned += VertexWeights[i] * (bModelMatrices[first + 1u + uint(VertexJoints[i] + 0.5)] * position);
            position = skinned;
        }
        position = bModelMatrices[first] * position;
        coord = position.xyz / position.w;
    }
    // n.b. w is kept so that a projective transformation matrix gets its perspective divide; for the
    // usual orthographic projection and affine transformation w is 1 regardless
    gl_Position = uProjectionMatrix * (uTransformationMatrix * position);
    color.a *= uOpacity;
}
