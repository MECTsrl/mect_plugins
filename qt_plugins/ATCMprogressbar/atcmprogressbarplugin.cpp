#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmprogressbar.h"
#include "atcmprogressbarplugin.h"
#include "atcmprogressbartaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMprogressbarPlugin::ATCMprogressbarPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMprogressbarPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    manager->registerExtensions(new atcmprogressbarTaskMenuFactory(manager),
            Q_TYPEID(QDesignerTaskMenuExtension));

	m_initialized = true;
}

bool ATCMprogressbarPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMprogressbarPlugin::createWidget(QWidget *parent)
{
	return new ATCMprogressbar(parent);
}

QString ATCMprogressbarPlugin::name() const
{
	return QLatin1String("ATCMprogressbar");
}

QString ATCMprogressbarPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMprogressbarPlugin::icon() const
{
	return QIcon(":/atcmprogressbar.png");
}

QString ATCMprogressbarPlugin::toolTip() const
{
	return QLatin1String("ATCM progressbar PLC");
}

QString ATCMprogressbarPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMprogressbar the variable read from PLC");
}

bool ATCMprogressbarPlugin::isContainer() const
{
	return false;
}

QString ATCMprogressbarPlugin::domXml() const
{
    return QLatin1String("<widget class=\"ATCMprogressbar\" name=\"atcmProgressbar\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>50</width>\n"
			" <height>100</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QProgressBar</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMprogressbar widget</string>\n"
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

QString ATCMprogressbarPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMprogressbarPlugin)
