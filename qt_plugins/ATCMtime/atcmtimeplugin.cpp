#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmtime.h"
#include "atcmtimeplugin.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMtimePlugin::ATCMtimePlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMtimePlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	m_initialized = true;
    Q_UNUSED( formEditor )
}

bool ATCMtimePlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMtimePlugin::createWidget(QWidget *parent)
{
	return new ATCMtime(parent);
}

QString ATCMtimePlugin::name() const
{
	return QLatin1String("ATCMtime");
}

QString ATCMtimePlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMtimePlugin::icon() const
{
	return QIcon(":/atcmtime.png");
}

QString ATCMtimePlugin::toolTip() const
{
	return QLatin1String("ATCM time PLC");
}

QString ATCMtimePlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMtime the variable read from PLC");
}

bool ATCMtimePlugin::isContainer() const
{
	return false;
}

QString ATCMtimePlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMtime\" name=\"atcmTime\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>100</width>\n"
			" <height>40</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QTime</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMtime widget</string>\n"
			" </property>\n"
			" <property name=\"borderWidth\" >\n"
			" <number>1</number>\n"
			" </property>\n"
			" <property name=\"borderRadius\" >\n"
			" <number>0</number>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMtimePlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMtimePlugin)
