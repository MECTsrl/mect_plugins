#include "atcmlcdnumber.h"
#include "atcmlcdnumberplugin.h"

#include <QtCore/QtPlugin>

	ATCMlcdnumberPlugin::ATCMlcdnumberPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMlcdnumberPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here

	m_initialized = true;
}

bool ATCMlcdnumberPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMlcdnumberPlugin::createWidget(QWidget *parent)
{
	return new ATCMlcdnumber(parent);
}

QString ATCMlcdnumberPlugin::name() const
{
	return QLatin1String("ATCMlcdnumber");
}

QString ATCMlcdnumberPlugin::group() const
{
	return QLatin1String("ATCM plugin");
}

QIcon ATCMlcdnumberPlugin::icon() const
{
	return QIcon(":/atcmlcdnumber.png");
}

QString ATCMlcdnumberPlugin::toolTip() const
{
	return QLatin1String("ATCM lcdnumber PLC");
}

QString ATCMlcdnumberPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMlcdnumber the variable read from PLC");
}

bool ATCMlcdnumberPlugin::isContainer() const
{
	return false;
}

QString ATCMlcdnumberPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMlcdnumber\" name=\"atcmLCDNumber\">\n"
		" <property name=\"geometry\">\n"
		" <rect>\n"
		" <x>0</x>\n"
		" <y>0</y>\n"
		" <width>100</width>\n"
		" <height>30</height>\n"
		" </rect>\n"
		" </property>\n"
		" <property name=\"toolTip\" >\n"
		" <string>ATCM QLCDNumber</string>\n"
		" </property>\n"
		" <property name=\"whatsThis\" >\n"
		" <string>ATCMlcdnumber widget</string>\n"
		" </property>\n"
		" <property name=\"variable\" >\n"
		" <string notr=\"true\"></string>\n"
		" </property>\n"
		" <property name=\"refresh\" >\n"
		" <number>500</number>\n"
		" </property>\n"
        " <property name=\"viewStatus\" >\n"
        " <bool>true</bool>\n"
        " </property>\n"
		"</widget>\n");
}

QString ATCMlcdnumberPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMlcdnumberPlugin)
