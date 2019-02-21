#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmdate.h"
#include "atcmdateplugin.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMdatePlugin::ATCMdatePlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMdatePlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	m_initialized = true;
    Q_UNUSED( formEditor )
}

bool ATCMdatePlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMdatePlugin::createWidget(QWidget *parent)
{
	return new ATCMdate(parent);
}

QString ATCMdatePlugin::name() const
{
	return QLatin1String("ATCMdate");
}

QString ATCMdatePlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMdatePlugin::icon() const
{
	return QIcon(":/atcmdate.png");
}

QString ATCMdatePlugin::toolTip() const
{
	return QLatin1String("ATCM date PLC");
}

QString ATCMdatePlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMdate the variable read from PLC");
}

bool ATCMdatePlugin::isContainer() const
{
	return false;
}

QString ATCMdatePlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMdate\" name=\"atcmDate\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>40</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMdate widget</string>\n"
			" </property>\n"
			" <property name=\"borderWidth\" >\n"
			" <number>1</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMdatePlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMdatePlugin)
