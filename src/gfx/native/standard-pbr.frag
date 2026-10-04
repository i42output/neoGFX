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
// the shadow map atlas (see native_rendering_context::draw_scene_meshes): view 0 (the directional light's) is the bottom left
// quarter; views 1 to 48 (six cube faces for each of up to eight point lights) are 512 square tiles in the other quarters
#define PBR_SHADOW_ATLAS_SIZE 4096.0
#define PBR_SHADOW_BIAS 0.0005

// the vertices' texture coordinates (TexCoord) are glTF's (v flipped): wrapped (uPbrTextureWrap: 0 clamp to edge, 1 repeat,
// 2 mirrored repeat) then transformed (scale xy, offset zw) to each texture's
float pbr_wrap(float c, int mode)
{
    if (mode == 1)
        return fract(c);
    if (mode == 2)
    {
        float m = mod(c, 2.0);
        return m > 1.0 ? 2.0 - m : m;
    }
    return clamp(c, 0.0, 1.0);
}

vec4 pbr_texture(int source, vec4 transform)
{
    vec2 coord = vec2(pbr_wrap(TexCoord.x, uPbrTextureWrap.x), pbr_wrap(TexCoord.y, uPbrTextureWrap.y)) * transform.xy + transform.zw;
    // n.b. derivatives of the unwrapped coordinates (no seams where they wrap)
    vec2 dx = dFdx(TexCoord * transform.xy);
    vec2 dy = dFdy(TexCoord * transform.xy);
    switch(source)
    {
    case 0:
        return textureGrad(uPbrTexture0, coord, dx, dy);
    case 1:
        return textureGrad(uPbrTexture1, coord, dx, dy);
    case 2:
        return textureGrad(uPbrTexture2, coord, dx, dy);
    case 3:
        return textureGrad(uPbrTexture3, coord, dx, dy);
    default:
        return textureGrad(uPbrBaseTexture, coord, dx, dy);
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

// n.b. the environment texture is a rectangle texture (texel coordinates, clamped)
vec3 pbr_environment_band(float band, vec3 direction)
{
    vec2 uv = vec2(atan(direction.z, direction.x) / (2.0 * PI) + 0.5, 0.5 - asin(clamp(direction.y, -1.0, 1.0)) / PI);
    float column = clamp(uv.x * PBR_ENVIRONMENT_WIDTH, 0.5, PBR_ENVIRONMENT_WIDTH - 0.5);
    float row = clamp(uv.y * PBR_ENVIRONMENT_BAND_HEIGHT, 0.5, PBR_ENVIRONMENT_BAND_HEIGHT - 0.5);
    return texture(uPbrEnvironment, vec2(column, band * PBR_ENVIRONMENT_BAND_HEIGHT + row)).rgb;
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
    float column = clamp(nDotV * PBR_ENVIRONMENT_WIDTH, 0.5, PBR_ENVIRONMENT_WIDTH - 0.5);
    float row = clamp(roughness * PBR_ENVIRONMENT_BAND_HEIGHT, 0.5, PBR_ENVIRONMENT_BAND_HEIGHT - 0.5);
    return texture(uPbrEnvironment, vec2(column, PBR_ENVIRONMENT_BRDF_BAND * PBR_ENVIRONMENT_BAND_HEIGHT + row)).rg;
}

vec4 pbr_shadow_tile(int view)
{
    if (view == 0)
        return vec4(0.0, 0.0, 0.5, 0.5);
    int face = view - 1;
    int quadrant = 1 + face / 16;
    int local = face % 16;
    return vec4(float(quadrant % 2) * 0.5 + float(local % 4) * 0.125, float(quadrant / 2) * 0.5 + float(local / 4) * 0.125, 0.125, 0.125);
}

// the fraction of a shadow view's light reaching a position (3x3 percentage closer filtering)
float pbr_shadow(int view, vec3 position)
{
    vec4 p = bShadowMatrices[uPbrShadowMatrixBase + uint(view)] * vec4(position, 1.0);
    if (p.w <= 0.0)
        return 1.0;
    p.xyz /= p.w;
    if (p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0 || p.z > 1.0)
        return 1.0;
    vec4 tile = pbr_shadow_tile(view);
    float texel = 1.0 / PBR_SHADOW_ATLAS_SIZE;
    vec2 lo = tile.xy + vec2(texel * 0.5);
    vec2 hi = tile.xy + tile.zw - vec2(texel * 0.5);
    vec2 uv = tile.xy + p.xy * tile.zw;
    float lit = 0.0;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x)
            lit += p.z - PBR_SHADOW_BIAS <= textureLod(uPbrShadowAtlas, clamp(uv + vec2(x, y) * texel, lo, hi), 0.0).r ? 1.0 : 0.0;
    return lit / 9.0;
}

// a light arriving from direction l: Lambert diffuse plus GGX specular (height correlated Smith visibility)
vec3 pbr_direct(vec3 n, vec3 v, vec3 l, vec3 radiance, vec3 f0, vec3 diffuseColor, float alpha2, float nDotV)
{
    vec3 h = normalize(l + v);
    float nDotL = max(dot(n, l), 0.0);
    float nDotH = max(dot(n, h), 0.0);
    float vDotH = max(dot(v, h), 0.0);
    vec3 fresnel = f0 + (vec3(1.0) - f0) * pow(1.0 - vDotH, 5.0);
    float dDenominator = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
    float distribution = alpha2 / (PI * dDenominator * dDenominator);
    float visibilityDenominator = nDotL * sqrt(nDotV * nDotV * (1.0 - alpha2) + alpha2) + nDotV * sqrt(nDotL * nDotL * (1.0 - alpha2) + alpha2);
    float visibility = visibilityDenominator > 0.0 ? 0.5 / visibilityDenominator : 0.0;
    return ((vec3(1.0) - fresnel) * diffuseColor / PI + fresnel * distribution * visibility) * radiance * nDotL;
}

void standard_pbr_shader(inout vec4 color, inout vec4 function0, inout vec4 function1, inout vec4 function2, inout vec4 function3, inout vec4 function4, inout vec4 function5, inout vec4 function6)
{
    if (!uPbrEnabled)
        return;

    // the base colour: the vertex colour (the base colour factor) and any base colour texture, both sRGB encoded
    // n.b. all texture sampling (and derivatives) before any discard
    vec3 baseColor = pbr_to_linear(Color.rgb);
    if (uPbrBaseColorTextured)
        baseColor *= pbr_to_linear(pbr_texture(4, uPbrBaseTextureTransform).rgb);
    // the normal map's (unwrapped, for the tangent frame) coordinates
    vec2 texCoord1 = TexCoord * uPbrTextureTransform1.xy + uPbrTextureTransform1.zw;
    vec4 metallicRoughnessTexel = uPbrTextureSources.x != -1 ? pbr_texture(uPbrTextureSources.x, uPbrTextureTransform0) : vec4(1.0);
    vec4 normalTexel = uPbrTextureSources.y != -1 ? pbr_texture(uPbrTextureSources.y, uPbrTextureTransform1) : vec4(0.5, 0.5, 1.0, 1.0);
    vec4 occlusionTexel = uPbrTextureSources.z != -1 ? pbr_texture(uPbrTextureSources.z, uPbrTextureTransform2) : vec4(1.0);
    vec4 emissiveTexel = uPbrTextureSources.w != -1 ? pbr_texture(uPbrTextureSources.w, uPbrTextureTransform3) : vec4(1.0);
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
    // the surface's own (not normal mapped) normal, for offsetting shadow map lookups
    vec3 surfaceNormal = n;
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

    // the directional light (shadowed if it casts shadows; the lookup is offset along the normal by about a shadow map texel)
    float directionalLight = 1.0;
    if (uPbrDirectionalShadow >= 0)
        directionalLight = pbr_shadow(uPbrDirectionalShadow, Coord + surfaceNormal * (uPbrDirectionalShadowTexel * 1.5));
    vec3 direct = directionalLight * pbr_direct(n, v, uPbrLightDirection, PBR_LIGHT_RADIANCE, f0, diffuseColor, alpha2, nDotV);
    // the point lights (see i_standard_shader_program::scene_lights); those casting shadows have six (cube face) shadow views
    for (uint i = 0u; i < uPbrLightCount; ++i)
    {
        vec4 lightPosition = bLights[uPbrLightBase + i * 2u];
        vec4 lightRadiance = bLights[uPbrLightBase + i * 2u + 1u];
        vec3 toLight = lightPosition.xyz - Coord;
        float distance2 = max(dot(toLight, toLight), 1e-4);
        float window = 1.0;
        if (lightRadiance.w > 0.0)
        {
            float ratio = distance2 / (lightRadiance.w * lightRadiance.w);
            window = clamp(1.0 - ratio * ratio, 0.0, 1.0);
            window *= window;
        }
        if (window <= 0.0)
            continue;
        if (lightPosition.w >= 0.0)
        {
            // a cube face is 90 degrees across 512 texels: a texel is about distance / 256
            vec3 position = Coord + surfaceNormal * (sqrt(distance2) / 256.0 * 1.5);
            vec3 fromLight = position - lightPosition.xyz;
            vec3 a = abs(fromLight);
            int face = a.x >= a.y && a.x >= a.z ? (fromLight.x >= 0.0 ? 0 : 1) : a.y >= a.z ? (fromLight.y >= 0.0 ? 2 : 3) : (fromLight.z >= 0.0 ? 4 : 5);
            window *= pbr_shadow(int(lightPosition.w + 0.5) + face, position);
        }
        direct += pbr_direct(n, v, toLight * inversesqrt(distance2), lightRadiance.rgb * window / distance2, f0, diffuseColor, alpha2, nDotV);
    }

    // image based lighting: diffuse irradiance by the normal, prefiltered specular by the reflection (split sum)
    vec2 environmentBrdf = pbr_environment_brdf(nDotV, roughness);
    vec3 ambient = (diffuseColor * pbr_environment_band(PBR_ENVIRONMENT_IRRADIANCE_BAND, n) +
        pbr_environment_specular(reflect(-v, n), roughness) * (f0 * environmentBrdf.x + environmentBrdf.y)) * occlusion * uPbrEnvironmentIntensity;

    color.rgb = pbr_from_linear(pbr_tone_map(direct + ambient + emissive));
}
