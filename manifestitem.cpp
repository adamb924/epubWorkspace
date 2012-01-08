#include "manifestitem.h"

ManifestItem::ManifestItem() :
    QTreeWidgetItem()
{
}

ManifestItem::ManifestItem(QString id, QString href, QString fileContent, QString mediaType)
{
    this->id = id;
    this->href = href;
    this->fileContent = fileContent;
    this->mediaType = mediaType;
}
