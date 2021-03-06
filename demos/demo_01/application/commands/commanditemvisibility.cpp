#include "../../../lib/items/item.h"
#include "commands.h"
#include "commanditemvisibility.h"

CommandItemVisibility::CommandItemVisibility(const std::shared_ptr<QSchematic::Item>& item, bool newVisibility, QUndoCommand* parent) :
    QUndoCommand(parent),
    _item(item),
    _newVisibility(newVisibility)
{
    _oldVisibility = _item->isVisible();

    QObject::connect(_item.get(), &QObject::destroyed, [this]{
        setObsolete(true);
    });

    setText(QStringLiteral("Change visibility"));
}

int CommandItemVisibility::id() const
{
    return ItemVisibilityCommandType;
}

bool CommandItemVisibility::mergeWith(const QUndoCommand* command)
{
    if (id() != command->id()) {
        return false;
    }

    const CommandItemVisibility* myCommand = dynamic_cast<const CommandItemVisibility*>(command);
    if (!myCommand or _item != myCommand->_item) {
        return false;
    }

    _newVisibility = myCommand->_newVisibility;

    return true;
}

void CommandItemVisibility::undo()
{
    if (!_item) {
        return;
    }

    _item->setVisible(_oldVisibility);
}

void CommandItemVisibility::redo()
{
    if (!_item) {
        return;
    }

    _item->setVisible(_newVisibility);
}
