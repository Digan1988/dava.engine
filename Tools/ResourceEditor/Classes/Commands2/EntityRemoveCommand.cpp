#include "Commands2/EntityRemoveCommand.h"

EntityRemoveCommand::EntityRemoveCommand(DAVA::Entity* _entity)
    : Command2(CMDID_ENTITY_REMOVE, "Remove entity")
    , entity(_entity)
    , before(NULL)
    , parent(NULL)
{
    SafeRetain(entity);

    if (NULL != entity)
    {
        parent = entity->GetParent();
        if (NULL != parent)
        {
            before = parent->GetNextChild(entity);
        }
    }
}

EntityRemoveCommand::~EntityRemoveCommand()
{
    SafeRelease(entity);
}

void EntityRemoveCommand::Undo()
{
    if (NULL != entity && NULL != parent)
    {
        if (NULL != before)
        {
            parent->InsertBeforeNode(entity, before);
        }
        else
        {
            parent->AddNode(entity);
        }
    }
}

void EntityRemoveCommand::Redo()
{
    if (NULL != entity && NULL != parent)
    {
        parent->RemoveNode(entity);
    }
}

DAVA::Entity* EntityRemoveCommand::GetEntity() const
{
    return entity;
}
