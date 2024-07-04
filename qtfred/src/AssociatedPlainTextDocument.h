#ifndef ASSOCIATEDPLAINTEXTDOCUMENT_H
#define ASSOCIATEDPLAINTEXTDOCUMENT_H

#include <QPlainTextEdit>
#include <QPlainTextDocumentLayout>
#include <QTextDocument>

namespace fso {
namespace fred {

class AssociatedPlainTextDocument : public QTextDocument {
	Q_OBJECT
	QPlainTextEdit *m_edit{nullptr};

public:
	AssociatedPlainTextDocument(QObject *parent) :
		AssociatedPlainTextDocument("", parent)
	{}

	AssociatedPlainTextDocument(const QString &text = "", QObject *parent = nullptr) :
		QTextDocument(text, parent)
	{ setDocumentLayout(new QPlainTextDocumentLayout(this)); }

	~AssociatedPlainTextDocument() override { associateEdit(); /*disassociate*/ }

	void associateEdit(QPlainTextEdit *edit = nullptr, bool carry = false) {
		if (m_edit && (m_edit->document() == this))
			m_edit->setDocument(nullptr);
		if (! edit) return;
		if (carry && (m_edit != edit))
			setPlainText(edit->toPlainText());
		m_edit = edit;
		m_edit->setDocument(this);
	}

	inline operator QString() const { return toPlainText(); }
};
/*
 * Usage:
 * 1a) As parentless member of a model
 * 1b) In a container, pointers to this with the model (a QObject) as parent
 * 2) Initialize with parsed/loaded text, or setPlainText() it later
 * e.g. vec.emplace_back(new AssociatedPlainTextDocument(mytext, this);
 * 3) connect ::contentsChanged to Model::flagModified
 * 4) view passes a pointer of the edit to model, model passes the ptr to appropriate associateEdit
 *    whenever the displayed / editable text should change. Carrying of old text possible as QoL.
 *
 * --> manual updates of both model and view become unnecessary, only this submodel need be accessed
 * --> prevent long QString copy on keystroke & keystroke/UIupdate race
 *
 */
}
}

#endif // ASSOCIATEDPLAINTEXTDOCUMENT_H
