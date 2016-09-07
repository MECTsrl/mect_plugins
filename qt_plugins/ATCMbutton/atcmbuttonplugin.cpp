#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmbutton.h"
#include "atcmbuttonplugin.h"
#include "atcmbuttontaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMbuttonPlugin::ATCMbuttonPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMbuttonPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    manager->registerExtensions(new atcmbuttonTaskMenuFactory(manager),
            Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMbuttonPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMbuttonPlugin::createWidget(QWidget *parent)
{
	return new ATCMbutton(parent);
}

QString ATCMbuttonPlugin::name() const
{
	return QLatin1String("ATCMbutton");
}

QString ATCMbuttonPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMbuttonPlugin::icon() const
{
	return QIcon(":/atcmbutton.png");
}

QString ATCMbuttonPlugin::toolTip() const
{
	return QLatin1String("ATCM button PLC");
}

QString ATCMbuttonPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMbutton the variable read from PLC");
}

bool ATCMbuttonPlugin::isContainer() const
{
	return false;
}

QString ATCMbuttonPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMbutton\" name=\"atcmButton\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>40</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QPushButton</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMbutton widget</string>\n"
			" </property>\n"
			" <property name=\"pageName\" >\n"
            " <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"statusvar\" >\n"
            " <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"viewStatus\" >\n"
			" <bool>false</bool>\n"
			" </property>\n"
			" <property name=\"visibilityVar\" >\n"
            " <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"text\" >\n"
			" <string>text</string>\n"
			" </property>\n"
			" <property name=\"pressText\" >\n"
			" <string></string>\n"
			" </property>\n"
			" <property name=\"icon\" >\n"
			" </property>\n"
			" <property name=\"pressIcon\" >\n"
			" </property>\n"
			" <property name=\"borderWidth\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMbuttonPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMbuttonPlugin)
