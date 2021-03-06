#include <QJsonObject>
#include "../../../lib/items/label.h"
#include "operationdemo1.h"
#include "operationconnector.h"
#include "itemtypes.h"

struct ConnectorAttribute {
    QPoint point;
    QString name;
};

OperationDemo1::OperationDemo1() :
    Operation(::ItemType::OperationDemo1Type)
{
    setSize(8, 8);
    label()->setText("Demo 1");

    QVector<ConnectorAttribute> connectorAttributes = {
        { QPoint(0, 2), QStringLiteral("in 1") },
        { QPoint(0, 4), QStringLiteral("in 2") },
        { QPoint(0, 6), QStringLiteral("in 3") },
        { QPoint(8, 4), QStringLiteral("out") }
    };

    for (const auto& c : connectorAttributes) {
        auto connector = std::make_shared<OperationConnector>(c.point, c.name);
        connector->label()->setVisible(true);
        addConnector(connector);
    }
}

QJsonObject OperationDemo1::toJson() const
{
    QJsonObject object;

    object.insert("operation", Operation::toJson());

    return object;
}

bool OperationDemo1::fromJson(const QJsonObject& object)
{
    Operation::fromJson(object["operation"].toObject());

    return true;
}
