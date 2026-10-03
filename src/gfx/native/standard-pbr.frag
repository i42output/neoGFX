// physically based shading (glTF metallic-roughness BRDF) of model transformed meshes: Coord is the vertex's world
// position, WorldNormal its world normal; lit by a directional light (uPbrLightDirection: towards the light) and a
// sky/ground hemisphere
#define PBR_LIGHT_RADIANCE vec3(2.8)
#define PBR_SKY_RADIANCE vec3(0.30, 0.32, 0.36)
#define PBR_GROUND_RADIANCE vec3(0.12, 0.11, 0.10)
#define PBR_GAMMA 2.2

vec4 pbr_texture(int source, vec2 texCoord)
{
    switch(source)
    {
    case 0:
        return texture(uPbrTexture0, texCoord);
    case 1:
        return texture(uPbrTexture1, texCoord);
    case 2:
        return texture(uPbrTexture2, texCoord);
    case 3:
        return texture(uPbrTexture3, texCoord);
    default:
        return texture(uPbrBaseTexture, texCoord);
    }
}

vec3 pbr_to_linear(vec3 c)
{
    return pow(max(c, vec3(0.0)), vec3(PBR_GAMMA));
}

vec3 pbr_from_linear(vec3 c)
{
    return pow(max(c, vec3(0.0)), vec3(1.0 / PBR_GAMMA));
}

// Khronos PBR Neutral tone mapping
vec3 pbr_tone_map(vec3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;
    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;
    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression)
        return color;
    const float d = 1.0 - startCompression;
    float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;
    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return mix(color, vec3(newPeak), g);
}

vec3 pbr_hemisphere(vec3 direction)
{
    return mix(PBR_GROUND_RADIANCE, PBR_SKY_RADIANCE, clamp(direction.y * 0.5 + 0.5, 0.0, 1.0));
}

void standard_pbr_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (!uPbrEnabled)
        return;

    // the base colour (vertex colour multiplied by any base colour texture) is sRGB encoded
    vec3 baseColor = pbr_to_linear(color.rgb);

    // n.b. all texture sampling (and derivatives) before any discard
    vec2 texCoord1 = TexCoord * uPbrTextureTransform1.xy + uPbrTextureTransform1.zw;
    vec4 metallicRoughnessTexel = uPbrTextureSources.x != -1 ? pbr_texture(uPbrTextureSources.x, TexCoord * uPbrTextureTransform0.xy + uPbrTextureTransform0.zw) : vec4(1.0);
    vec4 normalTexel = uPbrTextureSources.y != -1 ? pbr_texture(uPbrTextureSources.y, texCoord1) : vec4(0.5, 0.5, 1.0, 1.0);
    vec4 occlusionTexel = uPbrTextureSources.z != -1 ? pbr_texture(uPbrTextureSources.z, TexCoord * uPbrTextureTransform2.xy + uPbrTextureTransform2.zw) : vec4(1.0);
    vec4 emissiveTexel = uPbrTextureSources.w != -1 ? pbr_texture(uPbrTextureSources.w, TexCoord * uPbrTextureTransform3.xy + uPbrTextureTransform3.zw) : vec4(1.0);
    vec3 dp1 = dFdx(Coord);
    vec3 dp2 = dFdy(Coord);
    vec2 duv1 = dFdx(texCoord1);
    vec2 duv2 = dFdy(texCoord1);

    if (uPbrAlphaCutoff >= 0.0 && color.a < uPbrAlphaCutoff)
        discard;

    float metallic = clamp(uPbrFactors.x * metallicRoughnessTexel.b, 0.0, 1.0);
    float roughness = clamp(uPbrFactors.y * metallicRoughnessTexel.g, 0.04, 1.0);
    float occlusion = 1.0 + uPbrFactors.w * (occlusionTexel.r - 1.0);
    vec3 emissive = uPbrEmissive * pbr_to_linear(emissiveTexel.rgb);

    vec3 v = normalize(uPbrViewPosition - Coord);
    vec3 n = WorldNormal;
    if (dot(n, n) > 0.0)
        n = normalize(n);
    else
    {
        // no normal: the face's
        n = normalize(cross(dp1, dp2));
        if (dot(n, v) < 0.0)
            n = -n;
    }
    if (uPbrDoubleSided && dot(n, v) < 0.0)
        n = -n;
    if (uPbrTextureSources.y != -1)
    {
        // normal mapping: the tangent frame from the screen space derivatives of position and texture coordinates
        vec3 mapNormal = normalTexel.xyz * 2.0 - 1.0;
        mapNormal.xy *= uPbrFactors.z;
        vec3 dp2perp = cross(dp2, n);
        vec3 dp1perp = cross(n, dp1);
        vec3 t = dp2perp * duv1.x + dp1perp * duv2.x;
        vec3 b = dp2perp * duv1.y + dp1perp * duv2.y;
        float scale = max(dot(t, t), dot(b, b));
        if (scale > 0.0)
        {
            float invScale = inversesqrt(scale);
            n = normalize(mat3(t * invScale, b * invScale, n) * mapNormal);
        }
    }

    vec3 f0 = mix(vec3(0.04), baseColor, metallic);
    vec3 diffuseColor = baseColor * (1.0 - metallic);
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float nDotV = max(dot(n, v), 1e-4);

    // the directional light: Lambert diffuse plus GGX specular (height correlated Smith visibility)
    vec3 l = uPbrLightDirection;
    vec3 h = normalize(l + v);
    float nDotL = max(dot(n, l), 0.0);
    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    vec3 fresnel = f0 + (vec3(1.0) - f0) * pow(1.0 - vDotH, 5.0);
    float dDenominator = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
    float distribution = alpha2 / (PI * dDenominator * dDenominator);
    float visibilityDenominator = nDotL * sqrt(nDotV * nDotV * (1.0 - alpha2) + alpha2) + nDotV * sqrt(nDotL * nDotL * (1.0 - alpha2) + alpha2);
    float visibility = visibilityDenominator > 0.0 ? 0.5 / visibilityDenominator : 0.0;
    vec3 direct = ((vec3(1.0) - fresnel) * diffuseColor / PI + fresnel * distribution * visibility) * PBR_LIGHT_RADIANCE * nDotL;

    // ambient: the hemisphere (diffuse by the normal, specular by the reflection) with an analytic environment BRDF
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * nDotV)) * r.x + r.y;
    vec2 environmentBrdf = vec2(-1.04, 1.04) * a004 + r.zw;
    vec3 ambient = (diffuseColor * pbr_hemisphere(n) + (f0 * environmentBrdf.x + environmentBrdf.y) * pbr_hemisphere(reflect(-v, n))) * occlusion;

    color.rgb = pbr_from_linear(pbr_tone_map(direct + ambient + emissive));
}
