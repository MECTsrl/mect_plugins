/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief header for the global style sheet of the HMI
 */
#ifndef STYLE_H
#define STYLE_H
#include "setstyle.h"

/**
 * @brief this macro is used to set the style for all the pages: this macro will be called in the constuctor of every pages
 * if you need to customize the style of a particular page, you must to setup the stylesheet of that page
 * the syntax is html stylesheet-like
 */
void setStyle::set(QWidget * parent)
{
    QString mystyle;
    mystyle.append(parent->styleSheet());
    mystyle.append("QWidget");
    mystyle.append("{");
    /* load the file BACKGROUND_FILE from the file system an set is as the default background. */
    /* WARNING if you wanted it for a particular page, set the stylesheet in that page the back ground as url(""); */
    mystyle.append(QString("background-image: url(") + QString(BACKGROUND_FILE) + QString(");"));
    /* set as default font type and size */
    mystyle.append(QString("font: ") + QString(FONT_SIZE) + QString("pt \"") + QString(FONT_TYPE) + QString("\");"));
    mystyle.append("}");
    /* set as default font type and size of the QPushButton */
    mystyle.append("QPushButton");
    mystyle.append("{");
    mystyle.append(QString("font: ") + QString(FONT_SIZE) + QString("pt \"") + QString(FONT_TYPE) + QString("\");"));
    mystyle.append("}");
    /* set as default background, font type and size of the QMessageBox */
    mystyle.append("QMessageBox");
    mystyle.append("{");
    mystyle.append(QString("font: ") + QString(FONT_SIZE) + QString("pt \"") + QString(FONT_TYPE) + QString("\");"));
    mystyle.append("    background-image: url("");");
    mystyle.append("}");
    mystyle.append("QDialog");
    mystyle.append("{");
    mystyle.append(QString("background-image: url(") + QString(BACKGROUND_FILE) + QString(");"));
    mystyle.append(QString("font: ") + QString(FONT_SIZE) + QString("pt \"") + QString(FONT_TYPE) + QString("\");"));
    mystyle.append("}");
    parent->setStyleSheet(mystyle);
}

#endif // STYLE_H

