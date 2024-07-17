#ifndef FIFO_H
#define FIFO_H

#include <array>
#include <cassert>
#include <utility>

namespace M2Utils {

/// \tparam ElementType The type of the elements held by this container.
/// \tparam Elements The number of elements in the FIFO. Must be less than 2^32.
template <typename ElementType, std::uint32_t Elements> class fifo {
  static_assert(Elements > 0 && Elements < std::uint32_t{1} << 31,
                "Number of elements (e) must be : 0 < e > 2^32");

public:
  fifo()
      : writeIndex_{0}, writeIndexWrap_{0}, readIndex_{0}, readIndexWrap_{0} {}
  /// \brief Inserts an element at the end.
  /// \param value  The value of the element to append.
  /// \returns True if the element was appended, false if the container was full.
  bool push_back(ElementType const& value) {
    if (this->full()) {
      return false;
    }
    arr_[writeIndex_++] = value;
    if (writeIndex_ >= Elements) {
      writeIndex_ = 0U;
      // Flip the bit indicating that we've wrapped round.
      writeIndexWrap_ = !writeIndexWrap_;
    }
    return true;
  }
  /// \brief Removes the first element of the container and returns it.
  /// If there are no elements in the container, the behavior is undefined.
  ElementType pop_front() {
    assert(!this->empty());
    auto const value = std::move(arr_[readIndex_++]);
    if (readIndex_ >= Elements) {
      readIndex_ = 0U;
      readIndexWrap_ = !readIndexWrap_;
    }
    return value;
  }
  /// \brief Checks whether the container is empty.
  /// The FIFO is empty when both indices, including the "wrap_" fields are
  /// equal. \returns True if the container is empty, false otherwise.
  constexpr bool empty() const {
    return writeIndex_ == readIndex_ && writeIndexWrap_ == readIndexWrap_;
  }
  /// \brief Checks whether the container is full.
  /// The FIFO is full then when both indices are equal but the "wrap_" fields
  /// different. \returns True if the container is full, false otherwise.
  constexpr bool full() const {
    return writeIndex_ == readIndex_ && writeIndexWrap_ != readIndexWrap_;
  }
  /// \brief Returns the number of elements.
  constexpr std::size_t size() const {
    return writeIndex_ + (writeIndexWrap_ != readIndexWrap_ ? Elements : 0U) -
           readIndex_;
  }
  /// \brief Returns the maximum possible number of elements.
  constexpr std::size_t max_size() const { return Elements; }

private:
  /// Returns the number of bits required for value.
  template <typename T, typename = typename std::enable_if<
                            std::is_unsigned<T>::value>::type>
  static constexpr unsigned bitsRequired(T const value) {
    return value == 0U ? 0U : 1U + bitsRequired(static_cast<T>(value >> 1U));
  }

  std::array<ElementType, Elements> arr_{};

  static constexpr auto bits = bitsRequired(Elements);
  using bitfieldType = typename std::conditional<
      (bits < 4U), std::uint8_t,
      typename std::conditional<(bits < 8U), std::uint16_t,
                                std::uint32_t>::type>::type;

  bitfieldType writeIndex_ : bits;
  bitfieldType writeIndexWrap_ : 1;
  bitfieldType readIndex_ : bits;
  bitfieldType readIndexWrap_ : 1;
};

}  // end namespace M2Utils

#endif  // FIFO_H
