#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QXmlStreamWriter;
class QTabWidget;
class QTextEdit;
class TocItem;
class QAction;

class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_ENUMS(MetadataType)

public:

    enum MetadataType {
	title,
	creator,
	subject,
	description,
	publisher,
	contributor,
	date,
	type,
	format,
	identifier,
	source,
	language,
	relation,
	coverage,
	rights
    };

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QTextEdit *editor, *htmleditor;
    QStringList metadataLabels;

    QTreeWidget *metadata;
    QTreeWidget *manifest;
    QTreeWidget *toc;

    QTabWidget *tabs;

    void setupMenu();

    QTextEdit* currentEditor();
    QString currentHtml();
    void setCurrentHtml(QString html);

    void tryToReplaceIdWithText(TocItem *ti);
    void deleteTocReferencesTo(TocItem *ti, QString id);

    QAction *stripLinks;
    QAction *embedCharis;

private slots:
    void addMetadata();
    void removeMetadata();

    void newEpub();
    void open();

    void updateOtherEditor(int current);

    void addToManifest();
    void removeFromManifest();

    void addAllToToc();
    void removeFromToc();

    void saveEpub();

    void saveText(QTreeWidgetItem *current, QTreeWidgetItem *previous );

protected:

private:
    QString currentDirectory;
    QString toID(QString string);
    int writeNavPoint(QTreeWidgetItem *current, QXmlStreamWriter *ncx, int playOrder);
};

#endif // MAINWINDOW_H
