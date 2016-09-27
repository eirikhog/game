#pragma once


enum PlayerCommand {
    USERCMD_NONE = 0
};

typedef struct {
    PlayerCommand currentCommand;
} PlayerCommandState;

