#ifndef ATCMPLUGINOBJECT_H
#define ATCMPLUGINOBJECT_H

class ATCMpluginObject
{
public:
    ATCMpluginObject();
protected:
    void incdecHvar(bool isVisible, int ctIndex);
    void incdecHvar(bool isVisible, int ctIndex1, int ctIndex2);
    void incdecHvar(bool isVisible, int ctIndex1, int ctIndex2, int ctIndex3, int ctIndex4, int ctIndex5, int ctIndex6, int ctIndex7, int ctIndex8, int ctIndex9, int ctIndex10, int ctIndex11, int ctIndex12);
private:
    bool m_isVisible;
};

#endif // ATCMPLUGINOBJECT_H
