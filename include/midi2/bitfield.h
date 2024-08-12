#ifndef MIDI2_BITFIELD_H
#define MIDI2_BITFIELD_H

#include <cassert>
#include <concepts>
#include <cstdint>
#include <limits>

namespace midi2 {

///\returns The maximum value that can be held in \p Bits bits of type \p T.
template <std::unsigned_integral T, unsigned Bits>
  requires(Bits <= sizeof(T) * 8 && Bits <= 64U)
constexpr T max_value() noexcept {
  if constexpr (Bits == 8U) {
    return std::numeric_limits<std::uint8_t>::max();
  } else if constexpr (Bits == 16U) {
    return std::numeric_limits<std::uint16_t>::max();
  } else if constexpr (Bits == 32U) {
    return std::numeric_limits<std::uint32_t>::max();
  } else if constexpr (Bits == 64U) {
    return std::numeric_limits<std::uint64_t>::max();
  } else {
    return static_cast<T>((T{1} << Bits) - 1U);
  }
}

/// \tparam ContainerType The underlying type used to store the bitfield.
/// \tparam Index The bit number used as the first bit of the bitfield. Index 0 is the least-significant bit.
/// \tparam Bits The number of bits to be allocated for this value.
template <std::unsigned_integral ContainerType, unsigned Index, unsigned Bits>
  requires(Bits > 0 && Index + Bits <= sizeof(ContainerType) * 8)
class bitfield {
public:
  /// The underlying type used to store the bitfield.
  using value_type = ContainerType;

  using small_type = std::conditional_t<
      Bits <= 8, std::uint8_t,
      std::conditional_t<
          Bits <= 16, std::uint16_t,
          std::conditional_t<Bits <= 32, std::uint32_t, value_type>>>;

  /// The index of the first bit used by the bitfield.
  static constexpr auto first_bit = Index;
  /// The index of the last bit used by the bitfield.
  static constexpr auto last_bit = Index + Bits;

  /// Returns the smallest value that can be stored.
  [[nodiscard]] static constexpr small_type min() noexcept { return 0; }
  /// Returns the largest value that can be stored.
  [[nodiscard]] static constexpr small_type max() noexcept { return mask_; }

  /// Returns the value stored in the bitfield.
  [[nodiscard]] constexpr small_type value() const noexcept { return (value_ >> Index) & mask_; }

  /// Returns the value stored in the bitfield as a signed quantity.
  [[nodiscard]] constexpr std::make_signed_t<small_type> signed_value() const noexcept {
    // Taken from Bit Twiddling Hacks by Sean Eron Anderson:
    // https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
    constexpr auto m = value_type{1} << (Bits - 1U);
    return static_cast<std::make_signed_t<small_type>>((this->value() ^ m) - m);
  }

  /// Obtains the value of the bitfield.
  [[nodicard]] constexpr operator small_type() const noexcept { return this->value(); }

  /// Assigns a value to the bitfield.
  /// \param v  The value to be stored.
  void assign(small_type const v) noexcept {
    assert(v <= this->max() && "Value too large for bitfield");
    value_ =
        static_cast<value_type>(value_ & ~(mask_ << Index)) |
        static_cast<value_type>((static_cast<value_type>(v) & mask_) << Index);
  }

  /// Assigns the value \p v to the bitfield.
  /// \param v  The value to be stored.
  /// \returns *this
  bitfield& operator=(small_type const v) noexcept {
    this->assign(v);
    return *this;
  }

private:
  static constexpr auto mask_ = max_value<ContainerType, Bits>();
  static_assert(mask_ <= std::numeric_limits<small_type>::max(),
                "small_type must be able to hold mask_");
  value_type value_;
};

}  // end namespace midi2

#endif  // MIDI2_BITFIELD_H
