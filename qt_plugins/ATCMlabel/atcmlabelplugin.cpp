#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmlabel.h"
#include "atcmlabelplugin.h"
#include "atcmlabeltaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

	ATCMlabelPlugin::ATCMlabelPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMlabelPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	QExtensionManager *manager = formEditor->extensionManager();
	Q_ASSERT(manager != 0);

	manager->registerExtensions(new atcmlabelTaskMenuFactory(manager),
			Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMlabelPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMlabelPlugin::createWidget(QWidget *parent)
{
	return new ATCMlabel(parent);
}

QString ATCMlabelPlugin::name() const
{
	return QLatin1String("ATCMlabel");
}

QString ATCMlabelPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMlabelPlugin::icon() const
{
	return QIcon(":/atcmlabel.png");
}

QString ATCMlabelPlugin::toolTip() const
{
	return QLatin1String("ATCM label PLC");
}

QString ATCMlabelPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMlabel the variable read from PLC");
}

bool ATCMlabelPlugin::isContainer() const
{
	return false;
}

QString ATCMlabelPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMlabel\" name=\"atcmLabel\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>40</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QLabel</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMlabel widget</string>\n"
			" </property>\n"
			" <property name=\"variable\" >\n"
			" <string notr=\"true\"></string>\n"
			" </property>\n"
			" <property name=\"min\" >\n"
			" <string notr=\"true\">0</string>\n"
			" </property>\n"
			" <property name=\"max\" >\n"
			" <string notr=\"true\">0</string>\n"
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
            " <property name=\"editCaption\" >\n"
            " <string></string>\n"
            " </property>\n"
            " <property name=\"borderWidth\" >\n"
			" <number>1</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMlabelPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMlabelPlugin)
