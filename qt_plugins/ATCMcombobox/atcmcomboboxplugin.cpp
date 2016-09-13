#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmcombobox.h"
#include "atcmcomboboxplugin.h"
#include "atcmcomboboxtaskmenu.h"
#include "atcmcomboboxpropertysheet.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMcomboboxPlugin::ATCMcomboboxPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMcomboboxPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

    // Add extension registrations, etc. here
	QExtensionManager *manager = formEditor->extensionManager();
	Q_ASSERT(manager != 0);

	manager->registerExtensions(new atcmcomboboxTaskMenuFactory(manager),
			Q_TYPEID(QDesignerTaskMenuExtension));
    /*
    manager->registerExtensions(new ATCMcomboboxPropertyFactory(manager),
            Q_TYPEID(QDesignerPropertySheetExtension));
    */

#if 0
    QItemEditorFactory *factory = new QItemEditorFactory;
      QItemEditorCreatorBase *crosstableeditor =
          new QStandardItemEditorCreator<CrossTableEditor>();

      factory->registerEditor(QVariant::String, crosstableeditor);
      QItemEditorFactory::setDefaultFactory(factory);
#endif

    m_initialized = true;
}

bool ATCMcomboboxPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMcomboboxPlugin::createWidget(QWidget *parent)
{
	return new ATCMcombobox(parent);
}

QString ATCMcomboboxPlugin::name() const
{
	return QLatin1String("ATCMcombobox");
}

QString ATCMcomboboxPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMcomboboxPlugin::icon() const
{
	return QIcon(":/atcmcombobox.png");
}

QString ATCMcomboboxPlugin::toolTip() const
{
	return QLatin1String("ATCM combobox PLC");
}

QString ATCMcomboboxPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMcombobox the variable read from PLC");
}

bool ATCMcomboboxPlugin::isContainer() const
{
	return false;
}

QString ATCMcomboboxPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMcombobox\" name=\"atcmComboBox\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>30</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QComboBox</string>\n"
			" </property>\n"
			" <property name=\"mapping\" >\n"
			" <string></string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMcombobox widget</string>\n"
			" </property>\n"
			" <property name=\"variable\" >\n"
			" <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"viewStatus\" >\n"
			" <bool>false</bool>\n"
            " </property>\n"
            " <property name=\"writeAcknowledge\" >\n"
            " <bool>false</bool>\n"
            " </property>\n"
			" <property name=\"visibilityVar\" >\n"
			" <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"borderWidth\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMcomboboxPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMcomboboxPlugin)
