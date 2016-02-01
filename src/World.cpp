#include "World.h"
#include "Math.h"
#include "Platform.h"

static v2 MousePosition = {};
static v2 PlayerPosition = {};
static v2 PlayerSpeed = {};
static v2 PlayerAcceleration = {};

void world_update(GameWorld *world, game_input *input, real32 dt) {

    const real32 gravity = 9.81f;
    const real32 height = 1080.0f;

    PlayerAcceleration.x = 50 * input->joystick.x;
    if (input->buttons & BUTTON_RIGHT) {
        PlayerAcceleration.x = 50;
    } else if (input->buttons & BUTTON_LEFT) {
        PlayerAcceleration.x = -50;
    } else if (PlayerPosition.y == height - 180 - 100) {
        PlayerAcceleration.x += PlayerSpeed.x * -5.f;
    }

    PlayerSpeed += PlayerAcceleration * dt;
    if (PlayerSpeed.x > 20) {
        PlayerSpeed.x = 20;
    } else if (PlayerSpeed.x < -20) {
        PlayerSpeed.x = -20;
    }

    PlayerSpeed.y += dt * gravity;
    PlayerPosition += PlayerSpeed;
    if (PlayerPosition.y >= height - 180 - 100) {
        PlayerPosition.y = height - 180 - 100;
    }

}

void world_render(GameWorld *world, RenderContext *ctx) {

    const uint32 width = 1920;
    const uint32 height = 1080;


    render_rect(ctx, 0, 0, width, height - 180, { 0.486f, 0.678f, 0.965f });
    render_rect(ctx, 0, height - 180, width, 180, { 0.447f, 0.694f, 0.369f });

    render_image(ctx, PlayerPosition.x, PlayerPosition.y, 100, 100, 2);
}
