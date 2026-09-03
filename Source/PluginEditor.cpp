#include "PluginProcessor.h"
#include "PluginEditor.h"

static std::vector<int> gen_euclid(int pulses, int steps, int offset) {
    std::vector<int> pat(steps, 0); if (steps == 0) return pat;
    if (pulses >= steps) { std::fill(pat.begin(), pat.end(), 1); return pat; }
    int bucket = 0; for (int i = 0; i < steps; ++i) {
        bucket += pulses; if (bucket >= steps) { bucket -= steps; pat[(i + offset) % steps] = 1; }
    } return pat;
}

static const std::vector<std::vector<int>> SCALES_UI = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0, 2, 4, 5, 7, 9, 11}, {0, 2, 3, 5, 7, 8, 10}, {0, 2, 3, 5, 7, 9, 10}, 
    {0, 1, 3, 5, 7, 8, 10}, {0, 2, 4, 6, 7, 9, 11}, {0, 2, 4, 5, 7, 9, 10}, {0, 2, 4, 7, 9}, {0, 3, 5, 7, 10}, 
    {0, 2, 3, 5, 7, 8, 11}, {0, 1, 4, 5, 7, 8, 10}, {0, 2, 3, 7, 8}, {0, 2, 4, 6, 8, 10}, {0, 1, 3, 4, 6, 7, 9, 10}
};

static float quantize_pitch_ui(float raw_midi, int scale_idx, int root_idx) {
    if (scale_idx < 1) scale_idx = 1; if (scale_idx > 14) scale_idx = 14;
    const auto& scale = SCALES_UI[scale_idx - 1];
    int shifted = (int)std::round(raw_midi) - root_idx;
    int octave = (int)std::floor(shifted / 12.0f); int note = (shifted % 12 + 12) % 12;
    int closest_note = scale[0]; int min_dist = 100;
    for (int s : scale) { int dist = std::abs(note - s); if (dist < min_dist) { min_dist = dist; closest_note = s; } }
    return (float)(octave * 12 + closest_note + root_idx);
}

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
    { "00. Init / Empty", 120.0f, 0.0f, 0.0f, 1, 0,  0.3f, 0.0f, 0.0f, 0.0f, 1, {
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0},
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0},
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0},
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0},
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0},
        {16, 1, 0, 2, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.5f, 0.5f, 0.8f, 0.0f, 0.0f, 0.8f, 1, 0, 0} }
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
    {"Init / Default", {60,0,0,0,0, 0.01,0.5,0.5,0.8,0.1,0,0.8}, 0, 1},
    {"Buchla Bongo", {65,0.3,0,0.4,0.1, 0.001,0.2,0.1,0.6,0.4,0,0.9}, 0, 1},
    {"808 Sub Kick", {36,0.9,0,0,0, 0.001,0.8,0.8,0.3,0.1,0,0.9}, 0, 1},
    {"909 Hard Kick", {36,0.7,0.2,0.5,0.2, 0.001,0.6,0.9,0.5,0.2,0.3,0.9}, 0, 1},
    {"Tribal Zap", {72,0.8,0,0.6,0.3, 0.001,0.1,1.0,0.9,0.6,0,0.8}, 0, 1},
    {"Guaracha Pluck", {65,0.1,0.5,0.3,0, 0.01,0.3,0.8,1.0,0.5,0,0.8}, 0, 1},
    {"Cumbia Bass", {36,0,0,0,0, 0.05,0.4,0.2,0.4,0.2,0,0.9}, 1, 2},
    {"FM Snare", {60,0.2,1.0,0.8,0.9, 0.001,0.2,0.9,1.0,0.4,0.6,0.8}, 0, 0},
    {"White Noise Hat", {80,0,0,0,0, 0.001,0.05,1.0,1.0,0.0,1.0,0.7}, 0, 0},
    {"Open Hat Metal", {75,0,0.5,0.9,0.7, 0.01,0.4,0.9,0.8,0.3,0.3,0.7}, 0, 1},
    {"Wooden Plonk", {70,0.2,0,0.1,0, 0.001,0.15,0.05,0.4,0.5,0,0.9}, 0, 1},
    {"Dark Drone (Hold)", {48,0,0,0.4,0.1, 0.5,1.5,0.1,0.3,0.2,0,0.8}, 1, 2},
    {"Space Bell", {84,0,0,0.8,0.6, 0.01,1.5,0.6,0.9,0.7,0,0.7}, 0, 1},
    {"Acid Bassline", {40,0.1,0.5,0.8,0, 0.01,0.3,0.8,0.4,0.8,0,0.9}, 0, 1},
    {"IDM Glitch", {72,0.9,1.0,0.9,0.9, 0.001,0.05,1.0,1.0,0.5,0.1,0.8}, 0, 0},
    {"LFO Cycle Sweep", {48,0,0,0.8,0.5, 0.5,0.5,0.5,0.7,0.3,0,0.8}, 2, 1},
    {"FM Tom", {45,0.6,0,0.3,0.4, 0.001,0.4,0.7,0.5,0.2,0,0.9}, 0, 1},
    {"Cinematic Brass", {48,0,0.8,0.5,0.2, 0.3,1.0,0.2,0.7,0.4,0,0.8}, 1, 2},
    {"Glass Marimba", {76,0,0,0.2,0.3, 0.001,0.3,0.15,0.9,0.5,0,0.8}, 0, 1},
    {"Industrial Crash", {50,0.3,1.0,1.0,0.8, 0.01,1.2,0.9,1.0,0.6,0.8,0.8}, 0, 0},
    {"Ghost Choir", {60,0,0,0.2,0, 0.8,2.0,0.1,0.4,0.2,0.2,0.7}, 1, 2},
    {"Laser Gun", {80,1.0,0.5,0.4,0.1, 0.001,0.2,0.9,0.8,0.3,0,0.8}, 0, 1},
    {"Water Drop", {75,0.4,0,0.1,0.1, 0.001,0.1,0.2,0.7,0.6,0,0.9}, 0, 1},
    {"VCA Sub", {36,0,0,0,0, 0.01,0.5,0.5,1.0,0.1,0,0.9}, 0, 0},
    {"Morphing Lead", {60,0,0.5,0.6,0.2, 0.1,0.5,0.8,0.9,0.4,0,0.8}, 1, 1}
};

OrbitaLPGAudioProcessorEditor::OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel); setWantsKeyboardFocus(true);

    auto setParam = [&](const juce::String& id, float val) {
        if (auto* param = audioProcessor.apvts.getParameter(id)) param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    auto lbl = [this](juce::Label& l, const char* txt, juce::Colour col = juce::Colour(160,170,180)) {
        l.setText(txt, juce::dontSendNotification); l.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, col); l.setJustificationType(juce::Justification::centred); addAndMakeVisible(l);
    };
    auto btn = [this](juce::TextButton& b, juce::Colour col = juce::Colour(30,35,40), juce::Colour textCol = juce::Colours::white) {
        b.setColour(juce::TextButton::buttonColourId, col); b.setColour(juce::TextButton::textColourOffId, textCol); addAndMakeVisible(b);
    };
    auto masterSld = [this](juce::Slider& s, juce::String suffix) {
        s.setSliderStyle(juce::Slider::LinearBar); s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 18);
        s.setColour(juce::Slider::trackColourId, juce::Colour(38, 45, 52)); s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setTextValueSuffix(suffix); addAndMakeVisible(s);
    };

    titleLabel.setText("ORBITA-LPG   ///   6-VOICE MATRIX", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold)); titleLabel.setColour(juce::Label::textColourId, juce::Colour(200,210,220)); addAndMakeVisible(titleLabel);

    creditLabel.setText("coded by @laurorobles / extasis records", juce::dontSendNotification);
    creditLabel.setFont(juce::FontOptions(9.0f)); creditLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.2f));
    creditLabel.setJustificationType(juce::Justification::bottomRight); addAndMakeVisible(creditLabel);

    addAndMakeVisible(loadBtn); addAndMakeVisible(saveBtn);
    loadBtn.setTooltip("Load a custom .xml preset"); saveBtn.setTooltip("Save current state to .xml preset");
    loadBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Load Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (fc.getResult().existsAsFile()) {
                    juce::XmlDocument xmlDoc(fc.getResult());
                    if (auto xml = xmlDoc.getDocumentElement()) { audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*xml)); }
                }
            });
    };
    saveBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Save Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (auto xml = audioProcessor.apvts.copyState().createXml()) { xml->writeTo(fc.getResult()); }
            });
    };
    addAndMakeVisible(kitCombo);
    for (int i = 0; i < MASTER_KITS.size(); ++i) kitCombo.addItem(MASTER_KITS[i].name, i + 1);
    kitCombo.setSelectedItemIndex(1, juce::dontSendNotification); 
    kitCombo.setTooltip("Load a Master Genre Kit (Overwrites all sequencer and synth parameters instantly).");
    kitCombo.onChange = [this, setParam]() {
        int idx = kitCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < MASTER_KITS.size()) {
            const auto& k = MASTER_KITS[idx];
            setParam("bpm", k.bpm); setParam("swing", k.swing); setParam("chaos", k.chaos);
            setParam("global_scale", k.scale); setParam("global_root", k.root);
            setParam("echo_time", k.e_time); setParam("echo_fdbk", k.e_fdbk); setParam("echo_mix", k.e_mix); setParam("echo_wow", k.e_wow); setParam("echo_sync", k.e_sync);
            for (int i = 0; i < 6; ++i) {
                juce::String ts = "t" + juce::String(i+1) + "_";
                const auto& tr = k.t[i];
                setParam(ts+"steps", tr.steps); setParam(ts+"pulses", tr.pulses); setParam(ts+"offset", tr.offset); setParam(ts+"rate", tr.rate);
                setParam(ts+"pitch", tr.pitch); setParam(ts+"drop", tr.drop); setParam(ts+"morph", tr.morph); setParam(ts+"fold", tr.fold); setParam(ts+"fm", tr.fm);
                setParam(ts+"rise", tr.rise); setParam(ts+"fall", tr.fall); setParam(ts+"resp", tr.resp); setParam(ts+"brgt", tr.brgt); 
                setParam(ts+"reso", tr.reso); setParam(ts+"noise", tr.noise); setParam(ts+"vol", tr.vol);
                setParam(ts+"notemode", tr.notemode); setParam(ts+"mode281", tr.mode281); setParam(ts+"mode292", tr.mode292);
                
                // Reseteamos el texto del menú de preset individual de cada canal cuando se carga un master kit
                synthPresetCombo[i].setSelectedItemIndex(0, juce::dontSendNotification);
            }
        }
    };

    btn(playBtn, juce::Colour(40,90,60), juce::Colours::lightgreen); 
    btn(stopBtn, juce::Colour(90,40,40), juce::Colours::white); 
    btn(seqBtn, juce::Colour(30,35,42), juce::Colours::cyan);
    btn(configBtn, juce::Colour(25,30,38), juce::Colours::cyan);
    
    playBtn.setTooltip("Toggle master transport clock."); 
    stopBtn.setTooltip("Stop transport and reset all sequences to step zero.");
    seqBtn.setTooltip("Enable/Disable Euclidean sequencer algorithm.");
    
    playBtn.onClick = [this]() {
        bool cur = audioProcessor.isPlaying; audioProcessor.isPlaying = !cur;
        playBtn.setButtonText(!cur ? "PAUSE" : "PLAY"); playBtn.setColour(juce::TextButton::buttonColourId, !cur ? juce::Colour(100,80,30) : juce::Colour(40,90,60));
    };
    
    stopBtn.onClick = [this]() {
        audioProcessor.isPlaying = false;
        playBtn.setButtonText("PLAY"); playBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(40,90,60));
        for (int i = 0; i < 6; ++i) {
            audioProcessor.track_samples_counter[i] = 0.0f;
            audioProcessor.voices[i].reset();
        }
    };
    
    seqBtn.onClick = [this]() {
        audioProcessor.seqEnabled = !audioProcessor.seqEnabled; seqBtn.setButtonText(audioProcessor.seqEnabled ? "SEQ: ON" : "SEQ: OFF");
    };

    lbl(mVolLbl, "M.VOL", juce::Colours::yellow); lbl(mDriveLbl, "DRIVE", juce::Colour(230,140,30));
    lbl(mBpmLbl, "BPM", juce::Colour(160,170,180)); lbl(mSwingLbl, "SWING", juce::Colour(160,170,180)); lbl(mChaosLbl, "CHAOS", juce::Colour(160,170,180));
    lbl(mScaleLbl, "G.SCALE", juce::Colours::cyan); lbl(mRootLbl, "G.ROOT", juce::Colours::cyan);

    masterSld(mVolSld, ""); mVolSld.setTooltip("Master output volume post-effects.");
    masterSld(mDriveSld, ""); mDriveSld.setTooltip("Global analog soft-clip saturation.");
    masterSld(mBpmSld, ""); mBpmSld.setTooltip("Internal tempo. Syncs to host DAW if transport is playing.");
    masterSld(mSwingSld, ""); mSwingSld.setTooltip("Rhythmic humanization groove.");
    masterSld(mChaosSld, ""); mChaosSld.setTooltip("West Coast instability injection (Xorshift noise).");

    addAndMakeVisible(globalScaleCombo); globalScaleCombo.addItemList({"Chromatic","Major","Minor","Dorian","Phrygian","Lydian","Mixolydian","Pent. Maj","Pent. Min","Harm. Min","Phryg. Dom","Hirajoshi","Whole Tone","Diminished"}, 1); globalScaleCombo.setTooltip("Global pitch quantization scale.");
    addAndMakeVisible(globalRootCombo); globalRootCombo.addItemList({"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}, 1); globalRootCombo.setTooltip("Global root note.");

    mAtt[0] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_vol", mVolSld); mAtt[1] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_drive", mDriveSld);
    mAtt[2] = std::make_unique<SldAtt>(audioProcessor.apvts, "bpm", mBpmSld); mAtt[3] = std::make_unique<SldAtt>(audioProcessor.apvts, "swing", mSwingSld);
    mAtt[4] = std::make_unique<SldAtt>(audioProcessor.apvts, "chaos", mChaosSld);
    gScaleAtt = std::make_unique<CmbAtt>(audioProcessor.apvts, "global_scale", globalScaleCombo); gRootAtt = std::make_unique<CmbAtt>(audioProcessor.apvts, "global_root", globalRootCombo);

    auto updatePitchTexts = [this]() { for (int trackIdx = 0; trackIdx < 6; ++trackIdx) vSliders[trackIdx][0].slider.updateText(); };
    globalScaleCombo.onChange = updatePitchTexts; globalRootCombo.onChange = updatePitchTexts;

    for (int t = 0; t < 6; ++t) {
        tBtns[t].setButtonText("T" + juce::String(t+1)); btn(tBtns[t], juce::Colour(22,28,35), juce::Colours::white);
        tBtns[t].setTooltip("Select Track for editing. Right-click to MUTE. External MIDI C1-F1 (Ch 1) for Drums, Ch 1-6 for Synths.");
        tBtns[t].onClick = [this, t]() { selectTrack(t); }; 
        tBtns[t].onRightClick = [this, t]() { toggleMute(t); };

        // INICIALIZACIÓN DE LOS 6 MENÚS DE PRESETS DE SÍNTESIS INDIVIDUALES
        synthPresetCombo[t].addItemList({"--- SYNTH PRESETS ---"}, 1);
        for (int i = 0; i < PRESETS.size(); ++i) synthPresetCombo[t].addItem(PRESETS[i].name, i + 2);
        synthPresetCombo[t].setSelectedItemIndex(0, juce::dontSendNotification);
        synthPresetCombo[t].setTooltip("Load a West Coast patch for Voice " + juce::String(t+1));
        
        synthPresetCombo[t].onChange = [this, t, setParam]() {
            int id = synthPresetCombo[t].getSelectedId() - 2;
            if (id >= 0 && id < PRESETS.size()) {
                juce::String ts = "t" + juce::String(t+1) + "_";
                juce::String sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol"};
                for (int i=0; i<12; i++) setParam(ts+sIds[i], PRESETS[id].p[i]);
                setParam(ts+"mode281", PRESETS[id].m281); setParam(ts+"mode292", PRESETS[id].m292);
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
    patternsCombo.setSelectedItemIndex(0);
    patternsCombo.setTooltip("Ethnomusicological rhythm library based on the Euclidean algorithm.");
    
    patternsCombo.onChange = [this]() {
        int id = patternsCombo.getSelectedId();
        auto stp = [&](int p, int s) {
            if (auto* sp = audioProcessor.apvts.getParameter("t"+juce::String(currentTrack+1)+"_steps")) sp->setValueNotifyingHost(sp->convertTo0to1(s));
            if (auto* pp = audioProcessor.apvts.getParameter("t"+juce::String(currentTrack+1)+"_pulses")) pp->setValueNotifyingHost(pp->convertTo0to1(p));
            if (auto* op = audioProcessor.apvts.getParameter("t"+juce::String(currentTrack+1)+"_offset")) op->setValueNotifyingHost(op->convertTo0to1(0));
        };
        if (id==3) stp(3,8); else if (id==4) stp(5,8); else if (id==5) stp(5,12); else if (id==6) stp(7,12);
        else if (id==7) stp(5,16); else if (id==8) stp(7,16); else if (id==9) stp(9,16); else if (id==10) stp(11,24);
        else if (id==12) stp(3,7); else if (id==13) stp(4,9); else if (id==14) stp(5,11); else if (id==15) stp(7,13);
        else if (id==17) stp(15,24); else if (id==18) stp(17,24);
        if (id > 1) patternsCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    };

    btn(rMutateBtn, juce::Colour(40,20,50), juce::Colours::violet);
    rMutateBtn.setTooltip("Organically mutates the current pattern (+/- 1 in Pulses, Steps or Offset).");
    rMutateBtn.onClick = [this]() {
        juce::String ts = "t" + juce::String(currentTrack+1) + "_";
        auto* pS = audioProcessor.apvts.getParameter(ts+"steps"); auto* pP = audioProcessor.apvts.getParameter(ts+"pulses"); auto* pO = audioProcessor.apvts.getParameter(ts+"offset");
        int S = std::round(pS->convertFrom0to1(pS->getValue())); int P = std::round(pP->convertFrom0to1(pP->getValue())); int O = std::round(pO->convertFrom0to1(pO->getValue()));
        int roll = juce::Random::getSystemRandom().nextInt(3); int dir = juce::Random::getSystemRandom().nextBool() ? 1 : -1;
        if (roll == 0) { S = juce::jlimit(2, 24, S + dir); if (P > S) P = S; } 
        else if (roll == 1) { P = juce::jlimit(1, S, P + dir); } 
        else { O = (O + dir + S) % S; }
        pS->setValueNotifyingHost(pS->convertTo0to1(S)); pP->setValueNotifyingHost(pP->convertTo0to1(P)); pO->setValueNotifyingHost(pO->convertTo0to1(O));
    };

    auto rhythmSld = [this](juce::Slider& s) { s.setSliderStyle(juce::Slider::LinearHorizontal); s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 28, 18); addAndMakeVisible(s); };
    lbl(stepsLbl, "STEPS", juce::Colour(160,170,180)); lbl(pulsesLbl, "PULSES", juce::Colour(160,170,180)); 
    lbl(offsetLbl, "OFFSET", juce::Colour(160,170,180)); lbl(rateLbl, "RATE", juce::Colours::cyan);
    
    for (int t = 0; t < 6; ++t) {
        rhythmSld(stepsSld[t]); stepsSld[t].setTooltip("Pattern cycle length (2 to 24 steps).");
        rhythmSld(pulsesSld[t]); pulsesSld[t].setTooltip("Number of distributed hits (K in E(k,n)).");
        rhythmSld(offsetSld[t]); offsetSld[t].setTooltip("Rotates starting point for rhythm feel variation (0 to 23).");
        rateCombo[t].addItemList({"1/4", "1/8", "1/16", "1/32"}, 1); addChildComponent(rateCombo[t]);
        rateCombo[t].setTooltip("Track temporal clock divider (Spatial polymetry).");
    }

    lbl(mode281Lbl, "281:", juce::Colour(100,110,120)); lbl(mode292Lbl, "292:", juce::Colour(100,110,120));
    btn(sRandBtn, juce::Colour(40,20,50), juce::Colours::violet); sRandBtn.setTooltip("Randomize synth voice controls.");
    sRandBtn.onClick = [this, setParam]() {
        juce::String ts = "t" + juce::String(currentTrack+1) + "_";
        juce::String sIds[] = {"drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise"};
        for(auto id : sIds) setParam(ts+id, juce::Random::getSystemRandom().nextFloat());
    };

    btn(strikeBtn, juce::Colour(50,30,30), juce::Colours::yellow); strikeBtn.setButtonText("STRIKE");
    strikeBtn.setTooltip("Manually ping/strike the LPG vactrol circuit to test timbre.");
    strikeBtn.onClick = [this]() {
        float beat_samples = (audioProcessor.getSampleRate() * 60.0f) / (audioProcessor.p_bpm ? audioProcessor.p_bpm->load() : 124.0f);
        float raw_p = audioProcessor.tParams[currentTrack].pitch->load();
        bool note_mode = audioProcessor.tParams[currentTrack].notemode->load() > 0.5f;
        int g_scale = audioProcessor.p_global_scale ? (int)audioProcessor.p_global_scale->load() : 1;
        int g_root = audioProcessor.p_global_root ? (int)audioProcessor.p_global_root->load() : 0;
        float q_pitch = note_mode ? quantize_pitch_ui(raw_p, g_scale, g_root) : raw_p;
        audioProcessor.voices[currentTrack].trigger(q_pitch, audioProcessor.tParams[currentTrack].drop->load(), 0.0f, beat_samples * 0.25f);
    };

    btn(copyLastBtn, juce::Colour(30,35,42), juce::Colours::white); copyLastBtn.setButtonText("< COPY");
    copyLastBtn.setTooltip("Copy synthesizer parameters to the previous Track.");
    copyLastBtn.onClick = [this, setParam]() {
        int target = (currentTrack + 5) % 6; juce::String ids[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol","mode281","mode292","notemode"};
        for (auto id : ids) setParam("t"+juce::String(target+1)+"_"+id, audioProcessor.apvts.getRawParameterValue("t"+juce::String(currentTrack+1)+"_"+id)->load());
    };

    btn(copyNextBtn, juce::Colour(30,35,42), juce::Colours::white); copyNextBtn.setButtonText("COPY >");
    copyNextBtn.setTooltip("Copy synthesizer parameters to the next Track.");
    copyNextBtn.onClick = [this, setParam]() {
        int target = (currentTrack + 1) % 6; juce::String ids[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol","mode281","mode292","notemode"};
        for (auto id : ids) setParam("t"+juce::String(target+1)+"_"+id, audioProcessor.apvts.getRawParameterValue("t"+juce::String(currentTrack+1)+"_"+id)->load());
    };

    for (int t = 0; t < 6; ++t) {
        mode281Combo[t].addItemList({"TRANS", "SUST", "CYCLE"}, 1); addChildComponent(mode281Combo[t]);
        mode281Combo[t].setTooltip("Buchla 281 Envelope: TRANS (Percussive), SUST (Gate-driven sustain), CYCLE (Continuous loop).");
        mode292Combo[t].addItemList({"VCA", "LPG", "VCF"}, 1); addChildComponent(mode292Combo[t]);
        mode292Combo[t].setTooltip("Buchla 292 Output: VCA (Clean level), LPG (Organic optical vactrol), VCF (Filter).");

        noteBtns[t].setClickingTogglesState(true); noteBtns[t].setColour(juce::TextButton::buttonColourId, juce::Colour(30,35,42)); noteBtns[t].setColour(juce::TextButton::buttonOnColourId, juce::Colour(20,50,70));
        noteBtns[t].setColour(juce::TextButton::textColourOnId, juce::Colours::cyan); noteBtns[t].setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        noteBtns[t].setTooltip("Lock pitch to scale-quantized MIDI notes or free Hertz sweeping.");
        nAtt[t] = std::make_unique<BtnAtt>(audioProcessor.apvts, "t" + juce::String(t+1) + "_notemode", noteBtns[t]);
        auto updateButtonState = [this, t]() {
            bool noteMode = noteBtns[t].getToggleState(); noteBtns[t].setButtonText(noteMode ? "NOTE" : "Hz");
            auto& pitchSld = vSliders[t][0].slider;
            if (noteMode) pitchSld.setRange(24.0, 96.0, 1.0); else pitchSld.setRange(24.0, 96.0, 0.01); pitchSld.updateText();
        };
        noteBtns[t].onClick = updateButtonState; updateButtonState(); noteBtns[t].setVisible(t == currentTrack); addChildComponent(noteBtns[t]);
    }

    juce::String sNames[] = {"PITCH","P.DRP","MRPH","FOLD","FM","RISE","FALL","RESP","BRGT","RESO","NOISE","VOL"};
    juce::Colour sCols[]  = {juce::Colours::white, juce::Colours::white, juce::Colours::white, juce::Colours::white, juce::Colours::white, 
                             juce::Colour(80,200,100), juce::Colour(80,200,100), juce::Colours::cyan, juce::Colours::cyan, juce::Colours::orange, juce::Colours::grey, juce::Colours::white};
    juce::String sTips[] = {"Base pitch.", "Pitch drop envelope.", "Waveform morph.", "Wavefolder saturation.", "FM Feedback.", "Attack time.", "Decay time.", "Vactrol inertia.", "Filter cutoff.", "Filter resonance Q.", "White noise level.", "Voice gain level."};
                              
    for (int t = 0; t < 6; ++t) {
        juce::String ts = "t" + juce::String(t+1) + "_";
        rAtt[t][0] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"steps",  stepsSld[t]); rAtt[t][1] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"pulses", pulsesSld[t]);
        rAtt[t][2] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"offset", offsetSld[t]); rRateAtt[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts+"rate", rateCombo[t]);
        m281Att[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts+"mode281", mode281Combo[t]); m292Att[t] = std::make_unique<CmbAtt>(audioProcessor.apvts, ts+"mode292", mode292Combo[t]);

        juce::String sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","reso","noise","vol"};
        for (int i = 0; i < 12; ++i) {
            vSliders[t][i].slider.setSliderStyle(juce::Slider::LinearVertical); vSliders[t][i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 13);
            vSliders[t][i].slider.setTooltip(sTips[i]); addChildComponent(vSliders[t][i].slider);
            lbl(vSliders[t][i].label, sNames[i].toRawUTF8(), sCols[i]); 
            sAtt[t][i] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+sIds[i], vSliders[t][i].slider);
            if (i == 0) {
                vSliders[t][i].slider.textFromValueFunction = [this, t](double value) {
                    if (noteBtns[t].getToggleState()) {
                        int scale = (int)audioProcessor.apvts.getRawParameterValue("global_scale")->load(); int root = (int)audioProcessor.apvts.getRawParameterValue("global_root")->load();
                        int q_midi = (int)quantize_pitch_ui((float)value, scale, root); juce::String notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
                        return notes[q_midi % 12] + juce::String((q_midi / 12) - 1);
                    } else { return juce::String(440.0f * std::pow(2.0f, (value - 69.0f) / 12.0f), 1) + " Hz"; }
                };
                vSliders[t][i].slider.updateText();
            }
        }
    }

    echoSyncBtn.setClickingTogglesState(true); 
    eSyncAtt = std::make_unique<BtnAtt>(audioProcessor.apvts, "echo_sync", echoSyncBtn);
    echoSyncBtn.setTooltip("Sync Delay time to musical tempo (1/32, 1/16, 1/8, 1/8D, 1/4)."); 
    addAndMakeVisible(echoSyncBtn);

    juce::String eNames[] = {"TIME","FDBK","MIX","WOW"}; 
    juce::String eTips[] = {"Delay time spacing.","Feedback regeneration level.","Wet/Dry mix proportion.","Analog tape motor wow & flutter modulation."};
    juce::String eIds[] = {"echo_time","echo_fdbk","echo_mix","echo_wow"};
    
    for (int i = 0; i < 4; ++i) {
        echoKnobs[i].slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); 
        echoKnobs[i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 13);
        echoKnobs[i].slider.setTooltip(eTips[i]); 
        addAndMakeVisible(echoKnobs[i].slider);
        lbl(echoKnobs[i].label, eNames[i].toRawUTF8(), juce::Colours::white); 
        
        eAtt[i] = std::make_unique<SldAtt>(audioProcessor.apvts, eIds[i], echoKnobs[i].slider);
        
        if (i == 0) {
            echoKnobs[i].slider.textFromValueFunction = [this](double value) {
                if (echoSyncBtn.getToggleState()) {
                    if (value < 0.2) return juce::String("1/32");
                    else if (value < 0.4) return juce::String("1/16");
                    else if (value < 0.6) return juce::String("1/8");
                    else if (value < 0.8) return juce::String("1/8D");
                    else return juce::String("1/4");
                } else {
                    return juce::String(value * 1000.0, 0) + " ms";
                }
            };
        }
    }

    echoSyncBtn.onClick = [this]() {
        bool sync = echoSyncBtn.getToggleState(); 
        echoSyncBtn.setButtonText(sync ? "SYNC: ON" : "SYNC: OFF");
        echoSyncBtn.setColour(juce::TextButton::buttonColourId, sync ? juce::Colour(20,70,50) : juce::Colour(16,45,35));
        echoKnobs[0].slider.updateText();
    };
    echoSyncBtn.onClick(); 

    auto possiblePaths = {
        juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("assets/logo.png"),
        juce::File::getCurrentWorkingDirectory().getChildFile("assets/logo.png"),
        juce::File("/Users/babyonk1/Desktop/ExtasisRecords/Orbita-LPG-JUCE/assets/logo.png")
    };
    
    for (const auto& f : possiblePaths) {
        if (f.existsAsFile()) {
            logoImage = juce::ImageCache::getFromFile(f);
            if (logoImage.isValid()) break;
        }
    }

    setSize(960, 500); startTimerHz(30); selectTrack(0);
}

OrbitaLPGAudioProcessorEditor::~OrbitaLPGAudioProcessorEditor() { setLookAndFeel(nullptr); }

void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {
    currentTrack = t;
    for (int i = 0; i < 6; ++i) {
        bool sel = (i == t); bool mut = trackMutes[i];
        tBtns[i].setColour(juce::TextButton::buttonColourId, sel ? juce::Colour(50,55,20) : juce::Colour(22,28,35)); 
        tBtns[i].setColour(juce::TextButton::textColourOffId, mut ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));
        
        stepsSld[i].setVisible(sel); pulsesSld[i].setVisible(sel); offsetSld[i].setVisible(sel); rateCombo[i].setVisible(sel);
        noteBtns[i].setVisible(sel); mode281Combo[i].setVisible(sel); mode292Combo[i].setVisible(sel);
        
        // VISIBILIDAD DE LOS MENÚS INDIVIDUALES: SOLO SE VE EL DEL TRACK ACTIVO
        synthPresetCombo[i].setVisible(sel);
        
        for (int j = 0; j < 12; ++j) {
            vSliders[i][j].slider.setVisible(sel);
            vSliders[i][j].label.setVisible(sel); 
        }
    }
}

void OrbitaLPGAudioProcessorEditor::toggleMute(int t) {
    trackMutes[t] = !trackMutes[t]; audioProcessor.trackMutes[t] = trackMutes[t];
    bool mut = trackMutes[t]; bool sel = (t == currentTrack); tBtns[t].setColour(juce::TextButton::textColourOffId, mut ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));
}

bool OrbitaLPGAudioProcessorEditor::keyPressed(const juce::KeyPress& key) { if (key.isKeyCode(juce::KeyPress::spaceKey)) { playBtn.triggerClick(); return true; } return false; }
void OrbitaLPGAudioProcessorEditor::timerCallback() { repaint(radarArea); }

void OrbitaLPGAudioProcessorEditor::resized() {
    auto b = getLocalBounds().reduced(12);
    topArea = b.removeFromTop(24); b.removeFromTop(8); radarArea = b.removeFromLeft(420); b.removeFromLeft(12);
    auto rightCol = b; masterArea = rightCol.removeFromTop(70); rightCol.removeFromTop(8);
    synthArea = rightCol.removeFromTop(290); rightCol.removeFromTop(8); echoArea = rightCol;

    {
        auto r = topArea; 
        titleLabel.setBounds(r.removeFromLeft(145)); 
        loadBtn.setBounds(r.removeFromLeft(45).reduced(0, 2)); r.removeFromLeft(2);
        saveBtn.setBounds(r.removeFromLeft(45).reduced(0, 2)); r.removeFromLeft(5);
        kitCombo.setBounds(r.removeFromLeft(160).reduced(0, 2)); r.removeFromLeft(10);
        playBtn.setBounds(r.removeFromLeft(50).reduced(0,2)); r.removeFromLeft(5);
        stopBtn.setBounds(r.removeFromLeft(50).reduced(0,2)); r.removeFromLeft(5); 
        seqBtn.setBounds( r.removeFromLeft(60).reduced(0,2)); r.removeFromLeft(5); 
        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));
    }
    {
        auto ra = radarArea; auto controlZone = ra.removeFromBottom(110); controlZone.reduce(8, 6);
        auto rowA = controlZone.removeFromTop(22);
        for (int i = 0; i < 6; ++i) tBtns[i].setBounds(rowA.removeFromLeft(30).reduced(1,0));
        rowA.removeFromLeft(10); patternsCombo.setBounds(rowA.removeFromLeft(160).reduced(0,1));
        rowA.removeFromLeft(5); rMutateBtn.setBounds(rowA.removeFromLeft(70).reduced(0,1)); controlZone.removeFromTop(6);
        
        auto placeRhythm = [&](juce::Label& l, juce::Slider* sArr, juce::ComboBox* cArr) {
            auto row = controlZone.removeFromTop(20); int lw = 45; l.setBounds(row.removeFromLeft(lw)); row.removeFromLeft(2);
            if (sArr) { for(int i=0; i<6; i++) sArr[i].setBounds(row); } else { for(int i=0; i<6; i++) cArr[i].setBounds(row); }
            controlZone.removeFromTop(4);
        };
        placeRhythm(stepsLbl, stepsSld, nullptr); placeRhythm(pulsesLbl, pulsesSld, nullptr); 
        placeRhythm(offsetLbl, offsetSld, nullptr); placeRhythm(rateLbl, nullptr, rateCombo);
    }
    {
        auto mi = masterArea.reduced(10); mi.removeFromTop(14); 
        auto row1 = mi.removeFromTop(12); mi.removeFromTop(2); auto row2 = mi.removeFromTop(20);
        auto place = [&](juce::Label& l, juce::Slider& s, int w) {
            l.setBounds(row1.removeFromLeft(w)); s.setBounds(row2.removeFromLeft(w).reduced(2,0)); row1.removeFromLeft(8); row2.removeFromLeft(8);
        };
        place(mVolLbl, mVolSld, 50); place(mDriveLbl, mDriveSld, 50); place(mBpmLbl, mBpmSld, 50); place(mSwingLbl, mSwingSld, 50); place(mChaosLbl, mChaosSld, 50);
        row1.removeFromLeft(5); row2.removeFromLeft(5); mScaleLbl.setBounds(row1.removeFromLeft(85)); globalScaleCombo.setBounds(row2.removeFromLeft(85).reduced(2,0));
        row1.removeFromLeft(5); row2.removeFromLeft(5); mRootLbl.setBounds(row1.removeFromLeft(60)); globalRootCombo.setBounds(row2.removeFromLeft(60).reduced(2,0));
    }
    {
        auto si = synthArea.reduced(10); si.removeFromTop(14); auto sRow1 = si.removeFromTop(20);
        auto noteRect = sRow1.removeFromRight(70).reduced(0,1); 
        
        // Asignamos el espacio a los 6 menús individuales (se montan uno sobre otro, pero selectTrack oculta los que no tocan)
        auto presetRect = sRow1.removeFromLeft(150).reduced(0,1);
        for(int t=0; t<6; ++t) {
            noteBtns[t].setBounds(noteRect);
            synthPresetCombo[t].setBounds(presetRect);
        }
        
        sRow1.removeFromLeft(5); sRandBtn.setBounds(sRow1.removeFromLeft(50).reduced(0,1)); sRow1.removeFromRight(4);
        copyNextBtn.setBounds(sRow1.removeFromRight(50).reduced(0,1)); sRow1.removeFromRight(4); copyLastBtn.setBounds(sRow1.removeFromRight(50).reduced(0,1));
        
        si.removeFromTop(5); auto sRow2 = si.removeFromTop(20);
        mode281Lbl.setBounds(sRow2.removeFromLeft(28)); auto r281 = sRow2.removeFromLeft(70).reduced(0, 2);
        sRow2.removeFromLeft(10); mode292Lbl.setBounds(sRow2.removeFromLeft(28)); auto r292 = sRow2.removeFromLeft(70).reduced(0, 2);
        sRow2.removeFromLeft(10); strikeBtn.setBounds(sRow2.removeFromLeft(60).reduced(0, 1));
        for (int t = 0; t < 6; ++t) { mode281Combo[t].setBounds(r281); mode292Combo[t].setBounds(r292); }

        si.removeFromTop(10); int sw = si.getWidth() / 12; 
        for (int i = 0; i < 12; ++i) {
            auto cell = si.removeFromLeft(sw); 
            auto labelBounds = cell.removeFromTop(12); 
            cell.removeFromTop(2); 
            for (int t = 0; t < 6; ++t) {
                vSliders[t][i].label.setBounds(labelBounds); 
                vSliders[t][i].slider.setBounds(cell);
            }
        }
    }
    {
        auto ei = echoArea.reduced(10); ei.removeFromTop(14); 
        echoSyncBtn.setBounds(ei.removeFromLeft(80).withSizeKeepingCentre(70, 20)); ei.removeFromLeft(5);
        int kw = ei.getWidth() / 4;
        for (int i = 0; i < 4; ++i) {
            auto cell = ei.removeFromLeft(kw); auto kb = cell.withSizeKeepingCentre(38, 38);
            echoKnobs[i].slider.setBounds(kb); echoKnobs[i].label.setBounds(kb.getX()-8, kb.getY()-14, kb.getWidth()+16, 12);
        }
    }
    
    creditLabel.setBounds(getWidth() - 215, getHeight() - 18, 210, 15);
}

void OrbitaLPGAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(13, 16, 20));
    auto drawModule = [&](juce::Rectangle<int> r, juce::Colour bg, juce::Colour border) {
        g.setColour(bg); g.fillRoundedRectangle(r.toFloat(), 6.0f); g.setColour(border); g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.5f);
    };
    drawModule(radarArea, juce::Colour(11,14,18), juce::Colour(40,50,60)); drawModule(masterArea, juce::Colour(20,24,28), juce::Colour(50,60,70));
    drawModule(synthArea, juce::Colour(18,22,26), juce::Colour(50,55,65)); drawModule(echoArea, juce::Colour(14,18,22), juce::Colour(35,55,45));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold)); g.setColour(juce::Colour(110,125,140));
    g.drawText("MASTER SYSTEM", masterArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);
    g.setColour(juce::Colour(65,229,155)); g.drawText("RE-201 SPACE ECHO", echoArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);
    g.setColour(juce::Colour(200,150,50)); g.drawText("WEST COAST SYNTHESIS  259/281/292", synthArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);

    auto radarPure = radarArea; radarPure.removeFromBottom(110); 
    auto center = radarPure.getCentre().toFloat(); float maxR = std::min(radarPure.getWidth(), radarPure.getHeight()) * 0.45f;
    g.setColour(juce::Colour(8, 10, 13)); g.fillEllipse(center.x - maxR, center.y - maxR, maxR*2, maxR*2);
    g.setColour(juce::Colour(30, 38, 48)); 
    g.drawLine(center.x - maxR, center.y, center.x + maxR, center.y); 
    g.drawLine(center.x, center.y - maxR, center.x, center.y + maxR);

    if (logoImage.isValid()) {
        float logoSize = maxR * 0.55f;
        juce::Rectangle<float> logoBounds(center.x - logoSize * 0.5f, center.y - logoSize * 0.5f, logoSize, logoSize);
        g.drawImageWithin(logoImage, logoBounds.getX(), logoBounds.getY(), logoBounds.getWidth(), logoBounds.getHeight(),
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, true);
    } else {
        g.setColour(juce::Colour(65, 229, 155).withAlpha(0.2f));
        g.fillEllipse(center.x - 30.0f, center.y - 30.0f, 60.0f, 60.0f);
        g.setColour(juce::Colour(65, 229, 155));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText("LAO", center.x - 30.0f, center.y - 12.0f, 60.0f, 20.0f, juce::Justification::centred);
    }

    for (int t = 0; t < 6; ++t) {
        float r = (t + 1) * (maxR / 6.8f); bool sel = (t == currentTrack); bool mut = trackMutes[t];
        juce::Colour ringCol = mut ? juce::Colour(50,55,60) : sel ? juce::Colours::yellow.withAlpha(0.35f) : juce::Colours::cyan.withAlpha(0.18f);
        g.setColour(ringCol); g.drawEllipse(center.x - r, center.y - r, r*2, r*2, 1.0f);
        juce::String ts = "t" + juce::String(t+1) + "_";
        int steps = (int)audioProcessor.apvts.getRawParameterValue(ts+"steps")->load();
        int pulses = (int)audioProcessor.apvts.getRawParameterValue(ts+"pulses")->load();
        int offset = (int)audioProcessor.apvts.getRawParameterValue(ts+"offset")->load();
        auto pat = gen_euclid(pulses, steps, offset);
        juce::Colour dotCol = mut ? juce::Colour(60,65,70) : sel ? juce::Colours::yellow : juce::Colours::cyan;
        juce::Path poly; bool first = true;
        for (int i = 0; i < steps; ++i) {
            if (pat[i] != 1) continue;
            float angle = (i / (float)steps) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float px = center.x + std::cos(angle) * r; float py = center.y + std::sin(angle) * r;
            if (first) { poly.startNewSubPath(px, py); first = false; } else { poly.lineTo(px, py); }
            g.setColour(dotCol); g.fillEllipse(px - 3.5f, py - 3.5f, 7.0f, 7.0f);
        }
        if (!first) { poly.closeSubPath(); g.setColour(dotCol.withAlpha(0.55f)); g.strokePath(poly, juce::PathStrokeType(1.5f)); }
        int cur = audioProcessor.voices[t].current_step % (steps > 0 ? steps : 1);
        float pAngle = (cur / (float)(steps > 0 ? steps : 16)) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        float px = center.x + std::cos(pAngle) * r; float py = center.y + std::sin(pAngle) * r;
        g.setColour(juce::Colours::white.withAlpha(0.8f)); g.drawEllipse(px - 5.0f, py - 5.0f, 10.0f, 10.0f, 2.0f);
    }
}