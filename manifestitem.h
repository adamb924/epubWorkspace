#ifndef MANIFESTITEM_H
#define MANIFESTITEM_H

#include <QTreeWidgetItem>

class ManifestItem : public QTreeWidgetItem
{
public:
    explicit ManifestItem();
    ManifestItem(QString id, QString href, QString fileContent, QString mediaType = QString("application/xhtml+xml"));

    QString id, href, mediaType, fileContent;

signals:

public slots:

};

#endif // MANIFESTITEM_H
