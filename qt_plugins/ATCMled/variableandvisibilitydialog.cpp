#include <QtGui>
#include <QtDesigner>

#include "atcmled.h"
#include "variableandvisibilitydialog.h"
#include "crosstableeditor.h"

	variableandvisibilityDialog::variableandvisibilityDialog(ATCMled *widget, QWidget *parent)
: QDialog(parent)
{
	m_widget = widget;

    QLabel * ledVariable = new QLabel("Variable:");
    lineVariable = new QLineEdit(m_widget->variable().trimmed());
    QPushButton * buttonVariable = new QPushButton("...");
    buttonVariable->setFixedWidth(30);
    connect(buttonVariable, SIGNAL(clicked()), this, SLOT(chooseCtVariable()));

    QLabel * ledVisibility = new QLabel("VisibilityVar:");
    lineVisibility = new QLineEdit(m_widget->visibilityVar().trimmed());
    QPushButton * buttonVisibility = new QPushButton("...");
    buttonVisibility->setFixedWidth(30);
    connect(buttonVisibility, SIGNAL(clicked()), this, SLOT(chooseCtVisibility()));

    QGridLayout *variablesLayout = new QGridLayout;
    variablesLayout->addWidget(ledVariable,0,0);
    variablesLayout->addWidget(lineVariable,0,1);
    variablesLayout->addWidget(buttonVariable,0,2);
    variablesLayout->addWidget(ledVisibility,1,0);
    variablesLayout->addWidget(lineVisibility,1,1);
    variablesLayout->addWidget(buttonVisibility,1,2);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
			| QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(variablesLayout);
    mainLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
    setWindowTitle(tr("ATCM Label Variables"));
}

QSize variableandvisibilityDialog::sizeHint() const
{
    return QSize(250, 100);
}

void variableandvisibilityDialog::saveState()
{
	if (QDesignerFormWindowInterface *formWindow
			= QDesignerFormWindowInterface::findFormWindow(m_widget)) {

        formWindow->cursor()->setProperty("variable", lineVariable->text().trimmed());
        formWindow->cursor()->setProperty("visibilityVar", lineVisibility->text().trimmed());
    }
	accept();
}

void variableandvisibilityDialog::chooseCtVariable()
{
    QString value;
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        lineVariable->setText(value);
    }
}

void variableandvisibilityDialog::chooseCtVisibility()
{
    QString value;
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        lineVisibility->setText(value);
    }
}
