#ifndef XLSXSETTINGS_H
#define XLSXSETTINGS_H

#include <QDialog>

namespace Ui {
class xlsxsettings;
}

class xlsxsettings : public QDialog
{
    Q_OBJECT

public:
    explicit xlsxsettings(QWidget *parent = 0);
    ~xlsxsettings();

    void set_ID(QStringList options, int selected);
    int get_ID();

    void set_surname(QStringList options, int selected);
    int get_surname();

    void set_name(QStringList options, int selected);
    int get_name();

    void set_curricula(QStringList options, int selected);
    int get_curricula();

    void set_year(QStringList options, int selected);
    int get_year();

    void set_priority(QStringList options, int selected);
    int get_priority();

    void set_dest(QStringList options, int selected);
    int get_dest();

    void set_lslm(QStringList options, int selected);
    int get_lslm();

    void set_available(QStringList options, int selected);
    int get_available();

    void set_lm(QString text);
    QString get_lm();

    void set_meanvote(QStringList options, int selected);
    int get_meanvote();

    bool get_addcities();

    void set_requisites(QStringList options, int selected);
    int get_requisites();

    void set_former(QStringList options, int selected);
    int get_former();

private:
    Ui::xlsxsettings *ui;
};

#endif // XLSXSETTINGS_H
