#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmslider.h"
#include "atcmsliderplugin.h"
#include "atcmslidertaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMsliderPlugin::ATCMsliderPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMsliderPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    manager->registerExtensions(new atcmsliderTaskMenuFactory(manager),
            Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMsliderPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMsliderPlugin::createWidget(QWidget *parent)
{
	return new ATCMslider(parent);
}

QString ATCMsliderPlugin::name() const
{
	return QLatin1String("ATCMslider");
}

QString ATCMsliderPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMsliderPlugin::icon() const
{
	return QIcon(":/atcmslider.png");
}

QString ATCMsliderPlugin::toolTip() const
{
	return QLatin1String("ATCM slider PLC");
}

QString ATCMsliderPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMslider the variable read from PLC");
}

bool ATCMsliderPlugin::isContainer() const
{
	return false;
}

QString ATCMsliderPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMslider\" name=\"atcmSlider\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>30</width>\n"
			" <height>100</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QSlider</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMslider widget</string>\n"
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
			" <property name=\"icon\" >\n"
			" </property>\n"
			" <property name=\"borderWidth\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMsliderPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMsliderPlugin)
