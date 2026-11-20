#pragma once

// Forward declarations for engine types whose private members checkpoints save.  Such a type grants access with
//
//     template <typename>
//     friend struct checkpoint::Fields;
//
// so the generated field list of the type can name its members, or, for a type with a custom rule in
// checkpoint/rules.h,
//
//     template <typename, typename>
//     friend struct checkpoint::Rule;
//
// (see checkpoint/codec_impl.h).

namespace checkpoint {

template <typename T>
struct Fields;

template <typename T, typename Enable>
struct Rule;

} // namespace checkpoint
