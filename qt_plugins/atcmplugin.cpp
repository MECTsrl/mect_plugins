#include "atcmplugin.h"
#ifdef TARGET_ARM
#include "global_var.h" // ioComm
#endif

ATCMplugin::ATCMplugin()
{
    m_isVisible = false;
}

void ATCMplugin::incdecHvar(bool isVisible, int ctIndex)
{
#ifdef TARGET_ARM
    if (isVisible && !m_isVisible)
    {
        m_isVisible = true;
        if (ctIndex > 0)
        {
            ioComm->incHvarUsage(ctIndex);
        }
    }
    else if (!isVisible && m_isVisible)
    {
        m_isVisible = false;
        if (ctIndex > 0)
        {
            ioComm->decHvarUsage(ctIndex);
        }
    }
#endif
}

void ATCMplugin::incdecHvar(bool isVisible, int ctIndex1, int ctIndex2)
{
#ifdef TARGET_ARM
    if (isVisible && !m_isVisible)
    {
        m_isVisible = true;
        if (ctIndex1 > 0)
        {
            ioComm->incHvarUsage(ctIndex1);
        }
        if (ctIndex2 > 0)
        {
            ioComm->incHvarUsage(ctIndex2);
        }
    }
    else if (!isVisible && m_isVisible)
    {
        m_isVisible = false;
        if (ctIndex1 > 0)
        {
            ioComm->decHvarUsage(ctIndex1);
        }
        if (ctIndex2 > 0)
        {
            ioComm->decHvarUsage(ctIndex2);
        }
    }
#endif
}

void ATCMplugin::incdecHvar(bool isVisible, int ctIndex1, int ctIndex2, int ctIndex3, int ctIndex4, int ctIndex5, int ctIndex6, int ctIndex7, int ctIndex8, int ctIndex9, int ctIndex10, int ctIndex11, int ctIndex12)
{
#ifdef TARGET_ARM
    if (isVisible && !m_isVisible)
    {
        m_isVisible = true;
        if (ctIndex1 > 0)
        {
            ioComm->incHvarUsage(ctIndex1);
        }
        if (ctIndex2 > 0)
        {
            ioComm->incHvarUsage(ctIndex2);
        }
        if (ctIndex3 > 0)
        {
            ioComm->incHvarUsage(ctIndex3);
        }
        if (ctIndex4 > 0)
        {
            ioComm->incHvarUsage(ctIndex4);
        }
        if (ctIndex5 > 0)
        {
            ioComm->incHvarUsage(ctIndex5);
        }
        if (ctIndex6 > 0)
        {
            ioComm->incHvarUsage(ctIndex6);
        }
        if (ctIndex7 > 0)
        {
            ioComm->incHvarUsage(ctIndex7);
        }
        if (ctIndex8 > 0)
        {
            ioComm->incHvarUsage(ctIndex8);
        }
        if (ctIndex9 > 0)
        {
            ioComm->incHvarUsage(ctIndex9);
        }
        if (ctIndex10 > 0)
        {
            ioComm->incHvarUsage(ctIndex10);
        }
        if (ctIndex11 > 0)
        {
            ioComm->incHvarUsage(ctIndex11);
        }
        if (ctIndex12 > 0)
        {
            ioComm->incHvarUsage(ctIndex12);
        }
    }
    else if (!isVisible && m_isVisible)
    {
        m_isVisible = false;
        if (ctIndex1 > 0)
        {
            ioComm->decHvarUsage(ctIndex1);
        }
        if (ctIndex2 > 0)
        {
            ioComm->decHvarUsage(ctIndex2);
        }
        if (ctIndex3 > 0)
        {
            ioComm->decHvarUsage(ctIndex3);
        }
        if (ctIndex4 > 0)
        {
            ioComm->decHvarUsage(ctIndex4);
        }
        if (ctIndex5 > 0)
        {
            ioComm->decHvarUsage(ctIndex5);
        }
        if (ctIndex6 > 0)
        {
            ioComm->decHvarUsage(ctIndex6);
        }
        if (ctIndex7 > 0)
        {
            ioComm->decHvarUsage(ctIndex7);
        }
        if (ctIndex8 > 0)
        {
            ioComm->decHvarUsage(ctIndex8);
        }
        if (ctIndex9 > 0)
        {
            ioComm->decHvarUsage(ctIndex9);
        }
        if (ctIndex10 > 0)
        {
            ioComm->decHvarUsage(ctIndex10);
        }
        if (ctIndex11 > 0)
        {
            ioComm->decHvarUsage(ctIndex11);
        }
        if (ctIndex12 > 0)
        {
            ioComm->decHvarUsage(ctIndex12);
        }
    }
#endif
}

