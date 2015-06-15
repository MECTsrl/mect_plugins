#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmled.h"
#include "atcmledplugin.h"
#include "atcmledtaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMledPlugin::ATCMledPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMledPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	QExtensionManager *manager = formEditor->extensionManager();
	Q_ASSERT(manager != 0);

	manager->registerExtensions(new atcmledTaskMenuFactory(manager),
			Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMledPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMledPlugin::createWidget(QWidget *parent)
{
	return new ATCMled(parent);
}

QString ATCMledPlugin::name() const
{
	return QLatin1String("ATCMled");
}

QString ATCMledPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMledPlugin::icon() const
{
	return QIcon(":/atcmled.png");
}

QString ATCMledPlugin::toolTip() const
{
	return QLatin1String("ATCM led PLC");
}

QString ATCMledPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMled the variable read from PLC");
}

bool ATCMledPlugin::isContainer() const
{
	return false;
}

QString ATCMledPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMled\" name=\"atcmLed\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>30</width>\n"
			" <height>30</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QLed</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMled widget</string>\n"
			" </property>\n"
			" <property name=\"variable\" >\n"
			" <string></string>\n"
			" </property>\n"
			" <property name=\"refresh\" >\n"
			" <number>500</number>\n"
			" </property>\n"
			" <property name=\"viewStatus\" >\n"
			" <bool>false</bool>\n"
			" </property>\n"
			" <property name=\"visibilityVar\" >\n"
			" <string></string>\n"
			" </property>\n"
			" <property name=\"onIcon\" >\n"
			"   <iconset resource=\"\">\n"
			"     <normaloff>:/on.png</normaloff>\n"
			"   </iconset>\n"
			" </property>\n"
			" <property name=\"offIcon\" >\n"
			"   <iconset resource=\"\">\n"
			"     <normaloff>:/off.png</normaloff>\n"
			"   </iconset>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMledPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMledPlugin)
