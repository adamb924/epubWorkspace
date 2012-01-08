#ifndef TOCITEM_H
#define TOCITEM_H

#include <QTreeWidgetItem>

class TocItem : public QTreeWidgetItem
{
public:
    TocItem();

    QString label, id, src, clas;
};

#endif // TOCITEM_H
