#pragma once

#include "operation.h"

class OperationDemo1 : public Operation
{
    Q_OBJECT
    Q_DISABLE_COPY(OperationDemo1)

public:
    explicit OperationDemo1();
    virtual ~OperationDemo1() override = default;

    virtual QJsonObject toJson() const override;
    virtual bool fromJson(const QJsonObject& object) override;
};
