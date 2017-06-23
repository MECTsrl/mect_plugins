/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include <QTableWidget>
#include <QTableWidgetItem>

#include <QFileInfo>
#include <QMessageBox>
#include <unistd.h>

#include "main.h"
#include "app_logprint.h"
#include "recipe.h"
#include "utility.h"
#include "ui_recipe.h"

#define ENABLE_DESCR

QList<u_int16_t> testsIndexes;
QList<u_int32_t> testsTable[MAX_RCP_STEP];
int stepNbMax;
int varNbMax;

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
*/
/* this define set the window title */
#define WINDOW_TITLE "RECIPE"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ":/libicons/img/Script.png"

/**
 * @brief this macro is used to set the RECIPE style.
 * the syntax is html stylesheet-like
 */
#define SET_RECIPE_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
recipe::recipe(QWidget *parent) :
    page(parent),
    ui(new Ui::recipe)
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
    //SET_RECIPE_STYLE();

    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;

    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(ui->tableWidget->horizontalHeader(),SIGNAL(sectionClicked(int)), this,SLOT(horizontalHeaderClicked(int)));

    // ui->tableWidget->setColumnWidth(0,230);

    _familyName[0] = '\0';
    _recipeName[0] = '\0';

    state = 0;
}

void recipe::horizontalHeaderClicked(int column)
{
     // [NO MORE!!] column #0 is names, last is #(1+stepNbMax-1)
    if (column < 0 || column >= stepNbMax) {
        return;
    }

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->tableWidget->selectColumn(column);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void recipe::reload()
{
    /* clear the status label and reset the progressbar */
    ui->progressBarStatus->setVisible(false);
    state = 1;
    current_row = 0;
    current_column = 0;
    stepNbMax = 0;
    varNbMax = 0;
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void recipe::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }

    if (state == 1)
    {
        ui->progressBarStatus->setVisible(true);
        ui->progressBarStatus->setValue(0);
        ui->progressBarStatus->update();
        state = 0;
        current_row = 0;
        current_column = 0;
        /* load the actual receipt  */
        if (strlen(_actual_recipe_) == 0)
        {
            QMessageBox::critical(this,trUtf8("Empty recipe name"), trUtf8("No recipe is selected."));
            state = 0;
            goto_page("recipe_select", false);
            return;
        }
        stepNbMax = loadRecipe(_actual_recipe_, &testsIndexes, testsTable);
        varNbMax = testsIndexes.count();
        if (stepNbMax <= 0)
        {
            QMessageBox::critical(this,trUtf8("Malformed recipe"), trUtf8("The recipe '%1' is malformed.").arg(_actual_recipe_));
            state = 0;
            go_back();
            return;
        }
        state = 2;
    }
    else if (state == 2)
    {
        state = 0;
        /* clear the table
        ui->tableWidget->clear();
        ui->tableWidget->update(); */
        ui->labelStatus->clear();
        ui->labelStatus->repaint();
        // Clear Table
        ui->tableWidget->setVisible(false);
        ui->tableWidget->setEnabled(false);
        ui->tableWidget->clearSelection();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
        ui->tableWidget->clearContents();
        ui->tableWidget->clear();

        state = 3;
    }
    else  if (state == 3)
    {
        state = 0;
        getFamilyRecipe(_actual_recipe_, _familyName, _recipeName);
        showRecipe(_familyName, _recipeName);
    }
    //LOG_PRINT(error_e, "UPDATE %d %d visible %d\n", refresh_timer->isActive(), refresh_timer->interval(), this->isVisible());

    /* call the parent updateData member */
    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void recipe::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}
#endif

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
recipe::~recipe()
{
    delete ui;
}

void recipe::on_pushButtonHome_clicked()
{
    go_home();
}

void recipe::on_pushButtonBack_clicked()
{
    go_back();
}

void recipe::on_pushButtonRead_clicked()
{
    if (ui->tableWidget->currentColumn() < 0)
    {
        QMessageBox::information(this,trUtf8("No step selected"), trUtf8("No step selected.\nPlease select a column to load from the panel"));
        return;
    }
    ui->pushButtonLoad->setEnabled(false);
    ui->pushButtonRead->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->labelStatus->setText(trUtf8("Reading"));
    ui->labelStatus->repaint();

    int stepIndex = ui->tableWidget->currentColumn();
    readRecipe(stepIndex, &testsIndexes, testsTable);
    for (int varIndex = 0; varIndex < varNbMax; varIndex++)
    {
        int decimal = getVarDecimalByCtIndex(testsIndexes[varIndex]);
        ui->tableWidget->item(varIndex, stepIndex)->setText(QString::number(testsTable[stepIndex][varIndex] / pow(10,decimal), 'f', decimal));
    }

    ui->pushButtonLoad->setEnabled(true);
    ui->pushButtonRead->setEnabled(true);
    ui->pushButtonSave->setEnabled(true);

#ifdef ENABLE_DESCR
    ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
#endif
    ui->labelStatus->setText("");
    ui->labelStatus->repaint();

    return;
}

void recipe::on_pushButtonLoad_clicked()
{
    if (ui->tableWidget->currentColumn() < 0)
    {
        QMessageBox::information(this,trUtf8("No step selected"), trUtf8("No step selected.\nPlease select a column to load into the panel"));
        return;
    }
    ui->pushButtonLoad->setEnabled(false);
    ui->pushButtonRead->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->labelStatus->setText(trUtf8("Writing"));
    ui->labelStatus->repaint();
    writeRecipe(ui->tableWidget->currentColumn(), &testsIndexes, testsTable);
    ui->pushButtonLoad->setEnabled(true);
    ui->pushButtonRead->setEnabled(true);
    ui->pushButtonSave->setEnabled(true);
    ui->labelStatus->setText("");
#ifdef ENABLE_DESCR
    ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
#endif
    ui->labelStatus->repaint();

    return;
}

void recipe::on_pushButtonSave_clicked()
{
    /* Edit the selected item */
    QTableWidgetItem    *tItem;
    char fullfilename[FILENAME_MAX];
    FILE * fp;
    char value[DESCR_LEN];
    alphanumpad * dk;
    dk = new alphanumpad(value, QFileInfo(_actual_recipe_).baseName().toAscii().data());
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        LOG_PRINT(verbose_e, "Saving to '%s'\n", value);
        sprintf(fullfilename, "%s/%s.csv", QFileInfo(_actual_recipe_).absolutePath().toAscii().data(), value);
        fp = fopen(fullfilename, "w");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", fullfilename);
        }
        else
        {
            LOG_PRINT(error_e, "opened '%s'\n", fullfilename);
            for (int row = 0; row < varNbMax; row++)
            {
                // name
                tItem = ui->tableWidget->verticalHeaderItem(row);
                if (tItem != NULL) {
                    fprintf(fp, "%s; ", tItem->text().toAscii().data());
                }
                // values
                for (int col = 0; col < stepNbMax; col++)
                {
                    if (ui->tableWidget->item(row, col) != NULL)
                    {
                        fprintf(fp, "%s ", ui->tableWidget->item(row,col)->text().toAscii().data());
                        if (col < stepNbMax -1)
                        {
                            fprintf(fp, ";");
                        }
                    }
                }
                fprintf(fp, "\n");
            }
            fclose(fp);
            LOG_PRINT(error_e, "Saved '%s'\n", fullfilename);

            getFamilyRecipe(fullfilename, _familyName, _recipeName);
#ifdef ENABLE_DESCR
            ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
#endif
        }
    }
    else
    {
    }
    delete dk;
}

void recipe::on_pushButtonUp_clicked()
{
    current_row--;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
    if (current_row > 0)
    {
        ui->pushButtonDown->setEnabled(true);
    }
    else
    {
        ui->pushButtonUp->setEnabled(false);
    }
#endif
}

void recipe::on_pushButtonDown_clicked()
{
    current_row++;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
    if (current_row < ui->tableWidget->rowCount() - 1)
    {
        ui->pushButtonUp->setEnabled(true);
    }
    else
    {
        ui->pushButtonDown->setEnabled(false);
    }
#endif
}

void recipe::on_pushButtonLeft_clicked()
{
    current_column--;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
#endif
}

void recipe::on_pushButtonRight_clicked()
{
    current_column++;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_column,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
#endif
}

void recipe::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    if (item == NULL || ui->tableWidget->currentColumn() < 0)
    {
        return;
    }
    /* Edit the selected item */
    float value, min = 0, max = 0;
    numpad * dk;
    dk = new numpad(&value, item->text().toFloat(), 3, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        if (min < max && (value < min || value > max))
        {
            QMessageBox::critical(this,trUtf8("Invalid data"), trUtf8("The inserted value (%1) is invalid.\nThe value must ranging between %2 and %3").arg(value).arg(min).arg(max));
        }
        else
        {
            char token[LINE_SIZE]="";
            int ctIndex = testsIndexes[ui->tableWidget->currentRow()];
            int decimal = getVarDecimalByCtIndex(ctIndex);
            int ivalue;
            sprintf(token, "%.*f",decimal,value);
            ivalue = intFormattedVarByCtIndex(ctIndex, token);
            testsTable[ui->tableWidget->currentColumn()][ui->tableWidget->currentRow()] = ivalue;
            sprintf_fromValue(token, ctIndex, ivalue, decimal, 10);
            item->setText(token);
        }
    }
    delete dk;
}

bool recipe::getFamilyRecipe(const char * filename, char * familyName, char * recipeName)
{
    /* extract family name and recipe name */
    if (filename == NULL || familyName == NULL || recipeName == NULL)
    {
        return false;
    }
    familyName[0] = '\0';
    recipeName[0] = '\0';
#ifndef ENABLE_DESCR
    return false;
#endif

    strcpy(familyName, filename);
    /* if exists, cut the extension */
    char * p = strrchr(familyName, '.');
    if (p)
    {
        *p = '\0';
    }
    /* extract the recipe name */
    p = strrchr(familyName, '/');
    if (p)
    {
        *p = '\0';
        p++;
        strcpy(recipeName, p);
        /* extract the family name */
        p = strrchr(familyName, '/');
        if (p)
        {
            p++;
            strcpy(familyName, p);
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool recipe::showRecipe(const char * familyName, const char * recipeName)
{
    QStringList         lstRowNames;
#ifdef ENABLE_DESCR
    ui->labelStatus->setText(QString("%1/%2").arg(familyName).arg(recipeName));
#endif

    /* reset the current colum/row */
    current_row = 0;
    current_column = 0;
    lstRowNames.clear();

    // Reset colum / row counters in Table
    ui->tableWidget->setRowCount(varNbMax);
    ui->tableWidget->setColumnCount(stepNbMax);
    /* first row */
    // ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Tag"));

    for (int stepIndex = 0; stepIndex < stepNbMax; stepIndex++)
    {
        ui->tableWidget->setHorizontalHeaderItem(stepIndex + 1,
                                                 new QTableWidgetItem(QString::number(stepIndex))
                                                 );
    }
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    /* variable rows */
    char buf[42];
    QTableWidgetItem    *tItem;

    ui->progressBarStatus->setMaximum(varNbMax);
    for (int varIndex = 0; varIndex < varNbMax; varIndex++)
    {
        int ctIndex = testsIndexes[varIndex];
        int decimal = getVarDecimalByCtIndex(ctIndex);

        ui->progressBarStatus->setValue(varIndex);

        lstRowNames.append(QString(varNameArray[ctIndex].tag));
        // ui->tableWidget->setItem(varIndex, 0, new QTableWidgetItem(QString(varNameArray[ctIndex].tag)));

        for (int stepIndex = 0; stepIndex < stepNbMax; stepIndex++)
        {
            int value = testsTable[stepIndex].at(varIndex);

            sprintf_fromValue(buf, ctIndex, value, decimal, 10);
            // Get Item from cell or allocate a new one
            tItem = ui->tableWidget->item(varIndex, stepIndex);
            if (tItem == 0)
                tItem = new QTableWidgetItem(QString(buf));
            else
                tItem->setText(QString(buf));
            // Set Item in cell
            ui->tableWidget->setItem(varIndex, stepIndex, tItem);
            // ui->tableWidget->setItem(varIndex, stepIndex + 1, new QTableWidgetItem(QString(buf)));
        }
    }

    // Selection mode of items and First Column width
    //ui->tableWidget->setColumnWidth(0,230);
    ui->tableWidget->setVerticalHeaderLabels(lstRowNames);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setVisible(true);
    ui->tableWidget->setEnabled(true);
    ui->progressBarStatus->setVisible(false);

    return true;
}
