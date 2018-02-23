#include "common/flags.glsl"
#include "common/planes.glsl"

uniform mat4        uMvpMatrix;
uniform sampler2D   uTexOffsets;
uniform float       uCurrentTime;

DENG_ATTRIB float   aTexture0; // front texture
DENG_ATTRIB float   aTexture1; // back texture
DENG_ATTRIB vec2    aIndex1; // tex offset (front, back)

     DENG_VAR vec2  vUV;
     DENG_VAR vec3  vNormal;
flat DENG_VAR float vTexture;
flat DENG_VAR uint  vFlags;

vec4 fetchTexOffset(uint offsetIndex) {
    uint dw = uint(textureSize(uTexOffsets, 0).x);
    return texelFetch(uTexOffsets, ivec2(offsetIndex % dw, offsetIndex / dw), 0);
}

void main(void) {
    Surface surface = Gloom_LoadVertexSurface();

    gl_Position = uMvpMatrix * surface.vertex;
    vUV         = aUV.xy;
    vFlags      = surface.flags;
    vNormal     = surface.normal;
    vTexture    = floatBitsToUint(surface.isFrontSide? aTexture0 : aTexture1);

    // Texture coordinate mapping.
    if (testFlag(surface.flags, Surface_WorldSpaceYToTexCoord)) {
        vUV.t = surface.vertex.y -
            (surface.isFrontSide ^^ testFlag(surface.flags, Surface_AnchorTopPlane)?
            surface.botPlaneY : surface.topPlaneY);
    }

    if (testFlag(surface.flags, Surface_WorldSpaceXZToTexCoords)) {
        vUV += aVertex.xz;
    }

    // Texture scrolling.
    if (testFlag(surface.flags, Surface_TextureOffset)) {
        vec4 texOffset = fetchTexOffset(
            floatBitsToUint(surface.isFrontSide? aIndex1.x : aIndex1.y));
        vUV += texOffset.xy + uCurrentTime * texOffset.zw;
    }

    // Texture rotation.
    if (aUV.w != 0.0) {
        float angle = radians(aUV.w);
        vUV = mat2(cos(angle), sin(angle), -sin(angle), cos(angle)) * vUV;
    }

    // Align with the left edge.
    if (!surface.isFrontSide) {
        vUV.s = surface.wallLength - vUV.s;
    }

    if (!testFlag(surface.flags, Surface_FlipTexCoordY)) {
        vUV.t = -vUV.t;
    }
}
