#include "ImageRenderer.h"

#include <bmpman/bmpman.h> // bm_load, bm_get_info, bm_has_alpha_channel
#include <graphics/2d.h>

#include <QtGlobal>

namespace fso::fred::util {

static void setError(QString* outError, const QString& text)
{
	if (outError)
		*outError = text;
}

bool loadHandleToQImage(int bmHandle, QImage& outImage, QString* outError)
{
	outImage = QImage(); // clear

	if (bmHandle < 0) {
		setError(outError, QStringLiteral("Invalid bitmap handle."));
		return false;
	}

	int w = 0, h = 0;
	uint flags = 0;
	int nframes = 0, fps = 0;

	// Use the returned handle (first frame if this is an animation.. TODO: Handle animations. Will be useful for Heads)
	int srcHandle = bm_get_info(bmHandle, &w, &h, &flags, &nframes, &fps);
	if (srcHandle < 0 || w <= 0 || h <= 0) {
		setError(outError, QStringLiteral("Bitmap has invalid info."));
		return false;
	}

	if (w <= 0 || h <= 0) {
		setError(outError, QStringLiteral("Bitmap has invalid dimensions."));
		return false;
	}

	const bool hasAlpha = bm_has_alpha_channel(bmHandle);
	const int channels = hasAlpha ? 4 : 3;
	const size_t bufSize = static_cast<size_t>(w) * static_cast<size_t>(h) * channels;

	// Allocate a temporary buffer and let the renderer copy pixels into it
	QByteArray buffer;
	buffer.resize(static_cast<int>(bufSize));
	if (buffer.size() != static_cast<int>(bufSize)) {
		setError(outError, QStringLiteral("Out of memory allocating pixel buffer."));
		return false;
	}

	// Copy RGBA pixels into the buffer
	gr_get_bitmap_from_texture(buffer.data(), bmHandle);

	// Build QImage by copying to own memory
	if (hasAlpha) {
		QImage tmp(reinterpret_cast<const uchar*>(buffer.constData()), w, h, QImage::Format_RGBA8888);
		outImage = tmp.copy();
	} else {
		QImage tmp(reinterpret_cast<const uchar*>(buffer.constData()), w, h, QImage::Format_RGB888);
		outImage = tmp.copy();
	}

	if (outImage.isNull()) {
		setError(outError, QStringLiteral("Failed to construct QImage."));
		return false;
	}

	return true;
}

bool loadImageToQImage(const std::string& filename, QImage& outImage, QString* outError)
{
	outImage = QImage();

	if (filename.empty()) {
		setError(outError, QStringLiteral("Empty filename."));
		return false;
	}

	// Let bmpman resolve the file
	int handle = bm_load(filename.c_str());
	if (handle < 0) {
		setError(outError, QStringLiteral("bm_load failed for \"%1\".").arg(QString::fromStdString(filename)));
		return false;
	}

	const bool ok = loadHandleToQImage(handle, outImage, outError);


	// bm_unload(handle); TODO test unloading

	return ok;
}

} // namespace fso::fred::util
