#include "ImageRenderer.h"

#include <bmpman/bmpman.h> // bm_load, bm_get_info, bm_lock, bm_unlock
#include <ddsutils/ddsutils.h>

#include <QtGlobal>

namespace fso::fred::util {

static void setError(QString* outError, const QString& text)
{
	if (outError)
		*outError = text;
}

// bm_lock_dds keeps compressed data as-is when the renderer reports s3tc/BPTC
// support, which would crash the regular 32-bpp QImage path. For the picker
// preview, ask ddsutils to decompress the top mip directly.
static bool decompressDdsToQImage(const char* bm_filename, QImage& outImage, QString* outError)
{
	int w = 0, h = 0;
	SCP_vector<ubyte> pixels;
	const int err = dds_decompress_top_mip_bgra(bm_filename, CF_TYPE_ANY, &w, &h, pixels);
	if (err != DDS_ERROR_NONE) {
		setError(outError, QStringLiteral("DDS decompress failed (%1).").arg(err));
		return false;
	}

	QImage tmp(pixels.data(), w, h, w * 4, QImage::Format_ARGB32);
	outImage = tmp.copy(); // detach before `pixels` goes out of scope
	return !outImage.isNull();
}

bool loadHandleToQImage(int bmHandle, QImage& outImage, QString* outError)
{
	outImage = QImage(); // clear

	if (bmHandle < 0) {
		setError(outError, QStringLiteral("Invalid bitmap handle."));
		return false;
	}

	if (bm_is_compressed(bmHandle)) {
		const char* fname = bm_get_filename(bmHandle);
		if (fname && *fname)
			return decompressDdsToQImage(fname, outImage, outError);
		setError(outError, QStringLiteral("Compressed DDS with no filename; cannot preview."));
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

	// bm_lock_dds also doesn't honor the requested bpp for uncompressed DDS
	// (e.g. 24-bpp RGB files), which would have us read past the buffer.
	if (bmp->bpp != 32) {
		bm_unlock(bmHandle);
		setError(outError, QStringLiteral("Unsupported bitmap bpp (%1) for QImage preview.").arg(bmp->bpp));
		return false;
	}

	const int bytesPerLine = bmp->w * 4;
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

	// bm_unload is load_count aware, so if another
	// part of qtfred is sharing the handle it stays alive for them.
	bm_unload(handle);

	return ok;
}

} // namespace fso::fred::util
