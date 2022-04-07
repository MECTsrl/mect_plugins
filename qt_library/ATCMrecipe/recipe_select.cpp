/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include "app_logprint.h"
#include "recipe_select.h"
#include "ui_recipe_select.h"
#include "global_functions.h"

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
*/
/* this define set the window title */
#define WINDOW_TITLE "RECIPE SELECTOR"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the RECIPE_SELECT style.
 * the syntax is html stylesheet-like
 */
#define SET_RECIPE_SELECT_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
recipe_select::recipe_select(QWidget *parent) :
    page(parent),
    ui(new Ui::recipe_select)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */

    /* if exist and is not empty enable the WINDOW_ICON */
    if (strlen(WINDOW_ICON) > 0)
    {
        /* enable this line, define the WINDOW_ICON and put a Qlabel named labelIcon in your ui file if you want have a window icon */
        labelIcon = ui->labelIcon;
    }
    /* if exist and is not empty enable the WINDOW_TITLE */
    if (strlen(WINDOW_TITLE) > 0)
    {
        /* enable this line, define the WINDOW_TITLE and put a Qlabel named labelTitle in your ui file if you want have a window title */
        labelTitle = ui->labelTitle;
    }

    //setStyle::set(this);
    SET_RECIPE_SELECT_STYLE();

    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;

    strcpy(_recipe_dir_to_browse, RECIPE_DIR);
    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void recipe_select::reload()
{
    /*clean the label loading text*/
    ui->labelLoading->setText(trUtf8(""));

    /* show the recipe present into the file system */
    ui->listWidget->clear();

    QStringList recipeList;
    QDir recipeDir(_recipe_dir_to_browse, "*.csv");

    if (recipeDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot).count() != 0)
    {
        recipeList = recipeDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot);
        for (int i = 0; i < recipeList.count(); i++)
        {
            ui->listWidget->addItem(recipeList.at(i));
            LOG_PRINT(verbose_e, "Recipe '%s'\n", recipeList.at(i).toAscii().data());
        }
    }
    else if (recipeDir.entryList(QDir::Files).count() != 0)
    {
        recipeList = recipeDir.entryList(QDir::Files);
        for (int i = 0; i < recipeList.count(); i++)
        {
            ui->listWidget->addItem(QFileInfo(recipeList.at(i)).fileName());
            LOG_PRINT(verbose_e, "Recipe '%s'\n", QFileInfo(recipeList.at(i)).fileName().toAscii().data());
        }
    }
    else
    {
        LOG_PRINT(verbose_e, "No recipe to show\n");
        return;
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void recipe_select::updateData()
{
    page::updateData();
    ui->pushButtonSaveUSB->setEnabled(USBCheck());
}

void recipe_select::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
recipe_select::~recipe_select()
{
    delete ui;
}

#undef VAR_TO_DISPLAY

void recipe_select::on_pushButtonHome_clicked()
{
    strcpy(_recipe_dir_to_browse, RECIPE_DIR);
    go_home();
}

void recipe_select::on_pushButtonBack_clicked()
{
    /* we are at main level, go to the previous page */
    if (strcmp(_recipe_dir_to_browse, RECIPE_DIR) == 0)
    {
        go_back();
    }
    /* we aren't at main level, go to previus level */
    else
    {
        strcpy(_recipe_dir_to_browse, RECIPE_DIR);
        reload();
    }
}

void recipe_select::on_pushButtonSaveUSB_clicked()
{
    if (!QFile::exists(ZIP_BIN))
    {
        LOG_PRINT(error_e, "no zip command found\n");
        return;
    }

    /* Check USB key */
    if (USBCheck())
    {
        /* save the current recipe */
        char srcfilename [MAX_LINE] = "";
        char dstfilename [MAX_LINE] = "";

		if (USBmount() == false)
        {
            LOG_PRINT(error_e, "cannot mount the usb key\n");
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
            return;
        }

        /* compose the source file name and the destination file name */
        /* if one is selected save the item */
        char basedir[STR_LEN];
        if (ui->listWidget->currentIndex().isValid())
        {
            sprintf(_actual_recipe_, "%s/%s", _recipe_dir_to_browse, ui->listWidget->currentItem()->text().toAscii().data());
            sprintf(srcfilename, "%s", ui->listWidget->currentItem()->text().toAscii().data());
            sprintf(dstfilename, "%s/%s_%s.zip",
                    usb_mnt_point,
                    QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss").toAscii().data(),
                    ui->listWidget->currentItem()->text().toAscii().data());
            LOG_PRINT(error_e, "Save family '%s' into '%s'\n", srcfilename, dstfilename);
            if (!QFile::exists(srcfilename))
            {
                LOG_PRINT(error_e, "cannot find the file to zip '%s'\n", srcfilename);
                return;
            }
        }
        /* if none is selected save all */
        else
        {
            strcpy(srcfilename, "*");
            LOG_PRINT(error_e, "srcfilename %s\n", srcfilename);
            LOG_PRINT(error_e, "RECIPE_DIR %s\n", RECIPE_DIR);
            sprintf(dstfilename, "%s/%s_recipes.zip",
                    usb_mnt_point,
                    QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss").toAscii().data());
            LOG_PRINT(error_e, "SAVE ALL\n");
            if (ui->listWidget->count() == 0)
            {
                LOG_PRINT(error_e, "cannot find any file to zip.\n");
                return;
            }
        }

        strcpy(basedir, _recipe_dir_to_browse);

        /* zip the file, the sign file and delete them */
        if (zipAndSave(QStringList() << srcfilename, QString(dstfilename), false, basedir) == false)
        {
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot create the zip file '%1'").arg(dstfilename));
            USBumount();
            return;
        }

        /* unmount USB key */
        USBumount();

        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(0, trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).baseName()));
    }
}

void recipe_select::on_listWidget_itemClicked(QListWidgetItem *item)
{
        /* if it is a directory it is a receipe family, so enter and display the items */
        if (QFileInfo(QString("%1/%2").arg(_recipe_dir_to_browse).arg(item->text())).isDir())
        {
            strcpy(_recipe_dir_to_browse, QString("%1/%2").arg(_recipe_dir_to_browse).arg(item->text()).toAscii().data());
            reload();
        }
        /* if it is a file, it is already a receipe, so and display it */
        else
        {
            ui->labelLoading->setText(trUtf8("Loading..."));
            ui->labelLoading->setStyleSheet("color: rgb(255,0,0);");
            ui->labelLoading->repaint();
            sprintf(_actual_recipe_, "%s/%s", _recipe_dir_to_browse, item->text().toAscii().data());
            strcpy(_recipe_dir_to_browse, RECIPE_DIR);
            goto_page("recipe");
        }
}
