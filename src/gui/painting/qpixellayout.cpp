/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qglobal.h>

#include "qdrawhelper_p.h"
#include "qpixellayout_p.h"
#include "qrgba64_p.h"
#include <QtCore/private/qsimd_p.h>

QT_BEGIN_NAMESPACE

template<QImage::Format> Q_DECL_CONSTEXPR uint redWidth();
template<QImage::Format> Q_DECL_CONSTEXPR uint redShift();
template<QImage::Format> Q_DECL_CONSTEXPR uint greenWidth();
template<QImage::Format> Q_DECL_CONSTEXPR uint greenShift();
template<QImage::Format> Q_DECL_CONSTEXPR uint blueWidth();
template<QImage::Format> Q_DECL_CONSTEXPR uint blueShift();
template<QImage::Format> Q_DECL_CONSTEXPR uint alphaWidth();
template<QImage::Format> Q_DECL_CONSTEXPR uint alphaShift();

template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGB16>() { return 5; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGB444>() { return 4; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGB555>() { return 5; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGB666>() { return 6; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGB888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_BGR888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_ARGB4444_Premultiplied>() { return 4; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_ARGB8555_Premultiplied>() { return 5; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_ARGB8565_Premultiplied>() { return 5; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_ARGB6666_Premultiplied>() { return 6; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGBX8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint redWidth<QImage::Format_RGBA8888_Premultiplied>() { return 8; }

template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGB16>() { return  11; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGB444>() { return  8; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGB555>() { return 10; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGB666>() { return 12; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGB888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_BGR888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_ARGB4444_Premultiplied>() { return  8; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_ARGB8555_Premultiplied>() { return 18; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_ARGB8565_Premultiplied>() { return 19; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_ARGB6666_Premultiplied>() { return 12; }
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBX8888>() { return 24; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBA8888>() { return 24; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBA8888_Premultiplied>() { return 24; }
#else
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBX8888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBA8888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint redShift<QImage::Format_RGBA8888_Premultiplied>() { return 0; }
#endif
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGB16>() { return 6; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGB444>() { return 4; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGB555>() { return 5; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGB666>() { return 6; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGB888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_BGR888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_ARGB4444_Premultiplied>() { return 4; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_ARGB8555_Premultiplied>() { return 5; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_ARGB8565_Premultiplied>() { return 6; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_ARGB6666_Premultiplied>() { return 6; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGBX8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenWidth<QImage::Format_RGBA8888_Premultiplied>() { return 8; }

template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGB16>() { return  5; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGB444>() { return 4; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGB555>() { return 5; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGB666>() { return 6; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGB888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_BGR888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_ARGB4444_Premultiplied>() { return  4; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_ARGB8555_Premultiplied>() { return 13; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_ARGB8565_Premultiplied>() { return 13; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_ARGB6666_Premultiplied>() { return  6; }
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBX8888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBA8888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBA8888_Premultiplied>() { return 16; }
#else
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBX8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint greenShift<QImage::Format_RGBA8888_Premultiplied>() { return 8; }
#endif
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGB16>() { return 5; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGB444>() { return 4; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGB555>() { return 5; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGB666>() { return 6; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGB888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_BGR888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_ARGB4444_Premultiplied>() { return 4; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_ARGB8555_Premultiplied>() { return 5; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_ARGB8565_Premultiplied>() { return 5; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_ARGB6666_Premultiplied>() { return 6; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGBX8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueWidth<QImage::Format_RGBA8888_Premultiplied>() { return 8; }

template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGB16>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGB444>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGB555>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGB666>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGB888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_BGR888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_ARGB4444_Premultiplied>() { return 0; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_ARGB8555_Premultiplied>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_ARGB8565_Premultiplied>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_ARGB6666_Premultiplied>() { return 0; }
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBX8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBA8888_Premultiplied>() { return 8; }
#else
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBX8888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBA8888>() { return 16; }
template<> Q_DECL_CONSTEXPR uint blueShift<QImage::Format_RGBA8888_Premultiplied>() { return 16; }
#endif
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGB16>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGB444>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGB555>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGB666>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGB888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_BGR888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_ARGB4444_Premultiplied>() { return  4; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_ARGB8555_Premultiplied>() { return  8; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_ARGB8565_Premultiplied>() { return  8; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_ARGB6666_Premultiplied>() { return  6; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGBX8888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGBA8888>() { return 8; }
template<> Q_DECL_CONSTEXPR uint alphaWidth<QImage::Format_RGBA8888_Premultiplied>() { return 8; }

template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGB16>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGB444>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGB555>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGB666>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGB888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_BGR888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_ARGB4444_Premultiplied>() { return 12; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_ARGB8555_Premultiplied>() { return  0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_ARGB8565_Premultiplied>() { return  0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_ARGB6666_Premultiplied>() { return 18; }
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBX8888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBA8888>() { return 0; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBA8888_Premultiplied>() { return 0; }
#else
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBX8888>() { return 24; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBA8888>() { return 24; }
template<> Q_DECL_CONSTEXPR uint alphaShift<QImage::Format_RGBA8888_Premultiplied>() { return 24; }
#endif

template<QImage::Format> constexpr QPixelLayout::BPP bitsPerPixel();
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB16>() { return QPixelLayout::BPP16; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB444>() { return QPixelLayout::BPP16; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB555>() { return QPixelLayout::BPP16; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB666>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGB888>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_BGR888>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB4444_Premultiplied>() { return QPixelLayout::BPP16; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB8555_Premultiplied>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB8565_Premultiplied>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_ARGB6666_Premultiplied>() { return QPixelLayout::BPP24; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGBX8888>() { return QPixelLayout::BPP32; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGBA8888>() { return QPixelLayout::BPP32; }
template<> constexpr QPixelLayout::BPP bitsPerPixel<QImage::Format_RGBA8888_Premultiplied>() { return QPixelLayout::BPP32; }

template <QPixelLayout::BPP width> static
void QT_FASTCALL storePixel(uchar *dest, int index, uint pixel);

template <>
inline void QT_FASTCALL storePixel<QPixelLayout::BPP16>(uchar *dest, int index, uint pixel)
{
    reinterpret_cast<quint16 *>(dest)[index] = quint16(pixel);
}

template <>
inline void QT_FASTCALL storePixel<QPixelLayout::BPP24>(uchar *dest, int index, uint pixel)
{
    reinterpret_cast<quint24 *>(dest)[index] = quint24(pixel);
}

template<QImage::Format Format>
static Q_ALWAYS_INLINE uint convertPixelToRGB32(uint s)
{
    Q_CONSTEXPR uint redMask = ((1 << redWidth<Format>()) - 1);
    Q_CONSTEXPR uint greenMask = ((1 << greenWidth<Format>()) - 1);
    Q_CONSTEXPR uint blueMask = ((1 << blueWidth<Format>()) - 1);

    Q_CONSTEXPR uchar redLeftShift = 8 - redWidth<Format>();
    Q_CONSTEXPR uchar greenLeftShift = 8 - greenWidth<Format>();
    Q_CONSTEXPR uchar blueLeftShift = 8 - blueWidth<Format>();

    Q_CONSTEXPR uchar redRightShift = 2 * redWidth<Format>() - 8;
    Q_CONSTEXPR uchar greenRightShift = 2 * greenWidth<Format>() - 8;
    Q_CONSTEXPR uchar blueRightShift = 2 * blueWidth<Format>() - 8;

    uint red   = (s >> redShift<Format>()) & redMask;
    uint green = (s >> greenShift<Format>()) & greenMask;
    uint blue  = (s >> blueShift<Format>()) & blueMask;

    red = ((red << redLeftShift) | (red >> redRightShift)) << 16;
    green = ((green << greenLeftShift) | (green >> greenRightShift)) << 8;
    blue = (blue << blueLeftShift) | (blue >> blueRightShift);
    return 0xff000000 | red | green | blue;
}

template<QImage::Format Format>
static void QT_FASTCALL convertToRGB32(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGB32<Format>(buffer[i]);
}

#if defined(__SSE2__) && !defined(__SSSE3__) && QT_COMPILER_SUPPORTS_SSSE3
extern const uint * QT_FASTCALL fetchPixelsBPP24_ssse3(uint *dest, const uchar*src, int index, int count);
#endif

template<QImage::Format Format>
static const uint *QT_FASTCALL fetchRGBToRGB32(uint *buffer, const uchar *src, int index, int count,
                                               const QVector<QRgb> *, QDitherInfo *)
{
    constexpr QPixelLayout::BPP BPP = bitsPerPixel<Format>();
#if defined(__SSE2__) && !defined(__SSSE3__) && QT_COMPILER_SUPPORTS_SSSE3
    if (BPP == QPixelLayout::BPP24 && qCpuHasFeature(SSSE3)) {
        // With SSE2 can convertToRGB32 be vectorized, but it takes SSSE3
        // to vectorize the deforested version below.
        fetchPixelsBPP24_ssse3(buffer, src, index, count);
        convertToRGB32<Format>(buffer, count, nullptr);
        return buffer;
    }
#endif
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGB32<Format>(qFetchPixel<BPP>(src, index + i));
    return buffer;
}

template<QImage::Format Format>
static Q_ALWAYS_INLINE QRgba64 convertPixelToRGB64(uint s)
{
    return QRgba64::fromArgb32(convertPixelToRGB32<Format>(s));
}

template<QImage::Format Format>
static const QRgba64 *QT_FASTCALL convertToRGB64(QRgba64 *buffer, const uint *src, int count,
                                                 const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGB64<Format>(src[i]);
    return buffer;
}

template<QImage::Format Format>
static const QRgba64 *QT_FASTCALL fetchRGBToRGB64(QRgba64 *buffer, const uchar *src, int index, int count,
                                                  const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGB64<Format>(qFetchPixel<bitsPerPixel<Format>()>(src, index + i));
    return buffer;
}

template<QImage::Format Format>
static Q_ALWAYS_INLINE uint convertPixelToARGB32PM(uint s)
{
    Q_CONSTEXPR uint alphaMask = ((1 << alphaWidth<Format>()) - 1);
    Q_CONSTEXPR uint redMask = ((1 << redWidth<Format>()) - 1);
    Q_CONSTEXPR uint greenMask = ((1 << greenWidth<Format>()) - 1);
    Q_CONSTEXPR uint blueMask = ((1 << blueWidth<Format>()) - 1);

    Q_CONSTEXPR uchar alphaLeftShift = 8 - alphaWidth<Format>();
    Q_CONSTEXPR uchar redLeftShift = 8 - redWidth<Format>();
    Q_CONSTEXPR uchar greenLeftShift = 8 - greenWidth<Format>();
    Q_CONSTEXPR uchar blueLeftShift = 8 - blueWidth<Format>();

    Q_CONSTEXPR uchar alphaRightShift = 2 * alphaWidth<Format>() - 8;
    Q_CONSTEXPR uchar redRightShift = 2 * redWidth<Format>() - 8;
    Q_CONSTEXPR uchar greenRightShift = 2 * greenWidth<Format>() - 8;
    Q_CONSTEXPR uchar blueRightShift = 2 * blueWidth<Format>() - 8;

    Q_CONSTEXPR bool mustMin = (alphaWidth<Format>() != redWidth<Format>()) ||
                               (alphaWidth<Format>() != greenWidth<Format>()) ||
                               (alphaWidth<Format>() != blueWidth<Format>());

    uint alpha = (s >> alphaShift<Format>()) & alphaMask;
    uint red   = (s >> redShift<Format>()) & redMask;
    uint green = (s >> greenShift<Format>()) & greenMask;
    uint blue  = (s >> blueShift<Format>()) & blueMask;

    alpha = (alpha << alphaLeftShift) | (alpha >> alphaRightShift);
    red   = (red << redLeftShift) | (red >> redRightShift);
    green = (green << greenLeftShift) | (green >> greenRightShift);
    blue  = (blue << blueLeftShift) | (blue >> blueRightShift);

    if (mustMin) {
        red   = qMin(alpha, red);
        green = qMin(alpha, green);
        blue  = qMin(alpha, blue);
    }

    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

template<QImage::Format Format>
static void QT_FASTCALL convertARGBPMToARGB32PM(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToARGB32PM<Format>(buffer[i]);
}

template<QImage::Format Format>
static const uint *QT_FASTCALL fetchARGBPMToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                     const QVector<QRgb> *, QDitherInfo *)
{
    constexpr QPixelLayout::BPP BPP = bitsPerPixel<Format>();
#if defined(__SSE2__) && !defined(__SSSE3__) && QT_COMPILER_SUPPORTS_SSSE3
    if (BPP == QPixelLayout::BPP24 && qCpuHasFeature(SSSE3)) {
        // With SSE2 can convertToRGB32 be vectorized, but it takes SSSE3
        // to vectorize the deforested version below.
        fetchPixelsBPP24_ssse3(buffer, src, index, count);
        convertARGBPMToARGB32PM<Format>(buffer, count, nullptr);
        return buffer;
    }
#endif
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToARGB32PM<Format>(qFetchPixel<BPP>(src, index + i));
    return buffer;
}

template<QImage::Format Format>
static Q_ALWAYS_INLINE QRgba64 convertPixelToRGBA64PM(uint s)
{
    return QRgba64::fromArgb32(convertPixelToARGB32PM<Format>(s));
}

template<QImage::Format Format>
static const QRgba64 *QT_FASTCALL convertARGBPMToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                          const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGB64<Format>(src[i]);
    return buffer;
}

template<QImage::Format Format>
static const QRgba64 *QT_FASTCALL fetchARGBPMToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                        const QVector<QRgb> *, QDitherInfo *)
{
    constexpr QPixelLayout::BPP bpp = bitsPerPixel<Format>();
    for (int i = 0; i < count; ++i)
        buffer[i] = convertPixelToRGBA64PM<Format>(qFetchPixel<bpp>(src, index + i));
    return buffer;
}

template<QImage::Format Format, bool fromRGB>
static void QT_FASTCALL storeRGBFromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                             const QVector<QRgb> *, QDitherInfo *dither)
{
    Q_CONSTEXPR uchar rWidth = redWidth<Format>();
    Q_CONSTEXPR uchar gWidth = greenWidth<Format>();
    Q_CONSTEXPR uchar bWidth = blueWidth<Format>();
    constexpr QPixelLayout::BPP BPP = bitsPerPixel<Format>();

    // RGB32 -> RGB888 is not a precision loss.
    if (!dither || (rWidth == 8 && gWidth == 8 && bWidth == 8)) {
        Q_CONSTEXPR uint rMask = (1 << redWidth<Format>()) - 1;
        Q_CONSTEXPR uint gMask = (1 << greenWidth<Format>()) - 1;
        Q_CONSTEXPR uint bMask = (1 << blueWidth<Format>()) - 1;
        Q_CONSTEXPR uchar rRightShift = 24 - redWidth<Format>();
        Q_CONSTEXPR uchar gRightShift = 16 - greenWidth<Format>();
        Q_CONSTEXPR uchar bRightShift =  8 - blueWidth<Format>();

        for (int i = 0; i < count; ++i) {
            const uint c = fromRGB ? src[i] : qUnpremultiply(src[i]);
            const uint r = ((c >> rRightShift) & rMask) << redShift<Format>();
            const uint g = ((c >> gRightShift) & gMask) << greenShift<Format>();
            const uint b = ((c >> bRightShift) & bMask) << blueShift<Format>();
            storePixel<BPP>(dest, index + i, r | g | b);
        };
    } else {
        // We do ordered dither by using a rounding conversion, but instead of
        // adding half of input precision, we add the adjusted result from the
        // bayer matrix before narrowing.
        // Note: Rounding conversion in itself is different from the naive
        // conversion we do above for non-dithering.
        const uint *bayer_line = qt_bayer_matrix[dither->y & 15];
        for (int i = 0; i < count; ++i) {
            const uint c = fromRGB ? src[i] : qUnpremultiply(src[i]);
            const int d = bayer_line[(dither->x + i) & 15];
            const int dr = d - ((d + 1) >> rWidth);
            const int dg = d - ((d + 1) >> gWidth);
            const int db = d - ((d + 1) >> bWidth);
            int r = qRed(c);
            int g = qGreen(c);
            int b = qBlue(c);
            r = (r + ((dr - r) >> rWidth) + 1) >> (8 - rWidth);
            g = (g + ((dg - g) >> gWidth) + 1) >> (8 - gWidth);
            b = (b + ((db - b) >> bWidth) + 1) >> (8 - bWidth);
            const uint s = (r << redShift<Format>())
                         | (g << greenShift<Format>())
                         | (b << blueShift<Format>());
            storePixel<BPP>(dest, index + i, s);
        }
    }
}

template<QImage::Format Format, bool fromRGB>
static void QT_FASTCALL storeARGBPMFromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *dither)
{
    constexpr QPixelLayout::BPP BPP = bitsPerPixel<Format>();
    if (!dither) {
        Q_CONSTEXPR uint aMask = (1 << alphaWidth<Format>()) - 1;
        Q_CONSTEXPR uint rMask = (1 << redWidth<Format>()) - 1;
        Q_CONSTEXPR uint gMask = (1 << greenWidth<Format>()) - 1;
        Q_CONSTEXPR uint bMask = (1 << blueWidth<Format>()) - 1;

        Q_CONSTEXPR uchar aRightShift = 32 - alphaWidth<Format>();
        Q_CONSTEXPR uchar rRightShift = 24 - redWidth<Format>();
        Q_CONSTEXPR uchar gRightShift = 16 - greenWidth<Format>();
        Q_CONSTEXPR uchar bRightShift =  8 - blueWidth<Format>();

        Q_CONSTEXPR uint aOpaque = aMask << alphaShift<Format>();
        for (int i = 0; i < count; ++i) {
            const uint c = src[i];
            const uint a = fromRGB ? aOpaque : (((c >> aRightShift) & aMask) << alphaShift<Format>());
            const uint r = ((c >> rRightShift) & rMask) << redShift<Format>();
            const uint g = ((c >> gRightShift) & gMask) << greenShift<Format>();
            const uint b = ((c >> bRightShift) & bMask) << blueShift<Format>();
            storePixel<BPP>(dest, index + i, a | r | g | b);
        };
    } else {
        Q_CONSTEXPR uchar aWidth = alphaWidth<Format>();
        Q_CONSTEXPR uchar rWidth = redWidth<Format>();
        Q_CONSTEXPR uchar gWidth = greenWidth<Format>();
        Q_CONSTEXPR uchar bWidth = blueWidth<Format>();

        const uint *bayer_line = qt_bayer_matrix[dither->y & 15];
        for (int i = 0; i < count; ++i) {
            const uint c = src[i];
            const int d = bayer_line[(dither->x + i) & 15];
            const int da = d - ((d + 1) >> aWidth);
            const int dr = d - ((d + 1) >> rWidth);
            const int dg = d - ((d + 1) >> gWidth);
            const int db = d - ((d + 1) >> bWidth);
            int a = qAlpha(c);
            int r = qRed(c);
            int g = qGreen(c);
            int b = qBlue(c);
            if (fromRGB)
                a = (1 << aWidth) - 1;
            else
                a = (a + ((da - a) >> aWidth) + 1) >> (8 - aWidth);
            r = (r + ((dr - r) >> rWidth) + 1) >> (8 - rWidth);
            g = (g + ((dg - g) >> gWidth) + 1) >> (8 - gWidth);
            b = (b + ((db - b) >> bWidth) + 1) >> (8 - bWidth);
            uint s = (a << alphaShift<Format>())
                   | (r << redShift<Format>())
                   | (g << greenShift<Format>())
                   | (b << blueShift<Format>());
            storePixel<BPP>(dest, index + i, s);
        }
    }
}

template<QImage::Format Format>
static void QT_FASTCALL rbSwap(uchar *dst, const uchar *src, int count)
{
    Q_CONSTEXPR uchar aWidth = alphaWidth<Format>();
    Q_CONSTEXPR uchar aShift = alphaShift<Format>();
    Q_CONSTEXPR uchar rWidth = redWidth<Format>();
    Q_CONSTEXPR uchar rShift = redShift<Format>();
    Q_CONSTEXPR uchar gWidth = greenWidth<Format>();
    Q_CONSTEXPR uchar gShift = greenShift<Format>();
    Q_CONSTEXPR uchar bWidth = blueWidth<Format>();
    Q_CONSTEXPR uchar bShift = blueShift<Format>();
#ifdef Q_COMPILER_CONSTEXPR
    static_assert(rWidth == bWidth);
#endif
    Q_CONSTEXPR uint redBlueMask = (1 << rWidth) - 1;
    Q_CONSTEXPR uint alphaGreenMask = (((1 << aWidth) - 1) << aShift)
                                    | (((1 << gWidth) - 1) << gShift);
    constexpr QPixelLayout::BPP bpp = bitsPerPixel<Format>();

    for (int i = 0; i < count; ++i) {
        const uint c = qFetchPixel<bpp>(src, i);
        const uint r = (c >> rShift) & redBlueMask;
        const uint b = (c >> bShift) & redBlueMask;
        const uint t = (c & alphaGreenMask)
                     | (r << bShift)
                     | (b << rShift);
        storePixel<bpp>(dst, i, t);
    }
}

static void QT_FASTCALL rbSwap_rgb32(uchar *d, const uchar *s, int count)
{
    const uint *src = reinterpret_cast<const uint *>(s);
    uint *dest = reinterpret_cast<uint *>(d);
    for (int i = 0; i < count; ++i) {
        const uint c = src[i];
        const uint ag = c & 0xff00ff00;
        const uint rb = c & 0x00ff00ff;
        dest[i] = ag | (rb << 16) | (rb >> 16);
    }
}

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
template<>
void QT_FASTCALL rbSwap<QImage::Format_RGBA8888>(uchar *d, const uchar *s, int count)
{
    return rbSwap_rgb32(d, s, count);
}
#else
template<>
void QT_FASTCALL rbSwap<QImage::Format_RGBA8888>(uchar *d, const uchar *s, int count)
{
    const uint *src = reinterpret_cast<const uint *>(s);
    uint *dest = reinterpret_cast<uint *>(d);
    for (int i = 0; i < count; ++i) {
        const uint c = src[i];
        const uint rb = c & 0xff00ff00;
        const uint ga = c & 0x00ff00ff;
        dest[i] = ga | (rb << 16) | (rb >> 16);
    }
}
#endif

static void QT_FASTCALL rbSwap_rgb30(uchar *d, const uchar *s, int count)
{
    const uint *src = reinterpret_cast<const uint *>(s);
    uint *dest = reinterpret_cast<uint *>(d);
    UNALIASED_CONVERSION_LOOP(dest, src, count, qRgbSwapRgb30);
}

template<QImage::Format Format> Q_DECL_CONSTEXPR static inline QPixelLayout pixelLayoutRGB()
{
    return QPixelLayout{
        false,
        false,
        bitsPerPixel<Format>(),
        rbSwap<Format>,
        convertToRGB32<Format>,
        convertToRGB64<Format>,
        fetchRGBToRGB32<Format>,
        fetchRGBToRGB64<Format>,
        storeRGBFromARGB32PM<Format, false>,
        storeRGBFromARGB32PM<Format, true>
    };
}

template<QImage::Format Format> Q_DECL_CONSTEXPR static inline QPixelLayout pixelLayoutARGBPM()
{
    return QPixelLayout{
        true,
        true,
        bitsPerPixel<Format>(),
        rbSwap<Format>,
        convertARGBPMToARGB32PM<Format>,
        convertARGBPMToRGBA64PM<Format>,
        fetchARGBPMToARGB32PM<Format>,
        fetchARGBPMToRGBA64PM<Format>,
        storeARGBPMFromARGB32PM<Format, false>,
        storeARGBPMFromARGB32PM<Format, true>
    };
}

static void QT_FASTCALL convertIndexedToARGB32PM(uint *buffer, int count, const QVector<QRgb> *clut)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = qPremultiply(clut->at(buffer[i]));
}

template<QPixelLayout::BPP BPP>
static const uint *QT_FASTCALL fetchIndexedToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                      const QVector<QRgb> *clut, QDitherInfo *)
{
    for (int i = 0; i < count; ++i) {
        const uint s = qFetchPixel<BPP>(src, index + i);
        buffer[i] = qPremultiply(clut->at(s));
    }
    return buffer;
}

template<QPixelLayout::BPP BPP>
static const QRgba64 *QT_FASTCALL fetchIndexedToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                         const QVector<QRgb> *clut, QDitherInfo *)
{
    for (int i = 0; i < count; ++i) {
        const uint s = qFetchPixel<BPP>(src, index + i);
        buffer[i] = QRgba64::fromArgb32(clut->at(s)).premultiplied();
    }
    return buffer;
}

static const QRgba64 *QT_FASTCALL convertIndexedToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                           const QVector<QRgb> *clut, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(clut->at(src[i])).premultiplied();
    return buffer;
}

static void QT_FASTCALL convertPassThrough(uint *, int, const QVector<QRgb> *)
{
}

static const uint *QT_FASTCALL fetchPassThrough(uint *, const uchar *src, int index, int,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    return reinterpret_cast<const uint *>(src) + index;
}

static const QRgba64 *QT_FASTCALL fetchPassThrough64(QRgba64 *, const uchar *src, int index, int,
                                                     const QVector<QRgb> *, QDitherInfo *)
{
    return reinterpret_cast<const QRgba64 *>(src) + index;
}

static void QT_FASTCALL storePassThrough(uchar *dest, const uint *src, int index, int count,
                                         const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    if (d != src)
        memcpy(d, src, count * sizeof(uint));
}

static void QT_FASTCALL convertARGB32ToARGB32PM(uint *buffer, int count, const QVector<QRgb> *)
{
    qt_convertARGB32ToARGB32PM(buffer, buffer, count);
}

static const uint *QT_FASTCALL fetchARGB32ToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                     const QVector<QRgb> *, QDitherInfo *)
{
    return qt_convertARGB32ToARGB32PM(buffer, reinterpret_cast<const uint *>(src) + index, count);
}

static void QT_FASTCALL convertRGBA8888PMToARGB32PM(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = RGBA2ARGB(buffer[i]);
}

static const uint *QT_FASTCALL fetchRGBA8888PMToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                         const QVector<QRgb> *, QDitherInfo *)
{
    const uint *s  = reinterpret_cast<const uint *>(src) + index;
    UNALIASED_CONVERSION_LOOP(buffer, s, count, RGBA2ARGB);
    return buffer;
}

static void QT_FASTCALL convertRGBA8888ToARGB32PM(uint *buffer, int count, const QVector<QRgb> *)
{
    qt_convertRGBA8888ToARGB32PM(buffer, buffer, count);
}

static const uint *QT_FASTCALL fetchRGBA8888ToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                       const QVector<QRgb> *, QDitherInfo *)
{
    return qt_convertRGBA8888ToARGB32PM(buffer, reinterpret_cast<const uint *>(src) + index, count);
}

static void QT_FASTCALL convertAlpha8ToRGB32(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = qRgba(0, 0, 0, buffer[i]);
}

static const uint *QT_FASTCALL fetchAlpha8ToRGB32(uint *buffer, const uchar *src, int index, int count,
                                                  const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = qRgba(0, 0, 0, src[index + i]);
    return buffer;
}

static const QRgba64 *QT_FASTCALL convertAlpha8ToRGB64(QRgba64 *buffer, const uint *src, int count,
                                                       const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromRgba(0, 0, 0, src[i]);
    return buffer;
}
static const QRgba64 *QT_FASTCALL fetchAlpha8ToRGB64(QRgba64 *buffer, const uchar *src, int index, int count,
                                                     const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromRgba(0, 0, 0, src[index + i]);
    return buffer;
}

static void QT_FASTCALL convertGrayscale8ToRGB32(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i) {
        const uint s = buffer[i];
        buffer[i] = qRgb(s, s, s);
    }
}

static const uint *QT_FASTCALL fetchGrayscale8ToRGB32(uint *buffer, const uchar *src, int index, int count,
                                                      const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i) {
        const uint s = src[index + i];
        buffer[i] = qRgb(s, s, s);
    }
    return buffer;
}

static const QRgba64 *QT_FASTCALL convertGrayscale8ToRGB64(QRgba64 *buffer, const uint *src, int count,
                                                           const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromRgba(src[i], src[i], src[i], 255);
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchGrayscale8ToRGB64(QRgba64 *buffer, const uchar *src, int index, int count,
                                                         const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i) {
        const uint s = src[index + i];
        buffer[i] = QRgba64::fromRgba(s, s, s, 255);
    }
    return buffer;
}

static void QT_FASTCALL convertGrayscale16ToRGB32(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i) {
        const uint x = qt_div_257(buffer[i]);
        buffer[i] = qRgb(x, x, x);
    }
}

static const uint *QT_FASTCALL fetchGrayscale16ToRGB32(uint *buffer, const uchar *src, int index, int count,
                                                      const QVector<QRgb> *, QDitherInfo *)
{
    const unsigned short *s = reinterpret_cast<const unsigned short *>(src) + index;
    for (int i = 0; i < count; ++i) {
        const uint x = qt_div_257(s[i]);
        buffer[i] = qRgb(x, x, x);
    }
    return buffer;
}

static const QRgba64 *QT_FASTCALL convertGrayscale16ToRGBA64(QRgba64 *buffer, const uint *src, int count,
                                                           const QVector<QRgb> *, QDitherInfo *)
{
    const unsigned short *s = reinterpret_cast<const unsigned short *>(src);
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromRgba64(s[i], s[i], s[i], 65535);
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchGrayscale16ToRGBA64(QRgba64 *buffer, const uchar *src, int index, int count,
                                                         const QVector<QRgb> *, QDitherInfo *)
{
    const unsigned short *s = reinterpret_cast<const unsigned short *>(src) + index;
    for (int i = 0; i < count; ++i) {
        buffer[i] = QRgba64::fromRgba64(s[i], s[i], s[i], 65535);
    }
    return buffer;
}

static void QT_FASTCALL storeARGB32FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, [](uint c) { return qUnpremultiply(c); });
}

static void QT_FASTCALL storeRGBA8888PMFromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                    const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, ARGB2RGBA);
}

#ifdef __SSE2__
template<bool RGBA, bool maskAlpha>
static inline void qConvertARGB32PMToRGBA64PM_sse2(QRgba64 *buffer, const uint *src, int count)
{
    if (count <= 0)
        return;

    const __m128i amask = _mm_set1_epi32(0xff000000);
    int i = 0;
    for (; ((uintptr_t)buffer & 0xf) && i < count; ++i) {
        uint s = *src++;
        if (maskAlpha)
            s = s | 0xff000000;
        if (RGBA)
            s = RGBA2ARGB(s);
        *buffer++ = QRgba64::fromArgb32(s);
    }
    for (; i < count-3; i += 4) {
        __m128i vs = _mm_loadu_si128((const __m128i*)src);
        if (maskAlpha)
            vs = _mm_or_si128(vs, amask);
        src += 4;
        __m128i v1 = _mm_unpacklo_epi8(vs, vs);
        __m128i v2 = _mm_unpackhi_epi8(vs, vs);
        if (!RGBA) {
            v1 = _mm_shufflelo_epi16(v1, _MM_SHUFFLE(3, 0, 1, 2));
            v2 = _mm_shufflelo_epi16(v2, _MM_SHUFFLE(3, 0, 1, 2));
            v1 = _mm_shufflehi_epi16(v1, _MM_SHUFFLE(3, 0, 1, 2));
            v2 = _mm_shufflehi_epi16(v2, _MM_SHUFFLE(3, 0, 1, 2));
        }
        _mm_store_si128((__m128i*)(buffer), v1);
        buffer += 2;
        _mm_store_si128((__m128i*)(buffer), v2);
        buffer += 2;
    }

    SIMD_EPILOGUE(i, count, 3) {
        uint s = *src++;
        if (maskAlpha)
            s = s | 0xff000000;
        if (RGBA)
            s = RGBA2ARGB(s);
        *buffer++ = QRgba64::fromArgb32(s);
    }
}

template<QtPixelOrder PixelOrder>
static inline void qConvertRGBA64PMToA2RGB30PM_sse2(uint *dest, const QRgba64 *buffer, int count)
{
    const __m128i gmask = _mm_set1_epi32(0x000ffc00);
    const __m128i cmask = _mm_set1_epi32(0x000003ff);
    int i = 0;
    __m128i vr, vg, vb, va;
    for (; i < count && uintptr_t(buffer) & 0xF; ++i) {
        *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
    }

    for (; i < count-15; i += 16) {
        // Repremultiplying is really expensive and hard to do in SIMD without AVX2,
        // so we try to avoid it by checking if it is needed 16 samples at a time.
        __m128i vOr = _mm_set1_epi32(0);
        __m128i vAnd = _mm_set1_epi32(0xffffffff);
        for (int j = 0; j < 16; j += 2) {
            __m128i vs = _mm_load_si128((const __m128i*)(buffer + j));
            vOr = _mm_or_si128(vOr, vs);
            vAnd = _mm_and_si128(vAnd, vs);
        }
        const quint16 orAlpha = ((uint)_mm_extract_epi16(vOr, 3)) | ((uint)_mm_extract_epi16(vOr, 7));
        const quint16 andAlpha = ((uint)_mm_extract_epi16(vAnd, 3)) & ((uint)_mm_extract_epi16(vAnd, 7));

        if (andAlpha == 0xffff) {
            for (int j = 0; j < 16; j += 2) {
                __m128i vs = _mm_load_si128((const __m128i*)buffer);
                buffer += 2;
                vr = _mm_srli_epi64(vs, 6);
                vg = _mm_srli_epi64(vs, 16 + 6 - 10);
                vb = _mm_srli_epi64(vs, 32 + 6);
                vr = _mm_and_si128(vr, cmask);
                vg = _mm_and_si128(vg, gmask);
                vb = _mm_and_si128(vb, cmask);
                va = _mm_srli_epi64(vs, 48 + 14);
                if (PixelOrder == PixelOrderRGB)
                    vr = _mm_slli_epi32(vr, 20);
                else
                    vb = _mm_slli_epi32(vb, 20);
                va = _mm_slli_epi32(va, 30);
                __m128i vd = _mm_or_si128(_mm_or_si128(vr, vg), _mm_or_si128(vb, va));
                vd = _mm_shuffle_epi32(vd, _MM_SHUFFLE(3, 1, 2, 0));
                _mm_storel_epi64((__m128i*)dest, vd);
                dest += 2;
            }
        } else if (orAlpha == 0) {
            for (int j = 0; j < 16; ++j) {
                *dest++ = 0;
                buffer++;
            }
        } else {
            for (int j = 0; j < 16; ++j)
                *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
        }
    }

    SIMD_EPILOGUE(i, count, 15)
        *dest++ = qConvertRgb64ToRgb30<PixelOrder>(*buffer++);
}
#elif defined(__ARM_NEON__)
template<bool RGBA, bool maskAlpha>
static inline void qConvertARGB32PMToRGBA64PM_neon(QRgba64 *buffer, const uint *src, int count)
{
    if (count <= 0)
        return;

    const uint32x4_t amask = vdupq_n_u32(0xff000000);
#if defined(Q_PROCESSOR_ARM_64)
    const uint8x16_t rgbaMask  = { 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15};
#else
    const uint8x8_t rgbaMask  = { 2, 1, 0, 3, 6, 5, 4, 7 };
#endif
    int i = 0;
    for (; i < count-3; i += 4) {
        uint32x4_t vs32 = vld1q_u32(src);
        src += 4;
        if (maskAlpha)
            vs32 = vorrq_u32(vs32, amask);
        uint8x16_t vs8 = vreinterpretq_u8_u32(vs32);
        if (!RGBA) {
#if defined(Q_PROCESSOR_ARM_64)
            vs8 = vqtbl1q_u8(vs8, rgbaMask);
#else
            // no vqtbl1q_u8
            const uint8x8_t vlo = vtbl1_u8(vget_low_u8(vs8), rgbaMask);
            const uint8x8_t vhi = vtbl1_u8(vget_high_u8(vs8), rgbaMask);
            vs8 = vcombine_u8(vlo, vhi);
#endif
        }
        uint8x16x2_t v = vzipq_u8(vs8, vs8);

        vst1q_u16((uint16_t *)buffer, vreinterpretq_u16_u8(v.val[0]));
        buffer += 2;
        vst1q_u16((uint16_t *)buffer, vreinterpretq_u16_u8(v.val[1]));
        buffer += 2;
    }

    SIMD_EPILOGUE(i, count, 3) {
        uint s = *src++;
        if (maskAlpha)
            s = s | 0xff000000;
        if (RGBA)
            s = RGBA2ARGB(s);
        *buffer++ = QRgba64::fromArgb32(s);
    }
}
#endif

static const QRgba64 *QT_FASTCALL convertRGB32ToRGB64(QRgba64 *buffer, const uint *src, int count,
                                                      const QVector<QRgb> *, QDitherInfo *)
{
#ifdef __SSE2__
    qConvertARGB32PMToRGBA64PM_sse2<false, true>(buffer, src, count);
#elif defined(__ARM_NEON__)
    qConvertARGB32PMToRGBA64PM_neon<false, true>(buffer, src, count);
#else
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(0xff000000 | src[i]);
#endif
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchRGB32ToRGB64(QRgba64 *buffer, const uchar *src, int index, int count,
                                                    const QVector<QRgb> *, QDitherInfo *)
{
    return convertRGB32ToRGB64(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

static const QRgba64 *QT_FASTCALL convertARGB32ToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                          const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(src[i]).premultiplied();
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchARGB32ToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                        const QVector<QRgb> *, QDitherInfo *)
{
    return convertARGB32ToRGBA64PM(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

static const QRgba64 *QT_FASTCALL convertARGB32PMToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                            const QVector<QRgb> *, QDitherInfo *)
{
#ifdef __SSE2__
    qConvertARGB32PMToRGBA64PM_sse2<false, false>(buffer, src, count);
#elif defined(__ARM_NEON__)
    qConvertARGB32PMToRGBA64PM_neon<false, false>(buffer, src, count);
#else
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(src[i]);
#endif
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchARGB32PMToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                          const QVector<QRgb> *, QDitherInfo *)
{
    return convertARGB32PMToRGBA64PM(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

static const QRgba64 *QT_FASTCALL fetchRGBA64ToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                        const QVector<QRgb> *, QDitherInfo *)
{
    const QRgba64 *s = reinterpret_cast<const QRgba64 *>(src) + index;
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromRgba64(s[i]).premultiplied();
    return buffer;
}

static const QRgba64 *QT_FASTCALL convertRGBA8888ToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                            const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(RGBA2ARGB(src[i])).premultiplied();
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchRGBA8888ToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                          const QVector<QRgb> *, QDitherInfo *)
{
    return convertRGBA8888ToRGBA64PM(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

static const QRgba64 *QT_FASTCALL convertRGBA8888PMToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                              const QVector<QRgb> *, QDitherInfo *)
{
#ifdef __SSE2__
    qConvertARGB32PMToRGBA64PM_sse2<true, false>(buffer, src, count);
#elif defined(__ARM_NEON__)
    qConvertARGB32PMToRGBA64PM_neon<true, false>(buffer, src, count);
#else
    for (int i = 0; i < count; ++i)
        buffer[i] = QRgba64::fromArgb32(RGBA2ARGB(src[i]));
#endif
    return buffer;
}

static const QRgba64 *QT_FASTCALL fetchRGBA8888PMToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                            const QVector<QRgb> *, QDitherInfo *)
{
    return convertRGBA8888PMToRGBA64PM(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

static void QT_FASTCALL storeRGBA8888FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                  const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, [](uint c) { return ARGB2RGBA(qUnpremultiply(c)); });
}

static void QT_FASTCALL storeRGBXFromRGB32(uchar *dest, const uint *src, int index, int count,
                                           const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, [](uint c) { return ARGB2RGBA(0xff000000 | c); });
}

static void QT_FASTCALL storeRGBXFromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                              const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, [](uint c) { return ARGB2RGBA(0xff000000 | qUnpremultiply(c)); });
}

template<QtPixelOrder PixelOrder>
static void QT_FASTCALL convertA2RGB30PMToARGB32PM(uint *buffer, int count, const QVector<QRgb> *)
{
    for (int i = 0; i < count; ++i)
        buffer[i] = qConvertA2rgb30ToArgb32<PixelOrder>(buffer[i]);
}

template<QtPixelOrder PixelOrder>
static const uint *QT_FASTCALL fetchA2RGB30PMToARGB32PM(uint *buffer, const uchar *s, int index, int count,
                                                        const QVector<QRgb> *, QDitherInfo *dither)
{
    const uint *src = reinterpret_cast<const uint *>(s) + index;
    if (!dither) {
        UNALIASED_CONVERSION_LOOP(buffer, src, count, qConvertA2rgb30ToArgb32<PixelOrder>);
    } else {
        for (int i = 0; i < count; ++i) {
            const uint c = src[i];
            short d10 = (qt_bayer_matrix[dither->y & 15][(dither->x + i) & 15] << 2);
            short a10 = (c >> 30) * 0x155;
            short r10 = ((c >> 20) & 0x3ff);
            short g10 = ((c >> 10) & 0x3ff);
            short b10 = (c & 0x3ff);
            if (PixelOrder == PixelOrderBGR)
                std::swap(r10, b10);
            short a8 = (a10 + ((d10 - a10) >> 8)) >> 2;
            short r8 = (r10 + ((d10 - r10) >> 8)) >> 2;
            short g8 = (g10 + ((d10 - g10) >> 8)) >> 2;
            short b8 = (b10 + ((d10 - b10) >> 8)) >> 2;
            buffer[i] = qRgba(r8, g8, b8, a8);
        }
    }
    return buffer;
}

#ifdef __SSE2__
template<QtPixelOrder PixelOrder>
static inline void qConvertA2RGB30PMToRGBA64PM_sse2(QRgba64 *buffer, const uint *src, int count)
{
    if (count <= 0)
        return;

    const __m128i rmask = _mm_set1_epi32(0x3ff00000);
    const __m128i gmask = _mm_set1_epi32(0x000ffc00);
    const __m128i bmask = _mm_set1_epi32(0x000003ff);
    const __m128i afactor = _mm_set1_epi16(0x5555);
    int i = 0;

    for (; ((uintptr_t)buffer & 0xf) && i < count; ++i)
        *buffer++ = qConvertA2rgb30ToRgb64<PixelOrder>(*src++);

    for (; i < count-3; i += 4) {
        __m128i vs = _mm_loadu_si128((const __m128i*)src);
        src += 4;
        __m128i va = _mm_srli_epi32(vs, 30);
        __m128i vr = _mm_and_si128(vs, rmask);
        __m128i vb = _mm_and_si128(vs, bmask);
        __m128i vg = _mm_and_si128(vs, gmask);
        va = _mm_mullo_epi16(va, afactor);
        vr = _mm_or_si128(_mm_srli_epi32(vr, 14), _mm_srli_epi32(vr, 24));
        vg = _mm_or_si128(_mm_srli_epi32(vg, 4), _mm_srli_epi32(vg, 14));
        vb = _mm_or_si128(_mm_slli_epi32(vb, 6), _mm_srli_epi32(vb, 4));
        __m128i vrb;
        if (PixelOrder == PixelOrderRGB)
             vrb = _mm_or_si128(vr, _mm_slli_si128(vb, 2));
        else
             vrb = _mm_or_si128(vb, _mm_slli_si128(vr, 2));
        __m128i vga = _mm_or_si128(vg, _mm_slli_si128(va, 2));
        _mm_store_si128((__m128i*)(buffer), _mm_unpacklo_epi16(vrb, vga));
        buffer += 2;
        _mm_store_si128((__m128i*)(buffer), _mm_unpackhi_epi16(vrb, vga));
        buffer += 2;
    }

    SIMD_EPILOGUE(i, count, 3)
        *buffer++ = qConvertA2rgb30ToRgb64<PixelOrder>(*src++);
}
#endif

template<QtPixelOrder PixelOrder>
static const QRgba64 *QT_FASTCALL convertA2RGB30PMToRGBA64PM(QRgba64 *buffer, const uint *src, int count,
                                                             const QVector<QRgb> *, QDitherInfo *)
{
#ifdef __SSE2__
    qConvertA2RGB30PMToRGBA64PM_sse2<PixelOrder>(buffer, src, count);
#else
    for (int i = 0; i < count; ++i)
        buffer[i] = qConvertA2rgb30ToRgb64<PixelOrder>(src[i]);
#endif
    return buffer;
}

template<QtPixelOrder PixelOrder>
static const QRgba64 *QT_FASTCALL fetchA2RGB30PMToRGBA64PM(QRgba64 *buffer, const uchar *src, int index, int count,
                                                           const QVector<QRgb> *, QDitherInfo *)
{
    return convertA2RGB30PMToRGBA64PM<PixelOrder>(buffer, reinterpret_cast<const uint *>(src) + index, count, nullptr, nullptr);
}

template<QtPixelOrder PixelOrder>
static void QT_FASTCALL storeA2RGB30PMFromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                   const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, qConvertArgb32ToA2rgb30<PixelOrder>);
}

template<QtPixelOrder PixelOrder>
static void QT_FASTCALL storeRGB30FromRGB32(uchar *dest, const uint *src, int index, int count,
                                            const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, qConvertRgb32ToRgb30<PixelOrder>);
}

template<QtPixelOrder PixelOrder>
static void QT_FASTCALL storeRGB30FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                               const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = reinterpret_cast<uint *>(dest) + index;
    UNALIASED_CONVERSION_LOOP(d, src, count, qConvertRgb32ToRgb30<PixelOrder>);
}

template<bool RGBA>
void qt_convertRGBA64ToARGB32(uint *dst, const QRgba64 *src, int count)
{
    int i = 0;
#ifdef __SSE2__
    if (((uintptr_t)dst & 0x7) && count > 0) {
        uint s = (*src++).toArgb32();
        if (RGBA)
            s = ARGB2RGBA(s);
        *dst++ = s;
        i++;
    }
    const __m128i vhalf = _mm_set1_epi32(0x80);
    const __m128i vzero = _mm_setzero_si128();
    for (; i < count-1; i += 2) {
        __m128i vs = _mm_loadu_si128((const __m128i*)src);
        src += 2;
        if (!RGBA) {
            vs = _mm_shufflelo_epi16(vs, _MM_SHUFFLE(3, 0, 1, 2));
            vs = _mm_shufflehi_epi16(vs, _MM_SHUFFLE(3, 0, 1, 2));
        }
        __m128i v1 = _mm_unpacklo_epi16(vs, vzero);
        __m128i v2 = _mm_unpackhi_epi16(vs, vzero);
        v1 = _mm_add_epi32(v1, vhalf);
        v2 = _mm_add_epi32(v2, vhalf);
        v1 = _mm_sub_epi32(v1, _mm_srli_epi32(v1, 8));
        v2 = _mm_sub_epi32(v2, _mm_srli_epi32(v2, 8));
        v1 = _mm_srli_epi32(v1, 8);
        v2 = _mm_srli_epi32(v2, 8);
        v1 = _mm_packs_epi32(v1, v2);
        v1 = _mm_packus_epi16(v1, vzero);
        _mm_storel_epi64((__m128i*)(dst), v1);
        dst += 2;
    }
#endif
    for (; i < count; i++) {
        uint s = (*src++).toArgb32();
        if (RGBA)
            s = ARGB2RGBA(s);
        *dst++ = s;
    }
}
template void qt_convertRGBA64ToARGB32<false>(uint *dst, const QRgba64 *src, int count);
template void qt_convertRGBA64ToARGB32<true>(uint *dst, const QRgba64 *src, int count);


static void QT_FASTCALL storeAlpha8FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        dest[index + i] = qAlpha(src[i]);
}

static void QT_FASTCALL storeGrayscale8FromRGB32(uchar *dest, const uint *src, int index, int count,
                                                 const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        dest[index + i] = qGray(src[i]);
}

static void QT_FASTCALL storeGrayscale8FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                    const QVector<QRgb> *, QDitherInfo *)
{
    for (int i = 0; i < count; ++i)
        dest[index + i] = qGray(qUnpremultiply(src[i]));
}

static void QT_FASTCALL storeGrayscale16FromRGB32(uchar *dest, const uint *src, int index, int count,
                                                 const QVector<QRgb> *, QDitherInfo *)
{
    unsigned short *d = reinterpret_cast<unsigned short *>(dest) + index;
    for (int i = 0; i < count; ++i)
        d[i] = qGray(src[i]) * 257;
}

static void QT_FASTCALL storeGrayscale16FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                    const QVector<QRgb> *, QDitherInfo *)
{
    unsigned short *d = reinterpret_cast<unsigned short *>(dest) + index;
    for (int i = 0; i < count; ++i)
        d[i] = qGray(qUnpremultiply(src[i])) * 257;
}

static const uint *QT_FASTCALL fetchRGB64ToRGB32(uint *buffer, const uchar *src, int index, int count,
                                                 const QVector<QRgb> *, QDitherInfo *)
{
    const QRgba64 *s = reinterpret_cast<const QRgba64 *>(src) + index;
    for (int i = 0; i < count; ++i)
        buffer[i] = toArgb32(s[i]);
    return buffer;
}

static void QT_FASTCALL storeRGB64FromRGB32(uchar *dest, const uint *src, int index, int count,
                                            const QVector<QRgb> *, QDitherInfo *)
{
    QRgba64 *d = reinterpret_cast<QRgba64 *>(dest) + index;
    for (int i = 0; i < count; ++i)
        d[i] = QRgba64::fromArgb32(src[i]);
}

static const uint *QT_FASTCALL fetchRGBA64ToARGB32PM(uint *buffer, const uchar *src, int index, int count,
                                                     const QVector<QRgb> *, QDitherInfo *)
{
    const QRgba64 *s = reinterpret_cast<const QRgba64 *>(src) + index;
    for (int i = 0; i < count; ++i)
        buffer[i] = toArgb32(s[i].premultiplied());
    return buffer;
}

static void QT_FASTCALL storeRGBA64FromARGB32PM(uchar *dest, const uint *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    QRgba64 *d = reinterpret_cast<QRgba64 *>(dest) + index;
    for (int i = 0; i < count; ++i)
        d[i] = QRgba64::fromArgb32(src[i]).unpremultiplied();
}

// Note:
// convertToArgb32() assumes that no color channel is less than 4 bits.
// storeRGBFromARGB32PM() assumes that no color channel is more than 8 bits.
// QImage::rgbSwapped() assumes that the red and blue color channels have the same number of bits.
QPixelLayout qPixelLayouts[QImage::NImageFormats] = {
    { false, false, QPixelLayout::BPPNone, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }, // Format_Invalid
    { false, false, QPixelLayout::BPP1MSB, nullptr,
      convertIndexedToARGB32PM, convertIndexedToRGBA64PM,
      fetchIndexedToARGB32PM<QPixelLayout::BPP1MSB>, fetchIndexedToRGBA64PM<QPixelLayout::BPP1MSB>,
      nullptr, nullptr }, // Format_Mono
    { false, false, QPixelLayout::BPP1LSB, nullptr,
      convertIndexedToARGB32PM, convertIndexedToRGBA64PM,
      fetchIndexedToARGB32PM<QPixelLayout::BPP1LSB>, fetchIndexedToRGBA64PM<QPixelLayout::BPP1LSB>,
      nullptr, nullptr }, // Format_MonoLSB
    { false, false, QPixelLayout::BPP8, nullptr,
      convertIndexedToARGB32PM, convertIndexedToRGBA64PM,
      fetchIndexedToARGB32PM<QPixelLayout::BPP8>, fetchIndexedToRGBA64PM<QPixelLayout::BPP8>,
      nullptr, nullptr }, // Format_Indexed8
    // Technically using convertPassThrough to convert from ARGB32PM to RGB32 is wrong,
    // but everywhere this generic conversion would be wrong is currently overloaded.
    { false, false, QPixelLayout::BPP32, rbSwap_rgb32, convertPassThrough,
      convertRGB32ToRGB64, fetchPassThrough, fetchRGB32ToRGB64, storePassThrough, storePassThrough }, // Format_RGB32
    { true, false, QPixelLayout::BPP32, rbSwap_rgb32, convertARGB32ToARGB32PM,
      convertARGB32ToRGBA64PM, fetchARGB32ToARGB32PM, fetchARGB32ToRGBA64PM, storeARGB32FromARGB32PM, storePassThrough }, // Format_ARGB32
    { true, true, QPixelLayout::BPP32, rbSwap_rgb32, convertPassThrough,
      convertARGB32PMToRGBA64PM, fetchPassThrough, fetchARGB32PMToRGBA64PM, storePassThrough, storePassThrough }, // Format_ARGB32_Premultiplied
    pixelLayoutRGB<QImage::Format_RGB16>(),
    pixelLayoutARGBPM<QImage::Format_ARGB8565_Premultiplied>(),
    pixelLayoutRGB<QImage::Format_RGB666>(),
    pixelLayoutARGBPM<QImage::Format_ARGB6666_Premultiplied>(),
    pixelLayoutRGB<QImage::Format_RGB555>(),
    pixelLayoutARGBPM<QImage::Format_ARGB8555_Premultiplied>(),
    pixelLayoutRGB<QImage::Format_RGB888>(),
    pixelLayoutRGB<QImage::Format_RGB444>(),
    pixelLayoutARGBPM<QImage::Format_ARGB4444_Premultiplied>(),
    { false, false, QPixelLayout::BPP32, rbSwap<QImage::Format_RGBA8888>, convertRGBA8888PMToARGB32PM,
      convertRGBA8888PMToRGBA64PM, fetchRGBA8888PMToARGB32PM, fetchRGBA8888PMToRGBA64PM, storeRGBXFromARGB32PM, storeRGBXFromRGB32 }, // Format_RGBX8888
    { true, false, QPixelLayout::BPP32, rbSwap<QImage::Format_RGBA8888>, convertRGBA8888ToARGB32PM,
      convertRGBA8888ToRGBA64PM, fetchRGBA8888ToARGB32PM, fetchRGBA8888ToRGBA64PM, storeRGBA8888FromARGB32PM, storeRGBXFromRGB32 }, // Format_RGBA8888
    { true, true, QPixelLayout::BPP32, rbSwap<QImage::Format_RGBA8888>, convertRGBA8888PMToARGB32PM,
      convertRGBA8888PMToRGBA64PM, fetchRGBA8888PMToARGB32PM, fetchRGBA8888PMToRGBA64PM, storeRGBA8888PMFromARGB32PM, storeRGBXFromRGB32 },  // Format_RGBA8888_Premultiplied
    { false, false, QPixelLayout::BPP32, rbSwap_rgb30,
      convertA2RGB30PMToARGB32PM<PixelOrderBGR>,
      convertA2RGB30PMToRGBA64PM<PixelOrderBGR>,
      fetchA2RGB30PMToARGB32PM<PixelOrderBGR>,
      fetchA2RGB30PMToRGBA64PM<PixelOrderBGR>,
      storeRGB30FromARGB32PM<PixelOrderBGR>,
      storeRGB30FromRGB32<PixelOrderBGR>
    }, // Format_BGR30
    { true, true, QPixelLayout::BPP32, rbSwap_rgb30,
      convertA2RGB30PMToARGB32PM<PixelOrderBGR>,
      convertA2RGB30PMToRGBA64PM<PixelOrderBGR>,
      fetchA2RGB30PMToARGB32PM<PixelOrderBGR>,
      fetchA2RGB30PMToRGBA64PM<PixelOrderBGR>,
      storeA2RGB30PMFromARGB32PM<PixelOrderBGR>,
      storeRGB30FromRGB32<PixelOrderBGR>
    },  // Format_A2BGR30_Premultiplied
    { false, false, QPixelLayout::BPP32, rbSwap_rgb30,
      convertA2RGB30PMToARGB32PM<PixelOrderRGB>,
      convertA2RGB30PMToRGBA64PM<PixelOrderRGB>,
      fetchA2RGB30PMToARGB32PM<PixelOrderRGB>,
      fetchA2RGB30PMToRGBA64PM<PixelOrderRGB>,
      storeRGB30FromARGB32PM<PixelOrderRGB>,
      storeRGB30FromRGB32<PixelOrderRGB>
    }, // Format_RGB30
    { true, true, QPixelLayout::BPP32, rbSwap_rgb30,
      convertA2RGB30PMToARGB32PM<PixelOrderRGB>,
      convertA2RGB30PMToRGBA64PM<PixelOrderRGB>,
      fetchA2RGB30PMToARGB32PM<PixelOrderRGB>,
      fetchA2RGB30PMToRGBA64PM<PixelOrderRGB>,
      storeA2RGB30PMFromARGB32PM<PixelOrderRGB>,
      storeRGB30FromRGB32<PixelOrderRGB>
    },  // Format_A2RGB30_Premultiplied
    { true, true, QPixelLayout::BPP8, nullptr,
      convertAlpha8ToRGB32, convertAlpha8ToRGB64,
      fetchAlpha8ToRGB32, fetchAlpha8ToRGB64,
      storeAlpha8FromARGB32PM, nullptr }, // Format_Alpha8
    { false, false, QPixelLayout::BPP8, nullptr,
      convertGrayscale8ToRGB32, convertGrayscale8ToRGB64,
      fetchGrayscale8ToRGB32, fetchGrayscale8ToRGB64,
      storeGrayscale8FromARGB32PM, storeGrayscale8FromRGB32 }, // Format_Grayscale8
    { false, false, QPixelLayout::BPP64, nullptr,
      convertPassThrough, nullptr,
      fetchRGB64ToRGB32, fetchPassThrough64,
      storeRGB64FromRGB32, storeRGB64FromRGB32 }, // Format_RGBX64
    { true, false, QPixelLayout::BPP64, nullptr,
      convertARGB32ToARGB32PM, nullptr,
      fetchRGBA64ToARGB32PM, fetchRGBA64ToRGBA64PM,
      storeRGBA64FromARGB32PM, storeRGB64FromRGB32 }, // Format_RGBA64
    { true, true, QPixelLayout::BPP64, nullptr,
      convertPassThrough, nullptr,
      fetchRGB64ToRGB32, fetchPassThrough64,
      storeRGB64FromRGB32, storeRGB64FromRGB32 }, // Format_RGBA64_Premultiplied
    { false, false, QPixelLayout::BPP16, nullptr,
      convertGrayscale16ToRGB32, convertGrayscale16ToRGBA64,
      fetchGrayscale16ToRGB32, fetchGrayscale16ToRGBA64,
      storeGrayscale16FromARGB32PM, storeGrayscale16FromRGB32 }, // Format_Grayscale16
    pixelLayoutRGB<QImage::Format_BGR888>(),
};

static_assert(sizeof(qPixelLayouts) / sizeof(*qPixelLayouts) == QImage::NImageFormats);

static void QT_FASTCALL convertFromRgb64(uint *dest, const QRgba64 *src, int length)
{
    for (int i = 0; i < length; ++i) {
        dest[i] = toArgb32(src[i]);
    }
}

template<QImage::Format format>
static void QT_FASTCALL storeGenericFromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                 const QVector<QRgb> *clut, QDitherInfo *dither)
{
    uint buffer[BufferSize];
    convertFromRgb64(buffer, src, count);
    qPixelLayouts[format].storeFromARGB32PM(dest, buffer, index, count, clut, dither);
}

static void QT_FASTCALL storeARGB32FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = (uint*)dest + index;
    for (int i = 0; i < count; ++i)
        d[i] = toArgb32(src[i].unpremultiplied());
}

static void QT_FASTCALL storeRGBA8888FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                  const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = (uint*)dest + index;
    for (int i = 0; i < count; ++i)
        d[i] = toRgba8888(src[i].unpremultiplied());
}

template<QtPixelOrder PixelOrder>
static void QT_FASTCALL storeRGB30FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                               const QVector<QRgb> *, QDitherInfo *)
{
    uint *d = (uint*)dest + index;
#ifdef __SSE2__
    qConvertRGBA64PMToA2RGB30PM_sse2<PixelOrder>(d, src, count);
#else
    for (int i = 0; i < count; ++i)
        d[i] = qConvertRgb64ToRgb30<PixelOrder>(src[i]);
#endif
}

static void QT_FASTCALL storeRGBX64FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    QRgba64 *d = reinterpret_cast<QRgba64*>(dest) + index;
    for (int i = 0; i < count; ++i) {
        d[i] = src[i].unpremultiplied();
        d[i].setAlpha(65535);
    }
}

static void QT_FASTCALL storeRGBA64FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    QRgba64 *d = reinterpret_cast<QRgba64*>(dest) + index;
    for (int i = 0; i < count; ++i)
        d[i] = src[i].unpremultiplied();
}

static void QT_FASTCALL storeRGBA64PMFromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                  const QVector<QRgb> *, QDitherInfo *)
{
    QRgba64 *d = reinterpret_cast<QRgba64*>(dest) + index;
    if (d != src)
        memcpy(d, src, count * sizeof(QRgba64));
}

static void QT_FASTCALL storeGray16FromRGBA64PM(uchar *dest, const QRgba64 *src, int index, int count,
                                                const QVector<QRgb> *, QDitherInfo *)
{
    quint16 *d = reinterpret_cast<quint16*>(dest) + index;
    for (int i = 0; i < count; ++i) {
        QRgba64 s =  src[i].unpremultiplied();
        d[i] = qGray(s.red(), s.green(), s.blue());
    }
}

ConvertAndStorePixelsFunc64 qStoreFromRGBA64PM[QImage::NImageFormats] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    storeGenericFromRGBA64PM<QImage::Format_RGB32>,
    storeARGB32FromRGBA64PM,
    storeGenericFromRGBA64PM<QImage::Format_ARGB32_Premultiplied>,
    storeGenericFromRGBA64PM<QImage::Format_RGB16>,
    storeGenericFromRGBA64PM<QImage::Format_ARGB8565_Premultiplied>,
    storeGenericFromRGBA64PM<QImage::Format_RGB666>,
    storeGenericFromRGBA64PM<QImage::Format_ARGB6666_Premultiplied>,
    storeGenericFromRGBA64PM<QImage::Format_RGB555>,
    storeGenericFromRGBA64PM<QImage::Format_ARGB8555_Premultiplied>,
    storeGenericFromRGBA64PM<QImage::Format_RGB888>,
    storeGenericFromRGBA64PM<QImage::Format_RGB444>,
    storeGenericFromRGBA64PM<QImage::Format_ARGB4444_Premultiplied>,
    storeGenericFromRGBA64PM<QImage::Format_RGBX8888>,
    storeRGBA8888FromRGBA64PM,
    storeGenericFromRGBA64PM<QImage::Format_RGBA8888_Premultiplied>,
    storeRGB30FromRGBA64PM<PixelOrderBGR>,
    storeRGB30FromRGBA64PM<PixelOrderBGR>,
    storeRGB30FromRGBA64PM<PixelOrderRGB>,
    storeRGB30FromRGBA64PM<PixelOrderRGB>,
    storeGenericFromRGBA64PM<QImage::Format_Alpha8>,
    storeGenericFromRGBA64PM<QImage::Format_Grayscale8>,
    storeRGBX64FromRGBA64PM,
    storeRGBA64FromRGBA64PM,
    storeRGBA64PMFromRGBA64PM,
    storeGray16FromRGBA64PM,
    storeGenericFromRGBA64PM<QImage::Format_BGR888>,
};

QT_END_NAMESPACE