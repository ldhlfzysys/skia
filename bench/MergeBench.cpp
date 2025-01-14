/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"

#define FILTER_WIDTH_SMALL  SkIntToScalar(32)
#define FILTER_HEIGHT_SMALL SkIntToScalar(32)
#define FILTER_WIDTH_LARGE  SkIntToScalar(256)
#define FILTER_HEIGHT_LARGE SkIntToScalar(256)

static sk_sp<SkImage> make_bitmap() {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(80, 80)));
    surface->getCanvas()->clear(0x00000000);
    SkPaint paint;
    paint.setColor(0xFF884422);
    SkFont font;
    font.setSize(SkIntToScalar(96));
    surface->getCanvas()->drawSimpleText("g", 1, SkTextEncoding::kUTF8, 15, 55, font, paint);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_checkerboard() {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(80, 80)));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    for (int y = 0; y < 80; y += 16) {
        for (int x = 0; x < 80; x += 16) {
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas->drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas->drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas->restore();
        }
    }

    return surface->makeImageSnapshot();
}

class MergeBench : public Benchmark {
public:
    MergeBench(bool small) : fIsSmall(small), fInitialized(false) { }

protected:
    const char* onGetName() override {
        return fIsSmall ? "merge_small" : "merge_large";
    }

    void onDelayedSetup() override {
        if (!fInitialized) {
            fImage = make_bitmap();
            fCheckerboard = make_checkerboard();
            fInitialized = true;
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkRect r = fIsSmall ? SkRect::MakeWH(FILTER_WIDTH_SMALL, FILTER_HEIGHT_SMALL) :
                              SkRect::MakeWH(FILTER_WIDTH_LARGE, FILTER_HEIGHT_LARGE);
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Merge(SkImageFilters::Image(fCheckerboard),
                                                   SkImageFilters::Image(fImage)));
        for (int i = 0; i < loops; i++) {
            canvas->drawRect(r, paint);
        }
    }

private:
    bool fIsSmall;
    bool fInitialized;
    sk_sp<SkImage> fImage, fCheckerboard;

    using INHERITED = Benchmark;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MergeBench(true); )
DEF_BENCH( return new MergeBench(false); )
