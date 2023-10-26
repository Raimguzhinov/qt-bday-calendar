#pragma once

#include <QImage>

class ImageManager
{
public:
    static QImage normallyResized(const QImage &image, int maximumSideSize);
    static QImage squared(const QImage &image, int size);
    static QImage roundSquared(const QImage &image, int size, int radius);
    static QImage addShadow(const QImage &image,
                            QColor color,
                            QPoint offset,
                            bool canResize = false);
    static QImage rotatedImage(const QImage &image, qreal angle);
    static void drawNinePartImage(const QImage &image,
                                  QRectF paintRect,
                                  qreal borderLeft,
                                  qreal borderRight,
                                  qreal borderTop,
                                  qreal borderBottom,
                                  QPainter *painter);
    static void drawNinePartImage(const QImage &image,
                                  QRectF paintRect,
                                  qreal border,
                                  QPainter *painter);
    static QColor resolveColor(const QString &name);
};
