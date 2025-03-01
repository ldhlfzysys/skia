/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "src/core/SkBlurMask.h"

#define SMALL   SkIntToScalar(2)
#define REAL    1.5f
static const SkScalar kMedium = SkIntToScalar(5);
#define BIG     SkIntToScalar(10)
static const SkScalar kMedBig = SkIntToScalar(20);
#define REALBIG 30.5f

class BlurRectBench: public Benchmark {
    int         fLoopCount;
    SkScalar    fRadius;
    SkString    fName;

public:
    BlurRectBench(SkScalar rad) {
        fRadius = rad;

        if (fRadius > SkIntToScalar(25)) {
            fLoopCount = 100;
        } else if (fRadius > SkIntToScalar(5)) {
            fLoopCount = 1000;
        } else {
            fLoopCount = 10000;
        }
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    SkScalar radius() const {
        return fRadius;
    }

    void setName(const SkString& name) {
        fName = name;
    }

    void onDraw(int loops, SkCanvas*) override {
        SkPaint paint;
        this->setupPaint(&paint);

        paint.setAntiAlias(true);

        SkScalar pad = fRadius*3/2 + SK_Scalar1;
        SkRect r = SkRect::MakeWH(2 * pad + SK_Scalar1, 2 * pad + SK_Scalar1);

        preBenchSetup(r);

        for (int i = 0; i < loops; i++) {
            this->makeBlurryRect(r);
        }
    }

    virtual void makeBlurryRect(const SkRect&) = 0;
    virtual void preBenchSetup(const SkRect&) {}
private:
    using INHERITED = Benchmark;
};


class BlurRectDirectBench: public BlurRectBench {
 public:
    BlurRectDirectBench(SkScalar rad) : INHERITED(rad) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_direct_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_direct_%d", SkScalarRoundToInt(rad));
        }

        this->setName(name);
    }
protected:
    void makeBlurryRect(const SkRect& r) override {
        SkMask mask;
        if (!SkBlurMask::BlurRect(SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                  &mask, r, kNormal_SkBlurStyle)) {
            return;
        }
        SkMask::FreeImage(mask.fImage);
    }
private:
    using INHERITED = BlurRectBench;
};

class BlurRectSeparableBench: public BlurRectBench {

public:
    BlurRectSeparableBench(SkScalar rad) : INHERITED(rad) { }

    ~BlurRectSeparableBench() override {
        SkMask::FreeImage(fSrcMask.fImage);
    }

protected:
    void preBenchSetup(const SkRect& r) override {
        SkMask::FreeImage(fSrcMask.fImage);

        r.roundOut(&fSrcMask.fBounds);
        fSrcMask.fFormat = SkMask::kA8_Format;
        fSrcMask.fRowBytes = fSrcMask.fBounds.width();
        fSrcMask.fImage = SkMask::AllocImage(fSrcMask.computeTotalImageSize());

        memset(fSrcMask.fImage, 0xff, fSrcMask.computeTotalImageSize());
    }

    SkMask fSrcMask;
private:
    using INHERITED = BlurRectBench;
};

class BlurRectBoxFilterBench: public BlurRectSeparableBench {
public:
    BlurRectBoxFilterBench(SkScalar rad) : INHERITED(rad) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_boxfilter_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_boxfilter_%d", SkScalarRoundToInt(rad));
        }

        this->setName(name);
    }

protected:

    void makeBlurryRect(const SkRect&) override {
        SkMask mask;
        if (!SkBlurMask::BoxBlur(&mask, fSrcMask, SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                 kNormal_SkBlurStyle)) {
            return;
        }
        SkMask::FreeImage(mask.fImage);
    }
private:
    using INHERITED = BlurRectSeparableBench;
};

class BlurRectGaussianBench: public BlurRectSeparableBench {
public:
    BlurRectGaussianBench(SkScalar rad) : INHERITED(rad) {
        SkString name;

        if (SkScalarFraction(rad) != 0) {
            name.printf("blurrect_gaussian_%.2f", SkScalarToFloat(rad));
        } else {
            name.printf("blurrect_gaussian_%d", SkScalarRoundToInt(rad));
        }

        this->setName(name);
    }

protected:

    void makeBlurryRect(const SkRect&) override {
        SkMask mask;
        if (!SkBlurMask::BlurGroundTruth(SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                         &mask, fSrcMask, kNormal_SkBlurStyle)) {
            return;
        }
        SkMask::FreeImage(mask.fImage);
    }
private:
    using INHERITED = BlurRectSeparableBench;
};

DEF_BENCH(return new BlurRectBoxFilterBench(SMALL);)
DEF_BENCH(return new BlurRectBoxFilterBench(BIG);)
DEF_BENCH(return new BlurRectBoxFilterBench(REALBIG);)
DEF_BENCH(return new BlurRectBoxFilterBench(REAL);)
DEF_BENCH(return new BlurRectGaussianBench(SMALL);)
DEF_BENCH(return new BlurRectGaussianBench(BIG);)
DEF_BENCH(return new BlurRectGaussianBench(REALBIG);)
DEF_BENCH(return new BlurRectGaussianBench(REAL);)
DEF_BENCH(return new BlurRectDirectBench(SMALL);)
DEF_BENCH(return new BlurRectDirectBench(BIG);)
DEF_BENCH(return new BlurRectDirectBench(REALBIG);)
DEF_BENCH(return new BlurRectDirectBench(REAL);)

DEF_BENCH(return new BlurRectDirectBench(kMedium);)
DEF_BENCH(return new BlurRectDirectBench(kMedBig);)

DEF_BENCH(return new BlurRectBoxFilterBench(kMedium);)
DEF_BENCH(return new BlurRectBoxFilterBench(kMedBig);)

#if 0
// disable Gaussian benchmarks; the algorithm works well enough
// and serves as a baseline for ground truth, but it's too slow
// to use in production for non-trivial radii, so no real point
// in having the bots benchmark it all the time.

DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(1));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(2));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(3));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(4));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(5));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(6));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(7));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(8));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(9));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(10));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(11));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(12));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(13));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(14));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(15));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(16));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(17));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(18));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(19));)
DEF_BENCH(return new BlurRectGaussianBench(SkIntToScalar(20));)
#endif
