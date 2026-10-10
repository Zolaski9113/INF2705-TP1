#pragma once

// Le sol va de -0.5 à +0.5 en x et en z. s est associé a x et t à z
// s = (x + 0,5) × 5
// t = (z + 0,5) × 5

float ground[] =
    {
        // Position           // Texture coordinates
        -0.5f, 0.0f, -0.5f, 0.0f, 0.0f,
        0.5f, 0.0f, -0.5f, 5.0f, 0.0f,
        0.5f, 0.0f, 0.5f, 5.0f, 5.0f,
        -0.5f, 0.0f, 0.5f, 0.0f, 5.0f};

unsigned int planeElements[] =
    {
        0, 2, 1,
        0, 3, 2};
