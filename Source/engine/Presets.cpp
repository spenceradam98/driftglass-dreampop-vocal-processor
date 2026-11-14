// Factory presets. Each preset lists only what differs from the parameter defaults.
//
// Choice indices, for reference:
//   drive_type: 0 Tape, 1 Tube, 2 Fuzz
//   dbl_mode:   0 Double, 1 Octave Up, 2 Octave Down, 3 Fifth Up, 4 Octaves, 5 Choir
//   mod_mode:   0 Chorus, 1 Ensemble, 2 Vibrato, 3 Flanger, 4 Warble
//   echo_type:  0 Tape, 1 Analog, 2 Ping-Pong, 3 Diffuse
//   echo_sync:  0 Free, 1 1/32, 2 1/16T, 3 1/16, 4 1/16D, 5 1/8T, 6 1/8, 7 1/8D, 8 1/4T, 9 1/4, 10 1/4D, 11 1/2, 12 1/1
//   verb_type:  0 Plate, 1 Hall, 2 Cathedral, 3 Shimmer, 4 Spring, 5 Reverse, 6 Wash
#include "Engine.h"

namespace dg
{
namespace
{
const char* const kDream = "Dream Pop";
const char* const kShoe = "Shoegaze";
const char* const kSynth = "Synth Pop";
const char* const kVintage = "Vintage";
const char* const kLofi = "Lo-Fi";
const char* const kSpace = "Space & Utility";

std::vector<Preset> build()
{
    return {
        // ------------------------------------------------------------ Dream Pop
        { "Velvet Haze", kDream, { {"comp",0.4f}, {"dbl_on",1}, {"dbl_mix",0.35f}, {"mod_on",1}, {"mod_mode",1}, {"mod_rate",0.5f}, {"mod_mix",0.4f},
                                   {"echo_type",0}, {"echo_sync",10}, {"echo_fb",0.4f}, {"echo_mix",0.22f}, {"echo_duck",0.5f},
                                   {"verb_type",1}, {"verb_decay",4.5f}, {"verb_size",0.75f}, {"verb_mix",0.38f}, {"verb_duck",0.3f}, {"out_air",0.45f} } },
        { "Pillow Talk", kDream, { {"comp",0.5f}, {"deess",0.45f}, {"dbl_on",1}, {"dbl_detune",0.25f}, {"dbl_mix",0.3f},
                                   {"echo_type",1}, {"echo_time",320}, {"echo_fb",0.3f}, {"echo_mix",0.15f},
                                   {"verb_type",0}, {"verb_decay",2.8f}, {"verb_mix",0.3f}, {"verb_tone",0.45f}, {"out_air",0.5f} } },
        { "Lilac Bloom", kDream, { {"comp",0.35f}, {"mod_on",1}, {"mod_mode",0}, {"mod_rate",0.35f}, {"mod_depth",0.6f}, {"mod_mix",0.45f},
                                   {"echo_type",3}, {"echo_sync",9}, {"echo_fb",0.45f}, {"echo_mix",0.2f},
                                   {"verb_type",3}, {"verb_decay",6}, {"verb_char",0.35f}, {"verb_mix",0.35f}, {"verb_duck",0.35f} } },
        { "Cotton Cloud", kDream, { {"comp",0.45f}, {"dbl_on",1}, {"dbl_mode",5}, {"dbl_mix",0.3f},
                                    {"echo_on",0}, {"verb_type",1}, {"verb_decay",7}, {"verb_size",0.9f}, {"verb_pre",45}, {"verb_mix",0.42f}, {"verb_duck",0.4f}, {"out_width",1.3f} } },
        { "Slow Dissolve", kDream, { {"comp",0.4f}, {"echo_type",0}, {"echo_time",620}, {"echo_fb",0.55f}, {"echo_wow",0.35f}, {"echo_mix",0.25f}, {"echo_duck",0.6f},
                                     {"verb_type",2}, {"verb_decay",9}, {"verb_mix",0.35f}, {"verb_duck",0.45f} } },
        { "Glass Bedroom", kDream, { {"comp",0.3f}, {"col_on",1}, {"col_highcut",9000}, {"col_wobble",0.2f}, {"col_noise",0.1f},
                                     {"mod_on",1}, {"mod_mode",4}, {"mod_rate",0.6f}, {"mod_depth",0.35f}, {"mod_mix",1},
                                     {"echo_type",1}, {"echo_sync",7}, {"echo_fb",0.35f}, {"echo_mix",0.2f}, {"verb_type",0}, {"verb_decay",3.2f}, {"verb_mix",0.3f} } },
        { "Moonlit Choir", kDream, { {"comp",0.4f}, {"dbl_on",1}, {"dbl_mode",5}, {"dbl_detune",0.55f}, {"dbl_mix",0.45f}, {"dbl_width",1},
                                     {"echo_on",0}, {"verb_type",3}, {"verb_decay",8}, {"verb_char",0.25f}, {"verb_mix",0.4f}, {"verb_duck",0.3f} } },
        { "Sundial", kDream, { {"comp",0.35f}, {"drive",0.25f}, {"echo_type",2}, {"echo_sync",7}, {"echo_fb",0.4f}, {"echo_mix",0.22f},
                               {"verb_type",1}, {"verb_decay",3.5f}, {"verb_mix",0.3f}, {"out_air",0.4f} } },
        { "Honey Halo", kDream, { {"comp",0.5f}, {"drive",0.3f}, {"drive_type",1}, {"dbl_on",1}, {"dbl_mix",0.25f},
                                  {"echo_type",0}, {"echo_sync",9}, {"echo_fb",0.3f}, {"echo_mix",0.18f}, {"verb_type",0}, {"verb_decay",3.8f}, {"verb_size",0.8f}, {"verb_mix",0.34f} } },
        { "Petal Swell", kDream, { {"comp",0.4f}, {"echo_on",0}, {"verb_type",5}, {"verb_decay",3}, {"verb_char",0.55f}, {"verb_mix",0.45f}, {"verb_duck",0.0f},
                                   {"mod_on",1}, {"mod_mode",1}, {"mod_mix",0.35f} } },
        { "Faded Postcard", kDream, { {"comp",0.4f}, {"col_on",1}, {"col_lowcut",180}, {"col_highcut",7000}, {"col_wobble",0.35f},
                                      {"echo_type",0}, {"echo_sync",11}, {"echo_fb",0.45f}, {"echo_wow",0.5f}, {"echo_mix",0.22f}, {"verb_type",1}, {"verb_decay",5}, {"verb_mix",0.35f} } },
        { "Starlit Double", kDream, { {"comp",0.45f}, {"dbl_on",1}, {"dbl_detune",0.5f}, {"dbl_delay",0.6f}, {"dbl_mix",0.5f},
                                      {"echo_type",2}, {"echo_sync",10}, {"echo_fb",0.35f}, {"echo_mix",0.16f}, {"verb_type",1}, {"verb_decay",4}, {"verb_mix",0.3f} } },

        // ------------------------------------------------------------ Shoegaze
        { "Wall of Glass", kShoe, { {"comp",0.6f}, {"drive",0.35f}, {"drive_type",2}, {"dbl_on",1}, {"dbl_mode",4}, {"dbl_mix",0.25f},
                                    {"mod_on",1}, {"mod_mode",4}, {"mod_depth",0.5f}, {"mod_mix",0.6f},
                                    {"echo_type",3}, {"echo_sync",10}, {"echo_fb",0.55f}, {"echo_mix",0.3f}, {"echo_duck",0.2f},
                                    {"verb_type",6}, {"verb_decay",12}, {"verb_char",0.5f}, {"verb_mix",0.55f}, {"verb_duck",0.1f} } },
        { "Buried Vocal", kShoe, { {"comp",0.6f}, {"in_lowcut",160}, {"mod_on",1}, {"mod_mode",3}, {"mod_rate",0.15f}, {"mod_depth",0.6f}, {"mod_mix",0.5f},
                                   {"echo_type",0}, {"echo_time",450}, {"echo_fb",0.6f}, {"echo_mix",0.35f}, {"echo_duck",0.0f},
                                   {"verb_type",6}, {"verb_decay",15}, {"verb_char",0.3f}, {"verb_mix",0.7f}, {"verb_duck",0.0f}, {"out_air",0.1f} } },
        { "Glide Wash", kShoe, { {"comp",0.5f}, {"mod_on",1}, {"mod_mode",4}, {"mod_rate",0.4f}, {"mod_depth",0.8f}, {"mod_mix",1},
                                 {"echo_type",3}, {"echo_sync",9}, {"echo_fb",0.5f}, {"echo_mix",0.3f},
                                 {"verb_type",6}, {"verb_decay",10}, {"verb_char",0.2f}, {"verb_mod",0.8f}, {"verb_mix",0.5f} } },
        { "Fuzz Halo", kShoe, { {"comp",0.55f}, {"drive",0.55f}, {"drive_type",2}, {"echo_type",1}, {"echo_sync",7}, {"echo_fb",0.45f}, {"echo_mix",0.25f},
                                {"verb_type",6}, {"verb_decay",8}, {"verb_char",0.7f}, {"verb_mix",0.45f}, {"verb_duck",0.2f} } },
        { "Feedback Lullaby", kShoe, { {"comp",0.5f}, {"dbl_on",1}, {"dbl_mode",5}, {"dbl_mix",0.35f},
                                       {"echo_type",0}, {"echo_sync",11}, {"echo_fb",0.78f}, {"echo_wow",0.45f}, {"echo_drive",0.5f}, {"echo_mix",0.3f},
                                       {"verb_type",2}, {"verb_decay",14}, {"verb_mix",0.5f}, {"verb_duck",0.2f} } },
        { "Tremolo Sea", kShoe, { {"comp",0.5f}, {"mod_on",1}, {"mod_mode",2}, {"mod_rate",5.5f}, {"mod_depth",0.35f}, {"mod_mix",0.7f},
                                  {"echo_type",3}, {"echo_time",520}, {"echo_fb",0.5f}, {"echo_mix",0.28f},
                                  {"verb_type",6}, {"verb_decay",9}, {"verb_char",0.35f}, {"verb_mix",0.5f} } },
        { "Distant Shore", kShoe, { {"comp",0.6f}, {"col_on",1}, {"col_lowcut",250}, {"col_highcut",5000}, {"col_reso",0.2f},
                                    {"echo_type",3}, {"echo_time",700}, {"echo_fb",0.6f}, {"echo_mix",0.35f},
                                    {"verb_type",2}, {"verb_decay",18}, {"verb_size",0.9f}, {"verb_mix",0.6f}, {"verb_duck",0.0f} } },
        { "Shimmer Fog", kShoe, { {"comp",0.5f}, {"drive",0.3f}, {"drive_type",2}, {"mod_on",1}, {"mod_mode",1}, {"mod_mix",0.5f},
                                  {"echo_type",2}, {"echo_sync",10}, {"echo_fb",0.5f}, {"echo_mix",0.25f},
                                  {"verb_type",3}, {"verb_decay",12}, {"verb_char",0.6f}, {"verb_mix",0.55f}, {"verb_duck",0.15f} } },
        { "Overcast", kShoe, { {"comp",0.55f}, {"col_on",1}, {"col_highcut",6500}, {"col_noise",0.15f}, {"dbl_on",1}, {"dbl_mix",0.4f},
                               {"echo_type",0}, {"echo_sync",9}, {"echo_fb",0.5f}, {"echo_mix",0.25f}, {"verb_type",6}, {"verb_decay",7}, {"verb_char",0.25f}, {"verb_mix",0.45f} } },
        { "Kaleido Drone", kShoe, { {"comp",0.6f}, {"dbl_on",1}, {"dbl_mode",4}, {"dbl_mix",0.45f}, {"mod_on",1}, {"mod_mode",3}, {"mod_rate",0.1f}, {"mod_depth",0.8f}, {"mod_mix",0.6f},
                                    {"echo_type",3}, {"echo_sync",12}, {"echo_fb",0.7f}, {"echo_mix",0.3f},
                                    {"verb_type",6}, {"verb_decay",20}, {"verb_char",0.6f}, {"verb_mix",0.6f}, {"verb_duck",0.0f} } },
        { "Reverse Rain", kShoe, { {"comp",0.5f}, {"drive",0.25f}, {"drive_type",2}, {"echo_type",3}, {"echo_sync",9}, {"echo_fb",0.4f}, {"echo_mix",0.2f},
                                   {"verb_type",5}, {"verb_decay",4}, {"verb_char",0.8f}, {"verb_mix",0.55f}, {"verb_duck",0.0f} } },
        { "Velour Noise", kShoe, { {"comp",0.65f}, {"drive",0.7f}, {"drive_type",2}, {"col_on",1}, {"col_highcut",8000}, {"col_wobble",0.3f},
                                   {"echo_type",1}, {"echo_sync",6}, {"echo_fb",0.55f}, {"echo_mix",0.25f}, {"verb_type",6}, {"verb_decay",11}, {"verb_char",0.8f}, {"verb_mix",0.5f} } },

        // ------------------------------------------------------------ Synth Pop
        { "Neon Lead", kSynth, { {"comp",0.6f}, {"deess",0.45f}, {"drive",0.15f}, {"dbl_on",1}, {"dbl_detune",0.3f}, {"dbl_delay",0.2f}, {"dbl_mix",0.4f},
                                 {"echo_type",2}, {"echo_sync",7}, {"echo_fb",0.35f}, {"echo_tone",0.7f}, {"echo_mix",0.2f}, {"echo_duck",0.6f},
                                 {"verb_type",0}, {"verb_decay",2.2f}, {"verb_mix",0.25f}, {"out_air",0.55f} } },
        { "Gated Eighties", kSynth, { {"comp",0.65f}, {"echo_type",2}, {"echo_sync",6}, {"echo_fb",0.25f}, {"echo_mix",0.18f},
                                      {"verb_type",1}, {"verb_decay",1.2f}, {"verb_size",0.5f}, {"verb_mix",0.35f}, {"verb_duck",0.6f}, {"out_air",0.5f} } },
        { "Chrome Chorus", kSynth, { {"comp",0.55f}, {"mod_on",1}, {"mod_mode",0}, {"mod_rate",1.1f}, {"mod_depth",0.45f}, {"mod_mix",0.55f},
                                     {"echo_type",1}, {"echo_sync",7}, {"echo_fb",0.3f}, {"echo_mix",0.18f}, {"verb_type",0}, {"verb_decay",2.5f}, {"verb_mix",0.25f} } },
        { "Robot Heart", kSynth, { {"comp",0.6f}, {"dbl_on",1}, {"dbl_mode",2}, {"dbl_mix",0.45f}, {"dbl_width",0.3f},
                                   {"col_on",1}, {"col_crush",0.35f}, {"col_mix",0.5f}, {"echo_type",2}, {"echo_sync",3}, {"echo_fb",0.3f}, {"echo_mix",0.15f},
                                   {"verb_type",0}, {"verb_decay",1.6f}, {"verb_mix",0.2f} } },
        { "Night Drive", kSynth, { {"comp",0.55f}, {"dbl_on",1}, {"dbl_mix",0.3f}, {"mod_on",1}, {"mod_mode",1}, {"mod_mix",0.35f},
                                   {"echo_type",2}, {"echo_sync",10}, {"echo_fb",0.45f}, {"echo_mix",0.24f}, {"echo_duck",0.55f},
                                   {"verb_type",1}, {"verb_decay",3.5f}, {"verb_mix",0.3f}, {"verb_duck",0.3f}, {"out_width",1.25f} } },
        { "Octave Pop", kSynth, { {"comp",0.6f}, {"dbl_on",1}, {"dbl_mode",1}, {"dbl_mix",0.35f}, {"echo_type",2}, {"echo_sync",7}, {"echo_fb",0.3f}, {"echo_mix",0.18f},
                                  {"verb_type",0}, {"verb_decay",2}, {"verb_mix",0.22f}, {"out_air",0.5f} } },
        { "Flanged Mirror", kSynth, { {"comp",0.55f}, {"mod_on",1}, {"mod_mode",3}, {"mod_rate",0.25f}, {"mod_depth",0.55f}, {"mod_mix",0.5f},
                                      {"echo_type",2}, {"echo_sync",6}, {"echo_fb",0.3f}, {"echo_mix",0.15f}, {"verb_type",0}, {"verb_decay",2.4f}, {"verb_mix",0.25f} } },
        { "Arcade Love", kSynth, { {"comp",0.6f}, {"col_on",1}, {"col_crush",0.55f}, {"col_highcut",11000}, {"col_mix",0.6f},
                                   {"echo_type",2}, {"echo_sync",3}, {"echo_fb",0.35f}, {"echo_mix",0.2f}, {"verb_type",0}, {"verb_decay",1.5f}, {"verb_mix",0.2f} } },
        { "Fifth Avenue", kSynth, { {"comp",0.55f}, {"dbl_on",1}, {"dbl_mode",3}, {"dbl_mix",0.3f}, {"echo_type",1}, {"echo_sync",10}, {"echo_fb",0.35f}, {"echo_mix",0.2f},
                                    {"verb_type",1}, {"verb_decay",3}, {"verb_mix",0.28f} } },
        { "Satin Ballad", kSynth, { {"comp",0.5f}, {"deess",0.5f}, {"mod_on",1}, {"mod_mode",1}, {"mod_mix",0.3f}, {"echo_type",0}, {"echo_sync",9}, {"echo_fb",0.35f}, {"echo_mix",0.2f},
                                    {"verb_type",1}, {"verb_decay",4.2f}, {"verb_mix",0.34f}, {"verb_duck",0.3f}, {"out_air",0.55f} } },
        { "Vocoder-ish", kSynth, { {"comp",0.7f}, {"dbl_on",1}, {"dbl_mode",4}, {"dbl_mix",0.6f}, {"col_on",1}, {"col_lowcut",300}, {"col_highcut",6000}, {"col_reso",0.5f},
                                   {"mod_on",1}, {"mod_mode",3}, {"mod_rate",0.08f}, {"mod_depth",0.2f}, {"mod_mix",0.6f}, {"echo_mix",0.1f}, {"verb_type",0}, {"verb_mix",0.2f} } },
        { "Cassette Single", kSynth, { {"comp",0.55f}, {"col_on",1}, {"col_highcut",12000}, {"col_wobble",0.2f}, {"col_noise",0.08f},
                                       {"mod_on",1}, {"mod_mode",0}, {"mod_mix",0.35f}, {"echo_type",1}, {"echo_sync",7}, {"echo_fb",0.3f}, {"echo_mix",0.18f},
                                       {"verb_type",0}, {"verb_decay",2.2f}, {"verb_mix",0.25f} } },

        // ------------------------------------------------------------ Vintage
        { "Fifties Slapback", kVintage, { {"comp",0.45f}, {"drive",0.3f}, {"drive_type",1}, {"echo_type",0}, {"echo_time",115}, {"echo_fb",0.12f}, {"echo_mix",0.3f}, {"echo_duck",0.0f},
                                          {"verb_on",0}, {"out_air",0.1f} } },
        { "Echo Chamber", kVintage, { {"comp",0.45f}, {"drive",0.2f}, {"drive_type",1}, {"echo_on",0}, {"verb_type",1}, {"verb_decay",2.2f}, {"verb_size",0.4f}, {"verb_tone",0.35f},
                                      {"verb_mix",0.35f}, {"verb_duck",0.0f}, {"out_air",0.1f} } },
        { "Spring Crooner", kVintage, { {"comp",0.5f}, {"drive",0.25f}, {"drive_type",1}, {"echo_type",0}, {"echo_time",140}, {"echo_fb",0.15f}, {"echo_mix",0.15f},
                                        {"verb_type",4}, {"verb_decay",2.6f}, {"verb_char",0.5f}, {"verb_mix",0.35f} } },
        { "Girl Group Plate", kVintage, { {"comp",0.55f}, {"drive",0.2f}, {"drive_type",0}, {"dbl_on",1}, {"dbl_mix",0.25f}, {"echo_type",0}, {"echo_time",95}, {"echo_fb",0.1f}, {"echo_mix",0.15f},
                                          {"verb_type",0}, {"verb_decay",2.8f}, {"verb_tone",0.4f}, {"verb_mix",0.38f}, {"verb_duck",0.0f} } },
        { "Psych Sixties", kVintage, { {"comp",0.5f}, {"drive",0.35f}, {"drive_type",0}, {"mod_on",1}, {"mod_mode",3}, {"mod_rate",0.2f}, {"mod_depth",0.5f}, {"mod_mix",0.45f},
                                       {"echo_type",0}, {"echo_time",280}, {"echo_fb",0.45f}, {"echo_wow",0.4f}, {"echo_mix",0.25f}, {"verb_type",4}, {"verb_decay",3}, {"verb_mix",0.3f} } },
        { "Ribbon Mic", kVintage, { {"comp",0.4f}, {"col_on",1}, {"col_highcut",8500}, {"col_reso",0.1f}, {"drive",0.2f}, {"drive_type",1},
                                    {"echo_on",0}, {"verb_type",0}, {"verb_decay",1.8f}, {"verb_mix",0.2f}, {"out_air",0.0f} } },
        { "Tape Doubled", kVintage, { {"comp",0.5f}, {"drive",0.3f}, {"dbl_on",1}, {"dbl_detune",0.2f}, {"dbl_delay",0.15f}, {"dbl_width",0.5f}, {"dbl_mix",0.5f},
                                      {"echo_type",0}, {"echo_time",160}, {"echo_fb",0.2f}, {"echo_mix",0.12f}, {"verb_type",0}, {"verb_decay",2}, {"verb_mix",0.22f} } },
        { "Organ Speaker", kVintage, { {"comp",0.5f}, {"mod_on",1}, {"mod_mode",2}, {"mod_rate",6.2f}, {"mod_depth",0.4f}, {"mod_mix",0.6f},
                                       {"col_on",1}, {"col_lowcut",120}, {"col_highcut",7000}, {"echo_mix",0.12f}, {"verb_type",4}, {"verb_mix",0.25f} } },
        { "Motown Room", kVintage, { {"comp",0.6f}, {"drive",0.25f}, {"drive_type",1}, {"echo_type",0}, {"echo_time",120}, {"echo_fb",0.15f}, {"echo_mix",0.12f},
                                     {"verb_type",1}, {"verb_decay",1.6f}, {"verb_size",0.35f}, {"verb_tone",0.45f}, {"verb_mix",0.3f}, {"verb_duck",0.0f} } },
        { "Surf Ballad", kVintage, { {"comp",0.45f}, {"echo_type",0}, {"echo_time",210}, {"echo_fb",0.3f}, {"echo_mix",0.2f}, {"verb_type",4}, {"verb_decay",3.5f}, {"verb_char",0.7f},
                                     {"verb_mix",0.4f}, {"verb_duck",0.1f} } },
        { "Valve Warmth", kVintage, { {"comp",0.55f}, {"drive",0.5f}, {"drive_type",1}, {"echo_on",0}, {"verb_type",0}, {"verb_decay",2.4f}, {"verb_mix",0.22f}, {"out_air",0.3f} } },
        { "Dusty Ballroom", kVintage, { {"comp",0.45f}, {"col_on",1}, {"col_highcut",6000}, {"col_noise",0.2f}, {"col_wobble",0.15f},
                                        {"echo_on",0}, {"verb_type",1}, {"verb_decay",3.5f}, {"verb_tone",0.3f}, {"verb_mix",0.4f}, {"verb_duck",0.0f} } },

        // ------------------------------------------------------------ Lo-Fi
        { "Telephone", kLofi, { {"comp",0.6f}, {"col_on",1}, {"col_lowcut",400}, {"col_highcut",3400}, {"col_reso",0.35f}, {"drive",0.35f},
                                {"echo_on",0}, {"verb_type",0}, {"verb_decay",1}, {"verb_mix",0.12f}, {"out_air",0} } },
        { "AM Radio", kLofi, { {"comp",0.65f}, {"col_on",1}, {"col_lowcut",250}, {"col_highcut",4500}, {"col_reso",0.2f}, {"col_noise",0.3f}, {"drive",0.3f}, {"drive_type",1},
                               {"echo_mix",0.12f}, {"verb_type",4}, {"verb_mix",0.15f}, {"out_air",0} } },
        { "Walkman", kLofi, { {"comp",0.55f}, {"col_on",1}, {"col_highcut",8000}, {"col_wobble",0.55f}, {"col_noise",0.25f},
                              {"echo_type",1}, {"echo_sync",7}, {"echo_fb",0.3f}, {"echo_mix",0.18f}, {"verb_type",0}, {"verb_decay",2}, {"verb_mix",0.22f}, {"out_air",0} } },
        { "Bedroom Demo", kLofi, { {"comp",0.6f}, {"col_on",1}, {"col_highcut",10000}, {"col_wobble",0.25f}, {"col_noise",0.12f}, {"dbl_on",1}, {"dbl_mix",0.25f},
                                   {"echo_type",0}, {"echo_sync",9}, {"echo_fb",0.35f}, {"echo_mix",0.2f}, {"verb_type",1}, {"verb_decay",3}, {"verb_mix",0.3f} } },
        { "8-Bit Angel", kLofi, { {"comp",0.6f}, {"col_on",1}, {"col_crush",0.75f}, {"col_highcut",9000}, {"col_mix",0.8f},
                                  {"echo_type",2}, {"echo_sync",6}, {"echo_fb",0.35f}, {"echo_mix",0.2f}, {"verb_type",3}, {"verb_decay",5}, {"verb_char",0.4f}, {"verb_mix",0.35f} } },
        { "Megaphone", kLofi, { {"comp",0.7f}, {"drive",0.6f}, {"drive_type",2}, {"col_on",1}, {"col_lowcut",600}, {"col_highcut",3000}, {"col_reso",0.6f},
                                {"echo_on",0}, {"verb_on",0}, {"out_air",0} } },
        { "Warped Vinyl", kLofi, { {"comp",0.5f}, {"col_on",1}, {"col_highcut",7000}, {"col_wobble",0.8f}, {"col_noise",0.45f},
                                   {"echo_type",0}, {"echo_sync",9}, {"echo_fb",0.3f}, {"echo_wow",0.6f}, {"echo_mix",0.18f}, {"verb_type",1}, {"verb_decay",2.5f}, {"verb_mix",0.28f} } },
        { "Answering Machine", kLofi, { {"comp",0.7f}, {"col_on",1}, {"col_lowcut",350}, {"col_highcut",3200}, {"col_crush",0.4f}, {"col_wobble",0.3f}, {"col_noise",0.35f},
                                        {"echo_on",0}, {"verb_type",0}, {"verb_decay",0.8f}, {"verb_mix",0.12f}, {"out_air",0} } },
        { "Basement Tape", kLofi, { {"comp",0.6f}, {"drive",0.45f}, {"col_on",1}, {"col_highcut",6000}, {"col_wobble",0.4f}, {"col_noise",0.3f},
                                    {"echo_type",0}, {"echo_time",240}, {"echo_fb",0.4f}, {"echo_wow",0.5f}, {"echo_mix",0.22f}, {"verb_type",4}, {"verb_mix",0.25f} } },
        { "Toy Keyboard", kLofi, { {"comp",0.6f}, {"col_on",1}, {"col_crush",0.6f}, {"col_lowcut",300}, {"dbl_on",1}, {"dbl_mode",1}, {"dbl_mix",0.3f},
                                   {"echo_type",2}, {"echo_sync",3}, {"echo_fb",0.3f}, {"echo_mix",0.18f}, {"verb_type",0}, {"verb_decay",1.5f}, {"verb_mix",0.2f} } },

        // ------------------------------------------------------------ Space & Utility
        { "Clean Vocal", kSpace, { {"comp",0.45f}, {"deess",0.4f}, {"echo_on",0}, {"verb_type",0}, {"verb_decay",1.6f}, {"verb_mix",0.15f}, {"out_air",0.35f} } },
        { "Lead Polish", kSpace, { {"comp",0.55f}, {"deess",0.5f}, {"drive",0.15f}, {"echo_type",2}, {"echo_sync",7}, {"echo_fb",0.25f}, {"echo_mix",0.12f}, {"echo_duck",0.7f},
                                   {"verb_type",0}, {"verb_decay",2}, {"verb_mix",0.2f}, {"verb_duck",0.4f}, {"out_air",0.45f} } },
        { "Backing Stack", kSpace, { {"comp",0.6f}, {"deess",0.55f}, {"in_lowcut",150}, {"dbl_on",1}, {"dbl_mode",5}, {"dbl_mix",0.5f}, {"dbl_width",1},
                                     {"echo_on",0}, {"verb_type",1}, {"verb_decay",2.5f}, {"verb_mix",0.3f}, {"out_width",1.4f} } },
        { "Infinite Cathedral", kSpace, { {"comp",0.45f}, {"echo_on",0}, {"verb_type",2}, {"verb_decay",30}, {"verb_size",1}, {"verb_mix",0.6f}, {"verb_duck",0.0f} } },
        { "Shimmer Heaven", kSpace, { {"comp",0.45f}, {"echo_type",3}, {"echo_sync",10}, {"echo_fb",0.4f}, {"echo_mix",0.2f},
                                      {"verb_type",3}, {"verb_decay",14}, {"verb_char",0.75f}, {"verb_mix",0.5f}, {"verb_duck",0.2f} } },
        { "Reverse Intro", kSpace, { {"comp",0.45f}, {"echo_on",0}, {"verb_type",5}, {"verb_decay",2.5f}, {"verb_char",0.9f}, {"verb_mix",0.6f}, {"verb_duck",0.0f} } },
        { "Echo Send (100% Wet)", kSpace, { {"in_on",0}, {"echo_type",0}, {"echo_sync",10}, {"echo_fb",0.45f}, {"echo_mix",1}, {"echo_duck",0},
                                             {"verb_on",0}, {"out_air",0} } },
        { "Reverb Send (100% Wet)", kSpace, { {"in_on",0}, {"echo_on",0}, {"verb_type",1}, {"verb_decay",3.5f}, {"verb_mix",1}, {"verb_duck",0}, {"out_air",0} } },
        { "Throw Delay", kSpace, { {"comp",0.5f}, {"echo_type",0}, {"echo_sync",11}, {"echo_fb",0.5f}, {"echo_mix",0.35f}, {"echo_duck",1.0f},
                                   {"verb_type",0}, {"verb_decay",2}, {"verb_mix",0.18f} } },
        { "Orbit", kSpace, { {"comp",0.5f}, {"mod_on",1}, {"mod_mode",1}, {"mod_rate",0.2f}, {"mod_mix",0.5f}, {"echo_type",2}, {"echo_sync",10}, {"echo_fb",0.55f}, {"echo_mix",0.28f},
                             {"verb_type",1}, {"verb_decay",8}, {"verb_size",1}, {"verb_mix",0.45f}, {"out_width",1.5f} } },
        { "Frozen Lake", kSpace, { {"comp",0.45f}, {"echo_on",0}, {"verb_type",6}, {"verb_decay",30}, {"verb_char",0.0f}, {"verb_mod",0.5f}, {"verb_mix",0.55f}, {"verb_duck",0.0f} } },
        { "Init", kSpace, {} },
    };
}
} // namespace

const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> presets = build();
    return presets;
}

const std::vector<std::string>& presetBanks()
{
    static const std::vector<std::string> banks = { kDream, kShoe, kSynth, kVintage, kLofi, kSpace };
    return banks;
}

std::array<float, NumParams> presetValues (const Preset& p)
{
    std::array<float, NumParams> v {};
    const auto* d = paramDefs();
    for (int i = 0; i < NumParams; ++i) v[(size_t) i] = d[i].def;
    for (const auto& kv : p.values)
    {
        const int idx = paramIndex (kv.first.c_str());
        if (idx >= 0) v[(size_t) idx] = std::min (d[idx].max, std::max (d[idx].min, kv.second));
    }
    return v;
}
} // namespace dg
