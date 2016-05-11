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


enum EntityFlags {
    EntityFlag_None = 0,
    EntityFlag_Collidable = 0x01,
    EntityFlag_Selectable = 0x02,
};

inline EntityFlags operator|(EntityFlags a, EntityFlags b) {
    return (EntityFlags)((u32)a | (u32)b);
}

struct MoveWaypoint;

struct Entity {
    EntityType type;
    EntityFlags flags;
    v2f acceleration;
    v2f speed;
    v2f position;
    v2f moveTarget;
    EntityCommand command;
    bool32 selected;
    bool32 deleted;
    MoveWaypoint *moveWaypoints;
    MoveWaypoint *firstWaypoint;
};

inline bool32 IsCollidable(Entity *e) {
    return (e->flags & EntityFlag_Collidable);
}

inline bool32 IsSelectable(Entity *e) {
    return (e->flags & EntityFlag_Selectable);
}

#endif

