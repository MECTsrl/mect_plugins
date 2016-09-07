/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include "alphanumpad.h"
#include "app_logprint.h"
#include "item_selector.h"
#include "trend_option.h"
#include "ui_trend_option.h"
#include "global_functions.h"
#include "numpad.h"

#define DEFAULT_YMIN -1000
#define DEFAULT_YMAX  1000

/* this define set the window title */
#define WINDOW_TITLE "TREND MANAGER"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the TREND_OPTION style.
 * the syntax is html stylesheet-like
 */
#define SET_TREND_OPTION_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
trend_option::trend_option(QWidget *parent) :
    page(parent),
    ui(new Ui::trend_option)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_super_admin_e,pwd_admin_e, pwd_service_e, pwd_operator_e), default is pwd_operator_e
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
    SET_TREND_OPTION_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;
    
    QPalette pal = ui->comboBoxColor->palette(); // or comboBox->view()
    pal.setColor(QPalette::Highlight, Qt::transparent); // or partly transparent
    ui->comboBoxColor->setPalette(pal); // or comboBox->view()
    
    curve_palette << "FF0000" << "00FF00" << "0000FF" << "000000";
    
    //reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void trend_option::reload()
{
    if (_layout_ == PORTRAIT)
    {
        ui->pushButtonLayout->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Portrait.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
    }
    else
    {
        ui->pushButtonLayout->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Landscape.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
    }
    
    ui->pushButtonChangeTrend->setText(_actual_trend_);
    ui->pushButtonChangePen->setText(pens[actualPen].tag);
    ui->checkBoxVisible->setChecked(pens[actualPen].visible == 1);
    
    if (strlen(pens[actualPen].description) == 0)
    {
        ui->pushButtonDescription->setText("-");
    }
    else
    {
        ui->pushButtonDescription->setText(pens[actualPen].description);
    }
    
    int decimal = 1;
    if (pens[actualPen].tag[0] != '\0')
    {
        decimal =  getVarDecimalByName(pens[actualPen].tag);
    }
    ui->pushButtonYmin->setText(QString::number(pens[actualPen].yMin,'f',decimal));
    ui->pushButtonYmax->setText(QString::number(pens[actualPen].yMax,'f',decimal));
    
    QPixmap pixmap(25, 25);
    
    ui->comboBoxColor->clear();
    ui->comboBoxColor->addItem("");
    for (int i = 0; i < curve_palette.count(); i++)
    {
        ui->comboBoxColor->addItem("");
        pixmap.fill(QColor(QString("#%1").arg(curve_palette.at(i))));
        ui->comboBoxColor->setItemData(i + 1, pixmap, Qt::DecorationRole);
    }
    ui->comboBoxColor->addItem("");
    pixmap.fill(QColor(QString("#%1").arg(pens[actualPen].color)));
    ui->comboBoxColor->setItemData(curve_palette.count(), pixmap, Qt::DecorationRole);

    ui->comboBoxColor->setCurrentIndex(curve_palette.count());
    color[0] = '\0';
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void trend_option::updateData()
{
    //page::updateData();
    ui->pushButtonSaveUSB->setEnabled(USBCheck());
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void trend_option::changeEvent(QEvent * event)
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
trend_option::~trend_option()
{
    delete ui;
}

void trend_option::on_pushButtonHome_clicked()
{
    go_home();
}

void trend_option::on_pushButtonBack_clicked()
{
#if 0
    /* force a online */
    actualVisibleWindowSec = 0;
#endif
    // _trend_data_reload_ = true;
    go_back();
}

void trend_option::on_pushButtonLayout_clicked()
{
    if (_layout_ == PORTRAIT)
    {
        ui->pushButtonLayout->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Landscape.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
        _layout_ = LANDSCAPE;
    }
    else
    {
        ui->pushButtonLayout->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Portrait.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
        _layout_ = PORTRAIT;
    }
}

void trend_option::on_pushButtonChangeNew_clicked()
{
    char value[DESCR_LEN] = "trend";
    alphanumpad * dk;

    dk = new alphanumpad(value, value);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        char filename[MAX_LINE];
        LOG_PRINT(verbose_e, "Saving to '%s'\n", value);
        sprintf(filename, "%s/%s%s", CUSTOM_TREND_DIR, value, ".csv");
        if (!QFile::exists(filename))
        {
            /* set the default value */
            for (int i = 0; i < PEN_NB; i++)
            {
                pens[i].visible = 0;
                pens[i].tag[0]='\0';
                strcpy(pens[i].color, curve_palette.at(i).toAscii().data());
                pens[i].yMin = 0;
                pens[i].yMax = 1;
                pens[i].description[0]='\0';
            }
            strcpy(_actual_trend_, QFileInfo(filename).baseName().toAscii().data());
            Save(filename);
        }
        /* force a online */
        actualVisibleWindowSec = 0;
        LoadTrend(QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_).toAscii().data(), NULL);
        reload();
        _trend_data_reload_ = true;
    }
    else
    {
    }
    delete dk;
}

void trend_option::on_pushButtonChangeTrend_clicked()
{
    item_selector * sel;
    QString value;
    
    /* select a new trend */
    QStringList trendList;
    
    QDir trendDir(CUSTOM_TREND_DIR, "*.csv");
    
    if (trendDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot).count() != 0)
    {
        trendList = trendDir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot);
    }
    else if (trendDir.entryList(QDir::Files).count() != 0)
    {
        trendList = trendDir.entryList(QDir::Files);
    }
    else
    {
        LOG_PRINT(warning_e, "No trend to show\n");
        return;
    }
    
    sel = new item_selector(trendList, &value, trUtf8("TREND SELECTOR"));
    sel->showFullScreen();
    
    if (sel->exec() == QDialog::Accepted)
    {
        strcpy(_actual_trend_, QFileInfo(value).baseName().toAscii().data());
        /* force a online */
        actualVisibleWindowSec = 0;
        QString(errormsg);
        LoadTrend(QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_).toAscii().data(), &errormsg);
        reload();
        _trend_data_reload_ = true;
    }
    else
    {
    }
    delete sel;
}

void trend_option::on_pushButtonChangePen_clicked()
{
    /* select a new item */
    item_selector * sel;
    QString value;
    QStringList list;
    
    for ( int i = 0; StoreArrayS[i].tag[0] != '\0'; i++)
    {
        list.append(StoreArrayS[i].tag);
    }
    for ( int i = 0; StoreArrayF[i].tag[0] != '\0'; i++)
    {
        list.append(StoreArrayF[i].tag);
    }
    
    sel = new item_selector(list, &value,trUtf8("VARIABLE SELECTOR"));
    sel->showFullScreen();
    
    if (sel->exec() == QDialog::Accepted)
    {
        strcpy(pens[actualPen].tag, value.toAscii().data());
        ui->pushButtonChangePen->setText(value);
    }
    else
    {
    }
    delete sel;
}

void trend_option::on_pushButtonDeletePen_clicked()
{
    if (QMessageBox::question(this, trUtf8("Remove confirmation"), trUtf8("do you want remove the variable '%1'?\nThe operation will take effect after save the new options.").arg(pens[actualPen].tag), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        pens[actualPen].visible = 0;
        pens[actualPen].tag[0] = '\0';
        pens[actualPen].description[0] = '\0';
        reload();
    }
}

void trend_option::on_checkBoxVisible_clicked(bool checked)
{
    pens[actualPen].visible = checked;
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

void trend_option::on_pushButtonYmin_clicked()
{
    numpad * dk;
    float value;
    float min = MIN(DEFAULT_YMIN, pens[actualPen].yMax);
    float max = MIN(DEFAULT_YMAX, pens[actualPen].yMax);
    
    dk = new numpad(&value, ui->pushButtonYmin->text().toFloat(), min, max,false);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted)
    {
        if (value < min || value > max)
        {
            QMessageBox::critical(this,trUtf8("Invalid data"), trUtf8("The inserted value (%1) is invalid.\nThe value must ranging between %2 and %3").arg(value).arg(min).arg(max));
            delete dk;
            return;
        }
        pens[actualPen].yMin = value;
        //int decimal =  getVarDecimalByName(pens[actualPen].tag);
        ui->pushButtonYmin->setText(QString::number(pens[actualPen].yMin,'f',3));
    }
    else
    {
    }
    delete dk;
}

void trend_option::on_pushButtonYmax_clicked()
{
    numpad * dk;
    float value;
    float min = MAX(DEFAULT_YMIN, pens[actualPen].yMin);
    float max = MAX(DEFAULT_YMAX, pens[actualPen].yMin);
    
    dk = new numpad(&value, ui->pushButtonYmax->text().toFloat(), min, max,false);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted)
    {
        if (value < min || value > max)
        {
            QMessageBox::critical(this,trUtf8("Invalid data"), trUtf8("The inserted value (%1) is invalid.\nThe value must ranging between %2 and %3").arg(value).arg(min).arg(max));
            delete dk;
            return;
        }
        pens[actualPen].yMax = value;
        //int decimal =  getVarDecimalByName(pens[actualPen].tag);
        ui->pushButtonYmax->setText(QString::number(pens[actualPen].yMax,'f',3));
    }
    else
    {
    }
    delete dk;
}

void trend_option::on_pushButtonDescription_clicked()
{
    char value[DESCR_LEN];
    alphanumpad * dk;
    
    dk = new alphanumpad(value);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        LOG_PRINT(verbose_e, "Saving to '%s'\n", value);
        strcpy(pens[actualPen].description, value);
        ui->pushButtonDescription->setText(pens[actualPen].description);
    }
    else
    {
    }
    delete dk;
}

void trend_option::on_pushButtonSaveAs_clicked()
{
    /* Edit the selected item */
    char fullfilename[FILENAME_MAX];
    char value[DESCR_LEN];
    alphanumpad * dk;
    
    dk = new alphanumpad(value);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        LOG_PRINT(verbose_e, "Saving to '%s'\n", value);
        sprintf(fullfilename, "%s/%s.csv", CUSTOM_TREND_DIR, value);
        Save(fullfilename);
        strcpy(_actual_trend_, value);
        LoadTrend(QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_).toAscii().data(), NULL);
        reload();
        _trend_data_reload_ = true;
    }
    else
    {
    }
    delete dk;
}

void trend_option::on_pushButtonSave_clicked()
{
    char fullfilename[FILENAME_MAX];
    sprintf(fullfilename, "%s/%s.csv", CUSTOM_TREND_DIR, _actual_trend_);
    Save(fullfilename);
    ui->pushButtonChangeTrend->setText(_actual_trend_);
}

void trend_option::Save(const char * fullfilename)
{
    FILE * fp = fopen(fullfilename, "w");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", fullfilename);
    }
    else
    {
        /*
         * the file is formatted as
         * <Layout>
         * <Visible>;<Tag1>;<color>;<Ymin>;<Ymax>;<description>
         * <Visible>;<Tag2>;<color>;<Ymin>;<Ymax>;<description>
         * <Visible>;<Tag3>;<color>;<Ymin>;<Ymax>;<description>
         * <Visible>;<Tag4>;<color>;<Ymin>;<Ymax>;<description>
         */
        
        fprintf(fp, "%c\n", _layout_);
        
        if (strlen(color) > 0)
        {
            strcpy(pens[actualPen].color, color);
        }
        
        LOG_PRINT(verbose_e, "opened '%s'\n", fullfilename);
        for (int i = 0; i < PEN_NB; i++)
        {
            int decimal = 2;
            if (pens[i].CtIndex != -1)
            {
                decimal = getVarDecimal(pens[i].CtIndex);
            }
            fprintf(fp, "%d;%s;%s;%s;%s;%s\n",
                    pens[i].visible,
                    pens[i].tag,
                    pens[i].color,
                    QString::number(pens[i].yMin, 'f', decimal).toAscii().data(),
                    QString::number(pens[i].yMax, 'f', decimal).toAscii().data(),
                    pens[i].description
                    );
        }
        fclose(fp);
        LOG_PRINT(verbose_e, "Saved '%s'\n", fullfilename);
        QMessageBox::information(this,trUtf8("Save"), trUtf8("the trend configuration '%1' is saved into file '%2'").arg(_actual_trend_).arg(QFileInfo(fullfilename).baseName()));
    }
    /* force a reload */
    _trend_data_reload_ = true;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
}

void trend_option::on_comboBoxColor_currentIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }
    if (index > 0)
    {
        strcpy(color, curve_palette.at(index - 1).toAscii().data());
    }
    else
    {
        strcpy(color, pens[index].color);
    }
    LOG_PRINT(verbose_e, "New color at index %d '%s'\n", index, color);
}

void trend_option::on_pushButtonSaveUSB_clicked()
{
    /* save the trend description and the trend's screenshoot */
    
    /* Check USB key */
    if (USBCheck())
    {
        if (USBmount() == false)
        {
            LOG_PRINT(error_e, "cannot mount the usb key\n");
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
            return;
        }
        
        char dstfilename [MAX_LINE] = "";
        
        /* compose the source file name ans the destination file name */
        sprintf(dstfilename, "%s/%s_%s",
                usb_mnt_point,
                QDateTime::currentDateTime().toString("yy_MM_dd_hh_mm_ss").toAscii().data(),
                _actual_trend_);
        
        LOG_PRINT(verbose_e, "Save %s'\n", dstfilename);
        
        QStringList sourcelist;
        if (QFile::exists(QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_)))
        {
            sourcelist << QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_);
        }
        if (QDir().exists(QString("%1/%2").arg(SCREENSHOT_DIR).arg(_actual_trend_)))
        {
            sourcelist << QString("%1/%2").arg(SCREENSHOT_DIR).arg(_actual_trend_);
        }
        if (sourcelist.count() == 0)
        {
            QMessageBox::critical(this,trUtf8("No data"), trUtf8("Nothing to save for trend %1").arg(_actual_trend_));
            USBumount();
            return;
        }
        
        /* zip the file, the sign file and delete them */
        if (zipAndSave(sourcelist,QString("%1.zip").arg(dstfilename), true) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the zip file '%1'").arg(QString("%1.zip").arg(dstfilename)));
            USBumount();
            return;
        }
        
        /* unmount USB key */
        USBumount();
        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).baseName()));
    }
}


void trend_option::on_pushButtonPrev_clicked()
{
    actualPen--;
    actualPen = actualPen % PEN_NB;
    reload();
}

void trend_option::on_pushButtonNext_clicked()
{
    actualPen++;
    actualPen = actualPen % PEN_NB;
    reload();
}

void trend_option::on_pushButtonDeleteTrend_clicked()
{
    if (QMessageBox::question(this, trUtf8("Remove confirmation"), trUtf8("do you want remove the trend '%1'?").arg(ui->pushButtonChangeTrend->text()), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        char fullfilename [MAX_LINE];
        sprintf(fullfilename, "%s/%s.csv", CUSTOM_TREND_DIR, ui->pushButtonChangeTrend->text().toAscii().data());
        QFile::remove(fullfilename);
        QMessageBox::information(this,trUtf8("Removed"), trUtf8("Trend '%1' removed.").arg(ui->pushButtonChangeTrend->text()));
        on_pushButtonChangeTrend_clicked();
    }
}

