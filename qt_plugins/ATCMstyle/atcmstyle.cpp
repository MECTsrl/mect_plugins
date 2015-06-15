#include <QtGui>
#include <stdio.h>

#include "atcmstyle.h"

int _diameter_ = 12;
double _penWidth_ = 1.0;
QIcon _icon_ = QIcon();

void ATCMStyle::polish(QPalette &palette)
{
    QPixmap backgroundImage(":/images/background.png");
    QColor ATCM(207, 155, 95);
    QColor veryLightBlue(239, 239, 247);
    QColor lightBlue(223, 223, 239);
    QColor darkBlue(95, 95, 191);

    palette = QPalette(ATCM);
    palette.setBrush(QPalette::Window, QColor(255,0,0));
    //palette.setBrush(QPalette::Window, backgroundImage);
    palette.setBrush(QPalette::BrightText, Qt::white);
    palette.setBrush(QPalette::Base, veryLightBlue);
    palette.setBrush(QPalette::AlternateBase, lightBlue);
    palette.setBrush(QPalette::Highlight, darkBlue);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight,
                     Qt::cyan);
    palette.setBrush(QPalette::ButtonText, Qt::red);
    palette.setBrush(QPalette::WindowText, Qt::red);
    palette.setBrush(QPalette::Text, Qt::red);
    palette.setBrush(QPalette::HighlightedText, Qt::red);
    palette.setColor(QPalette::ButtonText, Qt::red);
    palette.setColor(QPalette::WindowText, Qt::red);
    palette.setColor(QPalette::Text, Qt::red);
    palette.setColor(QPalette::HighlightedText, Qt::red);
}

void ATCMStyle::polish(QWidget *widget)
{
    Q_UNUSED( widget )
#ifdef ATCMSPINBOX_H
    if (qobject_cast<QAbstractButton *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget))
        widget->setAttribute(Qt::WA_Hover, true);
#endif
#ifdef ATCMPROGRESSBAR_H
    if (qobject_cast<QProgressBar *>(widget))
        widget->installEventFilter(this);
#endif
}

void ATCMStyle::unpolish(QWidget *widget)
{
    Q_UNUSED( widget )
#ifdef ATCMSPINBOX_H
    if (qobject_cast<QAbstractButton *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget))
        widget->setAttribute(Qt::WA_Hover, false);
#endif
#ifdef ATCMPROGRESSBAR_H
    if (qobject_cast<QProgressBar *>(widget))
        widget->removeEventFilter(this);
#endif
}

int ATCMStyle::styleHint(StyleHint which, const QStyleOption *option,
                         const QWidget *widget,
                         QStyleHintReturn *returnData) const
{
    //printf("SH %d\n", which);
    switch (which) {
    case SH_DialogButtonLayout:
        return int(QDialogButtonBox::MacLayout);
    case SH_EtchDisabledText:
        return int(true);
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return int(true);
    case SH_UnderlineShortcut:
        return int(false);
    default:
        return QMotifStyle::styleHint(which, option, widget,
                                      returnData);
    }
}

int ATCMStyle::pixelMetric(PixelMetric which,
                           const QStyleOption *option,
                           const QWidget *widget) const
{
    //printf("PM %d\n", which);
#if 1
    switch (which) {
    case PM_ButtonDefaultIndicator:
        return 0;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return 16;
    case PM_CheckBoxLabelSpacing:
        return 8;
    case PM_DefaultFrameWidth:
        return 2;
    case PM_ComboBoxFrameWidth:
        return 8;
#if SLIDER
    case PM_SliderThickness:
        return 16;
#endif
    default:
#endif
        return QMotifStyle::pixelMetric(which, option, widget);
#if 1
    }
#endif
}

void ATCMStyle::drawPrimitive(PrimitiveElement which,
                              const QStyleOption *option,
                              QPainter *painter,
                              const QWidget *widget) const
{
    //fprintf(stdout,"PE %d vs PE_FrameLineEdit=%d\n", which, PE_FrameLineEdit);
    switch (which) {
    case PE_IndicatorCheckBox:
        drawATCMCheckBoxIndicator(option, painter);
        break;
    case PE_PanelButtonCommand:
        drawATCMVBevel(option, painter);
        break;
    case PE_Frame:
        drawATCMFrame(option, painter);
        break;
    case PE_FrameDefaultButton:
        drawATCMVBevel(option, painter);
        //drawATCMFrameDefaultButton(option, painter);
        break;
    case PE_PanelLineEdit:
        drawATCMPanelLineEdit(option, painter);
        break;
    case PE_FrameLineEdit:
        drawATCMPanelLineEdit(option, painter);
        break;
    case PE_IndicatorProgressChunk:
    {
        QRect chunkRect;
        if (const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option))
        {
            //fprintf(stdout, "PE_IndicatorProgressChunk: max %d min %d value %d\n", pb2->maximum, pb2->minimum, pb2->progress);
            if (pb2->orientation == Qt::Horizontal)
            {
                chunkRect = QRect (
                            pb2->rect.x(),
                            pb2->rect.y(),
                            pb2->progress*(pb2->rect.width())/(pb2->maximum - pb2->minimum),
                            pb2->rect.height()
                            );
                //fprintf(stdout, "Horizontal PE_IndicatorProgressChunk: x: %d, y: %d, w: %d, h: %d\n", chunkRect.x(), chunkRect.y(), chunkRect.width(), chunkRect.height());
            }
            else
            {
                chunkRect = QRect (
                            pb2->rect.x(),
                            pb2->rect.y() + (pb2->rect.height() * (pb2->maximum - pb2->progress)/(pb2->maximum - pb2->minimum)),
                            pb2->rect.width(),
                            pb2->rect.height() - (pb2->rect.height() * (pb2->maximum - pb2->progress)/(pb2->maximum - pb2->minimum))
                            );
                //fprintf(stdout, "Vertical PE_IndicatorProgressChunk: x: %d, y: %d, w: %d, h: %d\n", chunkRect.x(), chunkRect.y(), chunkRect.width(), chunkRect.height());
            }
            QStyleOption buttonOpt(*option);
            buttonOpt.rect = chunkRect;
            painter->save();
            if (_penWidth_ > 0)
            {
                painter->setPen(QPen(option->palette.foreground(), _penWidth_));
            }
            else
            {
                painter->setPen(Qt::NoPen);
            }
            painter->setBrush(option->palette.button());
            if (pb2->orientation == Qt::Horizontal)
            {
                drawATCMVBevel(&buttonOpt, painter);
            }
            else
            {
                drawATCMHBevel(&buttonOpt, painter);
            }
            painter->restore();
        }
    }
        break;
    default:
        QMotifStyle::drawPrimitive(which, option, painter, widget);
    }
}

void ATCMStyle::drawControl(ControlElement element,
                            const QStyleOption *option,
                            QPainter *painter,
                            const QWidget *widget) const
{
    //printf("CE %d\n", element);
    switch (element) {
    case CE_PushButtonLabel:
    {
        QStyleOptionButton myButtonOption;
        const QStyleOptionButton *buttonOption =
                qstyleoption_cast<const QStyleOptionButton *>(option);
        if (buttonOption) {
            myButtonOption = *buttonOption;
            if (myButtonOption.palette.currentColorGroup()
                    != QPalette::Disabled) {
                if (myButtonOption.state & (State_Sunken | State_On)) {
                    myButtonOption.palette.setBrush(QPalette::ButtonText,
                                                    myButtonOption.palette.brightText());
                }
            }
        }
        QMotifStyle::drawControl(element, &myButtonOption, painter, widget);
    }
        break;
    case CE_PushButton:
    {
        QStyleOptionButton myButtonOption;
        const QStyleOptionButton *buttonOption =
                qstyleoption_cast<const QStyleOptionButton *>(option);
        if (buttonOption) {
            myButtonOption = *buttonOption;
        }
        QString text = myButtonOption.text;
        myButtonOption.text = "";

        //QMotifStyle::drawControl(element, &myButtonOption, painter, widget);
        drawATCMVBevel(&myButtonOption, painter);
        myButtonOption.text = text;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(option->palette.buttonText(), 1.0));

        int padding = (int)(2 + _penWidth_);

        QRect rectText;
        QRect rectIcon;

        if (!myButtonOption.icon.isNull() && myButtonOption.iconSize.width() > 0 && myButtonOption.iconSize.height() > 0)
        {
            rectText = QRect(
                        myButtonOption.rect.x(),
                        myButtonOption.rect.y(),
                        myButtonOption.rect.width() - myButtonOption.iconSize.width() + padding,
                        myButtonOption.rect.height()
                        );
        }
        else
        {
            rectText = myButtonOption.rect;
        }

        if (myButtonOption.text.length() > 0)
        {
            //Draw the text
            drawItemText(painter,rectText,Qt::AlignCenter|Qt::AlignVCenter,myButtonOption.palette, true, myButtonOption.text);
            rectIcon = QRect(
                        myButtonOption.rect.width() - myButtonOption.iconSize.width() - padding,
                        myButtonOption.rect.y(),
                        myButtonOption.iconSize.width(),
                        myButtonOption.rect.height()
                        );

        }
        else
        {
            rectIcon = QRect(
                        (myButtonOption.rect.width() - myButtonOption.iconSize.width()) / 2,
                        myButtonOption.rect.y(),
                        myButtonOption.iconSize.width(),
                        myButtonOption.rect.height()
                        );
        }

        if (!myButtonOption.icon.isNull() && myButtonOption.iconSize.width() > 0 && myButtonOption.iconSize.height() > 0)
        {
            //Draw the icon
            drawItemPixmap(painter, rectIcon,Qt::AlignCenter|Qt::AlignVCenter, myButtonOption.icon.pixmap(myButtonOption.iconSize));
        }

        painter->restore();
    }
        break;
#if 0
    case CE_ProgressBar:
    {
#if 0
        QMotifStyle::drawControl(element, option, painter, widget);
#else
        if (const QStyleOptionProgressBarV2 *ProgressOpt =
                qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option)) {


            QRect roundRect = ProgressOpt->rect.adjusted(+(int)_penWidth_,+(int)_penWidth_,-(int)_penWidth_,-(int)_penWidth_);
            if (!roundRect.isValid())
                return;

            int cx;
            if (roundRect.width() > 0)
            {
                cx = 100 * _diameter_ / roundRect.width();
            }
            else
            {
                cx = 0;
            }
            int cy;
            if (roundRect.height() > 0)
            {
                cy = 100 * _diameter_ / roundRect.height();
            }
            else
            {
                cy = 0;
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            /* border color */
            if (_penWidth_ > 0)
            {
                painter->setPen(QPen(ProgressOpt->palette.foreground(), _penWidth_));
            }
            else
            {
                painter->setPen(Qt::NoPen);
            }
            painter->setBrush(ProgressOpt->palette.background());
            painter->drawRoundRect(roundRect, cx, cy);
            drawPrimitive(PE_IndicatorProgressChunk, option, painter);
            painter->restore();
        }
#endif
    }
        break;
#endif
#if 1
    case CE_ProgressBarContents:
    {
        if (const QStyleOptionProgressBarV2 *ProgressOpt =
                qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option)) {


            QRect roundRect = ProgressOpt->rect.adjusted(+(int)_penWidth_,+(int)_penWidth_,-(int)_penWidth_,-(int)_penWidth_);
            if (!roundRect.isValid())
                return;

            int cx;
            if (roundRect.width() > 0)
            {
                cx = 100 * _diameter_ / roundRect.width();
            }
            else
            {
                cx = 0;
            }
            int cy;
            if (roundRect.height() > 0)
            {
                cy = 100 * _diameter_ / roundRect.height();
            }
            else
            {
                cy = 0;
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            /* border color */
            if (_penWidth_ > 0)
            {
                painter->setPen(QPen(ProgressOpt->palette.foreground(), _penWidth_));
            }
            else
            {
                painter->setPen(Qt::NoPen);
            }
            painter->setBrush(ProgressOpt->palette.background());
            painter->drawRoundRect(roundRect, cx, cy);
            drawPrimitive(PE_IndicatorProgressChunk, option, painter);
            painter->restore();
        }
    }
        break;
#endif
#if 0
    case CE_ProgressBarLabel:
    {
        QStyleOption buttonOpt(*option);
        /* handler color */
        buttonOpt.palette.setColor(QPalette::Button, option->palette.background().color());

        const QStyleOptionProgressBarV2 *progressOption =
                qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option);
        if (progressOption->orientation == Qt::Vertical)
        {
            drawATCMHBevel(&buttonOpt, painter);
        }
        else
        {
            drawATCMVBevel(&buttonOpt, painter);
        }

        //drawPrimitive(PE_IndicatorProgressChunk, option, painter);

    }
        break;
#endif
    default:
        QMotifStyle::drawControl(element, option, painter, widget);
    }
}

void ATCMStyle::drawComplexControl(ComplexControl which,
                                   const QStyleOptionComplex *option,
                                   QPainter *painter,
                                   const QWidget *widget) const
{
    //printf("CC %d\n", which);
    switch (which)
    {
    case CC_SpinBox:
    {
        //printf("CC_SpinBox\n");
        drawATCMSpinBoxButton(SC_SpinBoxDown, option, painter);
        drawATCMSpinBoxButton(SC_SpinBoxUp, option, painter);
#if 1
        drawATCMSpinBoxButton(SC_SpinBoxFrame, option, painter);
        drawATCMSpinBoxButton(SC_SpinBoxEditField, option, painter);
#if 0
        QRect r = subControlRect(CC_SpinBox, option, SC_SpinBoxEditField);

                       QStyleOptionFrame lineOpt;
                       lineOpt.QStyleOption::operator=(*option);
                       lineOpt.rect = r;
                       lineOpt.midLineWidth = 0;
                       lineOpt.state |= QStyle::State_Sunken;
                       drawPrimitive(QStyle::PE_FrameLineEdit, &lineOpt, painter);
#endif
#else
        QRect rect = subControlRect(CC_SpinBox, option,
                                    SC_SpinBoxEditField);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        //painter->setBrush(option->palette.mid());
        //painter->setBrush(option->palette.background());
        painter->setBrush(QColor(255,0,0));
        painter->setPen(QPen(Qt::blue, 1.0));
        //painter->setPen(QPen(option->palette.mid(), 1.0));
        // painter->setPen(QPen(Qt::blue, 1.0));
        //painter->drawLine(rect.topLeft(), rect.bottomLeft());
        //painter->drawLine(rect.topRight(), rect.bottomRight());
        //painter->drawRect(rect);
        painter->drawLine(rect.topRight(), rect.bottomLeft());
        painter->restore();
        //printf("CC_SpinBox\n");
#endif
    }
        break;
    case CC_ComboBox:
    {
        //printf("CC_ComboBox\n");
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            if (SC_ComboBoxFrame || SC_ComboBoxEditField) {
                QStyleOptionButton btn;
                btn.QStyleOption::operator=(*option);
                btn.rect = option->rect;
                if (SC_ComboBoxArrow)
                    btn.features = QStyleOptionButton::HasMenu;
                btn.palette.setBrush(QPalette::ButtonText, option->palette.buttonText());
                btn.text = cmb->currentText;
                //printf("text %s\n", cmb->currentText.toAscii().data());
                drawControl(QStyle::CE_PushButton, &btn, painter, widget);
            }
        }
    }
        break;
    case CC_Slider:
    {
        //printf("CC_Slider");
        if (const QStyleOptionSlider *sliderOpt =
                qstyleoption_cast<const QStyleOptionSlider *>(option)) {

            QRect ticRect = subControlRect(CC_Slider, option,
                                           SC_SliderHandle);
            QRect roundRect = subControlRect(CC_Slider, option,
                                             SC_SliderGroove);

            int cx;
            if (roundRect.width() > 0)
            {
                cx = 100 * _diameter_ / roundRect.width();
            }
            else
            {
                cx = 0;
            }
            int cy;
            if (roundRect.height() > 0)
            {
                cy = 100 * _diameter_ / roundRect.height();
            }
            else
            {
                cy = 0;
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            /* border color */
            if (_penWidth_ > 0)
            {
                painter->setPen(QPen(option->palette.foreground(), _penWidth_));
            }
            else
            {
                painter->setPen(Qt::NoPen);
            }
            painter->setBrush(option->palette.background());
            painter->drawRoundRect(roundRect, cx, cy);
            //fprintf(stdout, "Groove: x: %d, y: %d, w: %d, h: %d\n", roundRect.x(), roundRect.y(), roundRect.width(), roundRect.height());
#if 0
            /* add color */
            /* vertical orientation */
            if (sliderOpt->orientation == Qt::Vertical)
            {
                roundRect = QRect(
                            sliderOpt->rect.x(),
                            sliderOpt->rect.y(),
                            sliderOpt->rect.y() + sliderOpt->sliderValue * sliderOpt->rect.height() / (sliderOpt->maximum - sliderOpt->minimum),
                            sliderOpt->rect.height()
                            );
            }
            /* horizontal orientation */
            else
            {
                roundRect = QRect(
                            sliderOpt->rect.x() + sliderOpt->sliderValue * sliderOpt->rect.width() / (sliderOpt->maximum - sliderOpt->minimum),
                            sliderOpt->rect.y(),
                            sliderOpt->rect.width(),
                            sliderOpt->rect.height()
                            );
            }

            if (roundRect.width() > 0)
            {
                cx = 100 * _diameter_ / roundRect.width();
            }
            else
            {
                cx = 0;
            }
            if (roundRect.height() > 0)
            {
                cy = 100 * _diameter_ / roundRect.height();
            }
            else
            {
                cy = 0;
            }

            //fprintf(stdout, "Add: x: %d, y: %d, w: %d, h: %d\n", roundRect.x(), roundRect.y(), roundRect.width(), roundRect.height());

            painter->setBrush(option->palette.windowText());
            painter->drawRoundRect(roundRect, cx, cy);
#endif
            /* sub color */
            /* vertical orientation */
            if (sliderOpt->orientation == Qt::Vertical)
            {
                roundRect = QRect(
                            sliderOpt->rect.x(),
                            sliderOpt->rect.height() - (ticRect.y() + sliderOpt->sliderValue * sliderOpt->rect.height() / (sliderOpt->maximum - sliderOpt->minimum)),
                            sliderOpt->rect.width(),
                            sliderOpt->rect.height() - (sliderOpt->rect.height() - (ticRect.y() + sliderOpt->sliderValue * sliderOpt->rect.height() / (sliderOpt->maximum - sliderOpt->minimum)))
                            );
            }
            /* horizontal orientation */
            else
            {
                roundRect = QRect(
                            sliderOpt->rect.x(),
                            sliderOpt->rect.y(),
                            sliderOpt->rect.x() + sliderOpt->sliderValue * sliderOpt->rect.width() / (sliderOpt->maximum - sliderOpt->minimum),
                            sliderOpt->rect.height()
                            );
            }
            if (roundRect.width() > 0)
            {
                cx = 100 * _diameter_ / roundRect.width();
            }
            else
            {
                cx = 0;
            }
            if (roundRect.height() > 0)
            {
                cy = 100 * _diameter_ / roundRect.height();
            }
            else
            {
                cy = 0;
            }

            //fprintf(stdout, "Sub: x: %d, y: %d, w: %d, h: %d\n", roundRect.x(), roundRect.y(), roundRect.width(), roundRect.height());

            painter->setBrush(option->palette.buttonText());
            painter->drawRoundRect(roundRect, cx, cy);

            /* handler color */
            QStyleOptionButton buttonOpt;
            buttonOpt.QStyleOption::operator=(*option);
            buttonOpt.rect = option->rect.adjusted(-1, -1, 1, 1);
            buttonOpt.palette.setColor(QPalette::Button, option->palette.button().color());

            /* vertical orientation */
            if (sliderOpt->orientation == Qt::Vertical)
            {
                buttonOpt.rect = QRect(
                            ticRect.x() - 1,
                            sliderOpt->rect.height() - (ticRect.y() + sliderOpt->sliderValue * sliderOpt->rect.height() / (sliderOpt->maximum - sliderOpt->minimum)) - ticRect.height()/2,
                            option->rect.width() - ticRect.x(),
                            ticRect.height() - ticRect.y());
                drawATCMHBevel(&buttonOpt, painter);
            }
            /* horizontal orientation */
            else
            {
                buttonOpt.rect = QRect(
                            ticRect.x() + sliderOpt->sliderValue * sliderOpt->rect.width() / (sliderOpt->maximum - sliderOpt->minimum) - ticRect.width()/2/* - (sliderOpt->sliderValue - sliderOpt->minimum)  * ticRect.width()/2 / (sliderOpt->maximum - sliderOpt->minimum)*/,
                            ticRect.y() - 1,
                            ticRect.width() - ticRect.x(),
                            option->rect.height() - ticRect.y());
                drawATCMVBevel(&buttonOpt, painter);
            }
            //fprintf(stdout, "handle: x: %d, y: %d, w: %d, h: %d value %d\n", buttonOpt.rect.x(), buttonOpt.rect.y(), buttonOpt.rect.width(), buttonOpt.rect.height(), sliderOpt->sliderValue);
            //Draw the icon
            if (!_icon_.isNull())
            {
                drawItemPixmap(painter, buttonOpt.rect,Qt::AlignCenter|Qt::AlignVCenter, _icon_.pixmap(buttonOpt.rect.height()));
            }
            painter->restore();
        }
    }
        break;
    default:
        //printf("CC default %d\n", which);
        return QMotifStyle::drawComplexControl(which, option, painter, widget);
    }
}

QRect ATCMStyle::subControlRect(ComplexControl whichControl,
                                const QStyleOptionComplex *option,
                                SubControl whichSubControl,
                                const QWidget *widget) const
{
    if (whichControl == CC_SpinBox) {
#if 0
        int frameWidth = pixelMetric(PM_DefaultFrameWidth, option,
                                     widget);
#else
        int frameWidth = (int)(_penWidth_);
#endif
        int buttonWidth = option->rect.height();

        //printf("SC %d\n", whichSubControl);
        switch (whichSubControl) {
        case SC_SpinBoxFrame:
            return option->rect;
        case SC_SpinBoxEditField:
            return option->rect.adjusted(+buttonWidth, +frameWidth,
                                         -buttonWidth, -frameWidth);
        case SC_SpinBoxDown:
            return visualRect(option->direction, option->rect,
                              QRect(option->rect.x(), option->rect.y(),
                                    buttonWidth,
                                    option->rect.height()));
        case SC_SpinBoxUp:
            return visualRect(option->direction, option->rect,
                              QRect(option->rect.right() - buttonWidth,
                                    option->rect.y(),
                                    buttonWidth,
                                    option->rect.height()));
        default:
            return QRect();
        }
    }
#if SLIDER
    if (whichControl == CC_Slider) {
        switch (whichSubControl) {
        case SC_SliderHandle:
        {
            int buttonWidth = pixelMetric(PM_SliderThickness, option,
                                          widget);
            return QRect(option->rect.right() - buttonWidth,
                         option->rect.y(),
                         buttonWidth,
                         buttonWidth
                         );
        }
            break;
        default:
            return QMotifStyle::subControlRect(whichControl, option,
                                               whichSubControl, widget);
            break;
        }
    }
    else
#endif
    {
        return QMotifStyle::subControlRect(whichControl, option,
                                           whichSubControl, widget);
    }
}

QIcon ATCMStyle::standardIconImplementation(StandardPixmap which,
                                            const QStyleOption *option, const QWidget *widget) const
{
    QImage image = QMotifStyle::standardPixmap(which, option, widget)
            .toImage();
    if (image.isNull())
        return QIcon();

    QPalette palette;
    if (option) {
        palette = option->palette;
    } else if (widget) {
        palette = widget->palette();
    }

    QPainter painter(&image);
    painter.setOpacity(0.25);
    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter.fillRect(image.rect(), palette.highlight());
    painter.end();

    return QIcon(QPixmap::fromImage(image));
}

void ATCMStyle::drawATCMFrame(const QStyleOption *option,
                              QPainter *painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(option->palette.foreground(), 1.0));
    painter->drawRect(option->rect.adjusted(+1, +1, -1, -1));
    painter->restore();
}

void ATCMStyle::drawATCMFrameDefaultButton(const QStyleOption *option,
                                           QPainter *painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(option->palette.foreground(), 1.0));
    painter->drawRect(option->rect.adjusted(+1, +1, -1, -1));
    painter->restore();
}

void ATCMStyle::drawATCMPanelLineEdit(const QStyleOption *option,
                                      QPainter *painter) const
{
    //fprintf(stdout, "SPINBOX drawATCMPanelLineEdit %d %d %d %d\n", option->rect.y(), option->rect.y(), option->rect.width(), option->rect.height());
    return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(QColor(255,0,0));
    //painter->setBrush(option->palette.background());
    painter->setPen(QPen(QColor(0,255,0), 1.0));
    painter->drawRect(option->rect);
#if 0
    painter->drawRect(option->rect.adjusted(+(int)(_penWidth_/2),
                                            +(int)(_penWidth_/2),
                                            -(int)(_penWidth_/2),
                                            -(int)(_penWidth_/2)));
#endif
    painter->restore();
}

void ATCMStyle::drawATCMHBevel(const QStyleOption *option,
                               QPainter *painter) const
{
    QLinearGradient gradient(0, 0, option->rect.width() - _penWidth_, 0);
    if (option->state & QStyle::State_Raised)
    {
        QColor buttonColor = option->palette.button().color();
        int coeff = (option->state  /*& State_MouseOver*/) ? 115 : 105;

        gradient.setColorAt(0.0, option->palette.dark().color());
        gradient.setColorAt(0.2, buttonColor.lighter(coeff));
        gradient.setColorAt(0.8, option->palette.light().color());
        gradient.setColorAt(1.0, buttonColor.darker(coeff));
    }
    else if (option->state & State_Sunken)
    {
        QColor buttonColor = option->palette.button().color();
        int coeff = (option->state  /*& State_MouseOver*/) ? 115 : 105;

        gradient.setColorAt(0.0, buttonColor.lighter(coeff));
        gradient.setColorAt(0.2, buttonColor.darker(coeff));
        gradient.setColorAt(0.8, option->palette.dark().color());
        gradient.setColorAt(1.0, option->palette.light().color());
    }

    QRect roundRect = option->rect.adjusted(+(int)_penWidth_,+(int)_penWidth_,-(int)_penWidth_,-(int)_penWidth_);
    if (!roundRect.isValid())
        return;

    int cx;
    if (roundRect.width() > 0)
    {
        cx = 100 * _diameter_ / roundRect.width();
    }
    else
    {
        cx = 0;
    }
    int cy;
    if (roundRect.height() > 0)
    {
        cy = 100 * _diameter_ / roundRect.height();
    }
    else
    {
        cy = 0;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (_penWidth_ > 0)
    {
        painter->setPen(QPen(option->palette.foreground(), _penWidth_));
    }
    else
    {
        painter->setPen(Qt::NoPen);
    }
    if ((option->state & QStyle::State_Raised) || (option->state & QStyle::State_Sunken))
    {
        painter->setBrush(gradient);
    }
    else
    {
        painter->setBrush(option->palette.button());
    }

    //roundRect = roundRect.adjusted(_penWidth_,_penWidth_,-_penWidth_,-_penWidth_);
    painter->drawRoundRect(roundRect, cx, cy);
    painter->restore();
}

void ATCMStyle::drawATCMVBevel(const QStyleOption *option,
                               QPainter *painter) const
{
    QLinearGradient gradient(0, 0, 0, option->rect.height() - _penWidth_);
    if (option->state & State_Raised)
    {
        QColor buttonColor = option->palette.button().color();
        int coeff = (option->state /*& State_MouseOver*/) ? 115 : 105;

        gradient.setColorAt(0.0, option->palette.light().color());
        gradient.setColorAt(0.2, buttonColor.lighter(coeff));
        gradient.setColorAt(0.8, buttonColor.darker(coeff));
        gradient.setColorAt(1.0, option->palette.dark().color());
    }
    else if (option->state & State_Sunken)
    {
        QColor buttonColor = option->palette.button().color();
        int coeff = (option->state  /*& State_MouseOver*/) ? 115 : 105;

        gradient.setColorAt(0.0, buttonColor.lighter(coeff));
        gradient.setColorAt(0.2, buttonColor.darker(coeff));
        gradient.setColorAt(0.8, buttonColor.lighter(coeff));
        gradient.setColorAt(1.0, option->palette.light().color());
    }

    QRect roundRect = option->rect.adjusted(+(int)_penWidth_,+(int)_penWidth_,-(int)_penWidth_,-(int)_penWidth_);
    if (!roundRect.isValid())
        return;

    int cx;
    if (roundRect.width() > 0)
    {
        cx = 100 * _diameter_ / roundRect.width();
    }
    else
    {
        cx = 0;
    }
    int cy;
    if (roundRect.height() > 0)
    {
        cy = 100 * _diameter_ / roundRect.height();
    }
    else
    {
        cy = 0;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    /* set pen */
    if (_penWidth_ > 0)
    {
        painter->setPen(QPen(option->palette.foreground(), _penWidth_));
    }
    else
    {
        painter->setPen(Qt::NoPen);
    }

    /* set brush */
    if ((option->state & QStyle::State_Raised) || (option->state & QStyle::State_Sunken))
    {
        painter->setBrush(gradient);
    }
    else
    {
        painter->setBrush(option->palette.button());
    }

    //roundRect = roundRect.adjusted(_penWidth_,_penWidth_,-_penWidth_,-_penWidth_);
    painter->drawRoundRect(roundRect, cx, cy);
    painter->restore();
}

void ATCMStyle::drawATCMCheckBoxIndicator(
        const QStyleOption *option, QPainter *painter) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (option->state  /*& State_MouseOver*/) {
        painter->setBrush(option->palette.alternateBase());
    } else {
        painter->setBrush(option->palette.base());
    }
    painter->drawRoundRect(option->rect.adjusted(+1, +1, -1, -1));

    if (option->state & (State_On | State_NoChange)) {
        QPixmap pixmap;
        if (!(option->state & State_Enabled)) {
            pixmap.load(":/images/checkmark-disabled.png");
        } else if (option->state & State_NoChange) {
            pixmap.load(":/images/checkmark-partial.png");
        } else {
            pixmap.load(":/images/checkmark.png");
        }

        QRect pixmapRect = pixmap.rect()
                .translated(option->rect.topLeft())
                .translated(+2, -6);
        QRect painterRect = visualRect(option->direction, option->rect,
                                       pixmapRect);
        if (option->direction == Qt::RightToLeft) {
            painter->scale(-1.0, +1.0);
            painterRect.moveLeft(-painterRect.right() - 1);
        }
        painter->drawPixmap(painterRect, pixmap);
    }
    painter->restore();
}

void ATCMStyle::drawATCMSpinBoxButton(SubControl which,
                                      const QStyleOptionComplex *option, QPainter *painter) const
{
    /* this part draw the line edit into the spinbox */
    if (which == SC_SpinBoxEditField /*|| which == SC_SpinBoxFrame*/)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QRect labelRect = subControlRect(CC_SpinBox, option, which).adjusted(+(int)_penWidth_,+(int)_penWidth_,-(int)_penWidth_,-(int)_penWidth_);

        painter->setPen(QPen(option->palette.buttonText(), _penWidth_));
        painter->setBrush(option->palette.buttonText());
        painter->drawRect(labelRect);
        painter->restore();
        /*QPalette palette;
                palette.setBrush(QPalette::ButtonText, Qt::red);
        palette.setBrush(QPalette::Text, Qt::blue);
        drawItemText(painter,labelRect,Qt::AlignCenter|Qt::AlignVCenter,palette, true, "sp->text");
        */
        //QMotifStyle::drawComplexControl(CC_SpinBox, option, painter);
    }
    /* this part draw the button up and the button down into the spinbox */
    else if (which == SC_SpinBoxUp || which == SC_SpinBoxDown)
    {
        PrimitiveElement arrow = PE_IndicatorArrowLeft;
        QRect buttonRect = option->rect;
        if ((which == SC_SpinBoxUp)
                != (option->direction == Qt::RightToLeft)) {
            arrow = PE_IndicatorArrowRight;
            buttonRect.translate(buttonRect.width() / 2, 0);
        }

        buttonRect.setWidth((buttonRect.width() + 1) / 2);

        QStyleOption buttonOpt(*option);

        /* draw the button */
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setClipRect(buttonRect, Qt::IntersectClip);

        drawATCMVBevel(&buttonOpt, painter);

        /* draw the arrow */
        buttonOpt.state = State_Enabled;
        QStyleOption arrowOpt(buttonOpt);
        arrowOpt.rect = subControlRect(CC_SpinBox, option, which)
                .adjusted(+4 + (int)_penWidth_, +4 + (int)_penWidth_, -4 - (int)_penWidth_, -4 - (int)_penWidth_);
        if (arrowOpt.rect.isValid())
            drawPrimitive(arrow, &arrowOpt, painter);
        painter->restore();
    }
    else
    {
    }
}
