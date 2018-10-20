
#pragma once

#include "scripting/ade_api.h"
#include "cfile/cfile.h"

namespace scripting {
namespace api {

class cfile_h {
	std::unique_ptr<CFILE> _cfp;

  public:
	explicit cfile_h(CFILE* cfp = nullptr);
	~cfile_h();

	cfile_h(cfile_h&& other) noexcept = default; // NOLINT
	cfile_h& operator=(cfile_h&& other) noexcept = default; // NOLINT

	void close();

	bool isValid() const;

	CFILE* get() const;
};

DECLARE_ADE_OBJ(l_File, cfile_h);

} // namespace api
} // namespace scripting
