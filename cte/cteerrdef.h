#ifndef CTE_ERRDEF_H
#define CTE_ERRDEF_H

#include <QString>
#include <QChar>

const QChar chSeverityError = QChar::fromAscii('E');
const QChar chSeverityWarning = QChar::fromAscii('W');

// Struttura definizione Errore in CT
enum errorMessages_e
{
    errCTNoError = 0,
    errCTDuplicateName,
    errCTNoPriority,
    errCTNoUpdate,
    errCTNoName,
    errCTNameTooLong,
    errCTNoType,
    errCTNoDecimals,
    errCTNoDecimalZero,
    errCTWrongDecimals,
    errCTNoVarDecimals,
    errCTNoProtocol,
    errCTNoBITAllowed,
    errCTNoIP,
    errCTBadIP,
    errCTNoPort,
    errCTNoDevicePort,
    errCTWrongTCPPort,
    errCTNoNode,
    errCTNoRegister,
    errCTRegisterTooBig,
    errCTRegisterUsedTwice,
    errCTRegisterOverlapping,
    errCTInputOnlyModbus,
    errCTModBusServerDuplicate,
    errCTNoBehavior,
    errCTNoBit,
    errCTBadPriorityUpdate,
    errCTNoVar1,
    errCTNoCondition,
    errCTRiseFallNotBit,
    errCTInvalidNum,
    errCTNegWithUnsigned,
    errCTEmptyCondExpression,
    errCTNoVar2,
    errCTIncompatibleVar,
    errCTTotals
};

const int nErrCols = 5;
enum errorCols_e
{
    colErrSeverity = 0,
    colErrRow,
    colErrColumn,
    colErrVarName,
    colErrValue,
    colErrCodeErr,
    colErrMsgErr,
    colErrTotals
};

// Struttura per memorizzazione errori
struct Err_CT {
    QChar           cSeverity;              // Livello Errore
    int             nRow;                   // Riga errore
    int             nCol;                   // Colonna Errore
    QString         szVarName;              // Nome della Variabile associata
    QString         szValue;                // Valore letto
    int             nCodErr;                // Codice errore
    QString         szErrMessage;           // Error Message
};

#endif // CTE_ERRDEF_H
