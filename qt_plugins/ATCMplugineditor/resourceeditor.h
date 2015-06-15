#ifndef RESOURCEEDITOR_H
#define RESOURCEEDITOR_H

#include <QDialog>
#include <QListWidget>

class QDialogButtonBox;

class ResourceEditor : public QDialog
{
    Q_OBJECT
    
public:
    ResourceEditor(QWidget *parent = 0, QString * selection = NULL);

	QSize sizeHint() const;
    
private slots:
    void chooseCTresource();

private:
	QListWidget *list;
	QDialogButtonBox *buttonBox;
    QString * m_selection;
};

#endif // RESOURCEEDITOR_H
