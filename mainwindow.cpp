#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_database();
    createLanguageMenu();
    loadLanguage(QString("it"));
    resetUi();
    on_actionNew_triggered();

    ui->easteregg->setAttribute(Qt::WA_UnderMouse, false);
    on_easteregg_clicked();
    easteregg--;

    QHeaderView *header = qobject_cast<QTableView *>(ui->students_table)->horizontalHeader();
    connect(header, &QHeaderView::sectionClicked, [this](int logicalIndex){
        ui->students_table->sortByColumn(logicalIndex,Qt::AscendingOrder);
        //qDebug() << "Sorting by" << ui->students_table->horizontalHeaderItem(logicalIndex)->text();
    });
    QHeaderView *header2 = qobject_cast<QTableView *>(ui->cities_table)->horizontalHeader();
    connect(header2, &QHeaderView::sectionClicked, [this](int logicalIndex){
        ui->cities_table->sortByColumn(logicalIndex,Qt::AscendingOrder);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetUi()
{
    ui->students_table->resizeColumnsToContents();
    ui->ranking_table->resizeColumnsToContents();
    ui->cities_table->resizeColumnsToContents();
    ui->st_tab->setCurrentIndex(0);
    QIcon icon(":/cresta.png");
    setWindowIcon(icon);
    setWindowTitle(tr("Cresta"));
    ui->searchhere->clear();
    for (int i = 0; i < ui->students_table->columnCount(); i++) {
        ui->searchhere->addItem(ui->students_table->horizontalHeaderItem(i)->text());
    }
    ui->searchhere_2->clear();
    for (int i = 0; i < ui->ranking_table->columnCount(); i++) {
        ui->searchhere_2->addItem(ui->ranking_table->horizontalHeaderItem(i)->text());
    }
}

void MainWindow::createLanguageMenu(void)
{
    QActionGroup* langGroup = new QActionGroup(ui->menuLanguage);
    langGroup->setExclusive(true);

    connect(langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));

    QString defaultLocale = QLocale::system().name();
    defaultLocale.truncate(defaultLocale.lastIndexOf('_'));

    m_langPath = ":/languages/";
    QDir dir(m_langPath);
    QStringList fileNames = dir.entryList(QStringList("Translation_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        QString locale;
        locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);

        QString lang = QLocale::languageToString(QLocale(locale).language());
        QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

        QAction *action = new QAction(ico, lang, this);
        action->setCheckable(true);
        action->setData(locale);

        ui->menuLanguage->addAction(action);
        langGroup->addAction(action);

        if (defaultLocale == locale)
        {
            action->setChecked(true);
        }
    }
}

void MainWindow::slotLanguageChanged(QAction* action)
{
    if(0 != action) {
        loadLanguage(action->data().toString());
        setWindowIcon(action->icon());
    }
}

void switchTranslator(QTranslator& translator, const QString& filename)
{
    qApp->removeTranslator(&translator);

    if(translator.load(filename)) {
        qApp->installTranslator(&translator);
    }
}

void MainWindow::loadLanguage(const QString& rLanguage)
{
    if(m_currLang != rLanguage) {
        m_currLang = rLanguage;
        QLocale locale = QLocale(m_currLang);
        QLocale::setDefault(locale);
        switchTranslator(m_translator, m_langPath + QString("Translation_%1.qm").arg(rLanguage));
        QString tr_qtpath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        if (tr_qtpath.right(1) != "/") tr_qtpath.append("/");
        switchTranslator(m_translatorQt, tr_qtpath + QString("qt_%1.qm").arg(rLanguage));
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event) {
        // this event is send if a translator is loaded
        if (event->type() == QEvent::LanguageChange) {
            ui->retranslateUi(this);
            resetUi();
        }

        // this event is send if the system language changes
        if (event->type() == QEvent::LocaleChange) {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
            resetUi();
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::init_database()
{
    dbfile = "";
    database = QJsonDocument::fromJson(QString("{\"cities\":[],\"students\":[],\"ranking\":[]}").toUtf8()).object();
}


void MainWindow::on_actionSave_project_triggered()
{
    if (dbfile == "") {
        on_actionSave_project_as_triggered();
        return;
    }

    if (!(dbfile.isEmpty())) {
        sanitizeTable(ui->cities_table);
        sanitizeTable(ui->students_table);
        sanitizeTable(ui->ranking_table);
        cities2db();
        students2db();
        ranking2db();
        QString dbstr = QJsonDocument(database).toJson();
        QString problem = "    ]}    ]}";
        dbstr = dbstr.replace(problem,"    ]}");
        QFile file(dbfile);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream << dbstr;
        }
    }
}

void MainWindow::on_actionSave_project_as_triggered()
{
    dbfile = QFileDialog::getSaveFileName(this, tr("Save project"), QDir::currentPath(), "*.cresta|Cresta Project (*.cresta)");
    if (!dbfile.isEmpty()) {
        if (dbfile.right(7)!=".cresta") {
            if (dbfile.right(1)!=".") dbfile = dbfile + ".";
            dbfile = dbfile + "cresta";
        }
        setWindowTitle(dbfile);
        on_actionSave_project_triggered();
    }
}

void MainWindow::on_actionOpen_project_triggered()
{
    dbfile = QFileDialog::getOpenFileName(this, tr("Open project"), QDir::currentPath(), "*.cresta|Cresta Project (*.cresta)");
    if (!(dbfile.isEmpty())) {
        QFile file(dbfile);
        QString data = "";
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        while (!in.atEnd()) {
            data += in.readLine();
        }
        file.close();

        QString problem = "    ]}    ]}";
        data = data.replace(problem,"    ]}");
        database = QJsonDocument::fromJson(data.toUtf8()).object();
    }
    db2cities();
    db2ranking();
    db2students();
    setWindowTitle(dbfile);
}

void MainWindow::on_new_student_clicked()
{
    if (findItemInColumn(ui->students_table,ui->frm_ID->text(),IDcol).count() > 0) {
        QMessageBox::critical(this,tr("This is not funny"),tr("You can't have two students with the same ID"));
        return;
    }
    int row = ui->students_table->rowCount();
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() + 1;
    ui->students_table->insertRow(row);
    QTableWidgetItem *titem = new QTableWidgetItem ;
    titem->setText(ui->frm_ID->text());
    ui->students_table->setItem(row,IDcol,titem);
    ui->students_table->setCurrentCell(row,IDcol);
    on_save_student_clicked();
}

void MainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Close"),tr("Do you really want to close this program?"), QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel) {
        return;
    }
    QCoreApplication::quit();
}

void MainWindow::on_del_student_clicked()
{
    if (ui->students_table->selectedItems().count() > 0) ui->students_table->removeRow(ui->students_table->selectedItems()[0]->row());
}

void MainWindow::on_save_student_clicked()
{
    if (ui->students_table->selectedItems().count() == 0) {
        on_new_student_clicked();
        return;
    }
    if (ui->students_table->selectedItems().count() > 0) {
        int row = ui->students_table->selectedItems()[0]->row();

        for (int column = 0; column < ui->students_table->columnCount(); column++) {
            QString mytext = "";
            if (column == curriculacol) mytext = ui->frm_curricola->text();
            if (column == dest1col) mytext = ui->frm_dest1->text();
            if (column == dest2col) mytext = ui->frm_dest2->text();
            if (column == dest3col) mytext = ui->frm_dest3->text();
            if (column == dest4col) mytext = ui->frm_dest4->text();
            if (column == formercol && ui->frm_former->isChecked()) mytext = "1";
            if (column == formercol && !ui->frm_former->isChecked()) mytext = "0";
            if (column == IDcol) mytext = ui->frm_ID->text();
            if (column == votecol) mytext = ui->frm_meanvote->text();
            if (column == namecol) mytext = ui->frm_name->text();
            if (column == yearcol) mytext = ui->frm_registration->currentText();
            if (column == requisitescol && ui->frm_requisites->isChecked()) mytext = "1";
            if (column == requisitescol && !ui->frm_requisites->isChecked()) mytext = "0";
            if (column == surnamecol) mytext = ui->frm_surname->text();
            if (column == reservedcol && ui->frm_reserved->isChecked()) mytext = "1";
            if (column == reservedcol && !ui->frm_reserved->isChecked()) mytext = "0";
            if (column == lettercol) mytext = ui->frm_letter->text();
            QTableWidgetItem *newItem = new QTableWidgetItem(mytext);
            ui->students_table->setItem(row, column, newItem);
        }
    }
    students2db();
}


void MainWindow::on_new_city_clicked()
{
    int row = ui->cities_table->rowCount();
    if (ui->cities_table->selectedItems().count() > 0) row = ui->cities_table->selectedItems()[0]->row() + 1;
    ui->cities_table->insertRow(row);
    QTableWidgetItem *titem = new QTableWidgetItem ;
    titem->setText("");
    ui->cities_table->setItem(row,0,titem);
    ui->cities_table->setCurrentCell(row,0);
}

void MainWindow::on_del_city_clicked()
{
    if (ui->cities_table->selectedItems().count() > 0) ui->cities_table->removeRow(ui->cities_table->selectedItems()[0]->row());
    on_cities_table_cellChanged(0,citycol);
}

void MainWindow::on_Save_cities_clicked()
{
    cities2db();
}


void MainWindow::cities2db()
{
    database.insert("cities",QJsonArray());
    QJsonArray allrows;
    for (int row = 0; row < ui->cities_table->rowCount(); row++) {
        QStringList thisrow;
        for (int col = 0; col < ui->cities_table->columnCount(); col++) {
            if (ui->cities_table->item(row,col)) {
                thisrow.append(ui->cities_table->item(row,col)->text());
            } else {
                thisrow.append(QString(""));
            }
        }
        allrows.append(QJsonArray::fromStringList(thisrow));
    }
    database.insert("cities",allrows);
}

void MainWindow::students2db()
{
    database.insert("students",QJsonArray());
    QJsonArray allrows;
    for (int row = 0; row < ui->students_table->rowCount(); row++) {
        QStringList thisrow;
        for (int col = 0; col < ui->students_table->columnCount(); col++) {
            if (ui->students_table->item(row,col)) {
                thisrow.append(ui->students_table->item(row,col)->text());
            } else {
                thisrow.append(QString(""));
            }
        }
        allrows.append(QJsonArray::fromStringList(thisrow));
    }
    database.insert("students",allrows);
}

void MainWindow::ranking2db()
{
    database.insert("ranking",QJsonArray());
    QJsonArray allrows;
    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
        QStringList thisrow;
        for (int col = 0; col < ui->ranking_table->columnCount(); col++) {
            if (ui->ranking_table->item(row,col)) {
                thisrow.append(ui->ranking_table->item(row,col)->text());
            } else {
                thisrow.append(QString(""));
            }
        }
        allrows.append(QJsonArray::fromStringList(thisrow));
    }
    database.insert("ranking",allrows);
}

void MainWindow::db2students()
{
    for (int row = 0; row < database["students"].toArray().count(); row++) {
        if (ui->students_table->rowCount() <= row) ui->students_table->insertRow(row);
        for (int col = 0; col < database["students"].toArray().at(row).toArray().count(); col++) {
            QString mytext = database["students"].toArray().at(row).toArray().at(col).toString();
            QTableWidgetItem *newItem = new QTableWidgetItem(mytext);
            ui->students_table->setItem(row, col, newItem);
        }
    }
}

void MainWindow::db2cities()
{
    for (int row = 0; row < database["cities"].toArray().count(); row++) {
        if (ui->cities_table->rowCount() <= row) ui->cities_table->insertRow(row);
        for (int col = 0; col < database["cities"].toArray().at(row).toArray().count(); col++) {
            QString mytext = database["cities"].toArray().at(row).toArray().at(col).toString();
            QTableWidgetItem *newItem = new QTableWidgetItem(mytext);
            ui->cities_table->setItem(row, col, newItem);
        }
    }
}

void MainWindow::db2ranking()
{
    for (int row = 0; row < database["ranking"].toArray().count(); row++) {
        if (ui->ranking_table->rowCount() <= row) ui->ranking_table->insertRow(row);
        for (int col = 0; col < database["ranking"].toArray().at(row).toArray().count(); col++) {
            QString mytext = database["ranking"].toArray().at(row).toArray().at(col).toString();
            QTableWidgetItem *newItem = new QTableWidgetItem(mytext);
            ui->ranking_table->setItem(row, col, newItem);
        }
    }
}

void MainWindow::on_checkincomplete_clicked()
{
    checkvalid = false;
    for (int row = 0; row < ui->students_table->rowCount(); row++) {
        for (int col = 0; col < ui->students_table->columnCount(); col++) {
            if (!ui->students_table->item(row,col)) sanitizeTable(ui->students_table);
            ui->students_table->item(row, col)->setBackgroundColor(Qt::white);
            if (ui->students_table->item(row,col)->text().isEmpty() && col != dest2col && col != dest3col && col != dest4col) {
                ui->students_table->item(row, col)->setBackgroundColor(Qt::red);
                if (col != votecol && col != formercol && col != requisitescol && col != lettercol && col != reservedcol) return;
            }
            ui->students_table->setCurrentCell(row,col);
        }
        if (findItemInColumn(ui->students_table,ui->students_table->item(row,IDcol)->text(),IDcol).count() > 1) {
            ui->students_table->item(row, IDcol)->setBackgroundColor(Qt::white);
            QMessageBox::critical(this,tr("This is not funny"),tr("You can't have two students with the same ID"));
            ui->students_table->item(row, IDcol)->setBackgroundColor(Qt::red);
            return;
        }
        /*ui->students_table->item(row, votecol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,votecol)->text().toDouble() < 18 || ui->students_table->item(row,votecol)->text().toDouble() > 111) {
            ui->students_table->item(row, votecol)->setBackgroundColor(Qt::red);
            return;
        }*/
        ui->students_table->item(row, yearcol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,yearcol)->text() != "1" && ui->students_table->item(row,yearcol)->text() != "2" && ui->students_table->item(row,yearcol)->text() != "3" && ui->students_table->item(row,yearcol)->text() != "4" && ui->students_table->item(row,yearcol)->text() != "5") {
            ui->students_table->item(row, yearcol)->setBackgroundColor(Qt::red);
            return;
        }
        ui->students_table->item(row, requisitescol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,requisitescol)->text() != "1" && ui->students_table->item(row,requisitescol)->text() != "0" && ui->students_table->item(row,requisitescol)->text() != "")  {
            ui->students_table->item(row, requisitescol)->setBackgroundColor(Qt::red);
            return;
        }
        /*ui->students_table->item(row, formercol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,formercol)->text() != "1" && ui->students_table->item(row,formercol)->text() != "0"  && ui->students_table->item(row,formercol)->text() != "") {
            ui->students_table->item(row, formercol)->setBackgroundColor(Qt::red);
            return;
        }*/
        ui->students_table->item(row, reservedcol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,reservedcol)->text() != "1" && ui->students_table->item(row,reservedcol)->text() != "0"  && ui->students_table->item(row,reservedcol)->text() != "") {
            ui->students_table->item(row, reservedcol)->setBackgroundColor(Qt::red);
            return;
        }
        ui->students_table->item(row, lettercol)->setBackgroundColor(Qt::white);
        if (ui->students_table->item(row,lettercol)->text() != "1" && ui->students_table->item(row,lettercol)->text() != "0"  && ui->students_table->item(row,lettercol)->text() != "" && ui->students_table->item(row,lettercol)->text() != "2") {
            ui->students_table->item(row, lettercol)->setBackgroundColor(Qt::red);
            return;
        }
    }
    checkvalid = true;
}

void MainWindow::on_next_student_clicked()
{
    int row = ui->students_table->rowCount() -1;
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() + 1;
    if (row < ui->students_table->rowCount()) {
        ui->students_table->selectRow(row);
        on_students_table_cellClicked(row, IDcol);
    }
}

void MainWindow::on_prev_student_clicked()
{
    int row = 0;
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() - 1;
    if (row >= 0){
        ui->students_table->selectRow(row);
        on_students_table_cellClicked(row, IDcol);
    }
}

void MainWindow::on_students_table_cellClicked(int row, int column)
{
    if (row >= ui->students_table->rowCount() || column >= ui->students_table->columnCount()) return;
    if (ui->students_table->selectedItems().count() > 0) {
        for (int column = 0; column < ui->students_table->columnCount(); column++) {
            if (!ui->students_table->item(row,column)) sanitizeTable(ui->students_table);
            if(ui->students_table->item(row,column)){
                QString mytext = ui->students_table->item(row,column)->text();
                if (column == curriculacol) ui->frm_curricola->setText(mytext);
                if (column == dest1col) ui->frm_dest1->setText(mytext);
                if (column == dest2col) ui->frm_dest2->setText(mytext);
                if (column == dest3col) ui->frm_dest3->setText(mytext);
                if (column == dest4col) ui->frm_dest4->setText(mytext);
                if (column == formercol && mytext == "1") ui->frm_former->setChecked(true);
                if (column == formercol && !(mytext == "1")) ui->frm_former->setChecked(false);
                if (column == IDcol) ui->frm_ID->setText(mytext);
                if (column == votecol) ui->frm_meanvote->setText(mytext);
                if (column == namecol) ui->frm_name->setText(mytext);
                if (column == yearcol) ui->frm_registration->setCurrentText(mytext);
                if (column == lettercol) ui->frm_letter->setValue(mytext.toInt());
                if (column == requisitescol && mytext == "1") ui->frm_requisites->setChecked(true);
                if (column == requisitescol && !(mytext == "1")) ui->frm_requisites->setChecked(false);
                if (column == reservedcol && mytext == "1") ui->frm_reserved->setChecked(true);
                if (column == reservedcol && !(mytext == "1")) ui->frm_reserved->setChecked(false);
                if (column == surnamecol) ui->frm_surname->setText(mytext);
            }
        }
    }
}

void MainWindow::on_frm_meanvote_textChanged(const QString &arg1)
{
    QString tmpstr(arg1);
    tmpstr = tmpstr.replace(",",".");
    tmpstr = tmpstr.replace(QRegExp("[^0-9\\.]",Qt::CaseInsensitive),"");
    ui->frm_meanvote->setText(tmpstr);
    bool ok;
    double tmpvote = tmpstr.toDouble(&ok);
    if (ok == false && tmpstr != "") {
        QMessageBox::warning(this,tr("This is a problem"),tr("This is not a number. And it should be. It really should."));
        checkvalid = false;
    } else if (tmpvote < 18 || tmpvote > 31) {
        ui->frm_meanvote->setStyleSheet("QLineEdit {background-color: red;}");
        checkvalid = false;
    } else {
        ui->frm_meanvote->setStyleSheet("QLineEdit {background-color: white;}");
    }
}

void MainWindow::on_frm_ID_textChanged(const QString &arg1)
{
    QString tmpstr(arg1);
    tmpstr = tmpstr.replace(QRegExp("[^0-9A-Z]",Qt::CaseInsensitive),"");
    ui->frm_ID->setText(tmpstr);
}

void MainWindow::on_frm_name_textChanged(const QString &arg1)
{
    QString tmpstr(arg1);
    tmpstr = tmpstr.replace(QRegExp("[^A-Z'\\- ]",Qt::CaseInsensitive),"");
    ui->frm_name->setText(tmpstr);
}

void MainWindow::on_frm_surname_textChanged(const QString &arg1)
{
    QString tmpstr(arg1);
    tmpstr = tmpstr.replace(QRegExp("[^A-Z'\\- ]",Qt::CaseInsensitive),"");
    ui->frm_surname->setText(tmpstr);
}

double MainWindow::map(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void MainWindow::on_calculateRanking_clicked()
{
    sanitizeTable(ui->cities_table);
    sanitizeTable(ui->students_table);
    on_checkincomplete_clicked();
    if (checkvalid == false) {
        QMessageBox::information(this,tr("Problem"), tr("Something's wrong in students data. You might need to check cells with red background."));
        return;
    }
    on_check_cities_clicked();
    if (checkvalid == false) {
        QMessageBox::information(this,tr("Problem"), tr("Something's wrong in destinations data. You might need to check cells with red background."));
        return;
    }
    ui->ranking_table->setRowCount(0);
    do_ranking();
    assign_destinations();
}

void MainWindow::do_ranking()
{
    //il calcolo è questo: diamo dagli 80 ai 10 punti in base all'anno che si frequenta
    //(di 20 in 20 a eccezione dell'ultimo anno della laurea magistrale che ne prende solo 10)
    //poi diamo altri 20 punti in base alla media dei voti.
    //Se lo studente è già stato in erasmus, il suo punteggio va da 0 a 100, se non è stato
    //in erasmus il punteggio viene mappato da 100 a 200.
    //Alla fine, se lo studente non ha i requisiti linguistici, il suo punteggio è 0.
    for (int row = 0; row < ui->students_table->rowCount(); row++) {
        if (ui->ranking_table->rowCount() <= row) ui->ranking_table->insertRow(row);
        for (int col = 0; col < rankingcol; col++) {
            QTableWidgetItem *titem = new QTableWidgetItem ;
            titem->setText(ui->students_table->item(row,col)->text());
            titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ui->ranking_table->setItem(row,col,titem);
        }
        double ranking = 0.0;

        if (!ui->students_table->item(row,yearcol)) return;
        //current year
        QString year = ui->students_table->item(row,yearcol)->text();
        if (year == "4") ranking += 80;
        if (year == "2") ranking += 60;
        if (year == "1") ranking += 40;
        if (year == "3") ranking += 20;
        if (year == "5") ranking += 10;
        if (year != "1" && year != "2" && year != "3" && year != "4" && year != "5") ranking += 10;
        //mean vote
        double vote = ui->students_table->item(row,votecol)->text().toDouble();
        if (year == "1") {
            if (vote < 60) vote= 60;
            if (vote > 101) vote= 101;
            ranking += map(vote,60,101,1,20);
        } else if (year == "4") {
            if (vote < 66) vote= 66;
            if (vote > 111) vote= 111;
            ranking += map(vote,66,111,1,20);
        } else {
        if (vote < 18.0) vote = 18.0;
        if (vote > 31.0) vote = 31.0;
        ranking += map(vote,18,31,1,20);
        }
        //former erasmus student or not
        //if (ui->students_table->item(row,formercol)->text().toDouble() == 0) ranking  =  map(ranking,0,100,100,200);
        if (ui->students_table->item(ui->students_table->item(row,formercol)->text() != "" && row,formercol)->text().toDouble() != 0) ranking  =  ranking -1;
        // lettera motivazione da 0 a 2, si somma al punteggio (nel range 0-1)
        ranking  =  ranking + (ui->students_table->item(row,lettercol)->text().toDouble()/2);
        ranking = ranking*ui->students_table->item(row,requisitescol)->text().toDouble(); //requisites
        // per alcuni studenti bisogna dare subito il punteggio massimo perché hanno già posti riservati (ese: studentu di Regensburg)
        if (ui->students_table->item(row,reservedcol)->text() == "1") ranking = 103;

        QTableWidgetItem *titem = new QTableWidgetItem ;
        titem->setData(Qt::EditRole,ranking);
        titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->ranking_table->setItem(row,rankingcol,titem);
    }
    ui->ranking_table->sortByColumn(rankingcol,Qt::DescendingOrder);
}

void MainWindow::assign_destinations()
{
    ui->ranking_table->sortByColumn(rankingcol,Qt::DescendingOrder);
    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
        // se il punteggio è 0, non assegno la destinazione
        if (ui->ranking_table->item(row,rankingcol)->text().toDouble() > 0) {
        int strow = findItemInColumn(ui->students_table,ui->ranking_table->item(row,IDcol)->text(),IDcol)[0]->row();
        QStringList destinations;
        destinations.append(ui->students_table->item(strow,dest1col)->text());
        destinations.append(ui->students_table->item(strow,dest2col)->text());
        destinations.append(ui->students_table->item(strow,dest3col)->text());
        destinations.append(ui->students_table->item(strow,dest4col)->text());
        for (int i = 0; i < destinations.count(); i++) {
            if (findItemInColumn(ui->cities_table,destinations[i],citycol).count() == 1) {
                int cityrow = findItemInColumn(ui->cities_table,destinations[i],citycol)[0]->row();
                int maxavailable = ui->cities_table->item(cityrow,availablecol)->text().toInt();
                int alreadyassigned = findItemInColumn(ui->ranking_table,destinations[i],autodestcol).count();
                if (maxavailable > alreadyassigned) {
                    QTableWidgetItem *titem = new QTableWidgetItem ;
                    titem->setText(destinations[i]);
                    titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                    ui->ranking_table->setItem(row,autodestcol,titem);
                    break;
                }
            }
        }
        if (!ui->ranking_table->item(row,autodestcol)){
            QTableWidgetItem *titem = new QTableWidgetItem ;
            titem->setText("");
            titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ui->ranking_table->setItem(row,autodestcol,titem);
        }
        }
    }

    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
        if (ui->ranking_table->item(row,autodestcol) && !ui->ranking_table->item(row,manualdestcol)) {
            QTableWidgetItem *titem = new QTableWidgetItem ;
            titem->setText(ui->ranking_table->item(row,autodestcol)->text());
            ui->ranking_table->setItem(row,manualdestcol,titem);
        }
    }
}

void MainWindow::on_actionAbout_Cresta_triggered()
{
    QString message = "Cresta means Code Ranking Erasmus Students for Trieste Automatically. It's a program designed for automatically ranking Erasmus candidates using University of Trieste's standards. This program has been written by Luca Tringali. Translations by Floriana Sciumbata.";
    QMessageBox::about(this,tr("About Cresta"), QString("<html>")+message+cow+QString("</html>"));
}

void MainWindow::on_actionHelp_triggered()
{
    QMessageBox::information(this,tr("About Cresta"), tr("THIS IS THE RECOMMENDED WORKFLOW: \nImport students data from XLSX (Project/Import), please check that all column names are correct in the importing dialog window. If you leave the destinations checkbox checked, destinations data will also be imported. \nNow, edit students or destinations data directly from Cresta, using the tables available. Then, you can calculate the ranking using the button in Ranking tab. If there are severe errors in your data, the program will refuse to perform calculation and highlight in red all the cells that contain some kind of error in the tables. \n When the ranking is done, you can change destinations manually, but you are offered only places still available. This allows you to set a student as RINUNCIATARIO or just blank, if needed. Then you can give his place in the destination that has been freed to another student.\nTo find out if there are students eligible for a seat, just press the Actually available destinations button: you'll see al the students that currently have no destination assigned, and those eligible for a seat will be shown in bold. \nAfter you're done, you can export data in XLSX format from Project menù."));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this,"About Qt Libraries");
}

void MainWindow::on_ranking_table_cellClicked(int row, int column)
{
    if (column == manualdestcol) {
        int strow = findItemInColumn(ui->students_table,ui->ranking_table->item(row,IDcol)->text(),IDcol)[0]->row();
        QStringList destinations;
        destinations.append(ui->students_table->item(strow,dest1col)->text());
        destinations.append(ui->students_table->item(strow,dest2col)->text());
        destinations.append(ui->students_table->item(strow,dest3col)->text());
        destinations.append(ui->students_table->item(strow,dest4col)->text());
        QComboBox *editor = new QComboBox(ui->ranking_table);
        editor->addItem("");
        for (int i = 0; i < destinations.count(); i++) {
            if (findItemInColumn(ui->cities_table,destinations[i],citycol).count() == 1) {
                int cityrow = findItemInColumn(ui->cities_table,destinations[i],citycol)[0]->row();
                int maxavailable = ui->cities_table->item(cityrow,availablecol)->text().toInt();
                int alreadyassigned = findItemInColumn(ui->ranking_table,destinations[i],manualdestcol).count();
                if (ui->ranking_table->item(row,manualdestcol)) {
                    if (ui->ranking_table->item(row,manualdestcol)->text() == destinations[i]) {
                        alreadyassigned = alreadyassigned -1;
                    }
                }
                if (maxavailable > alreadyassigned) {
                    editor->addItem(destinations[i]);
                }
            }
        }
        editor->addItem(tr("RINUNCIATARIO"));
        if (ui->ranking_table->item(row,autodestcol)) editor->setCurrentText(ui->ranking_table->item(row,autodestcol)->text());

        connect(
                    editor, QOverload<const QString &>::of(&QComboBox::activated),
                    [=]( const QString &newValue ) { this->changerankingitem(newValue,row, column); }
        );

        ui->ranking_table->setCellWidget(row,manualdestcol,editor);
    } else {
        //nothing here
    }
}

QList<QTableWidgetItem *> MainWindow::findItemInColumn(QTableWidget *table, QString pattern, int column, Qt::MatchFlags match)
{
    QList<QTableWidgetItem *> result;
    QList<QTableWidgetItem *> tmpresult = table->findItems(pattern,match);
    int maxresult = tmpresult.count();
    //if (maxresult > 1000) maxresult = 1000;
    for (int i = 0; i < maxresult; i++) {
        if (tmpresult[i] && tmpresult[i]->column() == column) result.append(tmpresult[i]);
    }
    return result;
}

void MainWindow::changerankingitem(const QString &arg1, int row, int column)
{
    QTableWidgetItem *titem = new QTableWidgetItem;
    titem->setText(arg1);
    ui->ranking_table->removeCellWidget(row,column);
    ui->ranking_table->setItem(row,column,titem);
}

void MainWindow::on_actionNew_triggered()
{
    ui->cities_table->setRowCount(0);
    ui->students_table->setRowCount(0);
    ui->ranking_table->setRowCount(0);
    ui->frm_curricola->setText("");
    ui->frm_dest1->setText("");
    ui->frm_dest2->setText("");
    ui->frm_dest3->setText("");
    ui->frm_dest4->setText("");
    ui->frm_former->setChecked(false);
    ui->frm_ID->setText("");
    ui->frm_meanvote->setText("");
    ui->frm_name->setText("");
    ui->frm_registration->setCurrentIndex(0);
    ui->frm_requisites->setChecked(false);
    ui->frm_surname->setText("");
    dbfile = "";
    setWindowTitle("Cresta");
    on_cities_table_cellChanged(0,citycol);
}

void MainWindow::on_check_cities_clicked()
{
    checkvalid = false;
    for (int row = 0; row < ui->cities_table->rowCount(); row++) {
        for (int col = 0; col < ui->cities_table->columnCount(); col++) {
            ui->cities_table->item(row, col)->setBackgroundColor(Qt::white);
            if (ui->cities_table->item(row,col)->text().isEmpty() && col != teachercol && col != monthscol) {
                ui->cities_table->item(row, col)->setBackgroundColor(Qt::red);
                return;
            }
        }
        if (findItemInColumn(ui->cities_table,ui->cities_table->item(row,citycol)->text(),citycol).count() > 1) {
            ui->cities_table->item(row, citycol)->setBackgroundColor(Qt::white);
            QMessageBox::critical(this,tr("This is not funny"),tr("You can't have two destinations with the same name"));
            ui->cities_table->item(row, citycol)->setBackgroundColor(Qt::red);
            return;
        }
    }
    checkvalid = true;
}

void MainWindow::on_cities_table_cellChanged(int row, int column)
{
    QStringList wordList;
    if (ui->cities_table->item(row,column)) {
        for (int row = 0; row < ui->cities_table->rowCount(); row++) {
            if (ui->cities_table->item(row,citycol)) wordList.append(ui->cities_table->item(row,citycol)->text());
        }
    }
    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->frm_dest1->setCompleter(completer);
    ui->frm_dest2->setCompleter(completer);
    ui->frm_dest3->setCompleter(completer);
    ui->frm_dest4->setCompleter(completer);

}

void MainWindow::on_actionImport_xls_triggered()
{
    QString xlsxfile = QFileDialog::getOpenFileName(this, tr("Open XLSX"), QDir::currentPath(), "*.xlsx|Xlsx spreadsheet (*.xlsx)");
    if (!(xlsxfile.isEmpty())) {
        QXlsx::Document xlsx(xlsxfile);
        //qDebug()<<xlsx.read("B1");

        //int xls_IDcol = 1;
        int xls_IDcol = 3;
        int xls_surnamecol = 4;
        int xls_namecol = 5;
        int xls_curriculacol = 8;
        int xls_yearcol = 10;
        int xls_lslmcol = 9;
        int xls_prioritycol = 14;
        int xls_destcol = 15;
        int xls_availablecol = 17;
        int xls_meanvotecol = 13;
        int xls_requisitescol = 30;
        int xls_formercol = 23;
        int xls_lettercol = 31;
        int xls_reservedcol = 32;
        int xls_teachercol = 21;
        int xls_monthscol = 18;
        QString lmtext = "magistrale";

        xlsxsettings dialog(this);
        QStringList header;
        for (int col=1; col<100000; col++) {
            if (QXlsx::Cell *cell=xlsx.cellAt(1, col)) {
                if (cell->value().toString().isEmpty()) break;
                header.append(cell->value().toString());
            }
        }

        dialog.set_ID(header,xls_IDcol);
        dialog.set_surname(header,xls_surnamecol);
        dialog.set_name(header,xls_namecol);
        dialog.set_curricula(header,xls_curriculacol);
        dialog.set_year(header,xls_yearcol);
        dialog.set_priority(header,xls_prioritycol);
        dialog.set_dest(header,xls_destcol);
        dialog.set_lslm(header,xls_lslmcol);
        dialog.set_available(header,xls_availablecol);
        dialog.set_lm(lmtext);
        dialog.set_meanvote(header,xls_meanvotecol);
        dialog.set_requisites(header,xls_requisitescol);
        dialog.set_former(header,xls_formercol);
        dialog.set_letter(header,xls_lettercol);
        dialog.set_reserved(header,xls_reservedcol);
        dialog.set_teacher(header,xls_teachercol);
        dialog.set_months(header,xls_monthscol);
        dialog.setWindowTitle(tr("XLSX file settings"));
        if(dialog.exec() == QDialog::Accepted){
            xls_IDcol = dialog.get_ID();
            xls_surnamecol = dialog.get_surname();
            xls_namecol = dialog.get_name();
            xls_curriculacol = dialog.get_curricula();
            xls_yearcol = dialog.get_year();
            xls_prioritycol = dialog.get_priority();
            xls_destcol = dialog.get_dest();
            xls_lslmcol = dialog.get_lslm();
            xls_availablecol = dialog.get_available();
            xls_teachercol = dialog.get_teacher();
            lmtext = dialog.get_lm();
            xls_meanvotecol = dialog.get_meanvote();
            xls_requisitescol = dialog.get_requisites();
            xls_formercol = dialog.get_former();
            xls_lettercol = dialog.get_letter();
            xls_reservedcol = dialog.get_reserved();
            xls_monthscol = dialog.get_months();
        } else {
            return;
        }

        int xlsmaxrow = 0;
        for (int row=1; row<100000; row++) {
            if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_IDcol)) {
                if (cell->value().toString().isEmpty()) break;
                xlsmaxrow = row;
            }
        }

        for (int row=2; row<xlsmaxrow+1; row++) {
            QString xlsID = "";
            if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_IDcol)) {
                xlsID = cell->value().toString();
            }

            if (findItemInColumn(ui->students_table,xlsID,IDcol).count() <= 0) {
                int strow = ui->students_table->rowCount();
                if (ui->students_table->selectedItems().count() > 0) strow = ui->students_table->selectedItems()[0]->row() + 1;
                ui->students_table->insertRow(strow);
                QTableWidgetItem *newItem = new QTableWidgetItem(xlsID);
                ui->students_table->setItem(strow, IDcol, newItem);
                ui->students_table->setCurrentCell(strow,IDcol);
            }

            if (findItemInColumn(ui->students_table,xlsID,IDcol).count() > 0) {
                int strow = findItemInColumn(ui->students_table,xlsID,IDcol)[0]->row();

                QString xlsname = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_namecol)) {
                    xlsname = cell->value().toString();
                }
                if (xlsname != "") {
                QTableWidgetItem *newItemname = new QTableWidgetItem(xlsname);
                ui->students_table->setItem(strow, namecol, newItemname);
                ui->students_table->setCurrentCell(strow,namecol);
                }

                QString xlssurname = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_surnamecol)) {
                    xlssurname = cell->value().toString();
                }
                if (xlssurname != "") {
                QTableWidgetItem *newItemsurname = new QTableWidgetItem(xlssurname);
                ui->students_table->setItem(strow, surnamecol, newItemsurname);
                ui->students_table->setCurrentCell(strow,surnamecol);
                }

                QString xlscurricula = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_curriculacol)) {
                    xlscurricula = cell->value().toString();
                }
                if (xlscurricula != "") {
                QTableWidgetItem *newItemcurricula = new QTableWidgetItem(xlscurricula);
                ui->students_table->setItem(strow, curriculacol, newItemcurricula);
                ui->students_table->setCurrentCell(strow,curriculacol);
                }

                QString xlsyear = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_yearcol)) {
                    xlsyear = cell->value().toString();
                }
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_lslmcol)) {
                    if (cell->value().toString() == lmtext) xlsyear = QString::number(xlsyear.toInt() + 3);
                }
                if (xlsyear != "") {
                QTableWidgetItem *newItemyear = new QTableWidgetItem(xlsyear);
                ui->students_table->setItem(strow, yearcol, newItemyear);
                ui->students_table->setCurrentCell(strow,yearcol);
                }

                QString xlsmeanvote = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_meanvotecol)) {
                    xlsmeanvote = cell->value().toString();
                }
                if (xlsmeanvote != "") {
                QTableWidgetItem *newItemmeanvote = new QTableWidgetItem(xlsmeanvote);
                ui->students_table->setItem(strow, votecol, newItemmeanvote);
                ui->students_table->setCurrentCell(strow,votecol);
                }

                QString xlsrequisites = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_requisitescol)) {
                    xlsrequisites = cell->value().toString();
                }
                if (xlsrequisites != "") {
                QTableWidgetItem *newItemrequisites = new QTableWidgetItem(xlsrequisites);
                ui->students_table->setItem(strow, requisitescol, newItemrequisites);
                ui->students_table->setCurrentCell(strow,requisitescol);
                }

                QString xlsformer = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_formercol)) {
                    xlsformer = cell->value().toString();
                }
                if (xlsformer != "") {
                QTableWidgetItem *newItemformer = new QTableWidgetItem(xlsformer);
                ui->students_table->setItem(strow, formercol, newItemformer);
                ui->students_table->setCurrentCell(strow,formercol);
                }

                QString xlsletter = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_lettercol)) {
                    xlsletter = cell->value().toString();
                }
                if (xlsletter != "") {
                QTableWidgetItem *newItemletter = new QTableWidgetItem(xlsletter);
                ui->students_table->setItem(strow, lettercol, newItemletter);
                ui->students_table->setCurrentCell(strow,lettercol);
                }

                QString xlsreserved = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_reservedcol)) {
                    xlsreserved = cell->value().toString();
                }
                if (xlsreserved != "") {
                QTableWidgetItem *newItemreserved = new QTableWidgetItem(xlsreserved);
                ui->students_table->setItem(strow, reservedcol, newItemreserved);
                ui->students_table->setCurrentCell(strow,reservedcol);
                }

                QString xlsavailable = "0";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_availablecol)) {
                    xlsavailable = cell->value().toString();
                }

                QString xlsteacher = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_teachercol)) {
                    xlsteacher = cell->value().toString();
                }

                QString xlsmonths = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_monthscol)) {
                    xlsmonths = cell->value().toString();
                }

                QString xlsdest = "";
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_destcol)) {
                    xlsdest = cell->value().toString();
                }
                int thisdestcol = dest1col;
                if (QXlsx::Cell *cell=xlsx.cellAt(row, xls_prioritycol)) {
                    if (cell->value().toString() == "2") thisdestcol = dest2col;
                    if (cell->value().toString() == "3") thisdestcol = dest3col;
                    if (cell->value().toString() == "4") thisdestcol = dest4col;
                }
                if (xlsdest != "") {
                QTableWidgetItem *newItemdest = new QTableWidgetItem(xlsdest);
                ui->students_table->setItem(strow, thisdestcol, newItemdest);
                ui->students_table->setCurrentCell(strow,thisdestcol);
                }

                /*if (dialog.get_requisites()) {
                    QTableWidgetItem *newItemrequisites = new QTableWidgetItem("1");
                    ui->students_table->setItem(strow, requisitescol, newItemrequisites);
                    ui->students_table->setCurrentCell(strow,requisitescol);
                }*/

                if (dialog.get_addcities()) {
                    if (findItemInColumn(ui->cities_table,xlsdest,citycol).count() <= 0) {
                        int crow = ui->cities_table->rowCount();
                        if (ui->cities_table->selectedItems().count() > 0) crow = ui->cities_table->selectedItems()[0]->row() + 1;
                        ui->cities_table->insertRow(crow);
                        QTableWidgetItem *newItemcity = new QTableWidgetItem(xlsdest);
                        ui->cities_table->setItem(crow, citycol, newItemcity);
                        QTableWidgetItem *newItemav = new QTableWidgetItem(xlsavailable);
                        ui->cities_table->setItem(crow, availablecol, newItemav);
                        QTableWidgetItem *newItemcountry = new QTableWidgetItem(xlsdest.split(" ").at(0));
                        ui->cities_table->setItem(crow, countycol, newItemcountry);
                        QTableWidgetItem *newItemteacher = new QTableWidgetItem(xlsteacher);
                        ui->cities_table->setItem(crow, teachercol, newItemteacher);
                        QTableWidgetItem *newItemmonths = new QTableWidgetItem(xlsmonths);
                        ui->cities_table->setItem(crow, monthscol, newItemmonths);
                    }
                }
            }

        }
    }
}

void MainWindow::sanitizeTable(QTableWidget *table) {
    for (int row=0; row < table->rowCount(); row++) {
        for (int col=0; col < table->columnCount(); col++) {
            if (!table->item(row,col)) {
                QTableWidgetItem *newItem = new QTableWidgetItem("");
                table->setItem(row, col, newItem);
            }
        }
    }
}

void MainWindow::on_searchst_clicked()
{
    //Qt::MatchFlags match = Qt::MatchContains;
    Qt::MatchFlags match = Qt::MatchWildcard;
    QList<QTableWidgetItem *> result = findItemInColumn(ui->students_table,ui->searchthis->text(),ui->searchhere->currentIndex(),match);
    if (result.count() > 0) {
        int i = 0;
        int row = result[i]->row();
        if (!ui->searchfromstart->isChecked() && ui->students_table->selectedItems().count() > 0) {
            while (row <= ui->students_table->selectedItems()[0]->row()) {
                i++;
                if (i>=result.count()) break;
                row = result[i]->row();
            }
        }
        ui->students_table->setCurrentCell(row,ui->searchhere->currentIndex());
    }
}

void MainWindow::on_students_table_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow != previousRow || currentColumn != previousColumn) {
        on_students_table_cellClicked(currentRow,currentColumn);
    }
}

void MainWindow::on_actionExport_ranking_xslx_triggered()
{
    if (ui->cities_table->rowCount() == 0 || ui->students_table->rowCount() == 0 || ui->ranking_table->rowCount() == 0) return;
    sanitizeTable(ui->ranking_table);
    QString xlsxfile = QFileDialog::getSaveFileName(this, tr("Export ranking on xlsx file"), QDir::currentPath(), "*.xslx|Excel Spreadsheet (*.xlsx)");
    if (!xlsxfile.isEmpty()) {
        if (xlsxfile.right(5)!=".xlsx") {
            if (xlsxfile.right(1)!=".") xlsxfile = xlsxfile + ".";
            xlsxfile = xlsxfile + "xlsx";
        }
        QXlsx::Document xlsx;

        QStringList mycolumns;
        mycolumns.append("cod_fis");  //0
        mycolumns.append("cognome"); //1
        mycolumns.append("ATENEO");  //2
        mycolumns.append("docente coordinatore"); //3
        mycolumns.append("ESITI (A=assegnatario, R=riserva, E=escluso)");  //4
        mycolumns.append("mesi assegnati da selezione"); //5
        mycolumns.append("NOTE COMMISSIONE"); //6
        mycolumns.append("punteggio"); //7
        mycolumns.append("nome"); //8
        mycolumns.append("PRIORITA_DEST_SCE"); //9

        for (int col = 0; col < mycolumns.count(); col++) {
            xlsx.write(1,col+1,mycolumns.at(col));
        }

        int outrow = -1;
        for (int row = 0; row < ui->students_table->rowCount(); row++) {
            //ID,Surname,manualdest,docente_coordinatore, esiti, mesi assegnati, note

                for (int dcol = 0; dcol < 4; dcol++) {
                    QString tmpdest = "";
                    if (dcol == 0) tmpdest = ui->students_table->item(row,dest1col)->text();
                    if (dcol == 1) tmpdest = ui->students_table->item(row,dest2col)->text();
                    if (dcol == 2) tmpdest = ui->students_table->item(row,dest3col)->text();
                    if (dcol == 3) tmpdest = ui->students_table->item(row,dest4col)->text();
                    if (tmpdest != "") {
                        outrow = outrow + 1;

                        int tmpcol = IDcol;
                        int mycol = 0;
                        QVariant val(ui->students_table->item(row,tmpcol)->text());
                        xlsx.write(outrow+2,mycol+1,val);

                        tmpcol = surnamecol;
                        mycol = 1;
                        val = ui->students_table->item(row,tmpcol)->text();
                        xlsx.write(outrow+2,mycol+1,val);

                        tmpcol = namecol;
                        mycol = 8;
                        val = ui->students_table->item(row,tmpcol)->text();
                        xlsx.write(outrow+2,mycol+1,val);

                        mycol = 9;
                        val = QString::number(dcol+1);
                        xlsx.write(outrow+2,mycol+1,val);

                        int rrow = findItemInColumn(ui->ranking_table,ui->students_table->item(row,IDcol)->text(),IDcol)[0]->row();
                        mycol = 7;
                        val = ui->ranking_table->item(rrow,rankingcol)->text();
                        xlsx.write(outrow+2,mycol+1,val.toDouble());

                        mycol = 2;
                        xlsx.write(outrow+2,mycol+1,tmpdest);
                        val = ui->ranking_table->item(rrow,manualdestcol)->text();
                        if (val == "RINUNCIATARIO") {
                            mycol = 4;
                            xlsx.write(outrow+2,mycol+1,"E");
                        } else {
                            mycol = 4;
                            if (val == tmpdest) xlsx.write(outrow+2,mycol+1,"A");
                            if (val != tmpdest) xlsx.write(outrow+2,mycol+1,"E");
                            if (val == "") xlsx.write(outrow+2,mycol+1,"R");
                            mycol = 3;
                            int tmpcol = teachercol;
                            int tmprow = -1;
                            if (findItemInColumn(ui->cities_table,tmpdest,citycol).count()>0) tmprow = findItemInColumn(ui->cities_table,tmpdest,citycol)[0]->row();
                            if (tmprow >=0) {
                                val = ui->cities_table->item(tmprow,tmpcol)->text();
                                if (ui->cities_table->item(tmprow,tmpcol)) xlsx.write(outrow+2,mycol+1,val);
                                mycol = 5;
                                tmpcol = monthscol;
                                val = ui->cities_table->item(tmprow,tmpcol)->text();
                                if (ui->cities_table->item(tmprow,tmpcol)) xlsx.write(outrow+2,mycol+1,val);
                            }
                        }
                    }
                }

                //outrow = outrow +1;
        }

            /*for (int col = 0; col < ui->ranking_table->columnCount(); col++) {
                QVariant val(ui->ranking_table->item(row,col)->text());
                if (col == rankingcol) val = ui->ranking_table->item(row,col)->text().toDouble();
                if (ui->ranking_table->item(row,col)) xlsx.write(row+2,col+1,val);
            }*/


        /*if (mycolumns.at(col) == "docente coordinatore") {
            int tmpcol = teachercol;
            int tmprow = -1;
            if (findItemInColumn(ui->cities_table,ui->ranking_table->item(row,manualdestcol)->text(),citycol).count()>0) tmprow = findItemInColumn(ui->cities_table,ui->ranking_table->item(row,manualdestcol)->text(),citycol)[0]->row();
            if (tmprow >=0) {
                QVariant val(ui->cities_table->item(tmprow,tmpcol)->text());
                if (ui->cities_table->item(tmprow,tmpcol)) xlsx.write(row+2,col+1,val);
            }
        }*/
        xlsx.saveAs(xlsxfile);
    }

}

void MainWindow::on_easteregg_clicked()
{
    easteregg++;
    if (easteregg > 6 && easteregg % 2 != 0) {
        ui->statusBar->showMessage("Developer mode enabled", 3000);
        ui->prev_student->show();
        ui->next_student->show();
        ui->actionMerge_projects->setVisible(true);
        //ui->free_destinations->show();
        cow = "</br></br><pre>This program has supercow powers.</pre></br></br><pre>                    (__) </pre></br><pre>                    (oo) </pre></br><pre>              /------\\/ </pre></br><pre>             / |    ||   </pre></br><pre>            *  /\\---/\\ </pre></br><pre>               ~~   ~~   </pre></br><pre>   ...\"Have you mooed today?\"...</pre>";
        for (int i = 0; i < ui->menuLanguage->actions().count(); i++) {
            ui->menuLanguage->actions().at(i)->setVisible(true);
        }
    } else {
        if (easteregg > 6) ui->statusBar->showMessage("Developer mode disabled", 3000);
        ui->prev_student->hide();
        ui->next_student->hide();
        //ui->free_destinations->hide();
        ui->actionMerge_projects->setVisible(false);
        cow = "";
        for (int i = 0; i < ui->menuLanguage->actions().count(); i++) {
            QStringList acceptablelangs;
            acceptablelangs << "Italian" << "English" << "&Italian" << "&English";
            if (acceptablelangs.indexOf(ui->menuLanguage->actions().at(i)->text())<0) {
                ui->menuLanguage->actions().at(i)->setVisible(false);
            }
        }
    }
}

void MainWindow::on_actionMerge_projects_triggered()
{
    //already loaded database
    cities2db();
    students2db();
    ranking2db();

    //the database we are importing
    QJsonObject newdatabase;
    QString tdbfile = QFileDialog::getOpenFileName(this, tr("Select project to merge"), QDir::currentPath(), "*.cresta|Cresta Project (*.cresta)");
    if (!(tdbfile.isEmpty())) {
        QFile file(tdbfile);
        QString data = "";
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        while (!in.atEnd()) {
            data += in.readLine();
        }
        file.close();

        QString problem = "    ]}    ]}";
        data = data.replace(problem,"    ]}");
        newdatabase = QJsonDocument::fromJson(data.toUtf8()).object();
    }

    if (newdatabase.isEmpty() || database == newdatabase) return;

    //cities
    QJsonArray ncities = merge_qja(database["cities"].toArray(), newdatabase["cities"].toArray(),1);
    database.insert("cities",QJsonArray());
    database.insert("cities",ncities);

    //students
    QJsonArray nstudents = merge_qja(database["students"].toArray(), newdatabase["students"].toArray(),2);
    database.insert("students",QJsonArray());
    database.insert("students",nstudents);
    //ranking
    QJsonArray nranking = merge_qja(database["ranking"].toArray(), newdatabase["ranking"].toArray(),3);
    database.insert("ranking",QJsonArray());
    database.insert("ranking",nranking);

    db2cities();
    db2students();
    db2ranking();
}

QStringList MainWindow::extract_qsim_row(QStandardItemModel *db, int row) {
    QStringList thisrow;
    for (int col = 0; col < db->columnCount(); col++) {
        if (db->item(row,col)) {
            thisrow.append(db->item(row,col)->text());
        } else {
            thisrow.append(QString(""));
        }
    }
    return thisrow;
}

QJsonArray MainWindow::merge_qja(QJsonArray db1, QJsonArray db2, int arraytype) {
    //TODO: maybe it's better just to use QString arraytype with the same names of the tables
    int uniquecol = 0;
    if (arraytype == 1) {
        uniquecol = citycol;
    }
    if (arraytype == 2) {
        uniquecol = IDcol;
    }
    if (arraytype == 3) {
        uniquecol = IDcol;
    }
    if (arraytype != 1 && arraytype !=2 && arraytype !=3) {
        QJsonArray empty;
        return empty;
    }

    QStandardItemModel db1table(db1.count(),db1[0].toArray().count());
    for (int row = 0; row < db1.count(); row++) {
        for (int col = 0; col < db1[row].toArray().count(); col++) {
            QStandardItem *tmpitem = new QStandardItem(db1[row].toArray().at(col).toString());
            db1table.setItem(row,col,tmpitem);
        }
    }
    db1table.sort(uniquecol,Qt::AscendingOrder);

    QStandardItemModel db2table(db2.count(),db2[0].toArray().count());
    for (int row = 0; row < db2.count(); row++) {
        for (int col = 0; col < db2[row].toArray().count(); col++) {
            QStandardItem *tmpitem = new QStandardItem(db2[row].toArray().at(col).toString());
            db2table.setItem(row,col,tmpitem);
        }
    }
    db2table.sort(uniquecol,Qt::AscendingOrder);

    QJsonArray result;
    for (int row = 0; row < db1.count(); row++) {
        QStringList thisrow1 = extract_qsim_row(&db1table, row);
        QStringList thisrow2;
        int row2 = -1;
        QList<QStandardItem *> fRes = db2table.findItems(db1table.item(row,uniquecol)->text(),Qt::MatchExactly,uniquecol);
        for (int f = 0; f < fRes.count(); f++) {
            row2 = fRes.at(f)->row();
        }
        if (row2 >= 0 && row2 < db2table.rowCount()) {
            thisrow2 = extract_qsim_row(&db2table, row2);

            if (thisrow1 == thisrow2) {
                result.append(QJsonArray::fromStringList(thisrow2));
            } else {
                QMessageBox::StandardButton reply;
                QString first = QJsonDocument(QJsonArray::fromStringList(thisrow1)).toJson();
                QString second = QJsonDocument(QJsonArray::fromStringList(thisrow2)).toJson();
                QString msg = tr("Actual file record:")+QString("\n")+first+QString("\n")+tr("New file record:")+second+QString("\n")+tr("Do you want to replace the first with the second?");
                reply = QMessageBox::question(this, tr("Close"),msg, QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    result.append(QJsonArray::fromStringList(thisrow2));
                } else {
                    result.append(QJsonArray::fromStringList(thisrow1));
                }
            }
        } else {
            QMessageBox::StandardButton reply;
            QString first = QJsonDocument(QJsonArray::fromStringList(thisrow1)).toJson();
            QString msg = tr("Actual file record:")+QString("\n")+first+QString("\n")+QString("\n")+tr("is missing in the file you selected for merging. Do you want to keep it anyway?");
            reply = QMessageBox::question(this, tr("Close"),msg, QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                result.append(QJsonArray::fromStringList(thisrow1));
            }
        }
    }

    for (int row = 0; row < db2.count(); row++) {
        QStringList thisrow2 = extract_qsim_row(&db2table, row);
        int row1 = -1;
        QList<QStandardItem *> fRes = db1table.findItems(db2table.item(row,uniquecol)->text(),Qt::MatchExactly,uniquecol);
        for (int f = 0; f < fRes.count(); f++) {
            row1 = fRes.at(f)->row();
        }
        if (row1 < 0 || row1 >= db1table.rowCount()) {
            QMessageBox::StandardButton reply;
            QString second = QJsonDocument(QJsonArray::fromStringList(thisrow2)).toJson();
            QString msg = tr("The new file record:")+QString("\n")+second+QString("\n")+QString("\n")+tr("is missing in the original project. Do you want to keep it anyway?");
            reply = QMessageBox::question(this, tr("Close"),msg, QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                result.append(QJsonArray::fromStringList(thisrow2));
            }
        }
    }

/*    if (arraytype == 1) {
        qDebug() << "FIRST";
        qDebug() << db1;
        qDebug() << "SECOND";
        qDebug() << db2;
        qDebug() << "RESULT";
        qDebug() << result;
    }*/
    return result;
}

void MainWindow::on_free_destinations_clicked()
{
    sanitizeTable(ui->ranking_table);
    QString listtext = "";
    QStringList tmpcities;
    QList<int> tmpavailabe;
    for (int row = 0; row < ui->cities_table->rowCount(); row++) {
        QString thiscity = ui->cities_table->item(row,citycol)->text();
        int thisactual = findItemInColumn(ui->ranking_table,thiscity,manualdestcol).count();
        int thisavailable = ui->cities_table->item(row,availablecol)->text().toInt();
        int difference = thisavailable - thisactual;
        if (difference > 0) listtext += thiscity + ": " + QString::number(difference) + "\n";
        tmpcities.append(thiscity);
        tmpavailabe.append(difference);
    }
    //QMessageBox::information(this,tr("List of actually available destinations"),listtext);
    QString liststudents = "";
    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
        QString thisdest = "";
        if (ui->ranking_table->item(row,manualdestcol)) thisdest = ui->ranking_table->item(row,manualdestcol)->text();
        if (thisdest == "") {
            QString stID = ui->ranking_table->item(row,IDcol)->text();
            liststudents += "<u>" + stID + "</u>";
            liststudents += ";";
            liststudents += ui->ranking_table->item(row,namecol)->text();
            liststudents += ";";
            liststudents += ui->ranking_table->item(row,surnamecol)->text();
            liststudents += ";";
            QStringList tmpdest;
            tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest1col)->text());
            tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest2col)->text());
            tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest3col)->text());
            tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest4col)->text());
            for (int t = 0; t < tmpdest.count(); t++) {
                int tmpindex = tmpcities.indexOf(tmpdest.at(t));
                if (tmpindex > 0 && tmpindex < tmpavailabe.count()){
                if (tmpavailabe[tmpindex] > 0) {
                    liststudents += "<b>";
                    liststudents += tmpdest.at(t);
                    liststudents += "</b>";
                    liststudents += ";";
                    tmpavailabe[tmpindex] = tmpavailabe[tmpindex] -1;
                } else {
                    liststudents += tmpdest.at(t);
                    liststudents += ";";
                }
                }
            }
            liststudents += "<br>";
            ui->ranking_table->item(row,manualdestcol)->setBackgroundColor(Qt::red);
        }
    }
    if (liststudents != "") QMessageBox::information(this,tr("List of students with no assigned destination"),liststudents);
}

void MainWindow::on_actionExport_ranking_by_destination_triggered()
{
    if (ui->cities_table->rowCount() == 0 || ui->students_table->rowCount() == 0 || ui->ranking_table->rowCount() == 0) return;
    sanitizeTable(ui->ranking_table);
    QString xlsxfile = QFileDialog::getSaveFileName(this, tr("Export ranking by destinations"), QDir::currentPath(), "*.xslx|Excel Spreadsheet (*.xlsx)");
    if (!xlsxfile.isEmpty()) {
        if (xlsxfile.right(5)!=".xlsx") {
            if (xlsxfile.right(1)!=".") xlsxfile = xlsxfile + ".";
            xlsxfile = xlsxfile + "xlsx";
        }
        QXlsx::Document xlsx;
        for (int crow = 0; crow < ui->cities_table->rowCount(); crow++) {
            QString thiscity = ui->cities_table->item(crow,citycol)->text();
            xlsx.addSheet(thiscity);
            xlsx.selectSheet(thiscity);
            int thisrow = 1;
            for (int col = 0; col < ui->ranking_table->columnCount(); col++) {
                xlsx.write(thisrow,col+1,ui->ranking_table->horizontalHeaderItem(col)->text());
            }
            xlsx.write(thisrow,ui->ranking_table->columnCount()+1,QString("ESITO"));
            for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
                if (ui->ranking_table->item(row,manualdestcol)->text() == thiscity) {
                    thisrow = thisrow +1;
                    int col = 0;
                    for (col = 0; col < ui->ranking_table->columnCount(); col++) {
                        QVariant val(ui->ranking_table->item(row,col)->text());
                        if (col == rankingcol) val = ui->ranking_table->item(row,col)->text().toDouble();
                        if (ui->ranking_table->item(row,col)) xlsx.write(thisrow,col+1,val);
                    }
                     xlsx.write(thisrow,col+1,QString("A"));
                }
            }
            for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
                if (ui->ranking_table->item(row,manualdestcol)->text() == "" || ui->ranking_table->item(row,manualdestcol)->text() == "RINUNCIATARIO") {
                    QString stID = ui->ranking_table->item(row,IDcol)->text();
                    QStringList tmpdest;
                    tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest1col)->text());
                    tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest2col)->text());
                    tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest3col)->text());
                    tmpdest.append(ui->students_table->item(findItemInColumn(ui->students_table,stID,IDcol).at(0)->row(),dest4col)->text());
                    for (int t = 0; t < tmpdest.count(); t++) {
                        if (tmpdest.at(t) == thiscity) {
                            thisrow = thisrow +1;
                            int col = 0;
                            for (col = 0; col < ui->ranking_table->columnCount(); col++) {
                                QVariant val(ui->ranking_table->item(row,col)->text());
                                if (col == rankingcol) val = ui->ranking_table->item(row,col)->text().toDouble();
                                if (ui->ranking_table->item(row,col)) xlsx.write(thisrow,col+1,val);
                            }
                             if (ui->ranking_table->item(row,manualdestcol)->text() == "RINUNCIATARIO") xlsx.write(thisrow,col+1,QString("E"));
                             if (ui->ranking_table->item(row,manualdestcol)->text() == "") xlsx.write(thisrow,col+1,QString("R"));
                        }
                    }

                }
            }
        }
        xlsx.saveAs(xlsxfile);
    }

}

void MainWindow::on_searchst_2_clicked()
{
    //Qt::MatchFlags match = Qt::MatchContains;
    Qt::MatchFlags match = Qt::MatchWildcard;
    QList<QTableWidgetItem *> result = findItemInColumn(ui->ranking_table,ui->searchthis_2->text(),ui->searchhere_2->currentIndex(),match);
    if (result.count() > 0) {
        int i = 0;
        int row = result[i]->row();
        if (!ui->searchfromstart_2->isChecked() && ui->ranking_table->selectedItems().count() > 0) {
            while (row <= ui->ranking_table->selectedItems()[0]->row()) {
                i++;
                if (i>=result.count()) break;
                row = result[i]->row();
            }
        }
        ui->ranking_table->setCurrentCell(row,ui->searchhere_2->currentIndex());
    }

}
