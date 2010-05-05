#ifndef ISISPROPERTYVIEWER_H
#define ISISPROPERTYVIEWER_H

#include "ui_isisPropertyViewer.h"

#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"
#include "boost/shared_ptr.hpp"


class isisPropertyViewer : public QMainWindow
{
	Q_OBJECT

public:
    isisPropertyViewer(QMainWindow *parent = 0);
	Qt::ItemFlags flags(const QModelIndex &index) const;
	

private slots:
	void on_action_Close_activated();
	void on_action_Open_activated();
	void on_action_Clear_activated();
	void edit_item(QTreeWidgetItem*, int);
signals:
	void itemDoubleClicked();
	
private:
     typedef std::set<std::string, isis::util::_internal::caselessStringLess> PropKeyListType;
	 PropKeyListType m_keyList;
     Ui::isisPropertyViewer ui;
	 void addImageToTree(const boost::shared_ptr<isis::data::Image> image, const QString& );
	 void addChildToItem( QTreeWidgetItem*, const QString&, const QString& ) const;
	
	 
};
 
#endif