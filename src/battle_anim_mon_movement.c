#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "task.h"
#include "trig.h"

#undef abs
#define abs(x) ((x) < 0 ? -(x) : (x))

static void AnimTask_ShakeMonStep(u8 taskId);
static void AnimTask_ShakeMon2Step(u8 taskId);
static void AnimTask_ShakeMonInPlaceStep(u8 taskId);
static void AnimTask_ShakeAndSinkMonStep(u8 taskId);
static void AnimTask_TranslateMonEllipticalStep(u8 taskId);
static void DoHorizontalLunge(struct Sprite * sprite);
static void ReverseHorizontalLungeDirection(struct Sprite * sprite);
static void DoVerticalDip(struct Sprite * sprite);
static void ReverseVerticalDipDirection(struct Sprite * sprite);
static void SlideMonToOriginalPos(struct Sprite * sprite);
static void SlideMonToOriginalPosStep(struct Sprite * sprite);
static void SlideMonToOffset(struct Sprite * sprite);
static void sub_8099394(struct Sprite * sprite);
static void sub_809946C(struct Sprite * sprite);
static void AnimTask_WindUpLungePart1(u8 taskId);
static void AnimTask_WindUpLungePart2(u8 taskId);
static void sub_80996B8(u8 taskId);
static void AnimTask_SwayMonStep(u8 taskId);
static void AnimTask_ScaleMonAndRestoreStep(u8 taskId);
static void sub_8099B54(u8 taskId);
static void sub_8099CB8(u8 taskId);

const struct SpriteTemplate gHorizontalLungeSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = DoHorizontalLunge,
};

const struct SpriteTemplate gVerticalDipSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = DoVerticalDip,
};

const struct SpriteTemplate gSlideMonToOriginalPosSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SlideMonToOriginalPos,
};

const struct SpriteTemplate gSlideMonToOffsetSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SlideMonToOffset,
};

const struct SpriteTemplate gSlideMonToOffsetAndBackSpriteTemplate =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = sub_8099394,
};

// Task to facilitate simple shaking of a pokemon's picture in battle.
// The shaking alternates between the original position and the target position.
// arg 0: anim battler
// arg 1: x pixel offset
// arg 2: y pixel offset
// arg 3: num times to shake
// arg 4: frame delay
void AnimTask_ShakeMon(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);

    if (spriteId == 0xFF)
        DestroyAnimVisualTask(taskId);
    else
    {
        gSprites[spriteId].pos2.x = gBattleAnimArgs[1];
        gSprites[spriteId].pos2.y = gBattleAnimArgs[2];
        gTasks[taskId].data[0] = spriteId;
        gTasks[taskId].data[1] = gBattleAnimArgs[3];
        gTasks[taskId].data[2] = gBattleAnimArgs[4];
        gTasks[taskId].data[3] = gBattleAnimArgs[4];
        gTasks[taskId].data[4] = gBattleAnimArgs[1];
        gTasks[taskId].data[5] = gBattleAnimArgs[2];
        gTasks[taskId].func = AnimTask_ShakeMonStep;
        gTasks[taskId].func(taskId);
    }
}

static void AnimTask_ShakeMonStep(u8 taskId)
{
    if (gTasks[taskId].data[3] == 0)
    {
        if (gSprites[gTasks[taskId].data[0]].pos2.x == 0)
            gSprites[gTasks[taskId].data[0]].pos2.x = gTasks[taskId].data[4];
        else
            gSprites[gTasks[taskId].data[0]].pos2.x = 0;
        if (gSprites[gTasks[taskId].data[0]].pos2.y == 0)
            gSprites[gTasks[taskId].data[0]].pos2.y = gTasks[taskId].data[5];
        else
            gSprites[gTasks[taskId].data[0]].pos2.y = 0;
        gTasks[taskId].data[3] = gTasks[taskId].data[2];
        if (--gTasks[taskId].data[1] == 0)
        {
            gSprites[gTasks[taskId].data[0]].pos2.x = 0;
            gSprites[gTasks[taskId].data[0]].pos2.y = 0;
            DestroyAnimVisualTask(taskId);
        }
    }
    else
        gTasks[taskId].data[3]--;
}

// Task to facilitate simple shaking of a pokemon's picture in battle.
// The shaking alternates between the positive and negative versions of the specified pixel offsets.
// arg 0: anim battler
// arg 1: x pixel offset
// arg 2: y pixel offset
// arg 3: num times to shake
// arg 4: frame delay
void AnimTask_ShakeMon2(u8 taskId)
{
    bool8 abort = FALSE;
    u8 spriteId;
    u8 battlerId;

    if (gBattleAnimArgs[0] < MAX_BATTLERS_COUNT)
    {
        spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);
        if (spriteId == 0xFF)
            abort = TRUE;
    }
    else if (gBattleAnimArgs[0] != 8)
    {
        switch (gBattleAnimArgs[0])
        {
        case 4:
            battlerId = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
            break;
        case 5:
            battlerId = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
            break;
        case 6:
            battlerId = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            break;
        default:
            battlerId = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
            break;
        }
        if (!IsBattlerSpriteVisible(battlerId))
            abort = TRUE;
        spriteId = gBattlerSpriteIds[battlerId];
    }
    else
        spriteId = gBattlerSpriteIds[gBattleAnimAttacker];
    if (abort)
        DestroyAnimVisualTask(taskId);
    else
    {
        gSprites[spriteId].pos2.x = gBattleAnimArgs[1];
        gSprites[spriteId].pos2.y = gBattleAnimArgs[2];
        gTasks[taskId].data[0] = spriteId;
        gTasks[taskId].data[1] = gBattleAnimArgs[3];
        gTasks[taskId].data[2] = gBattleAnimArgs[4];
        gTasks[taskId].data[3] = gBattleAnimArgs[4];
        gTasks[taskId].data[4] = gBattleAnimArgs[1];
        gTasks[taskId].data[5] = gBattleAnimArgs[2];
        gTasks[taskId].func = AnimTask_ShakeMon2Step;
        gTasks[taskId].func(taskId);
    }
}

static void AnimTask_ShakeMon2Step(u8 taskId)
{
    if (gTasks[taskId].data[3] == 0)
    {
        if (gSprites[gTasks[taskId].data[0]].pos2.x == gTasks[taskId].data[4])
            gSprites[gTasks[taskId].data[0]].pos2.x = -gTasks[taskId].data[4];
        else
            gSprites[gTasks[taskId].data[0]].pos2.x = gTasks[taskId].data[4];
        if (gSprites[gTasks[taskId].data[0]].pos2.y == gTasks[taskId].data[5])
            gSprites[gTasks[taskId].data[0]].pos2.y = -gTasks[taskId].data[5];
        else
            gSprites[gTasks[taskId].data[0]].pos2.y = gTasks[taskId].data[5];
        gTasks[taskId].data[3] = gTasks[taskId].data[2];
        if (--gTasks[taskId].data[1] == 0)
        {
            gSprites[gTasks[taskId].data[0]].pos2.x = 0;
            gSprites[gTasks[taskId].data[0]].pos2.y = 0;
            DestroyAnimVisualTask(taskId);
        }
    }
    else
        gTasks[taskId].data[3]--;
}

// Task to facilitate simple shaking of a pokemon's picture in battle.
// The shaking alternates between the positive and negative versions of the specified pixel offsets
// with respect to the current location of the mon's picture.
// arg 0: battler
// arg 1: x offset
// arg 2: y offset
// arg 3: num shakes
// arg 4: delay
void AnimTask_ShakeMonInPlace(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);

    if (spriteId == 0xFF)
        DestroyAnimVisualTask(taskId);
    else
    {
        gSprites[spriteId].pos2.x += gBattleAnimArgs[1];
        gSprites[spriteId].pos2.y += gBattleAnimArgs[2];
        gTasks[taskId].data[0] = spriteId;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = gBattleAnimArgs[3];
        gTasks[taskId].data[3] = 0;
        gTasks[taskId].data[4] = gBattleAnimArgs[4];
        gTasks[taskId].data[5] = gBattleAnimArgs[1] * 2;
        gTasks[taskId].data[6] = gBattleAnimArgs[2] * 2;
        gTasks[taskId].func = AnimTask_ShakeMonInPlaceStep;
        gTasks[taskId].func(taskId);
    }
}

static void AnimTask_ShakeMonInPlaceStep(u8 taskId)
{
    if (gTasks[taskId].data[3] == 0)
    {
        if (gTasks[taskId].data[1] & 1)
        {
            gSprites[gTasks[taskId].data[0]].pos2.x += gTasks[taskId].data[5];
            gSprites[gTasks[taskId].data[0]].pos2.y += gTasks[taskId].data[6];
        }
        else
        {
            gSprites[gTasks[taskId].data[0]].pos2.x -= gTasks[taskId].data[5];
            gSprites[gTasks[taskId].data[0]].pos2.y -= gTasks[taskId].data[6];
        }
        gTasks[taskId].data[3] = gTasks[taskId].data[4];
        if (++gTasks[taskId].data[1] >= gTasks[taskId].data[2])
        {
            if (gTasks[taskId].data[1] & 1)
            {
                gSprites[gTasks[taskId].data[0]].pos2.x += gTasks[taskId].data[5] / 2;
                gSprites[gTasks[taskId].data[0]].pos2.y += gTasks[taskId].data[6] / 2;
            }
            else
            {
                gSprites[gTasks[taskId].data[0]].pos2.x -= gTasks[taskId].data[5] / 2;
                gSprites[gTasks[taskId].data[0]].pos2.y -= gTasks[taskId].data[6] / 2;
            }
            DestroyAnimVisualTask(taskId);
        }
    }
    else
        gTasks[taskId].data[3]--;
}

// Shakes a mon bg horizontally and moves it downward linearly.
// arg 0: battler
// arg 1: x offset
// arg 2: frame delay between each movement
// arg 3: downward speed (subpixel)
// arg 4: duration
void AnimTask_ShakeAndSinkMon(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);

    gSprites[spriteId].pos2.x = gBattleAnimArgs[1];
    gTasks[taskId].data[0] = spriteId;
    gTasks[taskId].data[1] = gBattleAnimArgs[1];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].data[3] = gBattleAnimArgs[3];
    gTasks[taskId].data[4] = gBattleAnimArgs[4];
    gTasks[taskId].func = AnimTask_ShakeAndSinkMonStep;
    gTasks[taskId].func(taskId);
}

static void AnimTask_ShakeAndSinkMonStep(u8 taskId)
{
    u8 spriteId = gTasks[taskId].data[0];
    s16 data1 = gTasks[taskId].data[1];
    if (gTasks[taskId].data[2] == gTasks[taskId].data[8]++)
    {
        gTasks[taskId].data[8] = 0;
        if (gSprites[spriteId].pos2.x == data1)
            data1 = -data1;
        gSprites[spriteId].pos2.x += data1;
    }
    gTasks[taskId].data[1] = data1;
    gTasks[taskId].data[9] += gTasks[taskId].data[3];
    gSprites[spriteId].pos2.y = gTasks[taskId].data[9] >> 8;
    if (--gTasks[taskId].data[4] == 0)
        DestroyAnimVisualTask(taskId);
}

// Moves a mon bg picture along an elliptical path that begins
// and ends at the mon's origin location.
// arg 0: battler
// arg 1: ellipse width
// arg 2: ellipse height
// arg 3: num loops
// arg 4: speed (valid values are 0-5)
void AnimTask_TranslateMonElliptical(u8 taskId)
{
    u8 wavePeriod = 1;
    u8 i;
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);
    if (gBattleAnimArgs[4] > 5)
        gBattleAnimArgs[4] = 5;
    for (i = 0; i < gBattleAnimArgs[4]; i++)
        wavePeriod *= 2;
    gTasks[taskId].data[0] = spriteId;
    gTasks[taskId].data[1] = gBattleAnimArgs[1];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].data[3] = gBattleAnimArgs[3];
    gTasks[taskId].data[4] = wavePeriod;
    gTasks[taskId].func = AnimTask_TranslateMonEllipticalStep;
    gTasks[taskId].func(taskId);
}

static void AnimTask_TranslateMonEllipticalStep(u8 taskId)
{
    u8 spriteId = gTasks[taskId].data[0];
    gSprites[spriteId].pos2.x = Sin(gTasks[taskId].data[5], gTasks[taskId].data[1]);
    gSprites[spriteId].pos2.y = -Cos(gTasks[taskId].data[5], gTasks[taskId].data[2]);
    gSprites[spriteId].pos2.y += gTasks[taskId].data[2];
    gTasks[taskId].data[5] += gTasks[taskId].data[4];
    gTasks[taskId].data[5] &= 0xFF;
    if (gTasks[taskId].data[5] == 0)
        gTasks[taskId].data[3]--;
    if (gTasks[taskId].data[3] == 0)
    {
        gSprites[spriteId].pos2.x = 0;
        gSprites[spriteId].pos2.y = 0;
        DestroyAnimVisualTask(taskId);
    }
}

// Moves a mon bg picture along an elliptical path that begins
// and ends at the mon's origin location. Reverses the direction
// of the path if it's not on the player's side of the battle.
// arg 0: battler
// arg 1: ellipse width
// arg 2: ellipse height
// arg 3: num loops
// arg 4: speed (valid values are 0-5)
void AnimTask_TranslateMonEllipticalRespectSide(u8 taskId)
{
    if (GetBattlerSide(gBattleAnimAttacker) != B_SIDE_PLAYER)
        gBattleAnimArgs[1] = -gBattleAnimArgs[1];
    AnimTask_TranslateMonElliptical(taskId);
}

// Performs a simple horizontal lunge, where the mon moves
// horizontally, and then moves back in the opposite direction.
// arg 0: duration of single lunge direction
// arg 1: x pixel delta that is applied each frame
static void DoHorizontalLunge(struct Sprite * sprite)
{
    sprite->invisible = TRUE;
    if (GetBattlerSide(gBattleAnimAttacker) != B_SIDE_PLAYER)
        sprite->data[1] = -gBattleAnimArgs[1];
    else
        sprite->data[1] =  gBattleAnimArgs[1];
    sprite->data[0] = gBattleAnimArgs[0];
    sprite->data[2] = 0;
    sprite->data[3] = gBattlerSpriteIds[gBattleAnimAttacker];
    sprite->data[4] = gBattleAnimArgs[0];
    StoreSpriteCallbackInData6(sprite, ReverseHorizontalLungeDirection);
    sprite->callback = TranslateMonSpriteLinear;
}

static void ReverseHorizontalLungeDirection(struct Sprite * sprite)
{
    sprite->data[0] = sprite->data[4];
    sprite->data[1] = -sprite->data[1];
    sprite->callback = TranslateMonSpriteLinear;
    StoreSpriteCallbackInData6(sprite, DestroyAnimSprite);
}

// Performs a simple vertical dipping motion, where moves vertically, and then
// moves back in the opposite direction.
// arg 0: duration of single dip direction
// arg 1: y pixel delta that is applied each frame
// arg 2: battler
static void DoVerticalDip(struct Sprite * sprite)
{
    u8 spriteId;
    sprite->invisible = TRUE;
    spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[2]);
    sprite->data[0] = gBattleAnimArgs[0];
    sprite->data[1] = 0;
    sprite->data[2] = gBattleAnimArgs[1];
    sprite->data[3] = spriteId;
    sprite->data[4] = gBattleAnimArgs[0];
    StoreSpriteCallbackInData6(sprite, ReverseVerticalDipDirection);
    sprite->callback = TranslateMonSpriteLinear;
}

static void ReverseVerticalDipDirection(struct Sprite * sprite)
{
    sprite->data[0] = sprite->data[4];
    sprite->data[2] = -sprite->data[2];
    sprite->callback = TranslateMonSpriteLinear;
    StoreSpriteCallbackInData6(sprite, DestroyAnimSprite);
}

// Linearly slides a mon's bg picture back to its original sprite position.
// The sprite parameter is a dummy sprite used for facilitating the movement with its callback.
// arg 0: 1 = target or 0 = attacker
// arg 1: direction (0 = horizontal and vertical, 1 = horizontal only, 2 = vertical only)
// arg 2: duration
static void SlideMonToOriginalPos(struct Sprite * sprite)
{
    u8 spriteId;
    if (gBattleAnimArgs[0] == 0)
        spriteId = gBattlerSpriteIds[gBattleAnimAttacker];
    else
        spriteId = gBattlerSpriteIds[gBattleAnimTarget];
    sprite->data[0] = gBattleAnimArgs[2];
    sprite->data[1] = gSprites[spriteId].pos1.x + gSprites[spriteId].pos2.x;
    sprite->data[2] = gSprites[spriteId].pos1.x;
    sprite->data[3] = gSprites[spriteId].pos1.y + gSprites[spriteId].pos2.y;
    sprite->data[4] = gSprites[spriteId].pos1.y;
    InitSpriteDataForLinearTranslation(sprite);
    sprite->data[3] = 0;
    sprite->data[4] = 0;
    sprite->data[5] = gSprites[spriteId].pos2.x;
    sprite->data[6] = gSprites[spriteId].pos2.y;
    sprite->invisible = TRUE;
    if (gBattleAnimArgs[1] == 1)
        sprite->data[2] = 0;
    else if (gBattleAnimArgs[1] == 2)
        sprite->data[1] = 0;
    sprite->data[7] = gBattleAnimArgs[1];
    sprite->data[7] |= spriteId << 8;
    sprite->callback = SlideMonToOriginalPosStep;
}

static void SlideMonToOriginalPosStep(struct Sprite * sprite)
{
    u8 data7 = sprite->data[7];
    struct Sprite *otherSprite = &gSprites[sprite->data[7] >> 8];
    if (sprite->data[0] == 0)
    {
        if (data7 == 1 || data7 == 0)
            otherSprite->pos2.x = 0;
        if (data7 == 2 || data7 == 0)
            otherSprite->pos2.y = 0;
        DestroyAnimSprite(sprite);
    }
    else
    {
        sprite->data[0]--;
        sprite->data[3] += sprite->data[1];
        sprite->data[4] += sprite->data[2];
        otherSprite->pos2.x = (sprite->data[3] >> 8) + sprite->data[5];
        otherSprite->pos2.y = (sprite->data[4] >> 8) + sprite->data[6];
    }
}

// Linearly translates a mon to a target offset. The horizontal offset
// is mirrored for the opponent's pokemon, and the vertical offset
// is only mirrored if arg 3 is set to 1.
// arg 0: 0 = attacker, 1 = target
// arg 1: target x pixel offset
// arg 2: target y pixel offset
// arg 3: mirror vertical translation for opposite battle side
// arg 4: duration
static void SlideMonToOffset(struct Sprite * sprite)
{
    u8 battlerId;
    u8 spriteId;
    if (gBattleAnimArgs[0] == 0)
        battlerId = gBattleAnimAttacker;
    else
        battlerId = gBattleAnimTarget;
    spriteId = gBattlerSpriteIds[battlerId];
    if (GetBattlerSide(battlerId) != B_SIDE_PLAYER)
    {
        gBattleAnimArgs[1] = -gBattleAnimArgs[1];
        if (gBattleAnimArgs[3] == 1)
            gBattleAnimArgs[2] = -gBattleAnimArgs[2];
    }
    sprite->data[0] = gBattleAnimArgs[4];
    sprite->data[1] = gSprites[spriteId].pos1.x;
    sprite->data[2] = gSprites[spriteId].pos1.x + gBattleAnimArgs[1];
    sprite->data[3] = gSprites[spriteId].pos1.y;
    sprite->data[4] = gSprites[spriteId].pos1.y + gBattleAnimArgs[2];
    InitSpriteDataForLinearTranslation(sprite);
    sprite->data[3] = 0;
    sprite->data[4] = 0;
    sprite->data[5] = spriteId;
    sprite->invisible = TRUE;
    StoreSpriteCallbackInData6(sprite, DestroyAnimSprite);
    sprite->callback = TranslateMonSpriteLinearFixedPoint;
}

static void sub_8099394(struct Sprite * sprite)
{
    u8 battlerId;
    u8 spriteId;
    sprite->invisible = TRUE;
    if (gBattleAnimArgs[0] == 0)
        battlerId = gBattleAnimAttacker;
    else
        battlerId = gBattleAnimTarget;
    spriteId = gBattlerSpriteIds[battlerId];
    if (GetBattlerSide(battlerId) != B_SIDE_PLAYER)
    {
        gBattleAnimArgs[1] = -gBattleAnimArgs[1];
        if (gBattleAnimArgs[3] == 1)
            gBattleAnimArgs[2] = -gBattleAnimArgs[2];
    }
    sprite->data[0] = gBattleAnimArgs[4];
    sprite->data[1] = gSprites[spriteId].pos1.x + gSprites[spriteId].pos2.x;
    sprite->data[2] = sprite->data[1] + gBattleAnimArgs[1];
    sprite->data[3] = gSprites[spriteId].pos1.y + gSprites[spriteId].pos2.y;
    sprite->data[4] = sprite->data[3] + gBattleAnimArgs[2];
    InitSpriteDataForLinearTranslation(sprite);
    sprite->data[3] = gSprites[spriteId].pos2.x << 8;
    sprite->data[4] = gSprites[spriteId].pos2.y << 8;
    sprite->data[5] = spriteId;
    sprite->data[6] = gBattleAnimArgs[5];
    if (gBattleAnimArgs[5] == 0)
        StoreSpriteCallbackInData6(sprite, DestroyAnimSprite);
    else
        StoreSpriteCallbackInData6(sprite, sub_809946C);
    sprite->callback = TranslateMonSpriteLinearFixedPoint;
}

static void sub_809946C(struct Sprite * sprite)
{
    gSprites[sprite->data[5]].pos2.x = 0;
    gSprites[sprite->data[5]].pos2.y = 0;
    DestroyAnimSprite(sprite);
}

// Task to facilitate a two-part translation animation, in which the sprite
// is first translated in an arc to one position.  Then, it "lunges" to a target
// x offset.  Used in TAKE_DOWN, for example.
// arg 0: anim bank
// arg 1: horizontal speed (subpixel)
// arg 2: wave amplitude
// arg 3: first duration
// arg 4: delay before starting lunge
// arg 5: target x offset for lunge
// arg 6: lunge duration
void AnimTask_WindUpLunge(u8 taskId)
{
    u16 wavePeriod = 0x8000 / gBattleAnimArgs[3];
    if (GetBattlerSide(gBattleAnimAttacker) != B_SIDE_PLAYER)
    {
        gBattleAnimArgs[1] = -gBattleAnimArgs[1];
        gBattleAnimArgs[5] = -gBattleAnimArgs[5];
    }
    gTasks[taskId].data[0] = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);
    gTasks[taskId].data[1] = gBattleAnimArgs[1] * 256 / gBattleAnimArgs[3];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].data[3] = gBattleAnimArgs[3];
    gTasks[taskId].data[4] = gBattleAnimArgs[4];
    gTasks[taskId].data[5] = gBattleAnimArgs[5] * 256 / gBattleAnimArgs[6];
    gTasks[taskId].data[6] = gBattleAnimArgs[6];
    gTasks[taskId].data[7] = wavePeriod;
    gTasks[taskId].func = AnimTask_WindUpLungePart1;
}

static void AnimTask_WindUpLungePart1(u8 taskId)
{
    u8 spriteId = gTasks[taskId].data[0];
    gTasks[taskId].data[11] += gTasks[taskId].data[1];
    gSprites[spriteId].pos2.x = gTasks[taskId].data[11] >> 8;
    gSprites[spriteId].pos2.y = Sin((u8)(gTasks[taskId].data[10] >> 8), gTasks[taskId].data[2]);
    gTasks[taskId].data[10] += gTasks[taskId].data[7];
    if (--gTasks[taskId].data[3] == 0)
        gTasks[taskId].func = AnimTask_WindUpLungePart2;
}

static void AnimTask_WindUpLungePart2(u8 taskId)
{
    u8 spriteId;

    if (gTasks[taskId].data[4] > 0)
        gTasks[taskId].data[4]--;
    else
    {
        spriteId = gTasks[taskId].data[0];
        gTasks[taskId].data[12] += gTasks[taskId].data[5];
        gSprites[spriteId].pos2.x = (gTasks[taskId].data[12] >> 8) + (gTasks[taskId].data[11] >> 8);
        if (--gTasks[taskId].data[6] == 0)
            DestroyAnimVisualTask(taskId);
    }
}

void AnimTask_SlideOffScreen(u8 taskId)
{
    u8 spriteId;

    switch (gBattleAnimArgs[0])
    {
    case 0:
    case 1:
        spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[0]);
        break;
    case 2:
        if (!IsBattlerSpriteVisible(gBattleAnimAttacker ^ BIT_FLANK))
        {
            DestroyAnimVisualTask(taskId);
            return;
        }
        spriteId = gBattlerSpriteIds[gBattleAnimAttacker ^ BIT_FLANK];
        break;
    case 3:
        if (!IsBattlerSpriteVisible(gBattleAnimTarget ^ BIT_FLANK))
        {
            DestroyAnimVisualTask(taskId);
            return;
        }
        spriteId = gBattlerSpriteIds[gBattleAnimTarget ^ BIT_FLANK];
        break;
    default:
        DestroyAnimVisualTask(taskId);
        return;
    }
    gTasks[taskId].data[0] = spriteId;
    if (GetBattlerSide(gBattleAnimTarget) != B_SIDE_PLAYER)
        gTasks[taskId].data[1] = gBattleAnimArgs[1];
    else
        gTasks[taskId].data[1] = -gBattleAnimArgs[1];
    gTasks[taskId].func = sub_80996B8;
}

static void sub_80996B8(u8 taskId)
{
    u8 spriteId = gTasks[taskId].data[0];
    gSprites[spriteId].pos2.x += gTasks[taskId].data[1];
    if (gSprites[spriteId].pos2.x + gSprites[spriteId].pos1.x < -0x20 || gSprites[spriteId].pos2.x + gSprites[spriteId].pos1.x > 0x110)
        DestroyAnimVisualTask(taskId);
}

// Task that facilitates translating the mon bg picture back and forth
// in a swaying motion (uses Sine wave). It can sway either horizontally
// or vertically, but not both.
// arg 0: direction (0 = horizontal, 1 = vertical)
// arg 1: wave amplitude
// arg 2: wave period
// arg 3: num sways
// arg 4: which mon (0 = attacker, 1 = target)
void AnimTask_SwayMon(u8 taskId)
{
    u8 spriteId;
    if (GetBattlerSide(gBattleAnimAttacker) != B_SIDE_PLAYER)
        gBattleAnimArgs[1] = -gBattleAnimArgs[1];
    spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[4]);
    gTasks[taskId].data[0] = gBattleAnimArgs[0];
    gTasks[taskId].data[1] = gBattleAnimArgs[1];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].data[3] = gBattleAnimArgs[3];
    gTasks[taskId].data[4] = spriteId;
    if (gBattleAnimArgs[4] == 0)
        gTasks[taskId].data[5] = gBattleAnimAttacker;
    else
        gTasks[taskId].data[5] = gBattleAnimTarget;
    gTasks[taskId].data[12] = 1;
    gTasks[taskId].func = AnimTask_SwayMonStep;
}

static void AnimTask_SwayMonStep(u8 taskId)
{
    u8 spriteId;
    u32 waveIndex;
    s16 sineValue;
    u16 sineIndex;

    spriteId = gTasks[taskId].data[4];
    sineIndex = gTasks[taskId].data[10] + gTasks[taskId].data[2];
    gTasks[taskId].data[10] = sineIndex;
    waveIndex = sineIndex >> 8;
    sineValue = Sin(waveIndex, gTasks[taskId].data[1]);
    if (gTasks[taskId].data[0] == 0)
    {
        gSprites[spriteId].pos2.x = sineValue;
    }
    else if (GetBattlerSide(gTasks[taskId].data[5]) == B_SIDE_PLAYER)
    {
        gSprites[spriteId].pos2.y = abs(sineValue);
    }
    else
        gSprites[spriteId].pos2.y = -abs(sineValue);
    if ((waveIndex > 0x7F && gTasks[taskId].data[11] == 0 && gTasks[taskId].data[12] == 1) || (waveIndex < 0x7F && gTasks[taskId].data[11] == 1 && gTasks[taskId].data[12] == 0))
    {
        gTasks[taskId].data[11] ^= 1;
        gTasks[taskId].data[12] ^= 1;
        if (--gTasks[taskId].data[3] == 0)
        {
            gSprites[spriteId].pos2.x = 0;
            gSprites[spriteId].pos2.y = 0;
            DestroyAnimVisualTask(taskId);
        }
    }
}

// Scales a mon's sprite, and then scales back to its original dimensions.
// arg 0: x scale delta
// arg 1: y scale delta
// arg 2: duration
// arg 3: anim bank
// arg 4: sprite object mode
void AnimTask_ScaleMonAndRestore(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[3]);
    PrepareBattlerSpriteForRotScale(spriteId, gBattleAnimArgs[4]);
    gTasks[taskId].data[0] = gBattleAnimArgs[0];
    gTasks[taskId].data[1] = gBattleAnimArgs[1];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].data[3] = gBattleAnimArgs[2];
    gTasks[taskId].data[4] = spriteId;
    gTasks[taskId].data[10] = 0x100;
    gTasks[taskId].data[11] = 0x100;
    gTasks[taskId].func = AnimTask_ScaleMonAndRestoreStep;
}

static void AnimTask_ScaleMonAndRestoreStep(u8 taskId)
{
    u8 spriteId;
    gTasks[taskId].data[10] += gTasks[taskId].data[0];
    gTasks[taskId].data[11] += gTasks[taskId].data[1];
    spriteId = gTasks[taskId].data[4];
    SetSpriteRotScale(spriteId, gTasks[taskId].data[10], gTasks[taskId].data[11], 0);
    if (--gTasks[taskId].data[2] == 0)
    {
        if (gTasks[taskId].data[3] > 0)
        {
            gTasks[taskId].data[0] = -gTasks[taskId].data[0];
            gTasks[taskId].data[1] = -gTasks[taskId].data[1];
            gTasks[taskId].data[2] = gTasks[taskId].data[3];
            gTasks[taskId].data[3] = 0;
        }
        else
        {
            ResetSpriteRotScale(spriteId);
            DestroyAnimVisualTask(taskId);
        }
    }
}

void AnimTask_RotateMonSpriteToSide(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[2]);
    PrepareBattlerSpriteForRotScale(spriteId, 0);
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = gBattleAnimArgs[0];
    if (gBattleAnimArgs[3] != 1)
        gTasks[taskId].data[3] = 0;
    else
        gTasks[taskId].data[3] = gBattleAnimArgs[0] * gBattleAnimArgs[1];
    gTasks[taskId].data[4] = gBattleAnimArgs[1];
    gTasks[taskId].data[5] = spriteId;
    gTasks[taskId].data[6] = gBattleAnimArgs[3];
    if (IsContest())
        gTasks[taskId].data[7] = 1;
    else
    {
        if (gBattleAnimArgs[2] == 0)
            gTasks[taskId].data[7] = GetBattlerSide(gBattleAnimAttacker) == B_SIDE_PLAYER ? 1 : 0;
        else
            gTasks[taskId].data[7] = GetBattlerSide(gBattleAnimTarget) == B_SIDE_PLAYER ? 1 : 0;
    }
    if (gTasks[taskId].data[7] && !IsContest())
    {
        s16 tmp;
        tmp = gTasks[taskId].data[3];
        gTasks[taskId].data[3] = -tmp;
        tmp = gTasks[taskId].data[4];
        gTasks[taskId].data[4] = -tmp;
    }
    gTasks[taskId].func = sub_8099B54;
}

void AnimTask_RotateMonToSideAndRestore(u8 taskId)
{
    u8 spriteId = GetAnimBattlerSpriteId(gBattleAnimArgs[2]);
    PrepareBattlerSpriteForRotScale(spriteId, 0);
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = gBattleAnimArgs[0];
    if (gBattleAnimArgs[2] == 0)
    {
        if (GetBattlerSide(gBattleAnimAttacker) != B_SIDE_PLAYER)
            gBattleAnimArgs[1] = -gBattleAnimArgs[1];
    }
    else
    {
        if (GetBattlerSide(gBattleAnimTarget) != B_SIDE_PLAYER)
            gBattleAnimArgs[1] = -gBattleAnimArgs[1];
    }
    if (gBattleAnimArgs[3] != 1)
        gTasks[taskId].data[3] = 0;
    else
        gTasks[taskId].data[3] = gBattleAnimArgs[0] * gBattleAnimArgs[1];
    gTasks[taskId].data[4] = gBattleAnimArgs[1];
    gTasks[taskId].data[5] = spriteId;
    gTasks[taskId].data[6] = gBattleAnimArgs[3];
    gTasks[taskId].data[7] = 1;
    if (gTasks[taskId].data[7] )
    {
        s16 tmp;
        tmp = gTasks[taskId].data[3];
        gTasks[taskId].data[3] = -tmp;
        tmp = gTasks[taskId].data[4];
        gTasks[taskId].data[4] = -tmp;
    }
    gTasks[taskId].func = sub_8099B54;
}

static void sub_8099B54(u8 taskId)
{
    gTasks[taskId].data[3] += gTasks[taskId].data[4];
    SetSpriteRotScale(gTasks[taskId].data[5], 0x100, 0x100, gTasks[taskId].data[3]);
    if (gTasks[taskId].data[7])
        SetBattlerSpriteYOffsetFromRotation(gTasks[taskId].data[5]);
    if (++gTasks[taskId].data[1] >= gTasks[taskId].data[2])
    {
        switch (gTasks[taskId].data[6])
        {
        case 1:
            ResetSpriteRotScale(gTasks[taskId].data[5]);
            // fallthrough
        case 0:
        default:
            DestroyAnimVisualTask(taskId);
            break;
        case 2:
            gTasks[taskId].data[1] = 0;
            gTasks[taskId].data[4] *= -1;
            gTasks[taskId].data[6] = 1;
            break;
        }
    }
}

void AnimTask_ShakeTargetBasedOnMovePowerOrDmg(u8 taskId)
{
    if (gBattleAnimArgs[0] == 0)
    {
        gTasks[taskId].data[15] = gAnimMovePower / 12;
        if (gTasks[taskId].data[15] < 1)
            gTasks[taskId].data[15] = 1;
        if (gTasks[taskId].data[15] > 16)
            gTasks[taskId].data[15] = 16;
    }
    else
    {
        gTasks[taskId].data[15] = gAnimMoveDmg / 12;
        if (gTasks[taskId].data[15] < 1)
            gTasks[taskId].data[15] = 1;
        if (gTasks[taskId].data[15] > 16)
            gTasks[taskId].data[15] = 16;
    }
    gTasks[taskId].data[14] = gTasks[taskId].data[15] / 2;
    gTasks[taskId].data[13] = gTasks[taskId].data[14] + (gTasks[taskId].data[15] & 1);
    gTasks[taskId].data[12] = 0;
    gTasks[taskId].data[10] = gBattleAnimArgs[3];
    gTasks[taskId].data[11] = gBattleAnimArgs[4];
    gTasks[taskId].data[7] = GetAnimBattlerSpriteId(ANIM_TARGET);
    gTasks[taskId].data[8] = gSprites[gTasks[taskId].data[7]].pos2.x;
    gTasks[taskId].data[9] = gSprites[gTasks[taskId].data[7]].pos2.y;
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = gBattleAnimArgs[1];
    gTasks[taskId].data[2] = gBattleAnimArgs[2];
    gTasks[taskId].func = sub_8099CB8;
}

static void sub_8099CB8(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    if (++task->data[0] > task->data[1])
    {
        task->data[0] = 0;
        task->data[12] = (task->data[12] + 1) & 1;
        if (task->data[10])
        {
            if (task->data[12])
            {
                gSprites[task->data[7]].pos2.x = task->data[8] + task->data[13];
            }
            else
            {
                gSprites[task->data[7]].pos2.x = task->data[8] - task->data[14];
            }
        }
        if (task->data[11])
        {
            if (task->data[12])
            {
                gSprites[task->data[7]].pos2.y = task->data[15];
            }
            else
            {
                gSprites[task->data[7]].pos2.y = 0;
            }
        }
        if (!--task->data[2])
        {
            gSprites[task->data[7]].pos2.x = 0;
            gSprites[task->data[7]].pos2.y = 0;
            DestroyAnimVisualTask(taskId);
            return;
        }
    }
}