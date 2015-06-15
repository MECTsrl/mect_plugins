#include <QtDesigner>
#include <QtGui>
#include <QtPlugin>

#include "atcmgraph.h"
#include "atcmgraphplugin.h"
#include "atcmgraphtaskmenu.h"

#define _STR(x) #x
#define STR(x) _STR(x)

ATCMgraphPlugin::ATCMgraphPlugin(QObject *parent)
: QObject(parent)
{
	m_initialized = false;
}

void ATCMgraphPlugin::initialize(QDesignerFormEditorInterface * formEditor)
{
	if (m_initialized)
		return;

	// Add extension registrations, etc. here
	QExtensionManager *manager = formEditor->extensionManager();
	Q_ASSERT(manager != 0);

	manager->registerExtensions(new atcmgraphTaskMenuFactory(manager),
			Q_TYPEID(QDesignerTaskMenuExtension));

#if 0
	//using namespace descriptor;
	qRegisterMetaType<descriptor::atcm_axisDescriptor>("descriptor::atcm_axisDescriptor");
#endif

	m_initialized = true;
}

bool ATCMgraphPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *ATCMgraphPlugin::createWidget(QWidget *parent)
{
	return new ATCMgraph(parent);
}

QString ATCMgraphPlugin::name() const
{
	return QLatin1String("ATCMgraph");
}

QString ATCMgraphPlugin::group() const
{
    return QLatin1String(QString("ATCM plugin v%1").arg(STR(ATCM_VERSION)).toAscii().data());
}

QIcon ATCMgraphPlugin::icon() const
{
	return QIcon(":/atcmgraph.png");
}

QString ATCMgraphPlugin::toolTip() const
{
	return QLatin1String("ATCM graph PLC");
}

QString ATCMgraphPlugin::whatsThis() const
{
	return QLatin1String("Put into the ATCMgraph the variable read from PLC");
}

bool ATCMgraphPlugin::isContainer() const
{
	return false;
}

QString ATCMgraphPlugin::domXml() const
{
	return QLatin1String("<widget class=\"ATCMgraph\" name=\"atcmGraph\">\n"
			" <property name=\"geometry\">\n"
			" <rect>\n"
			" <x>0</x>\n"
			" <y>0</y>\n"
			" <width>200</width>\n"
			" <height>100</height>\n"
			" </rect>\n"
			" </property>\n"
			" <property name=\"toolTip\" >\n"
			" <string>ATCM QGraph</string>\n"
			" </property>\n"
			" <property name=\"whatsThis\" >\n"
			" <string>ATCMgraph widget</string>\n"
			" </property>\n"
			"</widget>\n");
}

QString ATCMgraphPlugin::includeFile() const
{
	return QLatin1String("atcmplugin.h");
}
Q_EXPORT_PLUGIN2(customwidgetplugin, ATCMgraphPlugin)
