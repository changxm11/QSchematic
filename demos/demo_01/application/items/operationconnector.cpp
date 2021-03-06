#include <QPainter>
#include <QRectF>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QJsonObject>
#include <QInputDialog>
#include "../../../lib/items/label.h"
#include "../../../lib/scene.h"
#include "../commands/commanditemvisibility.h"
#include "../commands/commandlabelrename.h"
#include "operationconnector.h"
#include "itemtypes.h"

#define SIZE (_settings.gridSize/2)
#define RECT (QRectF(-SIZE, -SIZE, 2*SIZE, 2*SIZE))

const QColor COLOR_BODY_FILL   = QColor(Qt::white);
const QColor COLOR_BODY_BORDER = QColor(Qt::black);
const qreal PEN_WIDTH          = 1.5;

OperationConnector::OperationConnector(const QPoint& gridPoint, const QString& text, QGraphicsItem* parent) :
    QSchematic::Connector(::ItemType::OperationConnectorType, gridPoint, text, parent)
{
    label()->setVisible(false);
    setForceTextDirection(true);
    setForcedTextDirection(QSchematic::LeftToRight);
}

QJsonObject OperationConnector::toJson() const
{
    QJsonObject object;

    object.insert("connector", QSchematic::Connector::toJson());
    addTypeIdentifierToJson(object);

    return object;
}

bool OperationConnector::fromJson(const QJsonObject& object)
{
    QSchematic::Connector::fromJson(object["connector"].toObject());

    return true;
}

std::unique_ptr<QSchematic::Item> OperationConnector::deepCopy() const
{
    auto clone = std::make_unique<OperationConnector>(gridPos(), text(), parentItem());
    copyAttributes(*(clone.get()));

    return clone;
}

void OperationConnector::copyAttributes(OperationConnector& dest) const
{
    QSchematic::Connector::copyAttributes(dest);
}

QRectF OperationConnector::boundingRect() const
{
    return RECT;
}

void OperationConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Body pen
    QPen bodyPen;
    bodyPen.setWidthF(PEN_WIDTH);
    bodyPen.setStyle(Qt::SolidLine);
    bodyPen.setColor(COLOR_BODY_BORDER);

    // Body brush
    QBrush bodyBrush;
    bodyBrush.setStyle(Qt::SolidPattern);
    bodyBrush.setColor(COLOR_BODY_FILL);

    // Draw the component body
    painter->setPen(bodyPen);
    painter->setBrush(bodyBrush);
    painter->drawEllipse(RECT);
}

void OperationConnector::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Create the menu
    QMenu menu;
    {
        // Label visibility
        QAction* labelVisibility = new QAction;
        labelVisibility->setCheckable(true);
        labelVisibility->setChecked(label()->isVisible());
        labelVisibility->setText("Label visible");
        connect(labelVisibility, &QAction::toggled, [this](bool enabled) {
            if (scene()) {
                scene()->undoStack()->push(new CommandItemVisibility(label(), enabled));
            } else {
                label()->setVisible(enabled);
            }
        });

        // Text
        QAction* text = new QAction;
        text->setText("Rename ...");
        connect(text, &QAction::triggered, [this] {
            const QString& newText = QInputDialog::getText(nullptr, "Rename Connector", "New connector text", QLineEdit::Normal, label()->text());

            if (scene()) {
                scene()->undoStack()->push(new CommandLabelRename(label(), newText));
            } else {
                label()->setText(newText);
            }
        });

        // Assemble
        menu.addAction(labelVisibility);
        menu.addAction(text);
    }

    // Sow the menu
    menu.exec(event->screenPos());
}
