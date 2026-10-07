#pragma once

// TODO: Compléter les coordonnées de texture.
//       On veut que la texture du sol se répète 5 fois sur chaque côté.

float ground[] =
{
    // Position           // Texture coordinates
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   0.0f, 0.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 0.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};
