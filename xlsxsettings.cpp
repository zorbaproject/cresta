#include "xlsxsettings.h"
#include "ui_xlsxsettings.h"

xlsxsettings::xlsxsettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::xlsxsettings)
{
    ui->setupUi(this);
}

xlsxsettings::~xlsxsettings()
{
    delete ui;
}

void xlsxsettings::set_ID(QStringList options, int selected) {
    ui->xs_ID->addItems(options);
    ui->xs_ID->setCurrentIndex(selected-1);
}

int xlsxsettings::get_ID() {
    return ui->xs_ID->currentIndex()+1;
}

void xlsxsettings::set_surname(QStringList options, int selected) {
    ui->xs_surname->addItems(options);
    ui->xs_surname->setCurrentIndex(selected-1);
}

int xlsxsettings::get_surname() {
    return ui->xs_surname->currentIndex()+1;
}

void xlsxsettings::set_name(QStringList options, int selected) {
    ui->xs_name->addItems(options);
    ui->xs_name->setCurrentIndex(selected-1);
}

int xlsxsettings::get_name() {
    return ui->xs_name->currentIndex()+1;
}

void xlsxsettings::set_curricula(QStringList options, int selected) {
    ui->xs_curricula->addItems(options);
    ui->xs_curricula->setCurrentIndex(selected-1);
}

int xlsxsettings::get_curricula() {
    return ui->xs_curricula->currentIndex()+1;
}

void xlsxsettings::set_year(QStringList options, int selected) {
    ui->xs_year->addItems(options);
    ui->xs_year->setCurrentIndex(selected-1);
}

int xlsxsettings::get_year() {
    return ui->xs_year->currentIndex()+1;
}

void xlsxsettings::set_priority(QStringList options, int selected) {
    ui->xs_priority->addItems(options);
    ui->xs_priority->setCurrentIndex(selected-1);
}

int xlsxsettings::get_priority() {
    return ui->xs_priority->currentIndex()+1;
}

void xlsxsettings::set_dest(QStringList options, int selected) {
    ui->xs_dest->addItems(options);
    ui->xs_dest->setCurrentIndex(selected-1);
}

int xlsxsettings::get_dest() {
    return ui->xs_dest->currentIndex()+1;
}

void xlsxsettings::set_lslm(QStringList options, int selected) {
    ui->xs_lslm->addItems(options);
    ui->xs_lslm->setCurrentIndex(selected-1);
}

int xlsxsettings::get_lslm() {
    return ui->xs_lslm->currentIndex()+1;
}

void xlsxsettings::set_available(QStringList options, int selected) {
    ui->xs_available->addItems(options);
    ui->xs_available->setCurrentIndex(selected-1);
}

int xlsxsettings::get_available() {
    return ui->xs_available->currentIndex()+1;
}

void xlsxsettings::set_lm(QString text) {
    ui->xs_lm->setText(text);
}

QString xlsxsettings::get_lm() {
    return ui->xs_lm->text();
}

bool xlsxsettings::get_addcities() {
    return ui->xs_addcities->isChecked();
}

void xlsxsettings::set_requisites(QStringList options, int selected) {
    ui->xs_requisites->addItems(options);
    ui->xs_requisites->setCurrentIndex(selected-1);
}

int xlsxsettings::get_requisites() {
    return ui->xs_requisites->currentIndex()+1;
}

void xlsxsettings::set_former(QStringList options, int selected) {
    ui->xs_former->addItems(options);
    ui->xs_former->setCurrentIndex(selected-1);
}

int xlsxsettings::get_former() {
    return ui->xs_former->currentIndex()+1;
}

void xlsxsettings::set_meanvote(QStringList options, int selected) {
    ui->xs_meanvote->addItems(options);
    ui->xs_meanvote->setCurrentIndex(selected-1);
}

int xlsxsettings::get_meanvote() {
    return ui->xs_meanvote->currentIndex()+1;
}

void xlsxsettings::set_letter(QStringList options, int selected) {
    ui->xs_letter->addItems(options);
    ui->xs_letter->setCurrentIndex(selected-1);
}

int xlsxsettings::get_letter() {
    return ui->xs_letter->currentIndex()+1;
}

void xlsxsettings::set_reserved(QStringList options, int selected) {
    ui->xs_reserved->addItems(options);
    ui->xs_reserved->setCurrentIndex(selected-1);
}

int xlsxsettings::get_reserved() {
    return ui->xs_reserved->currentIndex()+1;
}

void xlsxsettings::set_teacher(QStringList options, int selected) {
    ui->xs_teacher->addItems(options);
    ui->xs_teacher->setCurrentIndex(selected-1);
}

int xlsxsettings::get_teacher() {
    return ui->xs_teacher->currentIndex()+1;
}
