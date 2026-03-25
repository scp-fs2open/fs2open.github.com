#pragma once

#include <QDialog>
#include <QString>
#include <QList>

#include <memory>

namespace fso::fred::dialogs {

namespace Ui {
class TemplateBrowserDialog;
}

struct TemplateFileInfo {
	QString filepath;
	QString title;       // $Template Title, or filename stem if not found
	QString author;
	QString tags;
	QString description;
};

class TemplateBrowserDialog : public QDialog {
	Q_OBJECT

  public:
	explicit TemplateBrowserDialog(QWidget* parent, const QString& templatesDir);
	~TemplateBrowserDialog() override;

	/** Returns the full path of the selected template, or empty if none selected. */
	QString selectedTemplatePath() const;

  private:
	void onSearchChanged(const QString& text);
	void onSelectionChanged();
	void onItemDoubleClicked();
	void populateList(const QString& filter = {});
	void showDetails(const TemplateFileInfo& info);
	void clearDetails();

	static TemplateFileInfo readTemplateInfo(const QString& filepath);
	static QList<TemplateFileInfo> scanTemplates(const QString& dir);

	QList<TemplateFileInfo> _templates;
	std::unique_ptr<Ui::TemplateBrowserDialog> ui;
};

} // namespace fso::fred::dialogs
