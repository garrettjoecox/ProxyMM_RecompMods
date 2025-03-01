#include "recomp_api.h"
#include "z64extern.h"

RECOMP_CALLBACK("*", recomp_on_play_update)
void OnPlayUpdate(PlayState* play) {
    if (
        !recomp_get_config_u32("l_to_levitate") ||
        play->pauseCtx.state != PAUSE_STATE_OFF
    ) {
        return;
    }

    Player* player = GET_PLAYER(play);

    if (CHECK_BTN_ANY(play->state.input[0].cur.button, BTN_L)) {
        player->actor.velocity.y = 6.34375f;
    }
}
