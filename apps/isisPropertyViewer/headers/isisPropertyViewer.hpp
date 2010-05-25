#ifndef ISISPROPERTYVIEWER_H
#define ISISPROPERTYVIEWER_H

#include "ui_isisPropertyViewer.h"

#include "propertyHolder.hpp"

#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"
#include "boost/shared_ptr.hpp"


class isisPropertyViewer : public QMainWindow
{
	Q_OBJECT

public:
	isisPropertyViewer( const isis::util::slist&, QMainWindow *parent = 0 );
	Qt::ItemFlags flags( const QModelIndex &index ) const;


private slots:
	void on_action_Close_activated();
	void on_action_Open_activated();
	void on_action_Clear_activated();
	void on_actionSave_activated();
	void on_actionSaveAs_activated();
	void edit_item( QTreeWidgetItem*, int );
signals:
	void itemDoubleClicked();

private:
	typedef std::set<std::string, isis::util::_internal::caselessStringLess> PropKeyListType;
	typedef std::map<std::string, isis::util::PropMap> PropMapType;
	PropKeyListType m_keyList;
	Ui::isisPropertyViewer ui;
	void createTree( const boost::shared_ptr<isis::data::Image>, const QString& );
	void addPropToTree( const boost::shared_ptr<isis::data::Image>, PropKeyListType::const_reference, QTreeWidgetItem* );
	void addChildToItem( QTreeWidgetItem*, const QString&, const QString&, const QString& ) const;
	void addFileToTree( const QString& );
	void updateTree( void );
	PropertyHolder m_propHolder;


};

#endif