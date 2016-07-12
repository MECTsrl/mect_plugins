#include <QtGui>
#include <QtDesigner>

#include "atcmbutton.h"
#include "variableandvisibilitydialog.h"
#include "crosstableeditor.h"

	variableandvisibilityDialog::variableandvisibilityDialog(ATCMbutton *widget, QWidget *parent)
: QDialog(parent)
{
	m_widget = widget;

    QLabel * labelVariable = new QLabel("StatusVar:");
    lineVariable = new QLineEdit(m_widget->statusvar().trimmed());
    QPushButton * buttonVariable = new QPushButton("...");
    buttonVariable->setFixedWidth(30);
    connect(buttonVariable, SIGNAL(clicked()), this, SLOT(chooseCtVariable()));

    QLabel * labelVisibility = new QLabel("VisibilityVar:");
    lineVisibility = new QLineEdit(m_widget->visibilityVar().trimmed());
    QPushButton * buttonVisibility = new QPushButton("...");
    buttonVisibility->setFixedWidth(30);
    connect(buttonVisibility, SIGNAL(clicked()), this, SLOT(chooseCtVisibility()));

    QLabel * labelPassword = new QLabel("PasswordVar:");
    linePassword = new QLineEdit(m_widget->passwordVar().trimmed());
    QPushButton * buttonPassword = new QPushButton("...");
    buttonPassword->setFixedWidth(30);
    connect(buttonPassword, SIGNAL(clicked()), this, SLOT(chooseCtPassword()));

    QGridLayout *variablesLayout = new QGridLayout;
    variablesLayout->addWidget(labelVariable,0,0);
    variablesLayout->addWidget(lineVariable,0,1);
    variablesLayout->addWidget(buttonVariable,0,2);
    variablesLayout->addWidget(labelVisibility,1,0);
    variablesLayout->addWidget(lineVisibility,1,1);
    variablesLayout->addWidget(buttonVisibility,1,2);
    variablesLayout->addWidget(labelPassword,2,0);
    variablesLayout->addWidget(linePassword,2,1);
    variablesLayout->addWidget(buttonPassword,2,2);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
			| QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(variablesLayout);
    mainLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
    setWindowTitle(tr("ATCM Button Variables"));
}

QSize variableandvisibilityDialog::sizeHint() const
{
    return QSize(250, 100);
}

void variableandvisibilityDialog::saveState()
{
	if (QDesignerFormWindowInterface *formWindow
			= QDesignerFormWindowInterface::findFormWindow(m_widget)) {

        formWindow->cursor()->setProperty("statusvar", lineVariable->text().trimmed());
        formWindow->cursor()->setProperty("visibilityVar", lineVisibility->text().trimmed());
        formWindow->cursor()->setProperty("passwordVar", linePassword->text().trimmed());
    }
	accept();
}

void variableandvisibilityDialog::chooseCtVariable()
{
    QString value = lineVariable->text();
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        lineVariable->setText(value.trimmed());
    }
}

void variableandvisibilityDialog::chooseCtVisibility()
{
    QString value = lineVisibility->text();
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        lineVisibility->setText(value.trimmed());
    }
}

void variableandvisibilityDialog::chooseCtPassword()
{
    QString value = linePassword->text();
    CrossTableEditor dialog(m_widget, &value);
    if (dialog.exec() == Accepted)
    {
        linePassword->setText(value.trimmed());
    }
}
