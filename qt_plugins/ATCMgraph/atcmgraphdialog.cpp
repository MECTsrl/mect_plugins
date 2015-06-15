#include <QtGui>
#include <QtDesigner>

#include "atcmgraph.h"
#include "atcmgraphdialog.h"
#include "crosstableeditor.h"

atcmgraphDialog::atcmgraphDialog(ATCMgraph *widget, QWidget *parent)
    : QDialog(parent)
{
    m_widget = widget;

    /*
        X1min, X1max, X1
        Y1min, Y1max, Y1
        Display1
        --------
        X2min, X2max, X2
        Y2min, Y2max, Y2
        Display2
    */

    QLabel * labelX1Variable = new QLabel("X1 Variable:");
    lineX1Variable = new QLineEdit(m_widget->x1Variable().trimmed());
    QPushButton * buttonX1Variable = new QPushButton("...");
    buttonX1Variable->setFixedWidth(30);

    QLabel * labelX1minVariable = new QLabel("X1 min Variable:");
    lineX1minVariable = new QLineEdit(m_widget->x1Min().trimmed());
    QPushButton * buttonX1minVariable = new QPushButton("...");
    buttonX1minVariable->setFixedWidth(30);

    QLabel * labelX1maxVariable = new QLabel("X1 max Variable:");
    lineX1maxVariable = new QLineEdit(m_widget->x1Max().trimmed());
    QPushButton * buttonX1maxVariable = new QPushButton("...");
    buttonX1maxVariable->setFixedWidth(30);

    QLabel * labelY1Variable = new QLabel("Y1 Variable:");
    lineY1Variable = new QLineEdit(m_widget->y1Variable().trimmed());
    QPushButton * buttonY1Variable = new QPushButton("...");
    buttonY1Variable->setFixedWidth(30);

    QLabel * labelY1minVariable = new QLabel("Y1 min Variable:");
    lineY1minVariable = new QLineEdit(m_widget->y1Min().trimmed());
    QPushButton * buttonY1minVariable = new QPushButton("...");
    buttonY1minVariable->setFixedWidth(30);

    QLabel * labelY1maxVariable = new QLabel("Y1 max Variable:");
    lineY1maxVariable = new QLineEdit(m_widget->y1Max().trimmed());
    QPushButton * buttonY1maxVariable = new QPushButton("...");
    buttonY1maxVariable->setFixedWidth(30);

    QLabel * labelDisplay1Variable = new QLabel("Display1 Variable:");
    lineDisplay1Variable = new QLineEdit(m_widget->Display1().trimmed());
    QPushButton * buttonDisplay1Variable = new QPushButton("...");
    buttonDisplay1Variable->setFixedWidth(30);

    QGridLayout *groupbox1Layout = new QGridLayout;
    groupbox1Layout->addWidget(labelX1Variable,0,0);
    groupbox1Layout->addWidget(lineX1Variable,0,1);
    groupbox1Layout->addWidget(buttonX1Variable,0,2);

    groupbox1Layout->addWidget(labelX1minVariable,1,0);
    groupbox1Layout->addWidget(lineX1minVariable,1,1);
    groupbox1Layout->addWidget(buttonX1minVariable,1,2);

    groupbox1Layout->addWidget(labelX1maxVariable,2,0);
    groupbox1Layout->addWidget(lineX1maxVariable,2,1);
    groupbox1Layout->addWidget(buttonX1maxVariable,2,2);

    groupbox1Layout->addWidget(labelY1Variable,3,0);
    groupbox1Layout->addWidget(lineY1Variable,3,1);
    groupbox1Layout->addWidget(buttonY1Variable,3,2);

    groupbox1Layout->addWidget(labelY1minVariable,4,0);
    groupbox1Layout->addWidget(lineY1minVariable,4,1);
    groupbox1Layout->addWidget(buttonY1minVariable,4,2);

    groupbox1Layout->addWidget(labelY1maxVariable,5,0);
    groupbox1Layout->addWidget(lineY1maxVariable,5,1);
    groupbox1Layout->addWidget(buttonY1maxVariable,5,2);

    groupbox1Layout->addWidget(labelDisplay1Variable,6,0);
    groupbox1Layout->addWidget(lineDisplay1Variable,6,1);
    groupbox1Layout->addWidget(buttonDisplay1Variable,6,2);

    QGroupBox *group1 = new QGroupBox(tr("Pen 1:"));
    group1->setLayout(groupbox1Layout);
    group1->layout()->setMargin(4);

    QLabel * labelX2Variable = new QLabel("X2 Variable:");
    lineX2Variable = new QLineEdit(m_widget->x2Variable().trimmed());
    QPushButton * buttonX2Variable = new QPushButton("...");
    buttonX2Variable->setFixedWidth(30);

    QLabel * labelX2minVariable = new QLabel("X2 min Variable:");
    lineX2minVariable = new QLineEdit(m_widget->x2Min().trimmed());
    QPushButton * buttonX2minVariable = new QPushButton("...");
    buttonX2minVariable->setFixedWidth(30);

    QLabel * labelX2maxVariable = new QLabel("X2 max Variable:");
    lineX2maxVariable = new QLineEdit(m_widget->x2Max().trimmed());
    QPushButton * buttonX2maxVariable = new QPushButton("...");
    buttonX2maxVariable->setFixedWidth(30);

    QLabel * labelY2Variable = new QLabel("Y2 Variable:");
    lineY2Variable = new QLineEdit(m_widget->y2Variable().trimmed());
    QPushButton * buttonY2Variable = new QPushButton("...");
    buttonY2Variable->setFixedWidth(30);

    QLabel * labelY2minVariable = new QLabel("Y2 min Variable:");
    lineY2minVariable = new QLineEdit(m_widget->y2Min().trimmed());
    QPushButton * buttonY2minVariable = new QPushButton("...");
    buttonY2minVariable->setFixedWidth(30);

    QLabel * labelY2maxVariable = new QLabel("Y2 max Variable:");
    lineY2maxVariable = new QLineEdit(m_widget->y2Max().trimmed());
    QPushButton * buttonY2maxVariable = new QPushButton("...");
    buttonY2maxVariable->setFixedWidth(30);

    QLabel * labelDisplay2Variable = new QLabel("Display2 Variable:");
    lineDisplay2Variable = new QLineEdit(m_widget->Display2().trimmed());
    QPushButton * buttonDisplay2Variable = new QPushButton("...");
    buttonDisplay2Variable->setFixedWidth(30);

    QGridLayout *groupbox2Layout = new QGridLayout;
    groupbox2Layout->addWidget(labelX2Variable,0,0);
    groupbox2Layout->addWidget(lineX2Variable,0,1);
    groupbox2Layout->addWidget(buttonX2Variable,0,2);

    groupbox2Layout->addWidget(labelX2minVariable,1,0);
    groupbox2Layout->addWidget(lineX2minVariable,1,1);
    groupbox2Layout->addWidget(buttonX2minVariable,1,2);

    groupbox2Layout->addWidget(labelX2maxVariable,2,0);
    groupbox2Layout->addWidget(lineX2maxVariable,2,1);
    groupbox2Layout->addWidget(buttonX2maxVariable,2,2);

    groupbox2Layout->addWidget(labelY2Variable,3,0);
    groupbox2Layout->addWidget(lineY2Variable,3,1);
    groupbox2Layout->addWidget(buttonY2Variable,3,2);

    groupbox2Layout->addWidget(labelY2minVariable,4,0);
    groupbox2Layout->addWidget(lineY2minVariable,4,1);
    groupbox2Layout->addWidget(buttonY2minVariable,4,2);

    groupbox2Layout->addWidget(labelY2maxVariable,5,0);
    groupbox2Layout->addWidget(lineY2maxVariable,5,1);
    groupbox2Layout->addWidget(buttonY2maxVariable,5,2);

    groupbox2Layout->addWidget(labelDisplay2Variable,6,0);
    groupbox2Layout->addWidget(lineDisplay2Variable,6,1);
    groupbox2Layout->addWidget(buttonDisplay2Variable,6,2);

    QGroupBox *group2 = new QGroupBox(tr("Pen 2:"));
    group2->setLayout(groupbox2Layout);
    group2->layout()->setMargin(4);

    QHBoxLayout *variablesLayout = new QHBoxLayout;
    variablesLayout->addWidget(group1);
    variablesLayout->addWidget(group2);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(variablesLayout);
    mainLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("ATCM Graph Variables"));

    mapper = new QSignalMapper(this);

    mapper->setMapping(buttonX1Variable, 0);
    mapper->setMapping(buttonX1minVariable, 1);
    mapper->setMapping(buttonX1maxVariable, 2);
    mapper->setMapping(buttonY1Variable, 3);
    mapper->setMapping(buttonY1minVariable, 4);
    mapper->setMapping(buttonY1maxVariable, 5);
    mapper->setMapping(buttonDisplay1Variable, 6);

    mapper->setMapping(buttonX2Variable, 7);
    mapper->setMapping(buttonX2minVariable, 8);
    mapper->setMapping(buttonX2maxVariable, 9);
    mapper->setMapping(buttonY2Variable, 10);
    mapper->setMapping(buttonY2minVariable, 11);
    mapper->setMapping(buttonY2maxVariable, 12);
    mapper->setMapping(buttonDisplay2Variable, 13);

    connect(buttonX1Variable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonX1minVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonX1maxVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY1Variable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY1minVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY1maxVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonDisplay1Variable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonX2Variable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonX2minVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonX2maxVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY2Variable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY2minVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonY2maxVariable, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(buttonDisplay2Variable, SIGNAL(clicked()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(int)), this, SLOT(chooseVariable(int)));
}

QSize atcmgraphDialog::sizeHint() const
{
    return QSize(1000, 100);
}

void atcmgraphDialog::saveState()
{
    if (QDesignerFormWindowInterface *formWindow
            = QDesignerFormWindowInterface::findFormWindow(m_widget)) {

        formWindow->cursor()->setProperty("x1Variable", lineX1Variable->text().trimmed());
        formWindow->cursor()->setProperty("x1Min", lineX1minVariable->text().trimmed());
        formWindow->cursor()->setProperty("x1Max", lineX1maxVariable->text().trimmed());
        formWindow->cursor()->setProperty("y1Variable", lineY1Variable->text().trimmed());
        formWindow->cursor()->setProperty("y1Min", lineY1minVariable->text().trimmed());
        formWindow->cursor()->setProperty("y1Max", lineY1maxVariable->text().trimmed());
        formWindow->cursor()->setProperty("Display1", lineDisplay1Variable->text().trimmed());

        formWindow->cursor()->setProperty("x2Variable", lineX2Variable->text().trimmed());
        formWindow->cursor()->setProperty("x2Min", lineX2minVariable->text().trimmed());
        formWindow->cursor()->setProperty("x2Max", lineX2maxVariable->text().trimmed());
        formWindow->cursor()->setProperty("y2Variable", lineY2Variable->text().trimmed());
        formWindow->cursor()->setProperty("y2Min", lineY2minVariable->text().trimmed());
        formWindow->cursor()->setProperty("y2Max", lineY2maxVariable->text().trimmed());
        formWindow->cursor()->setProperty("Display2", lineDisplay2Variable->text().trimmed());
    }
    accept();
}

void atcmgraphDialog::chooseVariable(int index)
{
    QString value;
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        switch(index)
        {
        case 0:
            lineX1Variable->setText(value);
            break;
        case 1:
            lineX1minVariable->setText(value);
            break;
        case 2:
            lineX1maxVariable->setText(value);
            break;
        case 3:
            lineY1Variable->setText(value);
            break;
        case 4:
            lineY1minVariable->setText(value);
            break;
        case 5:
            lineY1maxVariable->setText(value);
            break;
        case 6:
            lineDisplay1Variable->setText(value);
            break;
        case 7:
            lineX2Variable->setText(value);
            break;
        case 8:
            lineX2minVariable->setText(value);
            break;
        case 9:
            lineX2maxVariable->setText(value);
            break;
        case 10:
            lineY2Variable->setText(value);
            break;
        case 11:
            lineY2minVariable->setText(value);
            break;
        case 12:
            lineY2maxVariable->setText(value);
            break;
        case 13:
            lineDisplay2Variable->setText(value);
            break;
        default:
            break;
        }
    }
}
