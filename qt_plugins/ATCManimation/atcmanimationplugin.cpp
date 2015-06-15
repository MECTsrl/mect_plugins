#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmanimation.h"
#include "atcmanimationplugin.h"
#include "atcmanimationtaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

	ATCManimationPlugin::ATCManimationPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCManimationPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	QExtensionManager *manager = formEditor->extensionManager();
	Q_ASSERT(manager != 0);

	manager->registerExtensions(new atcmanimationTaskMenuFactory(manager),
			Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCManimationPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCManimationPlugin::createWidget(QWidget *parent)
{
	return new ATCManimation(parent);
}

QString ATCManimationPlugin::name() const
{
	return QLatin1String("ATCManimation");
}

QString ATCManimationPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCManimationPlugin::icon() const
{
	return QIcon(":/atcmanimation.png");
}

QString ATCManimationPlugin::toolTip() const
{
	return QLatin1String("ATCM animation PLC");
}

QString ATCManimationPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCManimation the variable read from PLC");
}

bool ATCManimationPlugin::isContainer() const
{
	return false;
}

QString ATCManimationPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCManimation\" name=\"atcmAnimation\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>100</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QAnimation</string>\n"
			" </property>\n"
			" <property name=\"mapping\" >\n"
			" <string></string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCManimation widget</string>\n"
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
			"</widget>\n");
}

QString ATCManimationPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCManimationPlugin)
