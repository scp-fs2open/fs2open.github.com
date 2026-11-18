#pragma once

// Forward declarations for engine types whose private members checkpoints save.  Such a type grants access with
//
//     template <typename>
//     friend struct checkpoint::Fields;
//
// so the generated field list of the type can name its members (see checkpoint/codec.h).

namespace checkpoint {

template <typename T>
struct Fields;

template <typename T, typename Enable>
struct Codec;

} // namespace checkpoint
