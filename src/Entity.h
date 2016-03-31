#ifndef _ENTITY_H
#define _ENTITY_H

enum EntityType {
    EntityType_None,
    EntityType_Unit,
};

struct Entity {
    EntityType type;
    v2f position;
    v2f moveTarget;
    bool32 selected;
};

#endif

