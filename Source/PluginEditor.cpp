#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

struct TrackDef {
    int steps, pulses, offset, rate;
    float pitch, drop, morph, fold, fm, rise, fall, resp, brgt, reso, noise, vol;
    int notemode, mode281, mode292;
};

struct KitDef {
    const char* name;
    float bpm, swing, chaos; int scale, root;
    float e_time, e_fdbk, e_mix, e_wow; int e_sync;
    TrackDef t[6];
};

static const std::vector<KitDef> MASTER_KITS = {
    { "00. Init / Empty", 124.0f, 0.0f, 0.0f, 4, 0,  0.65f, 0.35f, 0.12f, 0.2f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.001f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.85f, 1, 0, 1},
        {16, 0, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1, 0, 1},
        {16, 0, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1, 0, 1},
        {16, 0, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1, 0, 1},
        {16, 0, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1, 0, 1},
        {16, 0, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1, 0, 1} }
    },
    { "01. House", 124.0f, 0.15f, 0.05f, 4, 0,  0.7f, 0.4f, 0.15f, 0.2f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.5f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.3f, 1.0f, 0.8f, 0.5f, 0.001f, 0.3f, 0.9f, 0.9f, 0.3f, 0.8f, 0.8f, 1, 0, 0},
        {16, 16,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 1.0f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 4, 2, 2, 80.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.01f, 0.4f, 0.9f, 0.8f, 0.2f, 1.0f, 0.7f, 1, 0, 1},
        {16, 5, 2, 2, 48.0f, 0.0f, 0.3f, 0.6f, 0.0f, 0.01f, 0.25f,0.6f, 0.4f, 0.6f, 0.0f, 0.8f, 1, 0, 1},
        {12, 5, 0, 2, 72.0f, 0.1f, 0.0f, 0.4f, 0.3f, 0.001f, 0.15f,0.4f, 0.6f, 0.4f, 0.0f, 0.7f, 1, 0, 1} }
    },
    { "02. Techno", 132.0f, 0.05f, 0.05f, 3, 0,  0.7f, 0.4f, 0.15f, 0.2f, 1, {
        {16, 4, 0, 2, 36.0f, 0.7f, 0.0f, 0.2f, 0.0f, 0.001f, 0.4f, 0.6f, 0.4f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 16,0, 2, 36.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.01f, 0.2f, 0.8f, 0.15f,0.0f, 0.0f, 0.7f, 1, 0, 1},
        {16, 4, 2, 2, 84.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.2f, 0.9f, 0.9f, 0.0f, 1.0f, 0.6f, 1, 0, 0},
        {16, 2, 4, 2, 60.0f, 0.2f, 1.0f, 0.5f, 0.5f, 0.001f, 0.3f, 0.9f, 0.7f, 0.2f, 0.8f, 0.7f, 1, 0, 0},
        {16, 5, 3, 2, 48.0f, 0.0f, 0.5f, 0.6f, 0.0f, 0.001f, 0.2f, 0.4f, 0.5f, 0.8f, 0.0f, 0.8f, 1, 0, 2},
        {14, 3, 0, 2, 72.0f, 0.1f, 0.0f, 0.4f, 0.7f, 0.001f, 0.15f,0.3f, 0.6f, 0.5f, 0.0f, 0.6f, 1, 0, 1} }
    },
    { "03. Tribal", 132.0f, 0.25f, 0.02f, 4, 0,  0.5f, 0.2f, 0.1f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.5f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.3f, 1.0f, 0.6f, 0.4f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.8f, 0.7f, 1, 0, 0},
        {16, 6, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.08f,1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {8,  3, 2, 2, 72.0f, 0.1f, 0.0f, 0.2f, 0.8f, 0.001f, 0.1f, 0.2f, 0.7f, 0.6f, 0.0f, 0.8f, 1, 0, 1},
        {8,  3, 5, 2, 76.0f, 0.1f, 0.0f, 0.2f, 0.9f, 0.001f, 0.1f, 0.2f, 0.7f, 0.6f, 0.0f, 0.8f, 1, 0, 1},
        {16, 1, 15,2, 84.0f, 0.9f, 0.5f, 0.4f, 0.2f, 0.001f, 0.6f, 0.7f, 0.8f, 0.0f, 0.0f, 0.7f, 1, 0, 1} }
    },
    { "04. Dubstep", 140.0f, 0.0f, 0.05f, 3, 0,  0.7f, 0.5f, 0.2f, 0.3f, 1, {
        {16, 2, 0, 2, 36.0f, 0.6f, 0.0f, 0.3f, 0.0f, 0.001f, 0.6f, 0.7f, 0.4f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 8, 2, 55.0f, 0.4f, 1.0f, 1.0f, 0.6f, 0.001f, 0.4f, 0.9f, 0.9f, 0.2f, 0.9f, 0.9f, 1, 0, 0},
        {16, 12,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.8f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 4, 0, 2, 40.0f, 0.0f, 0.5f, 0.8f, 0.0f, 0.1f,  0.8f, 0.4f, 0.3f, 0.7f, 0.0f, 0.9f, 1, 0, 2},
        {11, 3, 0, 2, 72.0f, 0.2f, 0.0f, 0.2f, 0.9f, 0.001f, 0.2f, 0.2f, 0.7f, 0.4f, 0.0f, 0.6f, 1, 0, 1},
        {16, 1, 14,2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,  2.0f, 0.1f, 0.3f, 0.2f, 0.0f, 0.8f, 1, 1, 2} }
    },
    { "05. UK Garage", 130.0f, 0.35f, 0.02f, 3, 0,  0.25f, 0.3f, 0.1f, 0.1f, 1, {
        {16, 3, 0, 2, 36.0f, 0.7f, 0.0f, 0.1f, 0.0f, 0.001f, 0.4f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.2f, 1.0f, 0.5f, 0.3f, 0.001f, 0.25f,0.9f, 0.8f, 0.1f, 0.7f, 0.8f, 1, 0, 0},
        {16, 7, 1, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 3, 2, 2, 38.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.05f, 0.5f, 0.5f, 0.2f, 0.0f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 14,2, 65.0f, 0.0f, 0.5f, 0.8f, 0.4f, 0.001f, 0.3f, 0.7f, 0.6f, 0.6f, 0.0f, 0.7f, 1, 0, 2},
        {16, 5, 7, 2, 72.0f, 0.0f, 0.8f, 0.9f, 0.1f, 0.01f, 0.2f, 0.4f, 0.5f, 0.8f, 0.0f, 0.6f, 1, 0, 2} }
    },
    { "06. Psy-Trance", 145.0f, 0.0f, 0.01f, 11, 0,  0.5f, 0.6f, 0.2f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.6f, 0.0f, 0.2f, 0.0f, 0.001f, 0.3f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 12,1, 2, 43.0f, 0.0f, 0.3f, 0.3f, 0.0f, 0.01f, 0.15f,0.9f, 0.3f, 0.4f, 0.0f, 0.85f,1, 0, 2},
        {16, 4, 2, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.3f, 0.9f, 0.9f, 0.0f, 1.0f, 0.6f, 1, 0, 0},
        {16, 8, 0, 2, 84.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.9f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 5, 0, 2, 67.0f, 0.0f, 0.5f, 0.8f, 0.0f, 0.01f, 0.2f, 0.5f, 0.6f, 0.9f, 0.0f, 0.7f, 1, 0, 2},
        {14, 3, 0, 2, 79.0f, 0.0f, 0.5f, 0.9f, 0.2f, 0.01f, 0.2f, 0.5f, 0.5f, 0.9f, 0.0f, 0.7f, 1, 0, 2} }
    },
    { "07. Cumbia", 85.0f, 0.15f, 0.05f, 3, 0,  0.8f, 0.6f, 0.4f, 0.3f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.6f, 0.8f, 0.2f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {8,  3, 0, 2, 65.0f, 0.1f, 0.0f, 0.3f, 0.8f, 0.001f, 0.2f, 0.5f, 0.8f, 0.5f, 0.0f, 0.7f, 1, 0, 1},
        {16, 8, 1, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.15f,0.9f, 0.7f, 0.0f, 1.0f, 0.5f, 1, 0, 1},
        {16, 4, 2, 2, 43.0f, 0.0f, 0.1f, 0.3f, 0.0f, 0.05f, 0.6f, 0.5f, 0.25f,0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 3, 4, 2, 72.0f, 0.0f, 0.3f, 0.5f, 0.0f, 0.01f, 0.3f, 0.6f, 0.5f, 0.5f, 0.0f, 0.7f, 1, 0, 1},
        {16, 1, 12,2, 84.0f, 0.9f, 0.5f, 0.2f, 0.0f, 0.01f, 0.8f, 0.8f, 0.9f, 0.8f, 0.0f, 0.6f, 1, 0, 2} }
    },
    { "08. Electro", 130.0f, 0.1f, 0.08f, 3, 0,  0.5f, 0.4f, 0.2f, 0.1f, 1, {
        {16, 3, 0, 2, 36.0f, 0.8f, 0.0f, 0.2f, 0.0f, 0.001f, 0.4f, 0.8f, 0.3f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.4f, 1.0f, 0.8f, 0.5f, 0.001f, 0.25f,0.9f, 0.8f, 0.3f, 0.9f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 9, 0, 2, 48.0f, 0.0f, 0.5f, 0.7f, 0.2f, 0.001f, 0.15f,0.4f, 0.5f, 0.7f, 0.0f, 0.85f,1, 0, 2},
        {12, 5, 0, 2, 72.0f, 0.0f, 0.0f, 0.2f, 0.8f, 0.001f, 0.2f, 0.2f, 0.6f, 0.5f, 0.0f, 0.7f, 1, 0, 1},
        {16, 2, 7, 2, 84.0f, 0.5f, 1.0f, 0.9f, 0.9f, 0.001f, 0.3f, 0.9f, 1.0f, 0.8f, 0.5f, 0.6f, 1, 0, 0} }
    },
    { "09. IDM", 115.0f, 0.0f, 0.25f, 1, 0,  0.25f, 0.6f, 0.3f, 0.4f, 1, {
        {11, 5, 0, 2, 40.0f, 0.8f, 0.0f, 0.5f, 0.2f, 0.001f, 0.3f, 0.8f, 0.4f, 0.3f, 0.1f, 0.9f, 1, 0, 1},
        {13, 3, 2, 2, 60.0f, 0.2f, 1.0f, 1.0f, 0.8f, 0.001f, 0.15f,0.9f, 0.9f, 0.5f, 0.9f, 0.7f, 1, 0, 0},
        {15, 11,0, 2, 84.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.001f, 0.05f,1.0f, 1.0f, 0.2f, 1.0f, 0.5f, 1, 0, 0},
        {9,  7, 1, 2, 72.0f, 0.5f, 0.0f, 0.9f, 0.9f, 0.001f, 0.1f, 0.1f, 0.8f, 0.8f, 0.0f, 0.6f, 1, 0, 1},
        {7,  4, 0, 2, 48.0f, 0.0f, 0.3f, 0.8f, 0.3f, 0.01f,  0.4f, 0.5f, 0.5f, 0.9f, 0.0f, 0.8f, 1, 0, 2},
        {16, 5, 3, 2, 80.0f, 0.9f, 1.0f, 0.9f, 0.5f, 0.001f, 0.2f, 0.8f, 0.9f, 0.6f, 0.3f, 0.5f, 1, 0, 0} }
    },
    { "10. Ambient", 65.0f, 0.0f, 0.3f, 12, 0,  0.8f, 0.75f, 0.55f, 0.6f, 1, {
        {11, 2, 0, 2, 48.0f, 0.0f, 0.5f, 0.4f, 0.1f, 0.8f,   2.5f, 0.2f, 0.3f, 0.2f, 0.0f, 0.7f, 1, 2, 1},
        {17, 3, 0, 2, 60.0f, 0.0f, 0.2f, 0.3f, 0.4f, 1.2f,   3.0f, 0.1f, 0.4f, 0.5f, 0.0f, 0.6f, 1, 2, 1},
        {13, 1, 0, 2, 72.0f, 0.0f, 0.0f, 0.1f, 0.8f, 0.01f,  2.0f, 0.5f, 0.8f, 0.7f, 0.0f, 0.5f, 1, 0, 1},
        {19, 2, 0, 2, 84.0f, 0.0f, 0.0f, 0.1f, 0.9f, 0.01f,  1.5f, 0.6f, 0.9f, 0.6f, 0.0f, 0.4f, 1, 0, 1},
        {7,  1, 0, 2, 36.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f,   2.0f, 0.1f, 0.2f, 0.1f, 0.0f, 0.8f, 1, 2, 2},
        {23, 3, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,   2.0f, 0.1f, 0.6f, 0.8f, 0.8f, 0.4f, 1, 2, 2} }
    },
    { "11. Jungle", 165.0f, 0.1f, 0.1f, 3, 0,  0.5f, 0.3f, 0.2f, 0.3f, 1, {
        {16, 5, 0, 2, 36.0f, 0.7f, 0.0f, 0.2f, 0.0f, 0.001f, 0.3f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.3f, 1.0f, 0.6f, 0.4f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 7, 2, 2, 60.0f, 0.1f, 1.0f, 0.2f, 0.2f, 0.001f, 0.1f, 0.9f, 0.4f, 0.1f, 0.6f, 0.35f,1, 0, 0},
        {16, 14,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 3, 0, 2, 36.0f, 0.0f, 0.5f, 0.7f, 0.2f, 0.1f,   1.5f, 0.8f, 0.2f, 0.4f, 0.0f, 0.9f, 1, 2, 2},
        {16, 1, 0, 2, 72.0f, 0.0f, 0.5f, 0.5f, 0.1f, 0.5f,   2.0f, 0.5f, 0.6f, 0.8f, 0.1f, 0.5f, 1, 2, 2} }
    },
    { "12. Footwork", 160.0f, 0.2f, 0.05f, 4, 0,  0.5f, 0.4f, 0.15f, 0.2f, 1, {
        {8,  3, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.6f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 5, 4, 2, 60.0f, 0.2f, 1.0f, 0.7f, 0.3f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 12,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 4, 3, 2, 48.0f, 0.4f, 0.0f, 0.2f, 0.6f, 0.001f, 0.3f, 0.6f, 0.6f, 0.5f, 0.0f, 0.7f, 1, 0, 1},
        {16, 3, 6, 2, 72.0f, 0.0f, 0.3f, 0.5f, 0.8f, 0.01f,  0.2f, 0.4f, 0.5f, 0.8f, 0.0f, 0.7f, 1, 0, 2},
        {16, 1, 14,2, 36.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f,  2.0f, 0.8f, 0.2f, 0.1f, 0.0f, 0.9f, 1, 0, 2} }
    },
    { "13. Reggaeton", 95.0f, 0.0f, 0.02f, 4, 0,  0.5f, 0.3f, 0.1f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.5f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 4, 3, 2, 60.0f, 0.3f, 1.0f, 0.6f, 0.4f, 0.001f, 0.3f, 0.9f, 0.8f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 2, 8, 2, 48.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.01f,  0.6f, 0.5f, 0.4f, 0.3f, 0.0f, 0.8f, 1, 0, 1},
        {8,  3, 0, 2, 72.0f, 0.1f, 0.0f, 0.2f, 0.8f, 0.001f, 0.2f, 0.2f, 0.7f, 0.6f, 0.0f, 0.6f, 1, 0, 1},
        {16, 1, 15,2, 84.0f, 0.5f, 0.5f, 0.4f, 0.2f, 0.001f, 0.4f, 0.7f, 0.8f, 0.0f, 0.0f, 0.5f, 1, 0, 1} }
    },
    { "14. Neoperreo", 100.0f, 0.0f, 0.15f, 3, 0,  0.6f, 0.5f, 0.25f, 0.3f, 1, {
        {16, 4, 0, 2, 36.0f, 0.9f, 0.0f, 0.5f, 0.2f, 0.001f, 0.4f, 0.8f, 0.5f, 0.3f, 0.1f, 0.9f, 1, 0, 1},
        {16, 4, 3, 2, 60.0f, 0.4f, 1.0f, 0.9f, 0.7f, 0.001f, 0.2f, 0.9f, 0.9f, 0.4f, 0.9f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 2, 8, 2, 40.0f, 0.0f, 0.3f, 0.8f, 0.2f, 0.01f,  0.5f, 0.5f, 0.3f, 0.8f, 0.0f, 0.9f, 1, 0, 2},
        {8,  3, 0, 2, 72.0f, 0.0f, 0.8f, 0.8f, 0.5f, 0.001f, 0.2f, 0.2f, 0.9f, 0.9f, 0.0f, 0.7f, 1, 0, 2},
        {16, 1, 15,2, 84.0f, 0.9f, 1.0f, 0.9f, 0.8f, 0.001f, 0.6f, 0.8f, 1.0f, 0.8f, 0.5f, 0.6f, 1, 0, 0} }
    },
    { "15. Baile Funk", 130.0f, 0.05f, 0.02f, 4, 0,  0.5f, 0.2f, 0.1f, 0.1f, 1, {
        {8,  3, 0, 2, 45.0f, 0.5f, 0.0f, 0.2f, 0.0f, 0.001f, 0.3f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {8,  3, 3, 2, 65.0f, 0.2f, 1.0f, 0.5f, 0.3f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.6f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 2, 12,2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.5f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 5, 2, 2, 72.0f, 0.0f, 0.0f, 0.4f, 0.8f, 0.001f, 0.15f,0.5f, 0.7f, 0.6f, 0.0f, 0.6f, 1, 0, 1},
        {16, 1, 7, 2, 84.0f, 0.5f, 0.0f, 0.2f, 0.0f, 0.001f, 0.4f, 0.7f, 0.8f, 0.0f, 0.0f, 0.5f, 1, 0, 1} }
    },
    { "16. Jersey Club", 135.0f, 0.1f, 0.05f, 4, 0,  0.5f, 0.4f, 0.15f, 0.1f, 1, {
        {16, 5, 0, 2, 36.0f, 0.8f, 0.0f, 0.2f, 0.0f, 0.001f, 0.4f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 8, 2, 60.0f, 0.3f, 1.0f, 0.5f, 0.5f, 0.001f, 0.2f, 0.9f, 0.9f, 0.3f, 0.8f, 0.8f, 1, 0, 0},
        {16, 16,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 1.0f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 4, 2, 2, 80.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.01f, 0.3f, 0.9f, 0.8f, 0.2f, 1.0f, 0.7f, 1, 0, 1},
        {16, 3, 4, 2, 72.0f, 0.1f, 0.0f, 0.2f, 0.8f, 0.001f, 0.1f, 0.2f, 0.7f, 0.6f, 0.0f, 0.8f, 1, 0, 1},
        {16, 2, 14,2, 48.0f, 0.0f, 0.3f, 0.6f, 0.0f, 0.01f, 0.4f, 0.6f, 0.4f, 0.6f, 0.0f, 0.8f, 1, 0, 1} }
    },
    { "17. Gqom", 125.0f, 0.0f, 0.05f, 3, 0,  0.6f, 0.5f, 0.2f, 0.1f, 1, {
        {16, 5, 0, 2, 36.0f, 0.7f, 0.0f, 0.3f, 0.0f, 0.001f, 0.5f, 0.8f, 0.3f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 3, 4, 2, 48.0f, 0.4f, 0.0f, 0.4f, 0.2f, 0.001f, 0.4f, 0.7f, 0.5f, 0.3f, 0.0f, 0.8f, 1, 0, 1},
        {16, 4, 2, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.8f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 2, 12,2, 60.0f, 0.2f, 1.0f, 0.5f, 0.2f, 0.001f, 0.2f, 0.9f, 0.7f, 0.1f, 0.8f, 0.7f, 1, 0, 0},
        {16, 3, 7, 2, 36.0f, 0.0f, 0.2f, 0.6f, 0.0f, 0.1f,  0.8f, 0.5f, 0.2f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 1, 15,2, 72.0f, 0.0f, 0.5f, 0.8f, 0.4f, 0.001f, 0.3f, 0.7f, 0.6f, 0.6f, 0.0f, 0.6f, 1, 0, 2} }
    },
    { "18. Amapiano", 113.0f, 0.0f, 0.02f, 4, 0,  0.5f, 0.3f, 0.1f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.4f, 0.8f, 0.3f, 0.1f, 0.0f, 0.8f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.1f, 0.0f, 0.2f, 0.0f, 0.001f, 0.1f, 0.9f, 0.8f, 0.1f, 0.2f, 0.6f, 1, 0, 1},
        {16, 16,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.9f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 3, 10,2, 40.0f, 0.2f, 0.0f, 0.4f, 0.6f, 0.01f,  0.6f, 0.6f, 0.4f, 0.4f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 14,2, 45.0f, 0.2f, 0.0f, 0.4f, 0.6f, 0.01f,  0.5f, 0.6f, 0.4f, 0.4f, 0.0f, 0.9f, 1, 0, 1},
        {16, 5, 0, 2, 72.0f, 0.0f, 0.2f, 0.2f, 0.2f, 0.05f,  0.4f, 0.8f, 0.6f, 0.2f, 0.0f, 0.6f, 1, 0, 1} }
    },
    { "19. Kuduro", 130.0f, 0.1f, 0.05f, 3, 0,  0.6f, 0.4f, 0.2f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.2f, 0.0f, 0.001f, 0.3f, 0.8f, 0.4f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 3, 3, 2, 60.0f, 0.2f, 1.0f, 0.5f, 0.3f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {8,  3, 0, 2, 72.0f, 0.1f, 0.0f, 0.4f, 0.6f, 0.001f, 0.15f,0.4f, 0.6f, 0.4f, 0.0f, 0.7f, 1, 0, 1},
        {8,  3, 2, 2, 65.0f, 0.1f, 0.0f, 0.4f, 0.5f, 0.001f, 0.15f,0.4f, 0.5f, 0.4f, 0.0f, 0.7f, 1, 0, 1},
        {16, 1, 14,2, 84.0f, 0.5f, 0.5f, 0.8f, 0.9f, 0.001f, 0.3f, 0.7f, 0.8f, 0.8f, 0.0f, 0.6f, 1, 0, 2} }
    },
    { "20. Trap", 140.0f, 0.0f, 0.02f, 3, 0,  0.5f, 0.4f, 0.1f, 0.1f, 1, {
        {16, 2, 0, 2, 36.0f, 0.6f, 0.0f, 0.1f, 0.0f, 0.001f, 1.2f, 0.8f, 0.2f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 8, 2, 60.0f, 0.4f, 1.0f, 0.8f, 0.3f, 0.001f, 0.2f, 0.9f, 0.9f, 0.2f, 0.9f, 0.9f, 1, 0, 0},
        {16, 16,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 3, 6, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.9f, 0.0f, 1.0f, 0.6f, 1, 0, 0},
        {16, 4, 0, 2, 72.0f, 0.0f, 0.0f, 0.2f, 0.8f, 0.01f,  0.3f, 0.2f, 0.7f, 0.6f, 0.0f, 0.6f, 1, 0, 1},
        {16, 1, 14,2, 48.0f, 0.0f, 0.5f, 0.6f, 0.0f, 0.01f,  0.4f, 0.6f, 0.4f, 0.6f, 0.0f, 0.8f, 1, 0, 2} }
    },
    { "21. Juke", 160.0f, 0.25f, 0.05f, 3, 0,  0.5f, 0.3f, 0.1f, 0.2f, 1, {
        {12, 3, 0, 2, 36.0f, 0.8f, 0.0f, 0.2f, 0.0f, 0.001f, 0.4f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 4, 4, 2, 60.0f, 0.2f, 1.0f, 0.7f, 0.3f, 0.001f, 0.2f, 0.9f, 0.8f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 12,0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 3, 10,2, 36.0f, 0.8f, 0.0f, 0.1f, 0.0f, 0.001f, 0.6f, 0.8f, 0.4f, 0.2f, 0.0f, 0.9f, 1, 0, 1},
        {16, 5, 2, 2, 72.0f, 0.0f, 0.3f, 0.5f, 0.8f, 0.01f,  0.2f, 0.4f, 0.5f, 0.8f, 0.0f, 0.7f, 1, 0, 2},
        {16, 2, 7, 2, 84.0f, 0.0f, 0.5f, 0.8f, 0.4f, 0.001f, 0.3f, 0.7f, 0.6f, 0.6f, 0.0f, 0.6f, 1, 0, 2} }
    },
    { "22. Hyperpop", 145.0f, 0.0f, 0.1f, 4, 0,  0.6f, 0.5f, 0.25f, 0.4f, 1, {
        {16, 4, 0, 2, 36.0f, 0.9f, 0.0f, 0.4f, 0.0f, 0.001f, 0.3f, 0.8f, 0.6f, 0.3f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.5f, 1.0f, 0.9f, 0.8f, 0.001f, 0.2f, 0.9f, 0.9f, 0.5f, 0.9f, 0.8f, 1, 0, 0},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.9f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 5, 2, 2, 72.0f, 0.6f, 0.0f, 0.5f, 0.9f, 0.001f, 0.15f,0.2f, 0.8f, 0.8f, 0.0f, 0.8f, 1, 0, 1},
        {16, 3, 6, 2, 84.0f, 0.0f, 0.5f, 0.8f, 0.0f, 0.01f,  0.2f, 0.5f, 0.6f, 0.9f, 0.0f, 0.7f, 1, 0, 2},
        {16, 2, 14,2, 48.0f, 0.0f, 0.5f, 0.8f, 0.0f, 0.01f,  0.4f, 0.6f, 0.4f, 0.6f, 0.0f, 0.8f, 1, 0, 1} }
    },
    { "23. Deconstructed", 120.0f, 0.0f, 0.3f, 1, 0,  0.8f, 0.6f, 0.3f, 0.5f, 1, {
        {7,  2, 0, 2, 36.0f, 0.8f, 0.0f, 0.5f, 0.0f, 0.001f, 0.5f, 0.8f, 0.5f, 0.2f, 0.1f, 0.9f, 1, 0, 1},
        {11, 3, 2, 2, 60.0f, 0.3f, 1.0f, 1.0f, 0.8f, 0.001f, 0.2f, 0.9f, 0.9f, 0.6f, 0.9f, 0.8f, 1, 0, 0},
        {13, 5, 0, 2, 80.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.001f, 0.1f, 1.0f, 1.0f, 0.2f, 1.0f, 0.5f, 1, 0, 0},
        {16, 1, 0, 2, 40.0f, 0.0f, 0.3f, 0.8f, 0.2f, 0.01f,  1.0f, 0.5f, 0.3f, 0.8f, 0.0f, 0.9f, 1, 0, 2},
        {17, 4, 3, 2, 72.0f, 0.5f, 0.0f, 0.9f, 0.9f, 0.001f, 0.1f, 0.1f, 0.8f, 0.8f, 0.0f, 0.6f, 1, 0, 1},
        {19, 2, 7, 2, 84.0f, 0.9f, 1.0f, 0.9f, 0.8f, 0.001f, 0.6f, 0.8f, 1.0f, 0.8f, 0.5f, 0.6f, 1, 0, 0} }
    },
    { "24. Minimal Synth", 125.0f, 0.1f, 0.0f, 3, 0,  0.6f, 0.4f, 0.2f, 0.1f, 1, {
        {16, 4, 0, 2, 36.0f, 0.7f, 0.0f, 0.1f, 0.0f, 0.001f, 0.3f, 0.8f, 0.2f, 0.1f, 0.0f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.1f, 0.5f, 0.2f, 0.0f, 0.001f, 0.15f,0.9f, 0.6f, 0.1f, 0.4f, 0.7f, 1, 0, 1},
        {16, 8, 0, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.05f,1.0f, 0.8f, 0.0f, 1.0f, 0.4f, 1, 0, 0},
        {16, 1, 14,2, 72.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 0.2f, 0.5f, 0.4f, 0.0f, 0.7f, 1, 0, 1},
        {16, 3, 2, 2, 48.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.01f,  0.2f, 0.5f, 0.3f, 0.2f, 0.0f, 0.8f, 1, 0, 1},
        {16, 2, 7, 2, 84.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 0.5f, 0.5f, 0.4f, 0.0f, 0.6f, 1, 0, 1} }
    },
    { "25. Lo-Fi House", 118.0f, 0.2f, 0.15f, 4, 0,  0.7f, 0.6f, 0.3f, 0.8f, 1, {
        {16, 4, 0, 2, 36.0f, 0.8f, 0.0f, 0.3f, 0.0f, 0.001f, 0.4f, 0.8f, 0.3f, 0.1f, 0.1f, 0.9f, 1, 0, 1},
        {16, 2, 4, 2, 60.0f, 0.3f, 1.0f, 0.5f, 0.3f, 0.001f, 0.3f, 0.9f, 0.6f, 0.2f, 0.8f, 0.8f, 1, 0, 0},
        {16, 8, 2, 2, 80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.1f, 1.0f, 0.5f, 0.0f, 1.0f, 0.5f, 1, 0, 0},
        {16, 5, 2, 2, 48.0f, 0.0f, 0.3f, 0.6f, 0.0f, 0.01f,  0.25f,0.6f, 0.3f, 0.6f, 0.1f, 0.8f, 1, 0, 1},
        {16, 3, 4, 2, 65.0f, 0.0f, 0.5f, 0.5f, 0.4f, 0.01f,  0.4f, 0.7f, 0.5f, 0.6f, 0.2f, 0.7f, 1, 0, 2},
        {16, 2, 12,2, 72.0f, 0.0f, 0.5f, 0.5f, 0.4f, 0.01f,  0.4f, 0.7f, 0.5f, 0.6f, 0.2f, 0.7f, 1, 0, 2} }
    }
};

struct SynthPreset { const char* name; float p[12]; int m281; int m292; };

static const std::vector<SynthPreset> PRESETS = {
    {"Init / Default", {60,0,0,0,0, 0.01f,0.5f,0.5f,0.8f,0.1f,0,0.8f}, 0, 1},
    {"Buchla Bongo", {65,0.3f,0,0.4f,0.1f, 0.001f,0.2f,0.1f,0.6f,0.4f,0,0.9f}, 0, 1},
    {"808 Sub Kick", {36,0.9f,0,0,0, 0.001f,0.8f,0.8f,0.3f,0.1f,0,0.9f}, 0, 1},
    {"909 Hard Kick", {36,0.7f,0.2f,0.5f,0.2f, 0.001f,0.6f,0.9f,0.5f,0.2f,0.3f,0.9f}, 0, 1},
    {"Tribal Zap", {72,0.8f,0,0.6f,0.3f, 0.001f,0.1f,1.0f,0.9f,0.6f,0,0.8f}, 0, 1},
    {"Guaracha Pluck", {65,0.1f,0.5f,0.3f,0, 0.01f,0.3f,0.8f,1.0f,0.5f,0,0.8f}, 0, 1},
    {"Cumbia Bass", {36,0,0,0,0, 0.05f,0.4f,0.2f,0.4f,0.2f,0,0.9f}, 1, 2},
    {"FM Snare", {60,0.2f,1.0f,0.8f,0.9f, 0.001f,0.2f,0.9f,1.0f,0.4f,0.6f,0.8f}, 0, 0},
    {"White Noise Hat", {80,0,0,0,0, 0.001f,0.05f,1.0f,1.0f,0.0f,1.0f,0.7f}, 0, 0},
    {"Open Hat Metal", {75,0,0.5f,0.9f,0.7f, 0.01f,0.4f,0.9f,0.8f,0.3f,0.3f,0.7f}, 0, 1},
    {"Wooden Plonk", {70,0.2f,0,0.1f,0, 0.001f,0.15f,0.05f,0.4f,0.5f,0,0.9f}, 0, 1},
    {"Dark Drone (Hold)", {48,0,0,0.4f,0.1f, 0.5f,1.5f,0.1f,0.3f,0.2f,0,0.8f}, 1, 2},
    {"Space Bell", {84,0,0,0.8f,0.6f, 0.01f,1.5f,0.6f,0.9f,0.7f,0,0.7f}, 0, 1},
    {"Acid Bassline", {40,0.1f,0.5f,0.8f,0, 0.01f,0.3f,0.8f,0.4f,0.8f,0,0.9f}, 0, 1},
    {"IDM Glitch", {72,0.9f,1.0f,0.9f,0.9f, 0.001f,0.05f,1.0f,1.0f,0.5f,0.1f,0.8f}, 0, 0},
    {"LFO Cycle Sweep", {48,0,0,0.8f,0.5f, 0.5f,0.5f,0.5f,0.7f,0.3f,0,0.8f}, 2, 1},
    {"FM Tom", {45,0.6f,0,0.3f,0.4f, 0.001f,0.4f,0.7f,0.5f,0.2f,0,0.9f}, 0, 1},
    {"Cinematic Brass", {48,0,0.8f,0.5f,0.2f, 0.3f,1.0f,0.2f,0.7f,0.4f,0,0.8f}, 1, 2},
    {"Glass Marimba", {76,0,0,0.2f,0.3f, 0.001f,0.3f,0.15f,0.9f,0.5f,0,0.8f}, 0, 1},
    {"Industrial Crash", {50,0.3f,1.0f,1.0f,0.8f, 0.01f,1.2f,0.9f,1.0f,0.6f,0.8f,0.8f}, 0, 0},
    {"Ghost Choir", {60,0,0,0.2f,0, 0.8f,2.0f,0.1f,0.4f,0.2f,0.2f,0.7f}, 1, 2},
    {"Laser Gun", {80,1.0f,0.5f,0.4f,0.1f, 0.001f,0.2f,0.9f,0.8f,0.3f,0,0.8f}, 0, 1},
    {"Water Drop", {75,0.4f,0,0.1f,0.1f, 0.001f,0.1f,0.2f,0.7f,0.6f,0,0.9f}, 0, 1},
    {"VCA Sub", {36,0,0,0,0, 0.01f,0.5f,0.5f,1.0f,0.1f,0,0.9f}, 0, 0},
    {"Morphing Lead", {60,0,0.5f,0.6f,0.2f, 0.1f,0.5f,0.8f,0.9f,0.4f,0,0.8f}, 1, 1}
};

OrbitaLPGAudioProcessorEditor::OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    setWantsKeyboardFocus(true);

    auto setParam = [&](const juce::String& id, float val) {
        if (auto* param = audioProcessor.apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    auto lbl = [this](juce::Label& l, const char* txt, juce::Colour col = juce::Colour(160, 175, 190)) {
        l.setText(txt, juce::dontSendNotification);
        l.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, col);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };
    auto btn = [this](juce::TextButton& b, juce::Colour col = juce::Colour(28, 34, 42), juce::Colour textCol = juce::Colours::white) {
        b.setColour(juce::TextButton::buttonColourId, col);
        b.setColour(juce::TextButton::textColourOffId, textCol);
        addAndMakeVisible(b);
    };
    auto masterSld = [this](juce::Slider& s, juce::Colour accent) {
        s.setSliderStyle(juce::Slider::LinearBar);
        s.setColour(juce::Slider::trackColourId, accent);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        addAndMakeVisible(s);
    };

    titleLabel.setText("ORBITA-LPG  ///  6-VOICE MATRIX", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(210, 224, 236));
    addAndMakeVisible(titleLabel);

    creditLabel.setText("v2.0.0 GPLv3  |  @laurorobles / extasis records", juce::dontSendNotification);
    creditLabel.setFont(juce::FontOptions(9.0f));
    creditLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.3f));
    creditLabel.setJustificationType(juce::Justification::bottomRight);
    addAndMakeVisible(creditLabel);

    btn(loadBtn, juce::Colour(28, 34, 42), juce::Colour(200, 215, 230));
    btn(saveBtn, juce::Colour(28, 34, 42), juce::Colour(200, 215, 230));
    loadBtn.setTooltip("Load a custom .xml preset.");
    saveBtn.setTooltip("Save current state to a .xml preset.");
    loadBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Load Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (fc.getResult().existsAsFile()) {
                    juce::XmlDocument xmlDoc(fc.getResult());
                    if (auto xml = xmlDoc.getDocumentElement())
                        audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*xml));
                }
            });
    };
    saveBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Save Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (fc.getResult() != juce::File()) {
                    if (auto xml = audioProcessor.apvts.copyState().createXml())
                        xml->writeTo(fc.getResult());
                }
            });
    };

    addAndMakeVisible(kitCombo);
    for (int i = 0; i < (int)MASTER_KITS.size(); ++i) kitCombo.addItem(MASTER_KITS[(size_t)i].name, i + 1);
    kitCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    kitCombo.setTooltip("Load a Master Genre Kit (overwrites all sequencer and synth parameters).");
    kitCombo.onChange = [this, setParam]() {
        int idx = kitCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < (int)MASTER_KITS.size()) {
            const auto& k = MASTER_KITS[(size_t)idx];
            setParam("bpm", k.bpm);
            setParam("swing", k.swing);
            setParam("chaos", k.chaos);
            setParam("global_scale", (float)k.scale);
            setParam("global_root", (float)k.root);
            setParam("echo_time", k.e_time);
            setParam("echo_fdbk", k.e_fdbk);
            setParam("echo_mix", k.e_mix);
            setParam("echo_wow", k.e_wow);
            setParam("echo_sync", (float)k.e_sync);
            for (int i = 0; i < 6; ++i) {
                juce::String ts = "t" + juce::String(i + 1) + "_";
                const auto& tr = k.t[i];
                setParam(ts + "steps", (float)tr.steps);
                setParam(ts + "pulses", (float)tr.pulses);
                setParam(ts + "offset", (float)tr.offset);
                setParam(ts + "rate", (float)tr.rate);
                setParam(ts + "pitch", tr.pitch);
                setParam(ts + "drop", tr.drop);
                setParam(ts + "morph", tr.morph);
                setParam(ts + "fold", tr.fold);
                setParam(ts + "fm", tr.fm);
                setParam(ts + "rise", tr.rise);
                setParam(ts + "fall", tr.fall);
                setParam(ts + "resp", tr.resp);
                setParam(ts + "brgt", tr.brgt);
                setParam(ts + "reso", tr.reso);
                setParam(ts + "noise", tr.noise);
                setParam(ts + "vol", tr.vol);
                setParam(ts + "notemode", (float)tr.notemode);
                setParam(ts + "mode281", (float)tr.mode281);
                setParam(ts + "mode292", (float)tr.mode292);
                synthPresetCombo[i].setSelectedItemIndex(0, juce::dontSendNotification);
            }
        }
    };

    btn(playBtn, juce::Colour(30, 82, 56), juce::Colour(140, 255, 190));
    btn(stopBtn, juce::Colour(86, 34, 38), juce::Colours::white);
    btn(seqBtn, juce::Colour(24, 40, 52), juce::Colours::cyan);
    btn(aboutBtn, juce::Colour(26, 34, 44), juce::Colours::cyan);

    playBtn.setTooltip("Toggle internal transport clock (Space).");
    stopBtn.setTooltip("Stop transport and reset all sequences to step 0.");
    seqBtn.setTooltip("Enable/Disable Euclidean sequencer.");
    aboutBtn.setTooltip("About Orbita-LPG (Open Source info, shortcuts & MIDI map).");

    playBtn.onClick = [this]() {
        bool next = !audioProcessor.isPlaying.load(std::memory_order_relaxed);
        audioProcessor.isPlaying.store(next, std::memory_order_relaxed);
    };
    stopBtn.onClick = [this]() {
        audioProcessor.isPlaying.store(false, std::memory_order_relaxed);
        audioProcessor.pendingReset.store(true, std::memory_order_relaxed);
    };
    seqBtn.onClick = [this]() {
        bool next = !audioProcessor.seqEnabled.load(std::memory_order_relaxed);
        audioProcessor.seqEnabled.store(next, std::memory_order_relaxed);
        seqBtn.setButtonText(next ? "SEQ: ON" : "SEQ: OFF");
        seqBtn.setColour(juce::TextButton::textColourOffId, next ? juce::Colours::cyan : juce::Colours::grey);
    };
    aboutBtn.onClick = [this]() {
        aboutOverlay.setVisible(true);
        aboutOverlay.toFront(true);
    };

    lbl(mVolLbl, "M.VOL", juce::Colours::yellow);
    lbl(mDriveLbl, "DRIVE", juce::Colour(245, 155, 45));
    lbl(mBpmLbl, "BPM", juce::Colour(170, 185, 200));
    lbl(mSwingLbl, "SWING", juce::Colour(170, 185, 200));
    lbl(mChaosLbl, "CHAOS", juce::Colour(190, 130, 255));
    lbl(mScaleLbl, "G.SCALE", juce::Colours::cyan);
    lbl(mRootLbl, "G.ROOT", juce::Colours::cyan);

    masterSld(mVolSld, juce::Colours::yellow);
    masterSld(mDriveSld, juce::Colour(245, 155, 45));
    masterSld(mBpmSld, juce::Colours::cyan);
    masterSld(mSwingSld, juce::Colour(65, 229, 155));
    masterSld(mChaosSld, juce::Colour(190, 130, 255));

    mVolSld.setTooltip("Master output volume post-effects.");
    mDriveSld.setTooltip("Global analog soft-clip saturation.");
    mBpmSld.setTooltip("Internal tempo (syncs automatically to DAW when playing).");
    mSwingSld.setTooltip("Groove swing amount applied to odd steps.");
    mChaosSld.setTooltip("West Coast pitch/fold instability injection (Xorshift PRNG).");

    addAndMakeVisible(globalScaleCombo);
    globalScaleCombo.addItemList({"Chromatic","Major","Minor","Dorian","Phrygian","Lydian","Mixolydian","Pent. Maj","Pent. Min","Harm. Min","Phryg. Dom","Hirajoshi","Whole Tone","Diminished"}, 1);
    globalScaleCombo.setTooltip("Global pitch quantization scale.");
    addAndMakeVisible(globalRootCombo);
    globalRootCombo.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1);
    globalRootCombo.setTooltip("Global root note.");

    mAtt[0] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_vol", mVolSld);
    mAtt[1] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_drive", mDriveSld);
    mAtt[2] = std::make_unique<SldAtt>(audioProcessor.apvts, "bpm", mBpmSld);
    mAtt[3] = std::make_unique<SldAtt>(audioProcessor.apvts, "swing", mSwingSld);
    mAtt[4] = std::make_unique<SldAtt>(audioProcessor.apvts, "chaos", mChaosSld);
    gScaleAtt = std::make_unique<CmbAtt>(audioProcessor.apvts, "global_scale", globalScaleCombo);
    gRootAtt = std::make_unique<CmbAtt>(audioProcessor.apvts, "global_root", globalRootCombo);

    auto updatePitchTexts = [this]() { for (int i = 0; i < 6; ++i) vSliders[i][0].slider.updateText(); };
    globalScaleCombo.onChange = updatePitchTexts;
    globalRootCombo.onChange = updatePitchTexts;

    for (int t = 0; t < 6; ++t) {
        tBtns[t].setButtonText("T" + juce::String(t + 1));
        btn(tBtns[t], juce::Colour(22, 28, 35), juce::Colours::white);
        tBtns[t].setTooltip("Select Track " + juce::String(t + 1) + " (Key " + juce::String(t + 1) + "). Right-click to MUTE.");
        tBtns[t].onClick = [this, t]() { selectTrack(t); };
        tBtns[t].onRightClick = [this, t]() { toggleMute(t); };

        synthPresetCombo[t].addItem("--- SYNTH PRESETS ---", 1);
        for (int i = 0; i < (int)PRESETS.size(); ++i) synthPresetCombo[t].addItem(PRESETS[(size_t)i].name, i + 2);
        synthPresetCombo[t].setSelectedItemIndex(0, juce::dontSendNotification);
        synthPresetCombo[t].setTooltip("Load a West Coast patch for Voice " + juce::String(t + 1) + ".");
        synthPresetCombo[t].onChange = [this, t, setParam]() {
            int id = synthPresetCombo[t].getSelectedId() - 2;
            if (id >= 0 && id < (int)PRESETS.size()) {
                juce::String ts = "t" + juce::String(t + 1) + "_";
                static const char* sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol"};
                for (int i = 0; i < 12; ++i) setParam(ts + sIds[i], PRESETS[(size_t)id].p[i]);
                setParam(ts + "mode281", (float)PRESETS[(size_t)id].m281);
                setParam(ts + "mode292", (float)PRESETS[(size_t)id].m292);
            }
        };
        addChildComponent(synthPresetCombo[t]);
    }

    addAndMakeVisible(patternsCombo);
    patternsCombo.addItem("TOUSSAINT RHYTHMS...", 1);
    patternsCombo.addItem("--- MUSICAL ---", 2); patternsCombo.setItemEnabled(2, false);
    patternsCombo.addItem("E(3,8) Tresillo", 3); patternsCombo.addItem("E(5,8) Cinquillo", 4);
    patternsCombo.addItem("E(5,12) Quintillo", 5); patternsCombo.addItem("E(7,12) African Bell", 6);
    patternsCombo.addItem("E(5,16) Bossa / Son", 7); patternsCombo.addItem("E(7,16) Samba", 8);
    patternsCombo.addItem("E(9,16) Rumba", 9); patternsCombo.addItem("E(11,24) Aka", 10);
    patternsCombo.addItem("--- POLY ---", 11); patternsCombo.setItemEnabled(11, false);
    patternsCombo.addItem("E(3,7)", 12); patternsCombo.addItem("E(4,9)", 13);
    patternsCombo.addItem("E(5,11)", 14); patternsCombo.addItem("E(7,13)", 15);
    patternsCombo.addItem("--- EXTENDED ---", 16); patternsCombo.setItemEnabled(16, false);
    patternsCombo.addItem("E(15,32)", 17); patternsCombo.addItem("E(17,32)", 18);
    patternsCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    patternsCombo.setTooltip("Ethnomusicological Euclidean rhythm library.");

    patternsCombo.onChange = [this, setParam]() {
        int id = patternsCombo.getSelectedId();
        auto stp = [&](int p, int s) {
            juce::String ts = "t" + juce::String(currentTrack + 1) + "_";
            setParam(ts + "steps", (float)s);
            setParam(ts + "pulses", (float)p);
            setParam(ts + "offset", 0.0f);
        };
        if (id == 3) stp(3, 8); else if (id == 4) stp(5, 8); else if (id == 5) stp(5, 12); else if (id == 6) stp(7, 12);
        else if (id == 7) stp(5, 16); else if (id == 8) stp(7, 16); else if (id == 9) stp(9, 16); else if (id == 10) stp(11, 24);
        else if (id == 12) stp(3, 7); else if (id == 13) stp(4, 9); else if (id == 14) stp(5, 11); else if (id == 15) stp(7, 13);
        else if (id == 17) stp(15, 32); else if (id == 18) stp(17, 32);
        if (id > 1) patternsCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    };

    btn(rMutateBtn, juce::Colour(44, 24, 58), juce::Colour(210, 150, 255));
    rMutateBtn.setTooltip("Organically mutates the current pattern (+/- 1 in Steps, Pulses, or Offset).");
    rMutateBtn.onClick = [this]() {
        juce::String ts = "t" + juce::String(currentTrack + 1) + "_";
        auto* pS = audioProcessor.apvts.getParameter(ts + "steps");
        auto* pP = audioProcessor.apvts.getParameter(ts + "pulses");
        auto* pO = audioProcessor.apvts.getParameter(ts + "offset");
        int S = (int)std::round(pS->convertFrom0to1(pS->getValue()));
        int P = (int)std::round(pP->convertFrom0to1(pP->getValue()));
        int O = (int)std::round(pO->convertFrom0to1(pO->getValue()));
        int roll = juce::Random::getSystemRandom().nextInt(3);
        int dir = juce::Random::getSystemRandom().nextBool() ? 1 : -1;
        if (roll == 0) { S = juce::jlimit(2, 32, S + dir); if (P > S) P = S; }
        else if (roll == 1) { P = juce::jlimit(0, S, P + dir); }
        else { O = (O + dir + S) % S; }
        pS->setValueNotifyingHost(pS->convertTo0to1((float)S));
        pP->setValueNotifyingHost(pP->convertTo0to1((float)P));
        pO->setValueNotifyingHost(pO->convertTo0to1((float)O));
    };

    auto rhythmSld = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 17);
        s.setColour(juce::Slider::trackColourId, juce::Colours::cyan);
        addChildComponent(s);
    };
    lbl(stepsLbl, "STEPS", juce::Colour(160, 175, 190));
    lbl(pulsesLbl, "PULSES", juce::Colour(160, 175, 190));
    lbl(offsetLbl, "OFFSET", juce::Colour(160, 175, 190));
    lbl(rateLbl, "RATE", juce::Colours::cyan);

    for (int t = 0; t < 6; ++t) {
        rhythmSld(stepsSld[t]); stepsSld[t].setTooltip("Pattern cycle length (2 to 32 steps).");
        rhythmSld(pulsesSld[t]); pulsesSld[t].setTooltip("Active Euclidean hits (0 to 32).");
        rhythmSld(offsetSld[t]); offsetSld[t].setTooltip("Pattern rotation offset (0 to 31).");
        rateCombo[t].addItemList({"1/4", "1/8", "1/16", "1/32"}, 1);
        rateCombo[t].setTooltip("Track clock divider (independent polymetry).");
        addChildComponent(rateCombo[t]);
    }

    lbl(mode281Lbl, "281:", juce::Colour(110, 210, 130));
    lbl(mode292Lbl, "292:", juce::Colours::cyan);
    btn(sRandBtn, juce::Colour(44, 24, 58), juce::Colour(210, 150, 255));
    sRandBtn.setTooltip("Randomize synth voice timbre controls.");
    sRandBtn.onClick = [this, setParam]() {
        juce::String ts = "t" + juce::String(currentTrack + 1) + "_";
        static const char* sIds[] = {"drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise"};
        for (auto id : sIds) setParam(ts + id, juce::Random::getSystemRandom().nextFloat());
    };

    btn(strikeBtn, juce::Colour(62, 42, 22), juce::Colours::yellow);
    strikeBtn.setTooltip("Manually ping/strike the LPG vactrol circuit (Enter).");
    strikeBtn.onClick = [this]() { audioProcessor.pendingStrike[(size_t)currentTrack].store(true, std::memory_order_relaxed); };

    btn(copyLastBtn, juce::Colour(28, 34, 42), juce::Colour(200, 215, 230));
    copyLastBtn.setTooltip("Copy synth parameters to the previous Track.");
    copyLastBtn.onClick = [this, setParam]() {
        int target = (currentTrack + 5) % 6;
        static const char* ids[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol","mode281","mode292","notemode"};
        for (auto id : ids) setParam("t" + juce::String(target + 1) + "_" + id, audioProcessor.apvts.getRawParameterValue("t" + juce::String(currentTrack + 1) + "_" + id)->load());
    };

    btn(copyNextBtn, juce::Colour(28, 34, 42), juce::Colour(200, 215, 230));
    copyNextBtn.setTooltip("Copy synth parameters to the next Track.");
    copyNextBtn.onClick = [this, setParam]() {
        int target = (currentTrack + 1) % 6;
        static const char* ids[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol","mode281","mode292","notemode"};
        for (auto id : ids) setParam("t" + juce::String(target + 1) + "_" + id, audioProcessor.apvts.getRawParameterValue("t" + juce::String(currentTrack + 1) + "_" + id)->load());
    };

    static const char* sNames[] = {"PITCH","P.DRP","MRPH","FOLD","FM","RISE","FALL","RESP","BRGT","RESO","NOISE","VOL"};
    static const juce::Colour sCols[] = {
        juce::Colour(235, 200, 90), juce::Colour(235, 200, 90), juce::Colour(235, 200, 90), juce::Colour(235, 200, 90), juce::Colour(235, 200, 90),
        juce::Colour(90, 220, 120), juce::Colour(90, 220, 120),
        juce::Colours::cyan, juce::Colours::cyan, juce::Colour(255, 155, 60),
        juce::Colour(180, 190, 200), juce::Colours::white
    };
    static const char* sTips[] = {
        "Base pitch.", "Pitch drop envelope.", "Triangle-to-Square morph.", "ADAA Wavefolder saturation.", "Auto-FM feedback.",
        "Envelope attack time.", "Envelope decay time.", "Vactrol optical decay inertia.", "LPG/VCF cutoff brightness.",
        "Filter resonance Q.", "White noise strike mix.", "Voice output level."
    };
    static const char* sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol"};

    for (int t = 0; t < 6; ++t) {
        juce::String ts = "t" + juce::String(t + 1) + "_";
        mode281Combo[t].addItemList({"TRANS", "SUST", "CYCLE"}, 1);
        mode281Combo[t].setTooltip("Buchla 281 Envelope: TRANS (Percussive), SUST (Gate sustain), CYCLE (LFO loop).");
        addChildComponent(mode281Combo[t]);

        mode292Combo[t].addItemList({"VCA", "LPG", "VCF"}, 1);
        mode292Combo[t].setTooltip("Buchla 292 Output: VCA (Clean), LPG (Optical vactrol), VCF (Filter).");
        addChildComponent(mode292Combo[t]);

        noteBtns[t].setClickingTogglesState(true);
        noteBtns[t].setColour(juce::TextButton::buttonColourId, juce::Colour(28, 34, 42));
        noteBtns[t].setColour(juce::TextButton::buttonOnColourId, juce::Colour(20, 52, 72));
        noteBtns[t].setColour(juce::TextButton::textColourOnId, juce::Colours::cyan);
        noteBtns[t].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        noteBtns[t].setTooltip("Switch between scale-quantized MIDI notes and free Hz tuning.");
        nAtt[t] = std::make_unique<BtnAtt>(audioProcessor.apvts, ts + "notemode", noteBtns[t]);

        rAtt[t][0] = std::make_unique<SldAtt>(audioProcessor.apvts, ts + "steps", stepsSld[t]);
        rAtt[t][1] = std::make_unique<SldAtt>(audioProcessor.apvts, ts + "pulses", pulsesSld[t]);
        rAtt[t][2] = std::make_unique<SldAtt>(audioProcessor.apvts, ts + "offset", offsetSld[t]);
        rRateAtt[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts + "rate", rateCombo[t]);
        m281Att[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts + "mode281", mode281Combo[t]);
        m292Att[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts + "mode292", mode292Combo[t]);

        for (int i = 0; i < 12; ++i) {
            vSliders[t][i].slider.setSliderStyle(juce::Slider::LinearVertical);
            vSliders[t][i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 14);
            vSliders[t][i].slider.setColour(juce::Slider::trackColourId, sCols[i]);
            vSliders[t][i].slider.setTooltip(sTips[i]);
            addChildComponent(vSliders[t][i].slider);

            vSliders[t][i].label.setText(sNames[i], juce::dontSendNotification);
            vSliders[t][i].label.setFont(juce::FontOptions(9.0f, juce::Font::bold));
            vSliders[t][i].label.setColour(juce::Label::textColourId, sCols[i]);
            vSliders[t][i].label.setJustificationType(juce::Justification::centred);
            addChildComponent(vSliders[t][i].label);

            sAtt[t][i] = std::make_unique<SldAtt>(audioProcessor.apvts, ts + sIds[i], vSliders[t][i].slider);
            if (i == 0) {
                vSliders[t][i].slider.textFromValueFunction = [this, t](double value) {
                    if (noteBtns[t].getToggleState()) {
                        int scale = (int)audioProcessor.apvts.getRawParameterValue("global_scale")->load();
                        int root = (int)audioProcessor.apvts.getRawParameterValue("global_root")->load();
                        int q_midi = (int)OrbitaDSP::quantizePitch((float)value, scale, root);
                        static const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
                        return juce::String(notes[((q_midi % 12) + 12) % 12]) + juce::String((q_midi / 12) - 1);
                    }
                    return juce::String(440.0f * std::pow(2.0f, ((float)value - 69.0f) / 12.0f), 1) + " Hz";
                };
            }
        }

        auto updateButtonState = [this, t]() {
            bool noteMode = noteBtns[t].getToggleState();
            noteBtns[t].setButtonText(noteMode ? "NOTE" : "Hz");
            auto& pitchSld = vSliders[t][0].slider;
            pitchSld.setRange(24.0, 96.0, noteMode ? 1.0 : 0.01);
            pitchSld.updateText();
        };
        noteBtns[t].onClick = updateButtonState;
        updateButtonState();
        addChildComponent(noteBtns[t]);
    }

    echoSyncBtn.setClickingTogglesState(true);
    eSyncAtt = std::make_unique<BtnAtt>(audioProcessor.apvts, "echo_sync", echoSyncBtn);
    echoSyncBtn.setTooltip("Sync Delay time to musical tempo (1/32, 1/16, 1/8, 1/8D, 1/4).");
    addAndMakeVisible(echoSyncBtn);

    static const char* eNames[] = {"TIME","FDBK","MIX","WOW"};
    static const char* eTips[] = {"Delay time spacing (tape pitch-glide).", "Tape feedback regeneration.", "Wet/Dry mix.", "Stereo tape wow & flutter."};
    static const char* eIds[] = {"echo_time","echo_fdbk","echo_mix","echo_wow"};
    for (int i = 0; i < 4; ++i) {
        echoKnobs[i].slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        echoKnobs[i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
        echoKnobs[i].slider.setColour(juce::Slider::thumbColourId, juce::Colour(65, 229, 155));
        echoKnobs[i].slider.setTooltip(eTips[i]);
        addAndMakeVisible(echoKnobs[i].slider);
        lbl(echoKnobs[i].label, eNames[i], juce::Colour(160, 235, 195));
        eAtt[i] = std::make_unique<SldAtt>(audioProcessor.apvts, eIds[i], echoKnobs[i].slider);
        if (i == 0) {
            echoKnobs[i].slider.textFromValueFunction = [this](double value) {
                if (echoSyncBtn.getToggleState()) {
                    if (value < 0.2) return juce::String("1/32");
                    if (value < 0.4) return juce::String("1/16");
                    if (value < 0.6) return juce::String("1/8");
                    if (value < 0.8) return juce::String("1/8D");
                    return juce::String("1/4");
                }
                return juce::String(value * 1000.0, 0) + " ms";
            };
        }
    }

    echoSyncBtn.onClick = [this]() {
        bool sync = echoSyncBtn.getToggleState();
        echoSyncBtn.setButtonText(sync ? "SYNC: ON" : "SYNC: OFF");
        echoSyncBtn.setColour(juce::TextButton::buttonColourId, sync ? juce::Colour(20, 70, 50) : juce::Colour(22, 34, 32));
        echoSyncBtn.setColour(juce::TextButton::textColourOffId, sync ? juce::Colour(110, 255, 185) : juce::Colour(160, 175, 185));
        echoKnobs[0].slider.updateText();
    };
    echoSyncBtn.onClick();

    logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);

    aboutOverlay.onClose = [this]() { aboutOverlay.setVisible(false); };
    addChildComponent(aboutOverlay);

    setResizable(true, true);
    setResizeLimits(860, 470, 1440, 820);
    setSize(980, 530);
    selectTrack(0);
    startTimerHz(30);
}

OrbitaLPGAudioProcessorEditor::~OrbitaLPGAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {
    currentTrack = juce::jlimit(0, 5, t);
    for (int i = 0; i < 6; ++i) {
        bool sel = (i == currentTrack);
        bool mut = audioProcessor.trackMutes[(size_t)i].load(std::memory_order_relaxed);
        tBtns[i].setColour(juce::TextButton::buttonColourId, sel ? juce::Colour(54, 58, 22) : juce::Colour(22, 28, 36));
        tBtns[i].setColour(juce::TextButton::textColourOffId, mut ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));

        stepsSld[i].setVisible(sel);
        pulsesSld[i].setVisible(sel);
        offsetSld[i].setVisible(sel);
        rateCombo[i].setVisible(sel);
        noteBtns[i].setVisible(sel);
        mode281Combo[i].setVisible(sel);
        mode292Combo[i].setVisible(sel);
        synthPresetCombo[i].setVisible(sel);

        for (int j = 0; j < 12; ++j) {
            vSliders[i][j].slider.setVisible(sel);
            vSliders[i][j].label.setVisible(sel);
        }
    }
    repaint(radarPureArea);
}

void OrbitaLPGAudioProcessorEditor::toggleMute(int t) {
    bool next = !audioProcessor.trackMutes[(size_t)t].load(std::memory_order_relaxed);
    audioProcessor.trackMutes[(size_t)t].store(next, std::memory_order_relaxed);
    bool sel = (t == currentTrack);
    tBtns[t].setColour(juce::TextButton::textColourOffId, next ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));
    repaint(radarPureArea);
}

bool OrbitaLPGAudioProcessorEditor::keyPressed(const juce::KeyPress& key) {
    if (key.isKeyCode(juce::KeyPress::spaceKey)) { playBtn.triggerClick(); return true; }
    if (key.isKeyCode(juce::KeyPress::returnKey)) { strikeBtn.triggerClick(); return true; }
    if (key.isKeyCode(juce::KeyPress::escapeKey) && aboutOverlay.isVisible()) { aboutOverlay.setVisible(false); return true; }
    auto ch = key.getTextCharacter();
    if (ch >= '1' && ch <= '6') { selectTrack((int)(ch - '1')); return true; }
    if (ch == 'm' || ch == 'M') { toggleMute(currentTrack); return true; }
    return false;
}

void OrbitaLPGAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    if (radarPureArea.contains(e.getPosition())) {
        auto center = radarPureArea.getCentre().toFloat();
        float maxR = std::min(radarPureArea.getWidth(), radarPureArea.getHeight()) * 0.44f;
        float dist = e.position.getDistanceFrom(center);
        if (dist <= maxR * 1.05f) {
            int ring = juce::jlimit(0, 5, (int)std::round(dist / (maxR / 6.8f)) - 1);
            if (e.mods.isRightButtonDown()) toggleMute(ring);
            else selectTrack(ring);
        }
    }
}

void OrbitaLPGAudioProcessorEditor::timerCallback() {
    bool playingNow = audioProcessor.isPlaying.load(std::memory_order_relaxed) || audioProcessor.hostPlaying.load(std::memory_order_relaxed);
    juce::String targetText = playingNow ? "PAUSE" : "PLAY";
    if (playBtn.getButtonText() != targetText) {
        playBtn.setButtonText(targetText);
        playBtn.setColour(juce::TextButton::buttonColourId, playingNow ? juce::Colour(105, 80, 26) : juce::Colour(30, 82, 56));
    }
    repaint(radarPureArea);
    repaint(meterArea);
}

void OrbitaLPGAudioProcessorEditor::resized() {
    auto b = getLocalBounds().reduced(12);
    topArea = b.removeFromTop(26);
    b.removeFromTop(8);

    int leftW = juce::jlimit(360, 520, (int)(b.getWidth() * 0.43f));
    radarArea = b.removeFromLeft(leftW);
    b.removeFromLeft(12);

    auto rightCol = b;
    masterArea = rightCol.removeFromTop(74);
    rightCol.removeFromTop(8);
    int echoH = 98;
    echoArea = rightCol.removeFromBottom(echoH);
    rightCol.removeFromBottom(8);
    synthArea = rightCol;

    {
        auto r = topArea;
        titleLabel.setBounds(r.removeFromLeft(195));
        loadBtn.setBounds(r.removeFromLeft(46).reduced(0, 2)); r.removeFromLeft(3);
        saveBtn.setBounds(r.removeFromLeft(46).reduced(0, 2)); r.removeFromLeft(6);
        kitCombo.setBounds(r.removeFromLeft(158).reduced(0, 2)); r.removeFromLeft(10);
        playBtn.setBounds(r.removeFromLeft(54).reduced(0, 2)); r.removeFromLeft(4);
        stopBtn.setBounds(r.removeFromLeft(50).reduced(0, 2)); r.removeFromLeft(4);
        seqBtn.setBounds(r.removeFromLeft(66).reduced(0, 2));
        aboutBtn.setBounds(r.removeFromRight(66).reduced(0, 2));
    }

    {
        auto ra = radarArea;
        auto controlZone = ra.removeFromBottom(122);
        radarPureArea = ra;
        controlZone.reduce(8, 6);

        auto rowA = controlZone.removeFromTop(22);
        for (int i = 0; i < 6; ++i) tBtns[i].setBounds(rowA.removeFromLeft(30).reduced(1, 0));
        rowA.removeFromLeft(6);
        rMutateBtn.setBounds(rowA.removeFromRight(64).reduced(0, 1));
        rowA.removeFromRight(5);
        patternsCombo.setBounds(rowA.reduced(0, 1));
        controlZone.removeFromTop(6);

        auto placeRhythm = [&](juce::Label& l, juce::Slider* sArr, juce::ComboBox* cArr) {
            auto row = controlZone.removeFromTop(19);
            l.setBounds(row.removeFromLeft(46));
            row.removeFromLeft(4);
            if (sArr) { for (int i = 0; i < 6; ++i) sArr[i].setBounds(row); }
            else { for (int i = 0; i < 6; ++i) cArr[i].setBounds(row); }
            controlZone.removeFromTop(3);
        };
        placeRhythm(stepsLbl, stepsSld, nullptr);
        placeRhythm(pulsesLbl, pulsesSld, nullptr);
        placeRhythm(offsetLbl, offsetSld, nullptr);
        placeRhythm(rateLbl, nullptr, rateCombo);
    }

    {
        auto mi = masterArea.reduced(10, 6);
        mi.removeFromTop(14);
        meterArea = mi.removeFromRight(22).reduced(2, 2);
        mi.removeFromRight(6);

        auto row1 = mi.removeFromTop(12);
        mi.removeFromTop(3);
        auto row2 = mi.removeFromTop(22);

        int unitW = (mi.getWidth() - 165) / 5;
        auto place = [&](juce::Label& l, juce::Slider& s) {
            l.setBounds(row1.removeFromLeft(unitW));
            s.setBounds(row2.removeFromLeft(unitW).reduced(2, 0));
            row1.removeFromLeft(4); row2.removeFromLeft(4);
        };
        place(mVolLbl, mVolSld);
        place(mDriveLbl, mDriveSld);
        place(mBpmLbl, mBpmSld);
        place(mSwingLbl, mSwingSld);
        place(mChaosLbl, mChaosSld);

        int scaleW = 92;
        mScaleLbl.setBounds(row1.removeFromLeft(scaleW));
        globalScaleCombo.setBounds(row2.removeFromLeft(scaleW).reduced(2, 0));
        row1.removeFromLeft(4); row2.removeFromLeft(4);
        mRootLbl.setBounds(row1);
        globalRootCombo.setBounds(row2.reduced(2, 0));
    }

    {
        auto si = synthArea.reduced(10, 8);
        si.removeFromTop(15);
        auto sRow1 = si.removeFromTop(21);
        auto noteRect = sRow1.removeFromRight(60).reduced(0, 1);
        sRow1.removeFromRight(4);
        copyNextBtn.setBounds(sRow1.removeFromRight(52).reduced(0, 1));
        sRow1.removeFromRight(4);
        copyLastBtn.setBounds(sRow1.removeFromRight(52).reduced(0, 1));
        sRow1.removeFromRight(6);

        auto presetRect = sRow1.removeFromLeft(160).reduced(0, 1);
        sRow1.removeFromLeft(5);
        sRandBtn.setBounds(sRow1.removeFromLeft(52).reduced(0, 1));
        for (int t = 0; t < 6; ++t) {
            noteBtns[t].setBounds(noteRect);
            synthPresetCombo[t].setBounds(presetRect);
        }

        si.removeFromTop(5);
        auto sRow2 = si.removeFromTop(21);
        mode281Lbl.setBounds(sRow2.removeFromLeft(30));
        auto r281 = sRow2.removeFromLeft(74).reduced(0, 1);
        sRow2.removeFromLeft(10);
        mode292Lbl.setBounds(sRow2.removeFromLeft(30));
        auto r292 = sRow2.removeFromLeft(74).reduced(0, 1);
        sRow2.removeFromLeft(10);
        strikeBtn.setBounds(sRow2.removeFromLeft(66).reduced(0, 1));
        for (int t = 0; t < 6; ++t) {
            mode281Combo[t].setBounds(r281);
            mode292Combo[t].setBounds(r292);
        }

        si.removeFromTop(16);
        synthSliderGridArea = si;
        int sw = si.getWidth() / 12;
        for (int i = 0; i < 12; ++i) {
            auto cell = si.removeFromLeft(sw);
            auto labelBounds = cell.removeFromTop(13);
            cell.removeFromTop(2);
            for (int t = 0; t < 6; ++t) {
                vSliders[t][i].label.setBounds(labelBounds);
                vSliders[t][i].slider.setBounds(cell);
            }
        }
    }

    {
        auto ei = echoArea.reduced(10, 6);
        ei.removeFromTop(15);
        echoSyncBtn.setBounds(ei.removeFromLeft(84).withSizeKeepingCentre(74, 22));
        ei.removeFromLeft(8);
        int kw = ei.getWidth() / 4;
        for (int i = 0; i < 4; ++i) {
            auto cell = ei.removeFromLeft(kw);
            echoKnobs[i].label.setBounds(cell.removeFromTop(12));
            echoKnobs[i].slider.setBounds(cell.withSizeKeepingCentre(std::min(cell.getWidth(), 54), std::min(cell.getHeight(), 54)));
        }
    }

    creditLabel.setBounds(getWidth() - 250, getHeight() - 15, 240, 13);
    aboutOverlay.setBounds(getLocalBounds());
}

void OrbitaLPGAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(11, 14, 19));

    auto drawModule = [&](juce::Rectangle<int> r, juce::Colour bg, juce::Colour border) {
        g.setColour(bg);
        g.fillRoundedRectangle(r.toFloat(), 6.0f);
        g.setColour(border);
        g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.2f);
    };

    drawModule(radarArea, juce::Colour(10, 13, 18), juce::Colour(38, 50, 64));
    drawModule(masterArea, juce::Colour(17, 21, 27), juce::Colour(48, 60, 74));
    drawModule(synthArea, juce::Colour(15, 19, 25), juce::Colour(48, 56, 68));
    drawModule(echoArea, juce::Colour(12, 18, 20), juce::Colour(34, 62, 50));

    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(130, 148, 168));
    g.drawText("MASTER SYSTEM", masterArea.withTrimmedLeft(10).withTrimmedTop(5).withHeight(14), juce::Justification::topLeft);
    g.setColour(juce::Colour(65, 229, 155));
    g.drawText("RE-201 SPACE ECHO", echoArea.withTrimmedLeft(10).withTrimmedTop(5).withHeight(14), juce::Justification::topLeft);
    g.setColour(juce::Colour(235, 180, 70));
    g.drawText("WEST COAST SYNTHESIS  259 / 281 / 292", synthArea.withTrimmedLeft(10).withTrimmedTop(5).withHeight(14), juce::Justification::topLeft);

    if (!synthSliderGridArea.isEmpty()) {
        int sw = synthSliderGridArea.getWidth() / 12;
        int x0 = synthSliderGridArea.getX();
        int yHdr = synthSliderGridArea.getY() - 13;
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        auto drawGroup = [&](int startCol, int count, const char* name, juce::Colour col) {
            int gx = x0 + startCol * sw;
            int gw = count * sw;
            g.setColour(col.withAlpha(0.65f));
            g.drawText(name, gx, yHdr, gw, 11, juce::Justification::centred);
            if (startCol > 0) {
                g.setColour(juce::Colour(40, 48, 58));
                g.drawVerticalLine(gx, (float)yHdr + 2.0f, (float)synthSliderGridArea.getBottom() - 4.0f);
            }
        };
        drawGroup(0, 5, "OSC 259", juce::Colour(235, 200, 90));
        drawGroup(5, 2, "ENV 281", juce::Colour(90, 220, 120));
        drawGroup(7, 3, "LPG 292", juce::Colours::cyan);
        drawGroup(10, 2, "MIX", juce::Colours::white);
    }

    if (!meterArea.isEmpty()) {
        auto mL = meterArea.withWidth(meterArea.getWidth() / 2 - 1);
        auto mR = meterArea.withTrimmedLeft(meterArea.getWidth() / 2 + 1);
        auto drawBar = [&](juce::Rectangle<int> bar, float level) {
            g.setColour(juce::Colour(10, 13, 18));
            g.fillRoundedRectangle(bar.toFloat(), 2.0f);
            float clamped = juce::jlimit(0.0f, 1.0f, level);
            int fillH = (int)(bar.getHeight() * clamped);
            if (fillH > 0) {
                auto fillR = bar.withTop(bar.getBottom() - fillH).toFloat();
                juce::Colour c = clamped > 0.9f ? juce::Colour(255, 80, 80) : (clamped > 0.65f ? juce::Colour(245, 190, 50) : juce::Colour(65, 229, 155));
                g.setColour(c);
                g.fillRoundedRectangle(fillR, 2.0f);
            }
        };
        drawBar(mL, audioProcessor.peakL.load(std::memory_order_relaxed));
        drawBar(mR, audioProcessor.peakR.load(std::memory_order_relaxed));
    }

    auto center = radarPureArea.getCentre().toFloat();
    float maxR = std::min(radarPureArea.getWidth(), radarPureArea.getHeight()) * 0.44f;
    g.setColour(juce::Colour(7, 9, 13));
    g.fillEllipse(center.x - maxR, center.y - maxR, maxR * 2.0f, maxR * 2.0f);
    g.setColour(juce::Colour(26, 34, 44));
    g.drawLine(center.x - maxR, center.y, center.x + maxR, center.y);
    g.drawLine(center.x, center.y - maxR, center.x, center.y + maxR);

    if (logoImage.isValid()) {
        float logoSize = maxR * 1.9f;
        juce::Rectangle<float> logoBounds(center.x - logoSize * 0.5f, center.y - logoSize * 0.5f, logoSize, logoSize);
        g.setOpacity(0.24f);
        g.drawImageWithin(logoImage, (int)logoBounds.getX(), (int)logoBounds.getY(), (int)logoBounds.getWidth(), (int)logoBounds.getHeight(),
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, false);
        g.setOpacity(1.0f);
    }

    for (int t = 0; t < 6; ++t) {
        float r = (t + 1) * (maxR / 6.8f);
        bool sel = (t == currentTrack);
        bool mut = audioProcessor.trackMutes[(size_t)t].load(std::memory_order_relaxed);
        float glow = mut ? 0.0f : audioProcessor.voices[t].visual_lpg.load(std::memory_order_relaxed);

        juce::Colour baseCol = mut ? juce::Colour(52, 58, 66) : (sel ? juce::Colours::yellow : juce::Colours::cyan);
        g.setColour(baseCol.withAlpha(mut ? 0.25f : (sel ? 0.35f + glow * 0.45f : 0.16f + glow * 0.4f)));
        g.drawEllipse(center.x - r, center.y - r, r * 2.0f, r * 2.0f, sel ? 1.4f + glow * 1.2f : 1.0f + glow * 0.8f);

        int steps = std::max(1, (int)audioProcessor.tParams[t].steps->load(std::memory_order_relaxed));
        int pulses = (int)audioProcessor.tParams[t].pulses->load(std::memory_order_relaxed);
        int offset = (int)audioProcessor.tParams[t].offset->load(std::memory_order_relaxed);

        juce::Path poly;
        bool first = true;
        for (int i = 0; i < steps; ++i) {
            float angle = ((float)i / (float)steps) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float px = center.x + std::cos(angle) * r;
            float py = center.y + std::sin(angle) * r;

            if (OrbitaDSP::isEuclideanHit(i, pulses, steps, offset)) {
                if (first) { poly.startNewSubPath(px, py); first = false; }
                else { poly.lineTo(px, py); }
                if (glow > 0.05f) {
                    g.setColour(baseCol.withAlpha(glow * 0.35f));
                    g.fillEllipse(px - 6.0f, py - 6.0f, 12.0f, 12.0f);
                }
                g.setColour(baseCol);
                g.fillEllipse(px - 3.5f, py - 3.5f, 7.0f, 7.0f);
            } else if (sel) {
                g.setColour(juce::Colour(60, 72, 86));
                g.fillEllipse(px - 1.5f, py - 1.5f, 3.0f, 3.0f);
            }
        }

        if (!first) {
            poly.closeSubPath();
            g.setColour(baseCol.withAlpha(mut ? 0.2f : (0.5f + glow * 0.4f)));
            g.strokePath(poly, juce::PathStrokeType(sel ? 1.8f : 1.3f));
        }

        int cur = audioProcessor.voices[t].current_step.load(std::memory_order_relaxed);
        if (cur >= 0) {
            int wrapped = cur % steps;
            float pAngle = ((float)wrapped / (float)steps) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float px = center.x + std::cos(pAngle) * r;
            float py = center.y + std::sin(pAngle) * r;
            g.setColour(juce::Colours::white.withAlpha(0.85f));
            g.drawEllipse(px - 5.0f, py - 5.0f, 10.0f, 10.0f, 1.8f);
        }
    }
}
