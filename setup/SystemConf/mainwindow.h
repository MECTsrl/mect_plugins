#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_pushButton_Save_clicked();


private:
    Ui::MainWindow *ui;
    void save_all();
    QHash<QString, QString> LanguageMap;
};

extern char projectPath [256];

#define MAX_SPACE_AVAILABLE_MAX 128
#define LANGUAGE_MAP_FILE "C:/Qt485/desktop/share/qtcreator/templates/wizards/ATCM-template-project/lang_table.csv"
#define LINE_SIZE 1024
#define DEFAULT_LANGUAGE "it"

#endif // MAINWINDOW_H
