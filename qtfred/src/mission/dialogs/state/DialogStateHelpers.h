#pragma once

// ---------------------------------------------------------------------------
// Shared helpers for dialog captureState() / restoreState() implementations.
//
// Each modal dialog's serialization lives in a dedicated
// qtfred/src/mission/dialogs/state/<Name>State.cpp file and includes this
// header. The QDataStream written by captureState() must be read back in
// exactly the same order by restoreState().
//
// SEXP stubs: fields backed by live SEXP node trees are not yet serialized.
// writeSexpStub() / readSexpStub() write/consume a sentinel so the stream
// stays valid. Replace with real serialization after sexp_tree_refactor.
// ---------------------------------------------------------------------------

#include <QByteArray>
#include <QDataStream>
#include <QString>

namespace fso::fred::state {

// TODO(sexp_tree_refactor): replace stub pair with real tree serialization.
inline void writeSexpStub(QDataStream& ds)
{
    ds << static_cast<qint32>(-1);
}

inline void readSexpStub(QDataStream& ds)
{
    qint32 sentinel;
    ds >> sentinel;
    // sentinel == -1 confirms this is an unimplemented SEXP slot
}

} // namespace fso::fred::state
