// physically based shading (glTF metallic-roughness BRDF) of model transformed meshes: Coord is the vertex's world
// position, WorldNormal its world normal; lit by a directional light (uPbrLightDirection: towards the light) and image
// based lighting (uPbrEnvironment); linear throughout: sRGB inputs are decoded and the result tone mapped and sRGB encoded
#define PBR_LIGHT_RADIANCE vec3(2.8)
// the environment texture (see standard_pbr_shader): equirectangular bands one above the other; 0 to 5 the environment
// prefiltered for roughness 0 to 1, 6 the irradiance (divided by pi), 7 the split sum BRDF (by n.v and roughness)
#define PBR_ENVIRONMENT_WIDTH 256.0
#define PBR_ENVIRONMENT_BAND_HEIGHT 128.0
#define PBR_ENVIRONMENT_BANDS 8.0
#define PBR_ENVIRONMENT_SPECULAR_BANDS 6.0
#define PBR_ENVIRONMENT_IRRADIANCE_BAND 6.0
#define PBR_ENVIRONMENT_BRDF_BAND 7.0

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

// the sRGB transfer functions
vec3 pbr_to_linear(vec3 c)
{
    c = clamp(c, 0.0, 1.0);
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

vec3 pbr_from_linear(vec3 c)
{
    c = clamp(c, 0.0, 1.0);
    return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));
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

vec3 pbr_environment_band(float band, vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * PI) + 0.5, 0.5 - asin(clamp(direction.y, -1.0, 1.0)) / PI);
    float row = clamp(uv.y * PBR_ENVIRONMENT_BAND_HEIGHT, 0.5, PBR_ENVIRONMENT_BAND_HEIGHT - 0.5);
    return texture(uPbrEnvironment, vec2(uv.x, (band * PBR_ENVIRONMENT_BAND_HEIGHT + row) / (PBR_ENVIRONMENT_BAND_HEIGHT * PBR_ENVIRONMENT_BANDS))).rgb;
}

vec3 pbr_environment_specular(vec3 direction, float roughness)
{
    float level = roughness * (PBR_ENVIRONMENT_SPECULAR_BANDS - 1.0);
    float level0 = floor(level);
    float level1 = min(level0 + 1.0, PBR_ENVIRONMENT_SPECULAR_BANDS - 1.0);
    return mix(pbr_environment_band(level0, direction), pbr_environment_band(level1, direction), level - level0);
}

vec2 pbr_environment_brdf(float nDotV, float roughness)
{
    float u = clamp(nDotV, 0.5 / PBR_ENVIRONMENT_WIDTH, 1.0 - 0.5 / PBR_ENVIRONMENT_WIDTH);
    float row = clamp(roughness * PBR_ENVIRONMENT_BAND_HEIGHT, 0.5, PBR_ENVIRONMENT_BAND_HEIGHT - 0.5);
    return texture(uPbrEnvironment, vec2(u, (PBR_ENVIRONMENT_BRDF_BAND * PBR_ENVIRONMENT_BAND_HEIGHT + row) / (PBR_ENVIRONMENT_BAND_HEIGHT * PBR_ENVIRONMENT_BANDS))).rg;
}

void standard_pbr_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (!uPbrEnabled)
        return;

    // the base colour: the vertex colour (the base colour factor) and any base colour texture, both sRGB encoded
    // n.b. all texture sampling (and derivatives) before any discard
    vec3 baseColor = pbr_to_linear(Color.rgb);
    if (uPbrBaseColorTextured)
        baseColor *= pbr_to_linear(texture(uPbrBaseTexture, TexCoord).rgb);
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

    // image based lighting: diffuse irradiance by the normal, prefiltered specular by the reflection (split sum)
    vec2 environmentBrdf = pbr_environment_brdf(nDotV, roughness);
    vec3 ambient = (diffuseColor * pbr_environment_band(PBR_ENVIRONMENT_IRRADIANCE_BAND, n) +
        pbr_environment_specular(reflect(-v, n), roughness) * (f0 * environmentBrdf.x + environmentBrdf.y)) * occlusion * uPbrEnvironmentIntensity;

    color.rgb = pbr_from_linear(pbr_tone_map(direct + ambient + emissive));
}
