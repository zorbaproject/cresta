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

    void on_Save_cities_clicked();

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

    int rankingcol= 4;
    int autodestcol= 5;
    int manualdestcol= 6;
};

#endif // MAINWINDOW_H
