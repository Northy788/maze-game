#ifndef PLAYER_H
#define PLAYER_H

typedef struct position_s
{
    /* data */
    float x;
    float y;
} position_t;

typedef struct speed_s
{
    /* data */
    float x;
    float y;
} speed_t;

typedef struct Gamemap_s
{
    int16_t x;
    int16_t y;
    uint16_t color;
    /* data */
} Gamemap_t;

#endif