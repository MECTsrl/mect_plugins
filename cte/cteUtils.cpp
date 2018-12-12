#include "ctecommon.h"
#include "parser.h"


QStringList lstPriority;                // Elenco dei valori di Priority
QStringList lstPriorityDesc;            // Descrizioni Priority

void initLists()
// Init delle Liste globali
{
    int nCol = 0;

    lstPriority.clear();
    for (nCol = 0; nCol < nNumPriority; nCol++)  {
        lstPriority.append(QString::number(nCol));
    }
    lstPriorityDesc.clear();
    lstPriorityDesc
        << QString::fromAscii("None")
        << QString::fromAscii("High")
        << QString::fromAscii("Medium")
        << QString::fromAscii("Low")
        ;

}
