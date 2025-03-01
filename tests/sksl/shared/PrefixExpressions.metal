#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    ok = ok && !(_uniforms.colorGreen.x == 1.0h);
    uint val = uint(_uniforms.colorGreen.x);
    uint2 mask = uint2(val, ~val);
    int2 imask = int2(~mask);
    mask = ~mask & uint2(~imask);
    ok = ok && all(mask == uint2(0u));
    half one = _uniforms.colorGreen.x;
    half4x4 m = half4x4(one);
    _out.sk_FragColor = ok ? (-1.0 * m) * -_uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
