#ifndef ATCMCOMBOBOXPROPERTYSHEET_H
#define ATCMCOMBOBOXPROPERTYSHEET_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QDesignerPropertySheetExtension>
#include <QExtensionFactory>

#include "atcmcombobox.h"

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
ATCMcomboboxPropertyFactory
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
class ATCMcomboboxPropertyFactory : public QExtensionFactory
{
Q_OBJECT

public:
ATCMcomboboxPropertyFactory(QExtensionManager *parent = 0);

protected:
QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
ATCMcomboboxPropertySheetExtension
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
class ATCMcomboboxPropertySheetExtension : public QObject, QDesignerPropertySheetExtension
{
Q_OBJECT
Q_INTERFACES(QDesignerPropertySheetExtension)

public:
ATCMcomboboxPropertySheetExtension(ATCMcombobox* p, QObject *parent);


int count (void) const;
bool hasReset ( int index ) const;
int indexOf ( const QString & name ) const;
bool isAttribute ( int index ) const;
bool isChanged ( int index ) const;
bool isVisible ( int index ) const;
QVariant property ( int index ) const;
QString propertyGroup ( int index ) const;
QString propertyName ( int index ) const;
bool reset ( int index );
void setAttribute ( int index, bool attribute );
void setChanged ( int index, bool changed );
void setProperty ( int index, const QVariant & value );
void setPropertyGroup ( int index, const QString & group );
void setVisible ( int index, bool visible );

protected:

ATCMcombobox* _powner;
const QMetaObject* _p_metaobject;

QMap<int, QString> _PropertyGroups;
};

#endif
