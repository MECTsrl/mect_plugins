#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmspinbox.h"
#include "atcmspinboxplugin.h"
#include "atcmspinboxtaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMspinboxPlugin::ATCMspinboxPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMspinboxPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    manager->registerExtensions(new atcmspinboxTaskMenuFactory(manager),
            Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMspinboxPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMspinboxPlugin::createWidget(QWidget *parent)
{
	return new ATCMspinbox(parent);
}

QString ATCMspinboxPlugin::name() const
{
	return QLatin1String("ATCMspinbox");
}

QString ATCMspinboxPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMspinboxPlugin::icon() const
{
	return QIcon(":/atcmspinbox.png");
}

QString ATCMspinboxPlugin::toolTip() const
{
	return QLatin1String("ATCM spinbox PLC");
}

QString ATCMspinboxPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMspinbox the variable read from PLC");
}

bool ATCMspinboxPlugin::isContainer() const
{
	return false;
}

QString ATCMspinboxPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMspinbox\" name=\"atcmSpinbox\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>30</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QSpinBox</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMspinbox widget</string>\n"
			" </property>\n"
			" <property name=\"variable\" >\n"
			" <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"refresh\" >\n"
			" <number>200</number>\n"
			" </property>\n"
			" <property name=\"viewStatus\" >\n"
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

QString ATCMspinboxPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMspinboxPlugin)
