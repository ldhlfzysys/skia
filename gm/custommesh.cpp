/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkCustomMesh.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkCanvasPriv.h"

#include <memory>

namespace skiagm {
class CustomMeshGM : public skiagm::GM {
public:
    CustomMeshGM() {}

protected:
    using Attribute = SkCustomMeshSpecification::Attribute;
    using Varying   = SkCustomMeshSpecification::Varying;

    SkISize onISize() override { return {435, 1180}; }

    void onOnceBeforeDraw() override {
        {
            static const Attribute kAttributes[]{
                    {Attribute::Type::kFloat4,       8, SkString{"xuyv"}},
                    {Attribute::Type::kUByte4_unorm, 4, SkString{"brag"}},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kHalf4,  SkString{"color"}},
                    {Varying::Type::kFloat2, SkString{"uv"}   },
            };
            static constexpr char kVS[] = R"(
                    half4 unswizzle_color(half4 color) { return color.garb; }

                    float2 main(in Attributes attributes, out Varyings varyings) {
                        varyings.color = unswizzle_color(attributes.brag);
                        varyings.uv    = attributes.xuyv.yw;
                        return attributes.xuyv.xz;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 main(in Varyings varyings, out float4 color) {
                        color = varyings.color;
                        return varyings.uv;
                    }
            )";
            auto [spec, error] = SkCustomMeshSpecification::Make(
                    SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                    sizeof(ColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithColor = std::move(spec);
        }
        {
            static const Attribute kAttributes[]{
                    {Attribute::Type::kFloat4, 0, SkString{"xuyv"}},
            };
            static const Varying kVaryings[]{
                    {Varying::Type::kFloat2, SkString{"vux2"}},
            };
            static constexpr char kVS[] = R"(
                    float2 main(in Attributes a, out Varyings v) {
                        v.vux2 = 2*a.xuyv.wy;
                        return a.xuyv.xz;
                    }
            )";
            static constexpr char kFS[] = R"(
                    float2 helper(in float2 vux2) { return vux2.yx/2; }
                    float2 main(in Varyings varyings) {
                        return helper(varyings.vux2);
                    }
            )";
            auto [spec, error] = SkCustomMeshSpecification::Make(
                    SkMakeSpan(kAttributes, SK_ARRAY_COUNT(kAttributes)),
                    sizeof(NoColorVertex),
                    SkMakeSpan(kVaryings, SK_ARRAY_COUNT(kVaryings)),
                    SkString(kVS),
                    SkString(kFS));
            if (!spec) {
                SkDebugf("%s\n", error.c_str());
            }
            fSpecWithNoColor = std::move(spec);
        }

        static constexpr SkColor kColors[] = {SK_ColorTRANSPARENT, SK_ColorWHITE};
        fShader = SkGradientShader::MakeRadial({10, 10},
                                               3,
                                               kColors,
                                               nullptr,
                                               2,
                                               SkTileMode::kMirror);
    }

    SkString onShortName() override { return SkString("custommesh"); }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        for (const sk_sp<SkBlender>& blender : {SkBlender::Mode(SkBlendMode::kDst),
                                                SkBlender::Mode(SkBlendMode::kSrc),
                                                SkBlender::Mode(SkBlendMode::kSaturation)}) {
            canvas->save();
            for (uint8_t alpha  : {0xFF , 0x40})
            for (bool    colors : {false, true})
            for (bool    shader : {false, true}) {

                SkCustomMesh cm;
                cm.spec   = colors ? fSpecWithColor : fSpecWithNoColor;
                cm.vb     = colors ? static_cast<const void*>(kColorQuad)
                                   : static_cast<const void*>(kNoColorQuad);
                cm.bounds = kRect;
                cm.vcount = 4;
                cm.mode   = SkCustomMesh::Mode::kTriangleStrip;

                SkPaint paint;
                paint.setColor(SK_ColorGREEN);
                paint.setShader(shader ? fShader : nullptr);
                paint.setAlpha(alpha);

                SkCanvasPriv::DrawCustomMesh(canvas, std::move(cm), blender, paint);

                canvas->translate(0, 150);
            }
            canvas->restore();
            canvas->translate(150, 0);
        }
        return DrawResult::kOk;
    }

private:
    struct ColorVertex {
        uint32_t pad;
        uint32_t brag;
        float    xuyv[4];
    };

    struct NoColorVertex {
        float xuyv[4];
    };

    static constexpr auto kRect = SkRect::MakeLTRB(20, 20, 120, 120);
    static constexpr auto kUV   = SkRect::MakeLTRB( 0,  0,  20,  20);

    static constexpr ColorVertex kColorQuad[] {
            {0, 0x00FFFF00, {kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {0, 0x00FFFFFF, {kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {0, 0xFFFF00FF, {kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {0, 0xFFFFFF00, {kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    static constexpr NoColorVertex kNoColorQuad[]{
            {{kRect.left(),  kUV.left(),  kRect.top(),    kUV.top()   }},
            {{kRect.right(), kUV.right(), kRect.top(),    kUV.top()   }},
            {{kRect.left(),  kUV.left(),  kRect.bottom(), kUV.bottom()}},
            {{kRect.right(), kUV.right(), kRect.bottom(), kUV.bottom()}},
    };

    sk_sp<SkShader> fShader;

    sk_sp<SkCustomMeshSpecification> fSpecWithColor;
    sk_sp<SkCustomMeshSpecification> fSpecWithNoColor;
};

constexpr SkRect CustomMeshGM::kRect;
constexpr SkRect CustomMeshGM::kUV;

constexpr CustomMeshGM::ColorVertex   CustomMeshGM::kColorQuad[];
constexpr CustomMeshGM::NoColorVertex CustomMeshGM::kNoColorQuad[];

DEF_GM( return new CustomMeshGM; )

}  // namespace skiagm