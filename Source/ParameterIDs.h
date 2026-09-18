#pragma once
#include <juce_core/juce_core.h>

namespace ParamIDs
{
    // --- Oscilador 1 ---
    inline constexpr auto osc1Wave   = "osc1_wave";
    inline constexpr auto osc1Pos    = "osc1_pos";
    inline constexpr auto osc1Octave = "osc1_octave";
    inline constexpr auto osc1Semi   = "osc1_semi";
    inline constexpr auto osc1Fine   = "osc1_fine";
    inline constexpr auto osc1Level  = "osc1_level";

    // --- Oscilador 2 ---
    inline constexpr auto osc2Wave   = "osc2_wave";
    inline constexpr auto osc2Pos    = "osc2_pos";
    inline constexpr auto osc2Octave = "osc2_octave";
    inline constexpr auto osc2Semi   = "osc2_semi";
    inline constexpr auto osc2Fine   = "osc2_fine";
    inline constexpr auto osc2Level  = "osc2_level";

    // --- Amp Envelope ---
    inline constexpr auto ampAttack  = "amp_attack";
    inline constexpr auto ampDecay   = "amp_decay";
    inline constexpr auto ampSustain = "amp_sustain";
    inline constexpr auto ampRelease = "amp_release";

    // --- Filter ---
    inline constexpr auto filterType     = "filter_type";
    inline constexpr auto filterCutoff   = "filter_cutoff";
    inline constexpr auto filterReso     = "filter_reso";
    inline constexpr auto filterEnvAmt   = "filter_env_amt";
    inline constexpr auto filterKeyTrack = "filter_key_track";

    // --- Filter Envelope ---
    inline constexpr auto filtAttack  = "filt_attack";
    inline constexpr auto filtDecay   = "filt_decay";
    inline constexpr auto filtSustain = "filt_sustain";
    inline constexpr auto filtRelease = "filt_release";

    // --- Master ---
    inline constexpr auto masterGain = "master_gain";

    // --- LFO 1 ---
    inline constexpr auto lfo1Wave  = "lfo1_wave";
    inline constexpr auto lfo1Rate  = "lfo1_rate";
    inline constexpr auto lfo1Depth = "lfo1_depth";
    inline constexpr auto lfo1Phase = "lfo1_phase";

    // --- LFO 2 ---
    inline constexpr auto lfo2Wave  = "lfo2_wave";
    inline constexpr auto lfo2Rate  = "lfo2_rate";
    inline constexpr auto lfo2Depth = "lfo2_depth";
    inline constexpr auto lfo2Phase = "lfo2_phase";

    // --- Matriz de Modulación (4 slots) ---
    inline constexpr auto mod1Source = "mod1_source";
    inline constexpr auto mod1Dest   = "mod1_dest";
    inline constexpr auto mod1Amount = "mod1_amount";

    inline constexpr auto mod2Source = "mod2_source";
    inline constexpr auto mod2Dest   = "mod2_dest";
    inline constexpr auto mod2Amount = "mod2_amount";

    inline constexpr auto mod3Source = "mod3_source";
    inline constexpr auto mod3Dest   = "mod3_dest";
    inline constexpr auto mod3Amount = "mod3_amount";

    inline constexpr auto mod4Source = "mod4_source";
    inline constexpr auto mod4Dest   = "mod4_dest";
    inline constexpr auto mod4Amount = "mod4_amount";
}
