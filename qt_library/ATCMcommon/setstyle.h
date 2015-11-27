/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief header for the global style sheet of the HMI
 */
#ifndef SET_STYLE_H
#define SET_STYLE_H

#include <QObject>
#include <QWidget>

#ifdef BACKGROUND_FILE
#undef BACKGROUND_FILE
#endif
#define BACKGROUND_FILE ""

#ifdef FONT_TYPE
#undef FONT_TYPE
#endif
#define FONT_TYPE "Ubuntu"

#ifdef FONT_SIZE
#undef FONT_SIZE
#endif
#define FONT_SIZE "20"

class setStyle: public QObject
{
public:
    setStyle():QObject(){};
    static void set(QWidget * parent);
    ~setStyle(){};
};

#endif // SET_STYLE_H
