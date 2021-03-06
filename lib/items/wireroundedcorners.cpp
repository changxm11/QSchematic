#include <QJsonObject>
#include <QPainter>
#include <QVector2D>
#include "wirepoint.h"
#include "line.h"
#include "../utils.h"
#include "wireroundedcorners.h"

const qreal BOUNDING_RECT_PADDING = 6.0;
const qreal HANDLE_SIZE           = 3.0;
const qreal WIRE_SHAPE_PADDING    = 10;
const int LINE_WIDTH              = 2;
const QColor COLOR                = QColor("#000000");
const QColor COLOR_HIGHLIGHTED    = QColor("#dc2479");
const QColor COLOR_SELECTED       = QColor("#0f16af");

using namespace QSchematic;

WireRoundedCorners::WireRoundedCorners(int type, QGraphicsItem* parent) :
    Wire(type, parent)
{
}

QJsonObject WireRoundedCorners::toJson() const
{
    QJsonObject object;

    object.insert("wire", QSchematic::Wire::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool WireRoundedCorners::fromJson(const QJsonObject& object)
{
    QSchematic::Wire::fromJson(object["wire"].toObject());

    return true;
}

void WireRoundedCorners::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Don't try to understand this without pen & paper.                                           //
    // Seriously.                                                                                  //
    // It took me four days and lots of drugs to get through this. I hope that no other human ever //
    // has to write this again. If you need something like this, take this code. Copy it, paste it //
    // in your own software, make a small donation to any open source project of your choosing and //
    // never look at it again.                                                                     //
    //////////////////////////////////////////////////////////////////////////////////////////////////

    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Retrieve the scene points as we'll need them a lot
    auto sceneWirePoints(sceneWirePointsRelative());
    simplify(sceneWirePoints);
    QVector<QPoint> scenePoints;
    for (const auto& wirePoint : sceneWirePoints) {
        scenePoints << wirePoint.toPoint();
    }

    // Nothing to do if there are no points
    if (scenePoints.count() < 2) {
        return;
    }

    QPen penJunction;
    penJunction.setStyle(Qt::NoPen);

    QBrush brushJunction;
    brushJunction.setStyle(Qt::SolidPattern);
    brushJunction.setColor(isHighlighted() ? COLOR_HIGHLIGHTED : COLOR);

    // Draw the actual line
    {
        // Pen
        QPen penLine;
        penLine.setStyle(Qt::SolidLine);
        penLine.setCapStyle(Qt::RoundCap);
        QColor penColor;
        if (isSelected()) {
            penColor = COLOR_SELECTED;
        } else if (isHighlighted()) {
            penColor = COLOR_HIGHLIGHTED;
        } else {
            penColor = COLOR;
        }
        penLine.setWidth(LINE_WIDTH);
        penLine.setColor(penColor);

        // Brush
        QBrush brushLine;
        brushLine.setStyle(Qt::NoBrush);

        // Prepare the painter
        painter->setPen(penLine);
        painter->setBrush(brushLine);

        // Render
        for (int i = 0; i <= scenePoints.count()-2; i++) {
            // Retrieve points
            QPoint p1 = scenePoints.at(i);
            QPoint p2 = scenePoints.at(i+1);

            // If there are just two points we need to render the line just like this and we're done
            if (scenePoints.count() == 2) {
                painter->drawLine(p1, p2);
                break;

            // If we have two line segments we want to render an arc connecting them
            } else if (i < scenePoints.count()-2) {
                // Retrieve the third point (now we have two wire segments defined by p1, p2 and p3)
                QPoint p3 = scenePoints.at(i+2);

                // Determine arc position/direction
                // Also determine the actual line segments
                QLineF line1(p1, p2);
                QLineF line2(p2, p3);
                QuarterCircleSegment segment = None;
                {
                    int linePointAdjust = _settings.gridSize;
                    bool hasPrevious = true;
                    bool hasNext = true;

                    // Figure out whether we have a previous point
                    hasPrevious = (i != 0);

                    // Figure out whether we have a next point
                    hasNext = (i != scenePoints.count()-3);

                    // There are three posibilities:
                    //  A: We have a normal corner, render a normal arc within a grid cell. This is the usual case when
                    //     the three points are "not close" to each other
                    //  B: We need an 'S' instead of an arc as we shift the line by one grid cell.
                    //  C: We need a 'U' as the lines makes a U-turn
                    QPoint d1(p2 - p1);
                    ConnectionType connectionType = Arc;
                    ConnectionDirection connectionDirection = Up;
                    if (i >= 1) {

                        if (qAbs(d1.y()) == settings().gridSize) {
                            if (p3.x() > p2.x()) {
                                connectionType = SCurve;
                            } else {
                                connectionType = UTurn;
                            }

                        } else if (qAbs(d1.y()) == settings().gridSize) {

                        }
                    }
                    //qDebug() << "connectionType = " << connectionType;
                    //qDebug() << "connectionDirection = " << connectionDirection;
#warning ToDo: Complete implementation of SCurve and UTurn connection elements
                    Q_UNUSED(connectionType)
                    Q_UNUSED(connectionDirection)

                    // Oh boy...
                    if (p3.x() < p1.x() and p3.y() < p1.y()) {
                        if (p2.x() == p3.x() and p2.y() == p1.y()) {
                            segment = BottomLeft;
                            line1.setP2(QPointF(p2.x() + linePointAdjust, p2.y()));
                            line2.setP1(QPointF(p2.x(), p2.y() - linePointAdjust));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x() - linePointAdjust, p1.y()));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x(), p3.y() + linePointAdjust));
                            }
                        } else if (p2.x() == p1.x() and p2.y() == p3.y()) {
                            segment = TopRight;
                            line1.setP2(QPointF(p2.x(), p2.y() + linePointAdjust));
                            line2.setP1(QPointF(p2.x() - linePointAdjust, p2.y()));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x(), p1.y() - linePointAdjust));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x() + linePointAdjust, p3.y()));
                            }
                        }
                    } else if (p3.x() > p1.x() and p3.y() < p1.y()) {
                        if (p2.x() == p1.x() and p2.y() == p3.y()) {
                            segment = TopLeft;
                            line1.setP2(QPointF(p2.x(), p2.y() + linePointAdjust));
                            line2.setP1(QPointF(p2.x() + linePointAdjust, p2.y()));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x(), p1.y() - linePointAdjust));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x() - linePointAdjust, p3.y()));
                            }
                        } else if (p2.x() == p3.x() and p2.y() == p1.y()) {
                            segment = BottomRight;
                            line1.setP2(QPointF(p2.x() - linePointAdjust, p2.y()));
                            line2.setP1(QPointF(p2.x(), p2.y() - linePointAdjust));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x() + linePointAdjust, p1.y()));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x(), p3.y() + linePointAdjust));
                            }
                        }
                    } else if (p3.x() > p1.x() and p3.y() > p1.y()) {
                        if (p2.x() == p3.x() and p2.y() == p1.y()) {
                            segment = TopRight;
                            line1.setP2(QPointF(p2.x() - linePointAdjust, p2.y()));
                            line2.setP1(QPointF(p2.x(), p2.y() + linePointAdjust));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x() + linePointAdjust, p1.y()));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x(), p3.y() - linePointAdjust));
                            }
                        } else if (p2.x() == p1.x() and p2.y() == p3.y()) {
                            segment = BottomLeft;
                            line1.setP2(QPointF(p2.x(), p2.y() - linePointAdjust));
                            line2.setP1(QPointF(p2.x() + linePointAdjust, p2.y()));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x(), p1.y() + linePointAdjust));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x() - linePointAdjust, p3.y()));
                            }
                        }
                    } else if (p3.x() < p1.x() and p3.y() > p1.y()) {
                        if (p2.x() == p1.x() and p2.y() == p3.y()) {
                            segment = BottomRight;
                            line1.setP2(QPointF(p2.x(), p2.y() - linePointAdjust));
                            line2.setP1(QPointF(p2.x() - linePointAdjust, p2.y()));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x(), p1.y() + linePointAdjust));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x() + linePointAdjust, p3.y()));
                            }
                        } else if (p2.x() == p3.x() and p2.y() == p1.y()) {
                            segment = TopLeft;
                            line1.setP2(QPointF(p2.x() + linePointAdjust, p2.y()));
                            line2.setP1(QPointF(p2.x(), p2.y() + linePointAdjust));
                            if (hasPrevious) {
                                line1.setP1(QPointF(p1.x() - linePointAdjust, p1.y()));
                            }
                            if (hasNext) {
                                line2.setP2(QPointF(p3.x(), p3.y() - linePointAdjust));
                            }
                        }
                    }
                }

                // Render lines
                if (i == 0) {
                    painter->drawLine(line1);
                }
                painter->drawLine(line2);

                // Calculate arc parameters
                int rectSize = 2*_settings.gridSize;
                QRect rect(0, 0, rectSize, rectSize);
                int angleStart = 0;
                int angleSpan = 90;
                switch (segment)
                {
                case None:
                    angleStart = 0;
                    angleSpan = 0;
                    break;

                case TopLeft:
                    rect.setX(p2.x());
                    rect.setY(p2.y());
                    angleStart = 90;
                    break;

                case TopRight:
                    rect.setX(p2.x() - rectSize);
                    rect.setY(p2.y());
                    angleStart = 0;
                    break;

                case BottomLeft:
                    rect.setX(p2.x());
                    rect.setY(p2.y() - rectSize);
                    angleStart = 180;
                    break;

                case BottomRight:
                    rect.setX(p2.x() - rectSize);
                    rect.setY(p2.y() - rectSize);
                    angleStart = 270;
                    break;
                }
                rect.setWidth(rectSize);
                rect.setHeight(rectSize);

                // Render the arc
                painter->drawArc(rect, 16*angleStart, 16*angleSpan);
            }
        }
    }

    // Draw the junction poins
    int junctionRadius = 4;
    for (const QSchematic::WirePoint& wirePoint : sceneWirePointsRelative()) {
        if (wirePoint.isJunction()) {
            painter->setPen(penJunction);
            painter->setBrush(brushJunction);
            painter->drawEllipse(wirePoint.toPoint(), junctionRadius, junctionRadius);
        }
    }

    // Draw the handles (if selected)
    if (isSelected()) {
        // Pen
        QPen penHandle;
        penHandle.setColor(Qt::black);
        penHandle.setStyle(Qt::SolidLine);

        // Brush
        QBrush brushHandle;
        brushHandle.setColor(Qt::black);
        brushHandle.setStyle(Qt::SolidPattern);

        // Render
        painter->setPen(penHandle);
        painter->setBrush(brushHandle);
        for (const QPoint& point : scenePoints) {
            QRectF handleRect(point.x() - HANDLE_SIZE, point.y() - HANDLE_SIZE, 2*HANDLE_SIZE, 2*HANDLE_SIZE);
            painter->drawRect(handleRect);
        }
    }

    // Draw debugging stuff
    if (_settings.debug) {
        painter->setPen(Qt::red);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());

        painter->setPen(Qt::blue);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(shape());
    }
}
