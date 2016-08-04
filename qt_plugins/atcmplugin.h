#ifndef ATCMPLUGIN_H
#define ATCMPLUGIN_H

class ATCMplugin
{
public:
    ATCMplugin();
protected:
    void incdecHvar(bool isVisible, int ctIndex);
    void incdecHvar(bool isVisible, int ctIndex1, int ctIndex2);
    void incdecHvar(bool isVisible, int ctIndex1, int ctIndex2, int ctIndex3, int ctIndex4, int ctIndex5, int ctIndex6, int ctIndex7, int ctIndex8, int ctIndex9, int ctIndex10, int ctIndex11, int ctIndex12);
private:
    bool m_isVisible;
};
#include "atcmlabel.h"
#include "atcmcombobox.h"
#include "atcmspinbox.h"
//#include "atcmlcdnumber.h"
#include "atcmbutton.h"
#include "atcmslider.h"
#include "atcmprogressbar.h"
#include "atcmled.h"
#include "atcmanimation.h"
#include "atcmgraph.h"
#include "atcmdate.h"
#include "atcmtime.h"

#endif // ATCMPLUGIN_H
