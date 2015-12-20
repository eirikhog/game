#include "Game.h"
#include "Platform.h"

extern "C"
void EXPORT UpdateGame(game_memory *memory, game_input *input, game_buffer *buffer) {
    game_state *state = (game_state *)memory;
    
    if (!state->initialized) {
        // Do initialization.
    }

    // Update and render the game...
}


