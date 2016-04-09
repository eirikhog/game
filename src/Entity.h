#ifndef _ENTITY_H
#define _ENTITY_H

enum EntityType {
    EntityType_None,
    EntityType_Unit,
};

enum EntityCommand {
    EntityCommand_None,
    EntityCommand_Move
};

struct Entity {
    EntityType type;
    v2f acceleration;
    v2f speed;
    v2f position;
    v2f moveTarget;
    EntityCommand command;
    bool32 selected;
    bool32 deleted;
};

#endif

