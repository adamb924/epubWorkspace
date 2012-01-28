#include "mainwindow.h"

#include <QtGui>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>
#include <QRegExp>
#include <QProcess>

#include <QXmlQuery>
#include <QXmlSerializer>
#include <QtXml>
#include <QtDebug>
#include <QXmlStreamReader>

#include "manifestitem.h"
#include "tocitem.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QString missingFiles = "";
    if( !QFile::exists( QDir::current().absoluteFilePath("tidy.exe") ) )
        missingFiles += "tidy.exe, ";
    if( !QFile::exists( QDir::current().absoluteFilePath("zip.exe") ) )
        missingFiles += "zip.exe, ";
    if( !QFile::exists( QDir::current().absoluteFilePath("unzip.exe") ) )
        missingFiles += "unzip.exe, ";
    if( !missingFiles.isEmpty() )
    {
        QMessageBox::critical(this,tr("Dreadful error"),tr("The following file(s) need to be with the application: %1").arg(missingFiles));
        this->close();
        return;
    }

    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    QHBoxLayout *hlayout = new QHBoxLayout;
    QVBoxLayout *vlayout = new QVBoxLayout;

//    qDebug() << "MainWindow::MainWindow" << "here";

    QHBoxLayout *hlayout_metadata = new QHBoxLayout;
    QPushButton *addMetadata, *removeMetadata;
    addMetadata = new QPushButton(tr("Add"));
    removeMetadata = new QPushButton(tr("Remove"));
    hlayout_metadata->addStretch(1);
    hlayout_metadata->addWidget(addMetadata,0);
    hlayout_metadata->addWidget(removeMetadata,0);
    hlayout_metadata->addStretch(1);

    QHBoxLayout *hlayout_manifest = new QHBoxLayout;
    QPushButton *addToManifest, *removeFromManifest;
    addToManifest = new QPushButton(tr("Add"));
    removeFromManifest = new QPushButton(tr("Remove"));
    hlayout_manifest->addStretch(1);
    hlayout_manifest->addWidget(addToManifest,0);
    hlayout_manifest->addWidget(removeFromManifest,0);
    hlayout_manifest->addStretch(1);

    QHBoxLayout *hlayout_toc = new QHBoxLayout;
    QPushButton *removeFromToc = new QPushButton(tr("Remove"));
    QPushButton *addAllToToc = new QPushButton(tr("Add all"));
    hlayout_toc->addStretch(1);
    hlayout_toc->addWidget(addAllToToc,0);
    hlayout_toc->addWidget(removeFromToc,0);
    hlayout_toc->addStretch(1);

    metadata = new QTreeWidget;
    manifest = new QTreeWidget;
    toc = new QTreeWidget;

    manifest->setHeaderHidden(true);
    toc->setHeaderHidden(true);

    manifest->setSelectionMode(QAbstractItemView::SingleSelection);
    manifest->setDragEnabled(true);
    manifest->setAcceptDrops(false);
    manifest->setDropIndicatorShown(true);

    toc->setDragEnabled(true);
    toc->viewport()->setAcceptDrops(true);
    toc->setDragDropMode( QAbstractItemView::DragDrop );
    toc->setEditTriggers(QAbstractItemView::CurrentChanged);

    QLabel *tmp;

    QGroupBox *metadataGroupBox = new QGroupBox(tr("Metadata"));
    QVBoxLayout *metadataLayout = new QVBoxLayout;
    tmp = new QLabel(tr("<i>This is information about your book click \"Add\" to start adding items. (Required: title, language, identifier)</i>"));
    tmp->setWordWrap(true);
    metadataLayout->addWidget(tmp);
    metadataLayout->addWidget(metadata);
    metadataLayout->addLayout(hlayout_metadata);
    metadataGroupBox->setLayout(metadataLayout);
    vlayout->addWidget(metadataGroupBox);

    QGroupBox *manifestGroupBox = new QGroupBox(tr("Manifest"));
    QVBoxLayout *manifestLayout = new QVBoxLayout;
    tmp = new QLabel(tr("<i>This is everything that is in your book. Click \"Add\" and type in a chapter name (e.g., Chapter 1). Then put the content in at the window on the right.</i>"));
    tmp->setWordWrap(true);
    manifestLayout->addWidget(tmp);
    manifestLayout->addWidget(manifest);
    manifestLayout->addLayout(hlayout_manifest);
    manifestGroupBox->setLayout(manifestLayout);
    vlayout->addWidget(manifestGroupBox);

    QGroupBox *tocGroupBox = new QGroupBox(tr("Table of Contents"));
    QVBoxLayout *tocLayout = new QVBoxLayout;
    tmp = new QLabel(tr("<i>What's in the manifest is not automatically in the table of contents. Drag items from the manifest to this window to create a table of contents.</i>"));
    tmp->setWordWrap(true);
    tocLayout->addWidget(tmp);
    tocLayout->addWidget(toc);
    tocLayout->addLayout(hlayout_toc);
    tocGroupBox->setLayout(tocLayout);
    vlayout->addWidget(tocGroupBox);

    editor = new QTextEdit;
    htmleditor = new QTextEdit;
    htmleditor->setAcceptRichText(false);

    tabs = new QTabWidget;
    tabs->addTab(editor,tr("Text Editor"));
    tabs->addTab(htmleditor,tr("HTML Editor"));
    connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(updateOtherEditor(int)));

    tabs->setEnabled(false);


    hlayout->addLayout(vlayout,1);
    hlayout->addWidget(tabs,5);

    widget->setLayout(hlayout);

//  metadata labels
    metadataLabels << "title";
    metadataLabels << "creator";
    metadataLabels << "subject";
    metadataLabels << "description";
    metadataLabels << "publisher";
    metadataLabels << "contributor";
    metadataLabels << "date";
    metadataLabels << "type";
    metadataLabels << "format";
    metadataLabels << "identifier";
    metadataLabels << "source";
    metadataLabels << "language";
    metadataLabels << "relation";
    metadataLabels << "coverage";
    metadataLabels << "rights";

    QStringList metadataWidgetHeaderLabels;
    metadataWidgetHeaderLabels << tr("Type") << tr("Value");
    metadata->setHeaderLabels(metadataWidgetHeaderLabels);
    metadata->setColumnWidth(0,75);
    metadata->setColumnWidth(1,75);

    // metadata connections
    connect(addMetadata,SIGNAL(clicked()),this,SLOT(addMetadata()));
    connect(removeMetadata,SIGNAL(clicked()),this,SLOT(removeMetadata()));

    // manifest connections
    connect(addToManifest,SIGNAL(clicked()),this,SLOT(addToManifest()));
    connect(removeFromManifest,SIGNAL(clicked()),this,SLOT(removeFromManifest()));
    connect(manifest,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT(saveText(QTreeWidgetItem*,QTreeWidgetItem*)));

    // spine connections
    connect(removeFromToc,SIGNAL(clicked()),this,SLOT(removeFromToc()));
    connect(addAllToToc,SIGNAL(clicked()),this,SLOT(addAllToToc()));

    setupMenu();
    this->setWindowTitle(tr("ePub Workspace"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupMenu()
{
    QMenu *file = new QMenu(tr("File"));
    menuBar()->addMenu(file);

    file->addAction(tr("New ePub"),this,SLOT(newEpub()),QKeySequence::New);
    file->addAction(tr("Open ePub"),this,SLOT(open()),QKeySequence::Open);
    file->addAction(tr("Save ePub"),this,SLOT(saveEpub()),QKeySequence::Save);
    file->addSeparator();
    file->addAction(tr("Quit"),this,SLOT(close()),QKeySequence::Quit);

    QMenu *options = new QMenu(tr("Options"));
    menuBar()->addMenu(options);

    stripLinks = new QAction(tr("Strip links before saving"),options);
    stripLinks->setToolTip(tr("This prevents certain catastrophic ePub errors."));
    stripLinks->setCheckable(true);
    stripLinks->setChecked(true);

    embedCharis = new QAction(tr("Embed Charis SIL"),options);
    embedCharis->setToolTip(tr("Charis reads well on devices"));
    embedCharis->setCheckable(true);
    embedCharis->setChecked(true);

    options->addAction(stripLinks);
    options->addAction(embedCharis);
}

QTextEdit* MainWindow::currentEditor()
{
    return (QTextEdit*)tabs->currentWidget();
}

QString MainWindow::currentHtml()
{
    if( tabs->currentIndex() == 0 ) // rich text
	return ((QTextEdit*)tabs->currentWidget())->toHtml();
    else if( tabs->currentIndex() == 1 ) // html
	return ((QTextEdit*)tabs->currentWidget())->toPlainText();

    return QString("");
}

void MainWindow::setCurrentHtml(QString html)
{
    if( tabs->currentIndex() == 0 ) // rich text
	((QTextEdit*)tabs->currentWidget())->setHtml(html);
    else if( tabs->currentIndex() == 1 ) // html
	((QTextEdit*)tabs->currentWidget())->setPlainText(html);
}

void MainWindow::addMetadata()
{
    bool ok;
    int type;
    QString text;
    QString item = QInputDialog::getItem(this, tr("Add metadata item"),
					 tr("Metadata Type:"), metadataLabels, 0, false, &ok);
    if (ok && !item.isEmpty())
    {
	text = QInputDialog::getText(this, tr("Add metadata item"),
					     tr("Value of ")+item, QLineEdit::Normal,
					     "", &ok);
	if (ok && !text.isEmpty())
	{
	    type = metadataLabels.indexOf(item);

	    QStringList tmp;
	    tmp << metadataLabels.at(type) << text;
	    metadata->addTopLevelItem(new QTreeWidgetItem(tmp,type+1000));
	}
    }
}

void MainWindow::removeMetadata()
{
    delete metadata->currentItem();
}

void MainWindow::addToManifest()
{
    bool ok;
    int i;
    QString text = QInputDialog::getText(this, tr("Add a file"),
					 tr("Enter a unique identifier that does not begin with a number and is not differentiated from other identifiers only by spaces."), QLineEdit::Normal,
					 "", &ok);
    if (ok && !text.isEmpty())
    {
	if(text.indexOf(QRegExp("^[0-9]")) != -1)
	{
            QMessageBox::critical(this,tr("Error"),tr("No, it can't begin with a number."));
	    return;
	}
	ok = true;

	for(i=0; i<manifest->topLevelItemCount(); i++)
	{
	    if( toID(text) == toID(manifest->topLevelItem(i)->text(0)) )
	    {
		ok = false;
		break;
	    }
	}
	if(!ok)
	    QMessageBox::critical(this,tr("Error"),tr("No, you must enter a <i>unique</i> identifier, different from all the others, which is not differentiated from other identifiers only by spaces."));
	else
	{
            ManifestItem *tmp = new ManifestItem( toID(text) , toID(text) + ".xml", "" );
            tmp->setText(0, text);
            manifest->addTopLevelItem(tmp);
	    manifest->setCurrentItem(manifest->topLevelItem(manifest->topLevelItemCount()-1));
	    tabs->setEnabled(true);
	}
    }
}

void MainWindow::removeFromManifest()
{
    ManifestItem *item = (ManifestItem*)manifest->currentItem();
    for(int i=0; i<toc->topLevelItemCount(); i++)
        deleteTocReferencesTo( (TocItem*)toc->topLevelItem(i) , item->id );

//    qDebug() << "deleting" << item;
    delete item;
//    qDebug() << "deleted" << item;
}

void MainWindow::saveText(QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
//    qDebug() << "MainWindow::saveText BEGIN";

    ManifestItem *cCurrent = (ManifestItem*)current;
    ManifestItem *cPrevious = (ManifestItem*)previous;

//    qDebug() << cCurrent << cPrevious;

    if(manifest->indexOfTopLevelItem(cPrevious) == -1) // this can happen when the previous item was just deleted
        cPrevious = 0;
    else
        qDebug() << cPrevious->id;

    if( cPrevious != 0 )
    {
/*
        qDebug() << "about to try to save" << manifest->indexOfTopLevelItem(cPrevious);
        qDebug() << currentHtml().left(20);
        qDebug() << "just about to try to save" << manifest->indexOfTopLevelItem(cPrevious) << cPrevious->id;
*/
        cPrevious->fileContent = currentHtml();
    }
    if( cCurrent == 0 ) // there's no file content
    {
        tabs->setEnabled(false);
        setCurrentHtml("");
    }
    else
    {
        tabs->setEnabled(true);
        setCurrentHtml(cCurrent->fileContent);
    }
//    qDebug() << "MainWindow::saveText END";
}

void MainWindow::addAllToToc()
{
    for(int i=0; i<manifest->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *tmp = new QTreeWidgetItem(toc);
        tmp->setText(0,manifest->topLevelItem(i)->text(0));
        toc->addTopLevelItem( tmp );
    }
}

void MainWindow::removeFromToc()
{
    delete toc->currentItem();
}

void MainWindow::saveEpub()
{
    bool t=false, l=false, id=false;
    for(int i=0; i<metadata->topLevelItemCount(); i++)
    {
	if(metadata->topLevelItem(i)->text(0)=="title") { t=true; }
	if(metadata->topLevelItem(i)->text(0)=="language") { l=true; }
	if(metadata->topLevelItem(i)->text(0)=="identifier") { id=true; }
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("ePub (*.epub)"));
    if(fileName=="") { qDebug() << fileName << "is no good"; return; }

    if(manifest->currentItem() && tabs->isEnabled())
        ((ManifestItem*)manifest->currentItem())->fileContent = currentHtml();
    if(manifest->topLevelItemCount()==1 && tabs->isEnabled())
        ((ManifestItem*)manifest->topLevelItem(0))->fileContent = currentHtml();

    QDir dir("OPS");
    if(dir.exists())
    {
	QFileInfoList list = dir.entryInfoList();
	for(int file=0; file<list.count(); file++)
	    if( list.at(file).isFile() )
		dir.remove(list.at(file).absoluteFilePath());
    }
    else
    {
	QDir dir2(QDir::current());
	dir2.mkdir("OPS");
    }

    QString identifier,title;

    // Content.opf
    QFile opfFile("OPS/Content.opf");
    if (!opfFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
	qDebug() << "Problem opening OPS/Content.opf";
	QMessageBox::critical(this,tr("Error"),tr("Problem opening OPS/Content.opf"));
	return;
    }
    QXmlStreamWriter opf(&opfFile);
    opf.setCodec("UTF-8");
    opf.setAutoFormatting(true);
    opf.writeStartDocument();

    opf.writeStartElement("package");
    opf.writeAttribute("xmlns","http://www.idpf.org/2007/opf");
    opf.writeAttribute("version","2.0");
    opf.writeAttribute("unique-identifier","PrimaryID");

    // metadata
    opf.writeStartElement("metadata");
    opf.writeNamespace("http://purl.org/dc/elements/1.1/","dc");
    opf.writeNamespace("http://www.idpf.org/2007/opf","opf");

    for(int i=0; i<metadata->topLevelItemCount(); i++)
    {
	opf.writeStartElement("http://purl.org/dc/elements/1.1/",metadata->topLevelItem(i)->text(0));
	if(metadata->topLevelItem(i)->text(0)=="title")
	{
	    title = metadata->topLevelItem(i)->text(1);
	}
	else if(metadata->topLevelItem(i)->text(0)=="identifier")
	{
	    opf.writeAttribute("id","PrimaryID");
	    identifier = metadata->topLevelItem(i)->text(1);
	}
	opf.writeCharacters(metadata->topLevelItem(i)->text(1));
	opf.writeEndElement();
    }

    opf.writeEndElement(); /// metadata

    // manifest
    opf.writeStartElement("manifest");

    opf.writeEmptyElement("item");
    opf.writeAttribute("id","ncx");
    opf.writeAttribute("href","toc.ncx");
    opf.writeAttribute("media-type","application/xhtml+xml");

    if(embedCharis->isChecked())
    {
        // write Charis SIL fonts
        opf.writeEmptyElement("item");
        opf.writeAttribute("id","font" + QString::number(1));
        opf.writeAttribute("href","CharisSILB.ttf");
        opf.writeAttribute("media-type","application/x-font-ttf");

        opf.writeEmptyElement("item");
        opf.writeAttribute("id","font" + QString::number(2));
        opf.writeAttribute("href","CharisSILBI.ttf");
        opf.writeAttribute("media-type","application/x-font-ttf");

        opf.writeEmptyElement("item");
        opf.writeAttribute("id","font" + QString::number(3));
        opf.writeAttribute("href","CharisSILI.ttf");
        opf.writeAttribute("media-type","application/x-font-ttf");

        opf.writeEmptyElement("item");
        opf.writeAttribute("id","font" + QString::number(4));
        opf.writeAttribute("href","CharisSILR.ttf");
        opf.writeAttribute("media-type","application/x-font-ttf");
    }

    QFile *pFile;
    // delete existing files
    if(QFile::exists("OPS/default.css"))
    {
	pFile = new QFile("OPS/default.css");
	pFile->setPermissions( QFile::ReadOther | QFile::WriteOther );
	pFile->remove();
	delete pFile;
    }
    if(QFile::exists("OPS/CharisSILB.ttf"))
    {
	pFile = new QFile("OPS/CharisSILB.ttf");
	pFile->setPermissions( QFile::ReadOther | QFile::WriteOther );
	pFile->remove();
	delete pFile;
    }
    if(QFile::exists("OPS/CharisSILBI.ttf"))
    {
	pFile = new QFile("OPS/CharisSILBI.ttf");
	pFile->setPermissions( QFile::ReadOther | QFile::WriteOther );
	pFile->remove();
	delete pFile;
    }
    if(QFile::exists("OPS/CharisSILR.ttf"))
    {
	pFile = new QFile("OPS/CharisSILR.ttf");
	pFile->setPermissions( QFile::ReadOther | QFile::WriteOther );
	pFile->remove();
	delete pFile;
    }
    if(QFile::exists("OPS/CharisSILI.ttf"))
    {
	pFile = new QFile("OPS/CharisSILI.ttf");
	pFile->setPermissions( QFile::ReadOther | QFile::WriteOther );
	pFile->remove();
	delete pFile;
    }

    // copy new files
    if(embedCharis->isChecked())
    {
        QFile cssFile(":/default.css");
        cssFile.open(QIODevice::ReadOnly);
        cssFile.setPermissions( QFile::ReadOther | QFile::WriteOther );
        cssFile.copy("OPS/default.css");
        cssFile.close();
    }
    else
    {
        QFile cssFile(":/default-nocharis.css");
        cssFile.open(QIODevice::ReadOnly);
        cssFile.setPermissions( QFile::ReadOther | QFile::WriteOther );
        cssFile.copy("OPS/default.css");
        cssFile.close();
    }


    if(embedCharis->isChecked())
    {
        QFile font1(":/CharisSILB.ttf");
        font1.open(QIODevice::ReadWrite);
        font1.copy("OPS/CharisSILB.ttf");
        font1.close();

        QFile font2(":/CharisSILBI.ttf");
        font2.open(QIODevice::ReadWrite);
        font2.copy("OPS/CharisSILBI.ttf");
        font2.close();

        QFile font3(":/CharisSILI.ttf");
        font3.open(QIODevice::ReadWrite);
        font3.copy("OPS/CharisSILI.ttf");
        font3.close();

        QFile font4(":/CharisSILR.ttf");
        font4.open(QIODevice::ReadWrite);
        font4.copy("OPS/CharisSILR.ttf");
        font4.close();
    }

    QFile *tmp;
    QTextStream *tmpstream;
    QString stripped;
    QRegExp styleTags(" style=\"*\"",Qt::CaseSensitive,QRegExp::Wildcard);
    styleTags.setMinimal(true);
    for(int i=0; i<manifest->topLevelItemCount(); i++)
    {
        ManifestItem *item = (ManifestItem*)manifest->topLevelItem(i);

	opf.writeEmptyElement("item");
        opf.writeAttribute("id",item->id);
        opf.writeAttribute("href",item->href);
        opf.writeAttribute("media-type",item->mediaType);

	// here's where the data are actually written
        QString tmpFilename = "OPS/"+item->href;
        tmp = new QFile(tmpFilename);
	if (!tmp->open(QIODevice::WriteOnly | QIODevice::Text))
	{
            qDebug() << "Problem opening: " << tmpFilename;
            QMessageBox::critical(this,tr("Error"),tr("Problem opening ")+ tmpFilename);
	    return;
	}
	tmpstream = new QTextStream(tmp);
	tmpstream->setCodec("UTF-8");

        stripped = item->fileContent;
	stripped.replace(QRegExp("<!DOCTYPE*<html>",Qt::CaseSensitive,QRegExp::Wildcard),"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE html\n  PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\">");
	stripped.remove(styleTags); // this is the wrong one
	stripped.remove(QRegExp("<style type=\"text/css\">*</style>",Qt::CaseSensitive,QRegExp::Wildcard));
	stripped.replace("<meta name=\"qrichtext\" content=\"1\" />","<link rel=\"stylesheet\" type=\"text/css\" href=\"default.css\" />"); // just a convenience to eliminate one and place the other at the same time
	stripped.replace("<head>","<head><title></title>");
        /*
//        stripped.remove(QRegExp("name=\"*\"",Qt::CaseSensitive,QRegExp::Wildcard));

*/
        if(stripLinks->isChecked())
        {
            stripped.remove(QRegExp("<a\\b[^>]*>",Qt::CaseInsensitive));
            stripped.remove(QRegExp("</a>",Qt::CaseInsensitive));
        }
//        qDebug() << stripped;

	*(tmpstream) << stripped;
	delete tmpstream;
	delete tmp;

        QFile::setPermissions( tmpFilename , QFile::ReadOther | QFile::WriteOther | QFile::ExeOther  );

	// run them through HTML Tidy first
	QProcess *myProcess = new QProcess(this);
        myProcess->setStandardErrorFile("tidy-output.txt");
	QStringList arguments;
        arguments << "-utf8" << "-asxhtml" << "-m" << tmpFilename;
	myProcess->start("tidy", arguments);

        if( !myProcess->waitForFinished() )
            return;
        if( myProcess->exitCode() == 2 )
            qDebug() << "tidy exit code: " << myProcess->exitCode();
    }

    opf.writeEmptyElement("item");
    opf.writeAttribute("id","thisisnotalikelyvalueforanidentifier");
    opf.writeAttribute("href","default.css");
    opf.writeAttribute("media-type","text/css");

    opf.writeEndElement(); /// manifest

    // spine
    opf.writeStartElement("spine");
    opf.writeAttribute("toc","ncx");
    for(int i=0; i<manifest->topLevelItemCount(); i++)
    {
        ManifestItem *item = (ManifestItem*)manifest->topLevelItem(i);

        opf.writeEmptyElement("itemref");
        opf.writeAttribute("idref",item->id);
	opf.writeAttribute("linear","yes");
    }
    opf.writeEndElement(); /// spine

    opf.writeEndElement(); // package
    opf.writeEndDocument();
    opfFile.close();

    // toc.ncx
    QFile ncxFile("OPS/toc.ncx");
    if (!ncxFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
	qDebug() << "Problem opening OPS/toc.ncx";
	QMessageBox::critical(this,tr("Error"),tr("Problem opening OPS/toc.ncx"));
	return;
    }
    QXmlStreamWriter ncx(&ncxFile);
    ncx.setCodec("UTF-8");
    ncx.setAutoFormatting(true);
    ncx.writeStartDocument();
    ncx.writeDTD("<!DOCTYPE ncx\n  PUBLIC \"-//NISO//DTD ncx 2005-1//EN\" \"http://www.daisy.org/z3986/2005/ncx-2005-1.dtd\">");

    ncx.writeStartElement("ncx");
    ncx.writeAttribute("xmlns","http://www.daisy.org/z3986/2005/ncx/");
    ncx.writeAttribute("version","2005-1");
    ncx.writeAttribute("http://www.w3.org/XML/1998/namespace","lang","en");

    ncx.writeStartElement("head");
    ncx.writeEmptyElement("meta");
    ncx.writeAttribute("name","dtb:uid");
    ncx.writeAttribute("content",identifier);
    ncx.writeEmptyElement("meta");
    ncx.writeAttribute("name","dtb:depth");
    ncx.writeAttribute("content","1");
    ncx.writeEmptyElement("meta");
    ncx.writeAttribute("name","dtb:totalPageCount");
    ncx.writeAttribute("content","0");
    ncx.writeEmptyElement("meta");
    ncx.writeAttribute("name","dtb:totalPageCount");
    ncx.writeAttribute("content","0");
    ncx.writeEndElement(); // head

    ncx.writeStartElement("docTitle");
    ncx.writeTextElement("text",title);
    ncx.writeEndElement(); // docTitle

    ncx.writeStartElement("docAuthor");
    ncx.writeTextElement("text","None");
    ncx.writeEndElement(); // docAuthor

    ncx.writeStartElement("navMap");

    int playOrder = 1;
    for(int i=0; i<toc->topLevelItemCount(); i++)
	playOrder = writeNavPoint(toc->topLevelItem(i), &ncx, playOrder);

    ncx.writeEndElement(); // navMap

    ncx.writeEndElement(); // ncx

    ncx.writeEndDocument();
    ncxFile.close();


    // build the zip
    if( QFile::exists( fileName ) )
        QFile::remove( fileName );

    /*
    QString templateFilename = "template.zip";
    QFile templatetmp(templateFilename);
    if( !templatetmp.exists() )
    {
	QMessageBox::information(this,tr("Error"),tr("I cannot find template.zip, which is a necessary file. Please try to find it in the following dialog."));
	templateFilename = QFileDialog::getOpenFileName(this, tr("Please find template.zip"),
							"",tr("template.zip"));
    }
*/
    QFile *templateFile = new QFile(":/template.zip");
    templateFile->open(QIODevice::ReadWrite);
    templateFile->copy(fileName);
    templateFile->setPermissions( QFile::ReadOther | QFile::WriteOther | QFile::ExeOther );
    templateFile->close();
    delete templateFile;

    QFile::setPermissions( fileName , QFile::ReadOther | QFile::WriteOther | QFile::ExeOther  );

    QProcess *myProcess = new QProcess(this);
    QStringList arguments;
    arguments << "-r" << fileName << "OPS";
    myProcess->start("zip", arguments);

    if( !myProcess->waitForFinished() )
        return;
    if( myProcess->exitCode() != 0 )
        qDebug() << "zip exit code: " << myProcess->exitCode();

    QMessageBox::information(this,tr("epub Workspace"),tr("The epub file has been created at: ") + fileName );
}

int MainWindow::writeNavPoint(QTreeWidgetItem *current, QXmlStreamWriter *ncx, int playOrder)
{
    ncx->writeStartElement("navPoint");
    ncx->writeAttribute("class","chapter");
    ncx->writeAttribute("id",toID(current->text(0)));
    ncx->writeAttribute("playOrder",QString::number(playOrder));
    playOrder++;

    ncx->writeStartElement("navLabel");
    ncx->writeTextElement("text",current->text(0));
    ncx->writeEndElement(); // navLabel

    ncx->writeEmptyElement("content");
    ncx->writeAttribute("src",current->text(0)+".xml");

    for(int i=0; i< current->childCount(); i++)
    {
        playOrder = writeNavPoint(current->child(i),ncx,playOrder);
    }

    ncx->writeEndElement(); // navPoint

    return playOrder;
}

QString MainWindow::toID(QString string)
{
//    qDebug() << string << string.replace(QRegExp("\\W"),"");
    return string.replace(QRegExp("\\W"),"");
}

void MainWindow::updateOtherEditor(int current)
{
    if(current == 0) // used to be 1 (=html)
	editor->setHtml( htmleditor->toPlainText() );
    else if(current == 1) // used to be 0 (=richtext)
	htmleditor->setPlainText( editor->toHtml() );
}

void MainWindow::newEpub()
{
    metadata->clear();
    manifest->clear();
    toc->clear();
    editor->setHtml( "" );
    htmleditor->setHtml( "" );
    tabs->setEnabled(false);
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select an ePub"),
                                                    "",tr("ePub Files (*.epub)"));
    if(filename.length() == 0)
        return;

    QFileInfo info(filename);

    QDir directory( info.baseName() );
    if(directory.exists())
    {
        if( QMessageBox::question( this, tr("ePub Workspace"), tr("A folder already exists for that ePub (%1). Click OK to use that folder, or cancel to quit.").arg(directory.absolutePath()), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel ) == QMessageBox::Cancel )
            return;
    }

    currentDirectory = directory.absolutePath();

    QProcess *myProcess = new QProcess(this);

    QStringList arguments;
    arguments << "-o" << "-q" << filename << "-d" << info.baseName();
    myProcess->start("unzip", arguments);

    if( !myProcess->waitForFinished() )
        return;

    directory.cd("META-INF");
    QString containerPath = directory.absoluteFilePath("container.xml");


    QFile container(containerPath);
    container.open(QFile::ReadOnly);
    QXmlStreamReader containerReader(&container);
    QString rootFile = "";
    while( !containerReader.atEnd() )
    {
        containerReader.readNext();

        if( containerReader.tokenType() == QXmlStreamReader::StartElement && containerReader.name() == "rootfile" )
        {
            if( !containerReader.attributes().value("full-path").isNull() )
                rootFile = containerReader.attributes().value("full-path").toString();
        }
    }
    if(rootFile.length() == 0)
        return;

    directory.cdUp(); // back to the folder

    QString opfPath = directory.absoluteFilePath(rootFile);
    QFile opfFile(opfPath);
    opfFile.open(QFile::ReadOnly);
    QXmlStreamReader opf(&opfFile);

    directory.cd("OPS");

    metadata->clear();
    manifest->clear();
    toc->clear();

    QString ncxHref = "";

    while( !opf.atEnd() )
    {
        opf.readNext();

        if( opf.tokenType() == QXmlStreamReader::StartElement )
        {
            if( opf.namespaceUri() == "http://purl.org/dc/elements/1.1/" ) // Dublin core
            {
                QStringList tmp;
                int type = metadataLabels.indexOf(opf.name().toString());
                tmp << opf.name().toString() << opf.readElementText();
                metadata->addTopLevelItem(new QTreeWidgetItem(tmp,type+1000));
            }
            else if( opf.namespaceUri() == "http://www.idpf.org/2007/opf" ) // Dublin core
            {
                if( opf.name().toString() == "item" )
                {
                    QXmlStreamAttributes attr = opf.attributes();
                    if( !attr.hasAttribute("id") || !attr.hasAttribute("href") || !attr.hasAttribute("media-type") )
                    {
                        qDebug() << "Defective id tag on line " << opf.lineNumber();
                        continue;
                    }

                    if( attr.value("id").toString() == "ncx" )
                    {
                        ncxHref = attr.value("href").toString();
                        continue;
                    }

                    if( attr.value("media-type").toString() != "application/xhtml+xml" )
                        continue;

                    ManifestItem *tmp = new ManifestItem;
                    tmp->id = attr.value("id").toString();
                    tmp->href = attr.value("href").toString();
                    tmp->mediaType = attr.value("media-type").toString();
                    tmp->setText(0,tmp->id);

                    QFile tmpFile( directory.absoluteFilePath(tmp->href) );
                    if (tmpFile.open(QFile::ReadOnly | QFile::Text))
                    {
                        QTextStream str(&tmpFile);
                        str.setCodec("UTF-8");
                        tmp->fileContent = str.readAll();
                    }
                    else
                    {
                        qDebug() << "Error trying to read " << tmp->href;
                    }

                    manifest->addTopLevelItem(tmp);
                }
            }
        }
    }

    if(ncxHref.isEmpty())
    {
        qDebug() << "Didn't find an NCX file.";
        return;
    }

    QFile ncxFile(directory.absoluteFilePath(ncxHref));
    if( !ncxFile.open(QFile::ReadOnly) )
    {
        qDebug() << "Problem opening ncxHref: " << ncxHref;
        return;
    }
    QXmlStreamReader ncx(&ncxFile);


    TocItem *item = 0;
    TocItem *parent = 0;
    while( !ncx.atEnd() )
    {
        ncx.readNext();
        if( ncx.tokenType() == QXmlStreamReader::StartElement )
        {
            if( ncx.name().toString() == "navPoint" )
            {
                QXmlStreamAttributes attr = ncx.attributes();
                if( !attr.hasAttribute("class") || !attr.hasAttribute("id") )
                {
                    qDebug() << "Defective navPoint tag on line " << ncx.lineNumber();
                    continue;
                }

                item = new TocItem;
                item->id = attr.value("id").toString();
                item->clas = attr.value("class").toString();
                item->setText(0,item->id);

//                qDebug() << "opening" << item->id;

                if( parent == 0 )
                    toc->addTopLevelItem(item);
                else
                    parent->addChild(item);

                parent = item;
            }
            else if( ncx.name().toString() == "text" )
            {
                if(item)
                {
                    item->label = ncx.readElementText();
                    item->setText(0,item->label);
                }
            }
            else if( ncx.name().toString() == "content" )
            {
                if( item && ncx.attributes().hasAttribute("src") )
                    item->src = ncx.attributes().value("src").toString();
            }
        }
        else if( ncx.tokenType() == QXmlStreamReader::EndElement )
        {
            if( ncx.name().toString() == "navPoint" )
            {
                parent = (TocItem*)item->parent();
                if( parent != 0 )
                    item = parent; // seems odd
            }
        }
    }

    // update the manifest names
//    tryToReplaceIdWithText( (TocItem*)toc->invisibleRootItem() );
    for(int i=0; i<toc->topLevelItemCount(); i++)
        tryToReplaceIdWithText( (TocItem*)toc->topLevelItem(i) );
}

void MainWindow::tryToReplaceIdWithText(TocItem *ti)
{
    QList<QTreeWidgetItem*> m = manifest->findItems( ti->id, Qt::MatchExactly, 0 );
    if( m.count() > 0 ) // by the epub standard, ids must be unique
        m.at(0)->setText(0, ti->label );
    for(int i=0; i< ti->childCount(); i++)
        tryToReplaceIdWithText( (TocItem*)ti->child(i) );
}

void MainWindow::deleteTocReferencesTo(TocItem *ti, QString id)
{
    for(int i=0; i < ti->childCount(); i++)
    {
        TocItem *item = (TocItem*)ti->child(i);
        if( item->id == id )
            delete item;
        else
            deleteTocReferencesTo( item, id );
    }
}
