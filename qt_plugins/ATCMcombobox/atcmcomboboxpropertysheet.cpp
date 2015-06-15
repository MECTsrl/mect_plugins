#include <stdio.h>
#include <QMetaProperty>
#include "atcmcomboboxpropertysheet.h"

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
ATCMcomboboxPropertyFactory
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
ATCMcomboboxPropertyFactory::ATCMcomboboxPropertyFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}


QObject *ATCMcomboboxPropertyFactory::createExtension(QObject *object,
                                                      const QString &iid,
                                                      QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (ATCMcombobox* pATCMcombobox = qobject_cast<ATCMcombobox*>(object))
        return new ATCMcomboboxPropertySheetExtension(pATCMcombobox, parent);

    return 0;
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
ATCMcomboboxPropertySheetExtension
*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


ATCMcomboboxPropertySheetExtension::ATCMcomboboxPropertySheetExtension(ATCMcombobox* p, QObject *parent) :
    QObject(parent),
    _powner(p),
    _p_metaobject(_powner->metaObject()),
    _PropertyGroups()
{
    printf("%d %s\n", __LINE__, __func__);
    const int i_object_index = indexOf("objectName");
    setPropertyGroup(i_object_index, "QObject");

    const int i_widget_index = indexOf("modal");
    setPropertyGroup(i_widget_index, "QWidget");

    const int i_abstractATCMcombobox_index = indexOf("text");
    setPropertyGroup(i_abstractATCMcombobox_index, "QAbstractATCMcombobox");

    const int i_pushATCMcombobox_index = indexOf("autoDefault");
    setPropertyGroup(i_pushATCMcombobox_index, "QPushATCMcombobox");
}

int ATCMcomboboxPropertySheetExtension::count () const
{
    printf("%d %s: %d\n", __LINE__, __func__,  _p_metaobject->propertyCount());
    return _p_metaobject->propertyCount();
}

bool ATCMcomboboxPropertySheetExtension::hasReset ( int index ) const
{
    printf("%d %s: %d\n", __LINE__, __func__, _p_metaobject->property(index).isResettable());
    return _p_metaobject->property(index).isResettable();
}

int ATCMcomboboxPropertySheetExtension::indexOf ( const QString & name ) const
{
    printf("%d %s - %d -> %s\n", __LINE__, __func__, _p_metaobject->indexOfProperty( name.toLatin1().constData() ), name.toAscii().data());
    return _p_metaobject->indexOfProperty( name.toLatin1().constData() );
}

bool ATCMcomboboxPropertySheetExtension::isAttribute ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    return false;
}

bool ATCMcomboboxPropertySheetExtension::isChanged ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    return false;
}

bool ATCMcomboboxPropertySheetExtension::isVisible ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    return true;
}

QVariant ATCMcomboboxPropertySheetExtension::property ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    return _p_metaobject->property(index).read(_powner);
}

QString ATCMcomboboxPropertySheetExtension::propertyGroup ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    QMap<int, QString>::const_iterator it = _PropertyGroups.begin();
    if (index > 0){
        while (it.key() <= index){
            if (it.value() != NULL)
            {
            printf("%d %s - %d -> %s\n", __LINE__, __func__, it.key(), it.value().toAscii().data());

            }
            it++;
        }
        it--;
    }
    return it.value();
}

QString ATCMcomboboxPropertySheetExtension::propertyName ( int index ) const
{
    printf("%d %s\n", __LINE__, __func__);
    return QString(_p_metaobject->property(index).name());
}

bool ATCMcomboboxPropertySheetExtension::reset ( int index )
{
    printf("%d %s\n", __LINE__, __func__);
    return _p_metaobject->property(index).reset(_powner);
}

void ATCMcomboboxPropertySheetExtension::setAttribute ( int index, bool attribute )
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    Q_UNUSED(attribute)
}

void ATCMcomboboxPropertySheetExtension::setChanged ( int index, bool changed )
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    Q_UNUSED(changed)
}

void ATCMcomboboxPropertySheetExtension::setProperty ( int index, const QVariant & value )
{
    printf("%d %s\n", __LINE__, __func__);
    QMetaProperty metaproperty = _p_metaobject->property(index);
    metaproperty.write(_powner, value);
}

void ATCMcomboboxPropertySheetExtension::setPropertyGroup ( int index, const QString & group )
{
    printf("%d %s\n", __LINE__, __func__);
    _PropertyGroups.insert(index, group);
}

void ATCMcomboboxPropertySheetExtension::setVisible ( int index, bool visible )
{
    printf("%d %s\n", __LINE__, __func__);
    Q_UNUSED(index)
    Q_UNUSED(visible)
}
