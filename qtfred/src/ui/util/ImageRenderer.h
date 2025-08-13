#pragma once

#include <QImage>
#include <QString>
#include <string>

namespace fso::fred::util {

/**
 * @brief Loads an image file (any format FSO supports) into a QImage for UI preview.
 * @param filename Path or VFS name relative to FSO search paths.
 * @param outImage On success, receives a valid QImage copy.
 * @param outError Optional: receives any error as a string.
 * @return true on success, false otherwise.
 */
bool loadImageToQImage(const std::string& filename, QImage& outImage, QString* outError = nullptr);

/**
 * @brief Same as above but using an existing bmpman handle.
 *        Useful if the caller already called bm_load().
 */
bool loadHandleToQImage(int bmHandle, QImage& outImage, QString* outError = nullptr);

} // namespace fso::fred::util
