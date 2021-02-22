/****************************************************************************
**
** Copyright (C) 2016 MECT srl.
** Contact: http://www.mect.it/
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General
** Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 3.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#include "cteplugin.h"
#include "ctedit.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/imode.h>
#include <coreplugin/modemanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>

#include <QAction>
#include <QFileInfo>
#include <QMenu>
#include <QString>
#include <QLatin1String>
#include <QtPlugin>
#include <QDebug>
#include <QAction>
#include <iostream>
#include <QResource>


namespace CTE {
namespace Internal {

ctedit * ctEditor;

/*! A mode with a push button based on BaseMode */

class CTEMode : public Core::IMode
{
public:

    CTEMode()
    {
        ctEditor = new ctedit();
        setWidget(ctEditor);
        setContext(Core::Context("CTE.MainView"));
        setDisplayName(tr("MectSuite"));
        setIcon(QIcon(szPathIMG + QLatin1String("Go to.png")));
        setPriority(0);
        setId("CTE.CTEMode");
        setContextHelpId(QString());
        setEnabled(false);
    }
};

/*! Constructs the Crosstable Editor plugin. Normally plugins don't do anything
    in their constructor except for initializing their member variables. The
    actual work is done later, in the initialize() and extensionsInitialized()
    methods.
*/
CTEPlugin::CTEPlugin()
{
}

/*! Plugins are responsible for deleting objects they created on the heap, and
    to unregister objects from the plugin manager that they registered there.
*/
CTEPlugin::~CTEPlugin()
{
}

/*! Initializes the plugin. Returns true on success.
    Plugins want to register objects with the plugin manager here.

    \a errorMessage can be used to pass an error message to the plugin system,
       if there was any.
*/
bool
CTEPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    QResource::registerResource(QLatin1String("./qtc.qrc"));

    // Create a unique context for our own view, that will be used for the
    // menu entry later.
    Core::Context context("CTE.MainView");


    // Create an action to be triggered by a menu entry
    // QAction *CTEAction = new QAction(tr("&MECT Editor"), this);
    // connect(CTEAction, SIGNAL(triggered()), SLOT(enableIfCT()));

    // Register the action with the action manager
    //Core::Command *command = Core::ActionManager::registerAction(CTEAction, "CTE.CTEAction", context);


    // Create our own menu to place in the Tools menu
    //Core::ActionContainer *CTEMenu = Core::ActionManager::createMenu("CTE.CTEMenu");
    //QMenu *menu = CTEMenu->menu();
    //menu->setTitle(tr("&MECT Editor"));
    //menu->setEnabled(true);

    // Add the Crosstable Editor action command to the menu
    //CTEMenu->addAction(command);

    // Request the Tools menu and add the Crosstable Editor menu to it
    //Core::ActionContainer *toolsMenu = Core::ActionManager::actionContainer(Core::Constants::M_TOOLS);
    //toolsMenu->addMenu(CTEMenu);


    // Add a mode with a push button based on BaseMode. Like the BaseView,
    // it will unregister itself from the plugin manager when it is deleted.
    m_cteMode = new CTEMode;
    addAutoReleasedObject(m_cteMode);

    m_CT_Opened = false;
    m_CT_File.clear();
    m_currentProject = 0;

    // Current project has changed (or Closing).
    connect(
        ProjectExplorer::ProjectExplorerPlugin::instance(),
        SIGNAL(currentProjectChanged(ProjectExplorer::Project*)),
        SLOT(enableIfCT(ProjectExplorer::Project*))
    );
    // Session Manager Save project (or Open?)
    connect(
        ProjectExplorer::ProjectExplorerPlugin::instance()->session(),
        SIGNAL(aboutToSaveSession()),
        SLOT(checkSave2())
    );
    // Project files have changed. (Removed as TrendEditor add files to Current project....)
    // connect(
    //    ProjectExplorer::ProjectExplorerPlugin::instance(),
    //    SIGNAL(fileListChanged()),
    //    SLOT(enableIfCT())
    // );

    return true;
}

/*! Notification that all extensions that this plugin depends on have been
    initialized. The dependencies are defined in the plugins .pluginspec file.

    Normally this method is used for things that rely on other plugins to have
    added objects to the plugin manager, that implement interfaces that we're
    interested in. These objects can now be requested through the
    PluginManagerInterface.

    The CTEPlugin doesn't need things from other plugins, so it does
    nothing here.
*/
void
CTEPlugin::extensionsInitialized()
{
}

/*! Enable the Crosstable editor only when there is an active
    project and if it has a Crosstable file (Crosstable.csv).
 */
void
CTEPlugin::enableIfCT(ProjectExplorer::Project* p)
{
    bool    fFileExists = false;
    QString szFileCT;

    // Ask to Replace current file
    qDebug("Loading New File....");
    if (m_currentProject != p) {
        qDebug("Checking unsaved changes on previos project....");
        checkSave();
    }
    // Clear Project Path for Editor
    szFileCT.clear();
    m_CT_Opened = false;
    m_CT_File.clear();
    ctEditor->setProjectPath(szFileCT);

    // Retrieve current project if any
    // ProjectExplorer::Project *p = ProjectExplorer::ProjectExplorerPlugin::currentProject();
    // No project
    if (p == NULL) {
        m_cteMode->setEnabled(false);
        return;
    }
    // Retrieve Project directory
    QString pd = p->projectDirectory();
    // No project path
    if (pd.isEmpty()) {
        m_cteMode->setEnabled(false);
        return;
    }
    // Building Crosstable File Name
    szFileCT = pd + QLatin1String("/") + QLatin1String(Constants::CT_PROJ_REL_PATH) + QLatin1String("/") + QLatin1String(Constants::CT_FILE_NAME);
    qDebug("Checking File: %s", szFileCT.toLatin1().data());
    qDebug("Project File Path %s", pd.toLatin1().data());
    // Any Crosstable in the project?
    QFileInfo ctFile(szFileCT);
    fFileExists = ctFile.exists() && ctFile.isFile();
    // Opening file
    if (fFileExists)  {
        qDebug("File Found: %s CTE Enabled", szFileCT.toLatin1().data());
        ctEditor->setProjectPath(pd);
        fFileExists = ctEditor->selectCTFile(szFileCT);
    }
    else  {
        qDebug("File Not Found: %s", szFileCT.toLatin1().data());
    }
    // Enabling CTEditor and saving current file
    m_cteMode->setEnabled(fFileExists);
    m_CT_Opened = fFileExists;
    m_currentProject = p;
    if (fFileExists)
        m_CT_File = szFileCT;
    else
        m_CT_File.clear();

}
void
CTEPlugin::checkSave2()
{
    // Used to Ask Saving only when leaving Crosstable Editor
    qDebug("Check To Save2. Leaving CTE?");
    if (ctEditor->isVisible())  {
        checkSave();
    }
}
void
CTEPlugin::checkSave()
{
    // To be used always on Project Change and leaving Crosstable Editor
    qDebug("Check To Save");
    if (m_CT_Opened && ! m_CT_File.isEmpty())  {
        if (ctEditor->needSave())  {
            ctEditor->querySave();
        }
    }
}

} // namespace Internal
} // namespace CTE

Q_EXPORT_PLUGIN(CTE::Internal::CTEPlugin)
