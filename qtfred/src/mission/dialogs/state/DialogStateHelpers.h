#pragma once

// ---------------------------------------------------------------------------
// Shared helpers for dialog captureState() / restoreState() implementations.
//
// Each modal dialog's serialization lives in a dedicated
// qtfred/src/mission/dialogs/state/<Name>State.cpp file and includes this
// header. The QDataStream written by captureState() must be read back in
// exactly the same order by restoreState().
//
// SEXP formulas are serialized structurally (the same walk as
// dup_sexp_chain, through the stream instead of the node pool), so a state
// blob is a self-contained copy with no live node references. readSexp()
// allocates a fresh chain in Sexp_nodes on every call; restore code must
// free the formula it is about to overwrite via freeSexpFormula() first.
// ---------------------------------------------------------------------------

#include <QByteArray>
#include <QDataStream>
#include <QString>

#include <parse/sexp.h>

namespace fso::fred::state {

// Stream markers for writeSexp/readSexp. Negative values are sentinels;
// SEXP_STREAM_NODE is followed by the node's text/type/subtype and children.
static constexpr qint32 SEXP_STREAM_NONE         = -1; // no formula (-1)
static constexpr qint32 SEXP_STREAM_LOCKED_TRUE  = -2; // Locked_sexp_true
static constexpr qint32 SEXP_STREAM_LOCKED_FALSE = -3; // Locked_sexp_false
static constexpr qint32 SEXP_STREAM_NODE         = 0;

inline void writeSexp(QDataStream& ds, int node)
{
	if (node < 0)                  { ds << SEXP_STREAM_NONE;         return; }
	if (node == Locked_sexp_true)  { ds << SEXP_STREAM_LOCKED_TRUE;  return; }
	if (node == Locked_sexp_false) { ds << SEXP_STREAM_LOCKED_FALSE; return; }

	ds << SEXP_STREAM_NODE;
	ds << QString::fromLatin1(Sexp_nodes[node].text);
	ds << static_cast<qint32>(Sexp_nodes[node].type);
	ds << static_cast<qint32>(Sexp_nodes[node].subtype);
	writeSexp(ds, Sexp_nodes[node].first);
	writeSexp(ds, Sexp_nodes[node].rest);
}

inline int readSexp(QDataStream& ds)
{
	qint32 marker;
	ds >> marker;
	if (marker == SEXP_STREAM_NONE)         return -1;
	if (marker == SEXP_STREAM_LOCKED_TRUE)  return Locked_sexp_true;
	if (marker == SEXP_STREAM_LOCKED_FALSE) return Locked_sexp_false;

	QString text;
	qint32 type, subtype;
	ds >> text >> type >> subtype;
	const int first = readSexp(ds);
	const int rest  = readSexp(ds);

	const int cur = alloc_sexp(text.toLatin1().constData(),
		static_cast<int>(type), static_cast<int>(subtype), first, rest);
	if (cur == -1) {
		if (first >= 0 && first != Locked_sexp_true && first != Locked_sexp_false)
			free_sexp(first);
		if (rest >= 0 && rest != Locked_sexp_true && rest != Locked_sexp_false)
			free_sexp(rest);
	}
	return cur;
}

// Free a live formula that restore is about to overwrite. No-op for -1 and
// the shared locked sexps.
inline void freeSexpFormula(int formula)
{
	if (formula >= 0 && formula != Locked_sexp_true && formula != Locked_sexp_false)
		free_sexp2(formula);
}

// Transitional stubs — removed once every state file uses writeSexp/readSexp.
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
