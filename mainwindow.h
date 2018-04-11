#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QTableWidgetItem>
#include <QStringList>
#include <QString>
#include <QColor>
#include <QRegExp>
#include <QComboBox>
#include <QFlags>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCompleter>
#include "xlsxdocument.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QJsonObject database;
    double map(double x, double in_min, double in_max, double out_min, double out_max);
    QList<QTableWidgetItem*> findItemInColumn(QTableWidget *table, QString pattern, int column, Qt::MatchFlags match = Qt::MatchExactly);
    void resetUi();
    bool checkvalid = false;

private slots:

    void on_actionSave_project_triggered();

    void on_actionSave_project_as_triggered();

    void on_actionOpen_project_triggered();

    void on_new_student_clicked();

    void on_actionExit_triggered();

    void on_del_student_clicked();

    void on_save_student_clicked();

    void on_new_city_clicked();

    void on_del_city_clicked();

    void on_checkincomplete_clicked();

    void on_next_student_clicked();

    void on_prev_student_clicked();

    void on_students_table_cellClicked(int row, int column);

    void on_frm_ID_textChanged(const QString &arg1);

    void on_frm_name_textChanged(const QString &arg1);

    void on_frm_surname_textChanged(const QString &arg1);

    void on_frm_meanvote_textChanged(const QString &arg1);

    void on_calculateRanking_clicked();

    void on_actionAbout_Cresta_triggered();

    void on_actionHelp_triggered();

    void on_actionAbout_Qt_triggered();

    void on_ranking_table_cellClicked(int row, int column);

    void changerankingitem(const QString &arg1, int row, int column);

    void on_actionNew_triggered();

    void on_check_cities_clicked();

    void on_cities_table_cellChanged(int row, int column);

    void on_actionImport_xls_triggered();

private:
    Ui::MainWindow *ui;
    QString dbfile;
    void cities2db();
    void students2db();
    void ranking2db();
    void init_database();
    void db2cities();
    void db2students();
    void db2ranking();
    void do_ranking();
    void assign_destinations();

    void on_Save_cities_clicked();

    //columns in cities table
    int countycol = 0;
    int citycol = 1;
    int availablecol =2;

    //columns in students table
    int IDcol = 0;
    int namecol = 1;
    int surnamecol = 2;
    int curriculacol = 3;
    int yearcol = 4;
    int votecol = 5;
    int requisitescol = 6;
    int formercol = 7;
    int dest1col = 8;
    int dest2col = 9;
    int dest3col = 10;
    int dest4col = 11;

    //columns in ranking table
    int rankingcol= 4;
    int autodestcol= 5;
    int manualdestcol= 6;

    void loadLanguage(const QString& rLanguage);
    void createLanguageMenu(void);
    QTranslator m_translator;
    QTranslator m_translatorQt;
    QString m_currLang;
    QString m_langPath;

protected:
    void changeEvent(QEvent*);

protected slots:
    void slotLanguageChanged(QAction* action);

};

#endif // MAINWINDOW_H
