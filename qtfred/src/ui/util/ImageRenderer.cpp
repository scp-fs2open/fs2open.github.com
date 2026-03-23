#include "ImageRenderer.h"

#include <bmpman/bmpman.h> // bm_load, bm_get_info, bm_lock, bm_unlock

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
	if (bm_get_info(bmHandle, &w, &h) < 0 || w <= 0 || h <= 0) {
		setError(outError, QStringLiteral("Bitmap has invalid info."));
		return false;
	}

	// All FSO animation types (ANI, APNG, EFF) produce BGRA byte-order data
	// at 32 bpp, which matches QImage::Format_ARGB32 on little-endian.
	auto* bmp = bm_lock(bmHandle, 32, BMP_TEX_XPARENT);
	if (bmp == nullptr || bmp->data == 0) {
		setError(outError, QStringLiteral("bm_lock failed."));
		return false;
	}

	// rowsize is stored in pixels; multiply by bytes-per-pixel for the Qt stride.
	const int bytesPerLine = bmp->w * (bmp->bpp >> 3);
	QImage tmp(reinterpret_cast<const uchar*>(bmp->data), bmp->w, bmp->h, bytesPerLine, QImage::Format_ARGB32);
	outImage = tmp.copy(); // detach from bmpman memory before unlock
	bm_unlock(bmHandle);

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
