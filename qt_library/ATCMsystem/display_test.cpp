/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include "display_test.h"
#include "ui_display_test.h"

/**
 * @brief this macro is used to set the display_test style.
 * the syntax is html stylesheet-like
 */
#define SET_SISPLAY_TEST_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
display_test::display_test(QWidget *parent) :
    page(parent),
    ui(new Ui::display_test)
{
    ui->setupUi(this);
}

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void display_test::reload()
{
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void display_test::updateData()
{
    static int counter = 1;
    QString mystyle;

    switch (counter * REFRESH_MS)
    {
    case 2000 :
        mystyle.append("QWidget"); \
        mystyle.append("{"); \
        mystyle.append("background-color: rgb(255, 0, 0);"); \
        mystyle.append("}"); \
        this->setStyleSheet(mystyle);
        this->update();
        break;
    case 4000 :
        mystyle.append("QWidget"); \
        mystyle.append("{"); \
        mystyle.append("background-color: rgb(0, 255, 0);"); \
        mystyle.append("}"); \
        this->setStyleSheet(mystyle);
        this->update();
        break;
    case 6000 :
        mystyle.append("QWidget"); \
        mystyle.append("{"); \
        mystyle.append("background-color: rgb(0, 0, 255);"); \
        mystyle.append("}"); \
        this->setStyleSheet(mystyle);
        this->update();
        break;
    default :
        break;
    }

    if (counter * REFRESH_MS >= 6000)
        counter = 1;
    else
        counter++;

    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void display_test::changeEvent(QEvent * event)
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
display_test::~display_test()
{
    delete ui;
}



void display_test::on_pushButton_clicked()
{
    goto_page("display_settings", false);
}
