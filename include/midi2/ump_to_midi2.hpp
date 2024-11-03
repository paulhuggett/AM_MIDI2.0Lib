//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#ifndef MIDI2_UMP_TO_MIDI2_HPP
#define MIDI2_UMP_TO_MIDI2_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "midi2/adt/fifo.hpp"
#include "midi2/ump_dispatcher.hpp"
#include "midi2/ump_types.hpp"
#include "midi2/utils.hpp"

namespace midi2 {

template <typename T> struct bits {};

template <> struct bits<std::uint32_t> : std::integral_constant<unsigned, 32> {};
template <> struct bits<std::uint16_t> : std::integral_constant<unsigned, 16> {};
template <> struct bits<std::uint8_t> : std::integral_constant<unsigned, 6> {};

template <typename T> struct bits<T &> : bits<std::remove_cvref_t<T>> {};

template <std::unsigned_integral Container, unsigned Index, unsigned Bits>
struct bits<bitfield<Container, Index, Bits>>
    : std::integral_constant<unsigned, bitfield<Container, Index, Bits>::bits()> {};

template <typename T> static constexpr auto bits_v = bits<T>{}();

class ump_to_midi2 {
public:
  using input_type = std::uint32_t;
  using output_type = std::uint32_t;

  constexpr ump_to_midi2() = default;
  explicit constexpr ump_to_midi2(std::uint8_t const default_group) : defaultGroup_{default_group} {
    assert(default_group <= 0b1111);
  }

  [[nodiscard]] constexpr bool empty() const { return context_.output.empty(); }
  [[nodiscard]] constexpr output_type pop() {
    assert(!context_.output.empty());
    return context_.output.pop_front();
  }

  void push(input_type const ump) { p_.processUMP(ump); }

private:
  static constexpr auto unknown = std::byte{0xFF};
  std::byte defaultGroup_ = std::byte{0};

  struct sysex7 {
    enum class status : std::uint8_t {
      single_ump = 0x0,  ///< A complete sysex message in one UMP
      start = 0x1,       ///< Sysex start
      cont = 0x02,       ///< Sysex continue UMP. There might be multiple 'cont' UMPs in a single message.
      end = 0x03,        ///< Sysex end
    };
    status state = status::single_ump;
    /// The number of system exclusive bytes in the current UMP [0,6]
    std::uint8_t pos = 0;
    /// System exclusive message bytes gathered for the current UMP
    std::array<std::byte, 6> bytes{};

    void reset() { std::ranges::fill(bytes, std::byte{0}); }
  };
  sysex7 sysex7_;
  // fifo<std::uint32_t, 4> output_{};

  // Channel Based Data
  struct channel {
    std::byte bankMSB = std::byte{0xFF};
    std::byte bankLSB = std::byte{0xFF};
    bool rpnMode = true;
    std::byte rpnMsbValue = std::byte{0xFF};
    std::byte rpnMsb = std::byte{0xFF};
    std::byte rpnLsb = std::byte{0xFF};
  };
  std::array<channel, 16> channel_{};

  [[nodiscard]] static constexpr std::uint32_t pack(std::byte const b0, std::byte const b1, std::byte const b2,
                                                    std::byte const b3) {
    return (std::to_integer<std::uint32_t>(b0) << 24) | (std::to_integer<std::uint32_t>(b1) << 16) |
           (std::to_integer<std::uint32_t>(b2) << 8) | std::to_integer<std::uint32_t>(b3);
  }

  [[nodiscard]] constexpr std::uint32_t pack(ump_message_type const message_type, std::byte const b1,
                                             std::byte const b2, std::byte const b3) const {
    return pack((static_cast<std::byte>(message_type) << 4) | defaultGroup_, b1, b2, b3);
  }

  void controllerToUMP(std::byte b0, std::byte b1, std::byte b2);
  void bsToUMP(std::byte b0, std::byte b1, std::byte b2);

  struct context_type {
    template <typename T, unsigned Index = 0>
      requires(std::tuple_size_v<T> >= 0)
    constexpr void push(T const &value) {
      if constexpr (Index >= std::tuple_size_v<T>) {
        return;
      } else {
        auto const value32 = std::bit_cast<std::uint32_t>(std::get<Index>(value));
        output.push_back(value32);
        push<T, Index + 1>(value);
      }
    }

    fifo<std::uint32_t, 4> output;
  };

  struct to_midi2_config {
    // utility messages go straight through.
    struct utility {
      static constexpr void noop(context_type *const ) { /* TODO: pass this on? */ }
      static constexpr void jr_clock(context_type *const ctxt, types::utility::jr_clock const &in) { ctxt->push(in.w); }
      static constexpr void jr_timestamp(context_type *const ctxt, types::utility::jr_timestamp const &in) { ctxt->push(in.w); }
      static constexpr void delta_clockstamp_tpqn(context_type *const ctxt, types::utility::delta_clockstamp_tpqn const &in) { ctxt->push(in.w); }
      static constexpr void delta_clockstamp(context_type *const ctxt, types::utility::delta_clockstamp const &in) { ctxt->push(in.w); }
      static constexpr void unknown(context_type *const , std::span<std::uint32_t> ) { /* TODO: pass this on? */ }
    };
    // system messages go straight through.
    struct system {
      static constexpr void midi_time_code(context_type *const ctxt, types::system::midi_time_code const &in) { ctxt->push(in.w); }
      static constexpr void song_position_pointer(context_type *const ctxt, types::system::song_position_pointer const &in) { ctxt->push(in.w); }
      static constexpr void song_select(context_type *const ctxt, types::system::song_select const &in) { ctxt->push(in.w); }
      static constexpr void tune_request(context_type *const ctxt, types::system::tune_request const &in) { ctxt->push(in.w); }
      static constexpr void timing_clock(context_type *const ctxt, types::system::timing_clock const &in) { ctxt->push(in.w); }
      static constexpr void seq_start(context_type *const ctxt, types::system::sequence_start const &in) { ctxt->push(in.w); }
      static constexpr void seq_continue(context_type *const ctxt, types::system::sequence_continue const &in) { ctxt->push(in.w); }
      static constexpr void seq_stop(context_type *const ctxt, types::system::sequence_stop const &in) { ctxt->push(in.w); }
      static constexpr void active_sensing(context_type *const ctxt, types::system::active_sensing const &in) { ctxt->push(in.w); }
      static constexpr void reset(context_type *const ctxt, types::system::reset const &in) { ctxt->push(in.w); }
    };
    // m1cvm messages are converted to m2cvm messages.
    struct m1cvm {
      static void note_off(context_type *const ctxt, types::m1cvm::note_off const &in) {
        auto const & noff_in = get<0>(in.w);

        types::m2cvm::note_off noff;
        auto & w0 = get<0>(noff.w);
        auto & w1 = get<1>(noff.w);
        w0.group = noff_in.group.value();
        w0.channel = noff_in.channel.value();
        w0.note = noff_in.note.value();
        w0.attribute = 0;
        constexpr auto m1bits = bits_v<decltype(noff_in.velocity)>;
        constexpr auto m2bits = bits_v<decltype(w1.velocity)>;
        w1.velocity = mcm_scale<m1bits, m2bits>(noff_in.velocity);
        w1.attribute = 0;
        ctxt->push(noff.w);
      }
      static void note_on(context_type *const ctxt, types::m1cvm::note_on const &in) {
        auto const &non_in = get<0>(in.w);

        types::m2cvm::note_on non;
        auto &w0 = get<0>(non.w);
        auto &w1 = get<1>(non.w);
        w0.group = non_in.group.value();
        w0.channel = non_in.channel.value();
        w0.note = non_in.note.value();
        w0.attribute = 0;
        constexpr auto m1bits = bits_v<decltype(non_in.velocity)>;
        constexpr auto m2bits = bits_v<decltype(w1.velocity)>;
        w1.velocity = mcm_scale<m1bits, m2bits>(non_in.velocity);
        w1.attribute = 0;
        ctxt->push(non.w);
      }
      static void poly_pressure(context_type *const ctxt, types::m1cvm::poly_pressure const &in) {
        auto const &pp_in = get<0>(in.w);

        types::m2cvm::poly_pressure out;
        auto &w0 = get<0>(out.w);
        auto &w1 = get<1>(out.w);
        w0.group = pp_in.group.value();
        w0.channel = pp_in.channel.value();
        w0.note = pp_in.note.value();
        constexpr auto m1bits = bits_v<decltype(pp_in.pressure)>;
        constexpr auto m2bits = bits_v<decltype(w1)>;
        w1 = mcm_scale<m1bits, m2bits>(pp_in.pressure);
        ctxt->push(out.w);
      }
      static constexpr void control_change(context_type *const ctxt, types::m1cvm::control_change const & in) {
        /* TODO: implement! */
      }
      static constexpr void program_change(context_type *const ctxt, types::m1cvm::program_change const &in) {
        /* TODO: implement! */
      }
      static constexpr void channel_pressure(context_type *const ctxt, types::m1cvm::channel_pressure const &in) {
        /* TODO: implement! */
      }
      static constexpr void pitch_bend(context_type *const ctxt, types::m1cvm::pitch_bend const &in) {
        auto const &pb_in = get<0>(in.w);

        types::m2cvm::pitch_bend out;
        auto &w0 = get<0>(out.w);
        auto &w1 = get<1>(out.w);
        w0.group = pb_in.group.value();
        w0.channel = pb_in.channel.value();
        constexpr auto lsb_bits = bits_v<decltype(pb_in.lsb_data)>;
        constexpr auto msb_bits = bits_v<decltype(pb_in.msb_data)>;
        constexpr auto m2bits = bits_v<decltype(w1)>;
        w1 = mcm_scale<lsb_bits + msb_bits, m2bits>((pb_in.msb_data << lsb_bits) | pb_in.lsb_data);
        ctxt->push(out.w);
      }
    };
    // data64 messages go straight through.
    struct data64 {
      static constexpr void sysex7_in_1(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
      static constexpr void sysex7_start(context_type *const ctxt, types::data64::sysex7 const &in) {
        ctxt->push(in.w);
      }
      static constexpr void sysex7_continue(context_type *const ctxt, types::data64::sysex7 const &in) {
        ctxt->push(in.w);
      }
      static constexpr void sysex7_end(context_type *const ctxt, types::data64::sysex7 const &in) { ctxt->push(in.w); }
    };
    // m2cvm messages go straight through.
    struct m2cvm {
    public:
      static constexpr void note_off(context_type *const ctxt, types::m2cvm::note_off const &in) { ctxt->push(in.w); }
      static constexpr void note_on(context_type *const ctxt, types::m2cvm::note_on const &in) { ctxt->push(in.w); }
      static constexpr void poly_pressure(context_type *const ctxt, types::m2cvm::poly_pressure const &in) { ctxt->push(in.w); }
      static constexpr void program_change(context_type *const ctxt, types::m2cvm::program_change const &in) { ctxt->push(in.w); }
      static constexpr void channel_pressure(context_type *const ctxt, types::m2cvm::channel_pressure const &in) { ctxt->push(in.w); }
      static constexpr void rpn_controller(context_type *const ctxt, types::m2cvm::rpn_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_controller(context_type *const ctxt, types::m2cvm::nrpn_controller const &in){ ctxt->push(in.w); }
      static constexpr void rpn_per_note_controller(context_type * const ctxt, midi2::types::m2cvm::rpn_per_note_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_per_note_controller(context_type * const ctxt, midi2::types::m2cvm::nrpn_per_note_controller const &in) { ctxt->push(in.w); }
      static constexpr void rpn_relative_controller(context_type * const ctxt, midi2::types::m2cvm::rpn_relative_controller const &in) { ctxt->push(in.w); }
      static constexpr void nrpn_relative_controller(context_type * const ctxt, midi2::types::m2cvm::nrpn_relative_controller const &in) { ctxt->push(in.w); }
      static constexpr void per_note_management(context_type * const ctxt, midi2::types::m2cvm::per_note_management const &in) { ctxt->push(in.w); }
      static constexpr void control_change(context_type * const ctxt, midi2::types::m2cvm::control_change const &in) { ctxt->push(in.w); }
      static constexpr void pitch_bend(context_type * const ctxt, midi2::types::m2cvm::pitch_bend const &in) { ctxt->push(in.w); }
      static constexpr void per_note_pitch_bend(context_type * const ctxt, midi2::types::m2cvm::per_note_pitch_bend const &in) { ctxt->push(in.w); }
    };
    context_type *context = nullptr;
    [[no_unique_address]] struct utility utility{};
    [[no_unique_address]] struct system system{};
    [[no_unique_address]] struct m1cvm m1cvm{};
    [[no_unique_address]] struct data64 data64{};
    [[no_unique_address]] struct m2cvm m2cvm{};
    [[no_unique_address]] data128_null<decltype(context)> data128{};
    [[no_unique_address]] ump_stream_null<decltype(context)> ump_stream{};
    [[no_unique_address]] flex_data_null<decltype(context)> flex{};
  };

  context_type context_;
  ump_dispatcher<to_midi2_config> p_{to_midi2_config{&context_}};
};

}  // end namespace midi2

#endif  // MIDI2_UMP_TO_MIDI2_HPP
