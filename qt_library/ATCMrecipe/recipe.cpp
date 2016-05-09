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
#include <QFileInfo>
#include <QMessageBox>
#include <unistd.h>

#include "app_logprint.h"
#include "recipe.h"
#include "utility.h"
#include "ui_recipe.h"

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
*/
/* this define set the window title */
#define WINDOW_TITLE "RECIPE"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ":/systemicons/img/Script.png"

/**
 * @brief this macro is used to set the RECIPE style.
 * the syntax is html stylesheet-like
 */
#define SET_RECIPE_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

row recipeMatrix[MAX_RCP_VAR];

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

    current_row = 0;
    current_column = 0;
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(ui->tableWidget->horizontalHeader(),SIGNAL(sectionClicked(int)), this,SLOT(horizontalHeaderClicked(int)));

    ui->tableWidget->setColumnWidth(0,200);

    _familyName[0] = '\0';
    _recipeName[0] = '\0';

    stepNbMax = 0;
    varNbMax = 0;

    state = 0;
}

void recipe::horizontalHeaderClicked(int column)
{
    if(column <= 0) return; //first column is names, doesn't represent a segment

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
        state = 0;
        /* load the actual receipt  */
        if (strlen(_actual_recipe_) == 0)
        {
            QMessageBox::critical(this,trUtf8("Empty recipe name"), trUtf8("No recipe is selected."));
            state = 0;
            go_back();
            return;
        }
        else if (loadRecipe(_actual_recipe_) == false)
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
        /* clear the table */
        ui->tableWidget->clear();
        ui->tableWidget->update();
        ui->labelStatus->clear();
        ui->labelStatus->repaint();
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
    if (ui->tableWidget->currentColumn() < 1)
    {
        return;
    }
    ui->pushButtonLoad->setEnabled(false);
    ui->pushButtonRead->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->labelStatus->setText(trUtf8("Writing"));
    ui->labelStatus->repaint();

    int stepIndex = ui->tableWidget->currentColumn() - 1;
    readRecipe(stepIndex);
    for (int varIndex = 0; varIndex < varNbMax; varIndex++)
    {
        int decimal = getVarDecimalByCtIndex(recipeMatrix[varIndex].step[stepIndex]);
        ui->tableWidget->item(varIndex, stepIndex + 1)->setText(QString::number(recipeMatrix[varIndex].step[stepIndex] / pow(10,decimal), 'f', decimal));
    }

    ui->pushButtonLoad->setEnabled(true);
    ui->pushButtonRead->setEnabled(true);
    ui->pushButtonSave->setEnabled(true);
    //ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
    ui->labelStatus->setText("");
    ui->labelStatus->repaint();

    return;
}

void recipe::on_pushButtonLoad_clicked()
{
    if (ui->tableWidget->currentColumn() < 1)
    {
        return;
    }
    ui->pushButtonLoad->setEnabled(false);
    ui->pushButtonRead->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->labelStatus->setText(trUtf8("Writing"));
    ui->labelStatus->repaint();
    writeRecipe(ui->tableWidget->currentColumn() - 1);
    ui->pushButtonLoad->setEnabled(true);
    ui->pushButtonRead->setEnabled(true);
    ui->pushButtonSave->setEnabled(true);
    ui->labelStatus->setText("");
    //ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
    ui->labelStatus->repaint();

    return;
}

void recipe::on_pushButtonSave_clicked()
{
    /* Edit the selected item */
    char fullfilename[FILENAME_MAX];
    FILE * fp;
    char value[DESCR_LEN];
    alphanumpad * dk;
    dk = new alphanumpad(value, QFileInfo(_actual_recipe_).baseName().toAscii().data());
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        LOG_PRINT(info_e, "Saving to '%s'\n", value);
        sprintf(fullfilename, "%s/%s.csv", QFileInfo(_actual_recipe_).absolutePath().toAscii().data(), value);
        fp = fopen(fullfilename, "w");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Cannot open '%s'\n", fullfilename);
        }
        else
        {
            LOG_PRINT(error_e, "opened '%s'\n", fullfilename);
            for (int row = 0; row < varNbMax; row++)
            {
                for (int col = 0; col < stepNbMax; col++)
                {
                    if (ui->tableWidget->item(row,col) != NULL)
                    {
                        fprintf(fp, "%s ", ui->tableWidget->item(row,col)->text().toAscii().data());
                        if (col < stepNbMax - 1)
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
            //ui->labelStatus->setText(QString("%1/%2").arg(_familyName).arg(_recipeName));
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

    if (current_row > 0)
    {
        ui->pushButtonDown->setEnabled(true);
    }
    else
    {
        ui->pushButtonUp->setEnabled(false);
    }
}

void recipe::on_pushButtonDown_clicked()
{
    current_row++;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (current_row < ui->tableWidget->rowCount() - 1)
    {
        ui->pushButtonUp->setEnabled(true);
    }
    else
    {
        ui->pushButtonDown->setEnabled(false);
    }
}

void recipe::on_pushButtonLeft_clicked()
{
    current_column--;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
}

void recipe::on_pushButtonRight_clicked()
{
    current_column++;
    current_row = (current_row < varNbMax) ? current_row : varNbMax - 1;
    current_column = (current_column < stepNbMax) ? current_column : stepNbMax - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
}

void recipe::on_tableWidget_itemClicked(QTableWidgetItem *item)
{
    if (item == NULL || ui->tableWidget->currentColumn() < 1)
    {
        return;
    }
    /* Edit the selected item */
    float value, min = 0, max = 0;
    numpad * dk;
    dk = new numpad(&value, item->text().toFloat(), min, max);
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
            int decimal = getVarDecimalByCtIndex(recipeMatrix[ui->tableWidget->currentRow()].ctIndex);
            sprintf(token, "%.*f",decimal,value);
            item->setText(token);
            recipeMatrix[ui->tableWidget->currentRow()].step[ui->tableWidget->currentColumn() - 1] = atof(token);
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
    return false;

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

/*
 * tag1; val1; val2 ... valn
 * ...
 * tagm; val1; val2 ... valn
 */
bool recipe::loadRecipe(const char * filename)
{
    FILE * fp;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(info_e, "Cannot open '%s'\n", filename);
        return false;
    }

    char line[1024] = "";
    char *p, *r;

    current_row = 0;
    current_column = 0;

    varNbMax = 0;
    stepNbMax = 0;

    for (int line_nb = 0; fgets(line, 1024, fp) != NULL && varNbMax < MAX_RCP_VAR; line_nb++)
    {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == 0) {
            LOG_PRINT(error_e, "skipping empty line\n");
            continue;
        }
        /* tag */
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s' at line %d\n", line, line_nb);
            continue;
        }
        int ctIndex;
        LOG_PRINT(info_e, "Loading variable '%s'\n", p);
        if (Tag2CtIndex(p, &ctIndex))
        {
            LOG_PRINT(error_e, "Invalid variable '%s' at line %d\n", p, line_nb);
            continue;
        }
        int decimal = getVarDecimalByCtIndex(ctIndex);
        LOG_PRINT(verbose_e, "recipeMatrix[%d].ctIndex = %d\n", varNbMax, (u_int16_t)ctIndex);
        recipeMatrix[varNbMax].ctIndex = (u_int16_t)ctIndex;

        /* values */
        u_int32_t value;
        float val_float;
        u_int8_t val_bit;
        int32_t val_int32;
        int16_t val_int16;

        for (stepNbMax = 0; stepNbMax < MAX_RCP_STEP && (p = strtok_csv(NULL, SEPARATOR, &r)) != NULL; stepNbMax++)
        {
            value = 0;
            // compute value
            switch (varNameArray[ctIndex].type)
            {
            case uintab_e:
            case uintba_e:
            case intab_e:
            case intba_e:
            {
                val_float = atof(p);
                for (int n = 0; n < decimal; ++n) {
                    val_float = val_float * 10;
                }
                val_int16 = (int16_t)val_float;
                value = (u_int32_t)val_int16;
                break;
            }
            case udint_abcd_e:
            case udint_badc_e:
            case udint_cdab_e:
            case udint_dcba_e:
            case dint_abcd_e:
            case dint_badc_e:
            case dint_cdab_e:
            case dint_dcba_e:
            {
                val_float = atof(p);
                for (int n = 0; n < decimal; ++n) {
                    val_float = val_float * 10;
                }
                val_int32 = (int32_t)val_float;
                memcpy(&value, &val_int32, sizeof(u_int32_t));
                break;
            }
            case fabcd_e:
            case fbadc_e:
            case fcdab_e:
            case fdcba_e:
            {
                val_float = atof(p);
                memcpy(&value, &val_float, sizeof(u_int32_t));
                break;
            }
            case bytebit_e:
            case wordbit_e:
            case dwordbit_e:
            case bit_e:
            {
                val_bit = atoi(p);
                value = (u_int32_t)val_bit;
                break;
            }
            default:
                /* unknown type */
                value = 0;
            }
            // assign value
            recipeMatrix[varNbMax].step[stepNbMax] = value;
            LOG_PRINT(verbose_e, "recipeMatrix[%d].step[%d] = %d;\n", varNbMax, stepNbMax, value);
        }
        varNbMax++;
    }

    LOG_PRINT(info_e, "row %d column %d\n", varNbMax, stepNbMax);
    fclose(fp);
    return true;
}


bool recipe::showRecipe(const char * familyName, const char * recipeName)
{
    //ui->labelStatus->setText(QString("%1/%2").arg(familyName).arg(recipeName));

    /* reset the current colum/row */
    current_row = 0;
    current_column = 0;

    /* first row */
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Tag"));

    for (int stepIndex = 0; stepIndex < stepNbMax; stepIndex++)
    {
        ui->tableWidget->setHorizontalHeaderItem(stepIndex + 1,
                                                 new QTableWidgetItem(QString::number(stepIndex + 1))
                                                 );
    }

    /* variable rows */
    ui->progressBarStatus->setVisible(true);
    ui->progressBarStatus->setMaximum(varNbMax);
    for (int varIndex = 0; varIndex < varNbMax; varIndex++)
    {
        ui->progressBarStatus->setValue(varIndex);
        int decimal = getVarDecimalByCtIndex(recipeMatrix[varIndex].ctIndex);
        ui->tableWidget->setItem(varIndex, 0, new QTableWidgetItem(varNameArray[recipeMatrix[varIndex].ctIndex].tag));

        for (int stepIndex = 0; stepIndex < stepNbMax; stepIndex++)
        {
            ui->tableWidget->setItem(varIndex, stepIndex + 1, new QTableWidgetItem(QString::number(recipeMatrix[varIndex].step[stepIndex] / pow(10,decimal), 'f', decimal)));
        }
    }
    ui->progressBarStatus->setVisible(false);

    return true;
}

int recipe::readRecipe(int step)
{
    QString value;
    int errors = 0;

    for (int varIndex = 0; varIndex < varNbMax; varIndex++)
    {
        //LOG_PRINT(error_e, "%d -> %d\n", varIndex, valuesTable[stepIndex]->at(varIndex));
        char msg[TAG_LEN];

        uint32_t valueu = 0;
        int ctIndex = recipeMatrix[varIndex].ctIndex;
        readFromDb(ctIndex, &valueu);
        LOG_PRINT(info_e, "%d -> %d\n", ctIndex, valueu);
        getFormattedVar(varNameArray[ctIndex].tag, &value, NULL);

        switch (getStatusVarByCtIndex(ctIndex, msg))
        {
        case BUSY:
            //retry_nb = 0;
            LOG_PRINT(info_e, "BUSY: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_PROGRESS);
            }
            errors++;
            break;
        case ERROR:
            LOG_PRINT(info_e, "ERROR: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_COMMUNICATION);
            }
            errors++;
            break;
        case DONE:
            strcpy(msg, value.toAscii().data());
            LOG_PRINT(info_e, "DONE %s\n", msg);
            recipeMatrix[varIndex].step[step] = atof(msg);
            break;
        default:
            LOG_PRINT(info_e, "OTHER: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_UNKNOWN);
            }
            errors++;
            break;
        }
        if (msg[0] != '\0')
        {
            LOG_PRINT(info_e, "Reading (%d) - '%s' - '%s'\n", varIndex, varNameArray[ctIndex].tag, msg);
        }
    }
    return errors;
}

int recipe::writeRecipe(int step)
{
    int errors = 0;

    beginWrite();
    for (int i = 0; i < varNbMax; i++)
    {
        u_int16_t addr = recipeMatrix[i].ctIndex;
        u_int32_t value = recipeMatrix[i].step[step];

        errors += addWrite(addr, &value);
        LOG_PRINT(info_e, "Writing (%d) - '%s' - '%d'\n", addr, varNameArray[addr].tag, value);
    }
    endWrite();
    sleep(1); // FIXME: HMI/PLC protocol

    return errors;
}

