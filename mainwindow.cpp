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
    int row = ui->students_table->rowCount();
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() + 1;
    ui->students_table->insertRow(row);
    QTableWidgetItem *titem = new QTableWidgetItem ;
    titem->setText("");
    ui->students_table->setItem(row,IDcol,titem);
    ui->students_table->setCurrentCell(row,IDcol);
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
    }
    if (ui->students_table->selectedItems().count() > 0) {
        int row = ui->students_table->selectedItems()[0]->row();

        if (findItemInColumn(ui->students_table,ui->frm_ID->text(),IDcol).count() > 0) {
            QMessageBox::critical(this,tr("This is not funny"),tr("You can't have two students with the same ID"));
            return;
        }

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
    for (int row = 0; row < ui->students_table->rowCount(); row++) {
        for (int col = 0; col < ui->students_table->columnCount(); col++) {
            if (ui->students_table->item(row,col)->text().isEmpty()) ui->students_table->item(row, col)->setBackgroundColor(Qt::red);
        }
        if (findItemInColumn(ui->students_table,ui->students_table->item(row,IDcol)->text(),IDcol).count() > 1) {
            QMessageBox::critical(this,tr("This is not funny"),tr("You can't have two students with the same ID"));
            ui->students_table->item(row, IDcol)->setBackgroundColor(Qt::red);
        }
    }
}

void MainWindow::on_next_student_clicked()
{
    int row = ui->students_table->rowCount() -1;
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() + 1;
    if (row < ui->students_table->rowCount()) ui->students_table->selectRow(row);
    on_students_table_cellClicked(row, IDcol);
}

void MainWindow::on_prev_student_clicked()
{
    int row = 0;
    if (ui->students_table->selectedItems().count() > 0) row = ui->students_table->selectedItems()[0]->row() - 1;
    if (row >= 0) ui->students_table->selectRow(row);
    on_students_table_cellClicked(row, IDcol);
}

void MainWindow::on_students_table_cellClicked(int row, int column)
{
    if (ui->students_table->selectedItems().count() > 0) {
        for (int column = 0; column < ui->students_table->columnCount(); column++) {
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
            if (column == requisitescol && mytext == "1") ui->frm_requisites->setChecked(true);
            if (column == requisitescol && !(mytext == "1")) ui->frm_requisites->setChecked(false);
            if (column == surnamecol) ui->frm_surname->setText(mytext);
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
    } else if (tmpvote < 18 || tmpvote > 31) {
        ui->frm_meanvote->setStyleSheet("QLineEdit {background-color: red;}");
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
    ui->ranking_table->setRowCount(0);
    do_ranking();
    assign_destinations();
}

void MainWindow::do_ranking()
{
    //il calcolo è questo: diamo dagli 80 ai 10 punti in base all'anno che si frequenta
    //(di 20 in 20 a eccezione dell'ultimo anno della laurea magistrale che ne prende solo 10)
    //poi diamo altri 20 punti in base alla media dei voti.
    //Se lo studente non ha i requisiti linguistici, il suo punteggio è 0.
    //Quindi il range del punteggio è da 0 a 100.
    //Se due studenti hanno lo stesso punteggio, quello dei due che ha già avuto una borsa Erasmus perde 0.5 punti.
    for (int row = 0; row < ui->students_table->rowCount(); row++) {
        if (ui->ranking_table->rowCount() <= row) ui->ranking_table->insertRow(row);
        for (int col = 0; col < 4; col++) {
            QTableWidgetItem *titem = new QTableWidgetItem ;
            titem->setText(ui->students_table->item(row,col)->text());
            titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            ui->ranking_table->setItem(row,col,titem);
        }
        double ranking = 0.0;

        //current year
        if (ui->students_table->item(row,4)->text() == "4") ranking += 80;
        if (ui->students_table->item(row,4)->text() == "2") ranking += 60;
        if (ui->students_table->item(row,4)->text() == "1") ranking += 40;
        if (ui->students_table->item(row,4)->text() == "3") ranking += 20;
        if (ui->students_table->item(row,4)->text() == "5") ranking += 10;
        ranking += map(ui->students_table->item(row,5)->text().toDouble(),18,31,1,20); //mean vote
        ranking = ranking*ui->students_table->item(row,6)->text().toDouble(); //requisites

        QTableWidgetItem *titem = new QTableWidgetItem ;
        titem->setText(QString::number(ranking));
        titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->ranking_table->setItem(row,rankingcol,titem);
    }

    ui->ranking_table->sortByColumn(rankingcol,Qt::DescendingOrder);
    //TODO: it would be useful to repeat this until there are no students with the same score
    double oldranking = 0.0;
    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
        double ranking = ui->ranking_table->item(row,rankingcol)->text().toDouble();
        if (ranking == oldranking) {
            for (int tmprow = (row-1); tmprow < (row+1); tmprow++) {  //we check both this and previous student
                int strow = findItemInColumn(ui->students_table,ui->ranking_table->item(tmprow,IDcol)->text(),IDcol)[0]->row();
                ranking = ranking - (ui->students_table->item(strow,7)->text().toDouble()*0.5); //former student
                if (ranking <= 0.0) ranking = 0.0;
                QTableWidgetItem *titem = new QTableWidgetItem ;
                titem->setText(QString::number(ranking));
                titem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->ranking_table->setItem(tmprow,rankingcol,titem);
            }
        } else {
            oldranking = ranking;
        }
        if (ranking == 0) ui->ranking_table->item(row,rankingcol)->setBackgroundColor(Qt::red);
        ui->ranking_table->sortByColumn(rankingcol,Qt::DescendingOrder);
    }
}

void MainWindow::assign_destinations()
{
    ui->ranking_table->sortByColumn(rankingcol,Qt::DescendingOrder);
    for (int row = 0; row < ui->ranking_table->rowCount(); row++) {
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
    QMessageBox::about(this,tr("About Cresta"), tr(qPrintable(message)));
}

void MainWindow::on_actionHelp_triggered()
{
    QString message = "Votes should be a number between 18 and 31 (which is 30 + laude). You can change destinations manually, but you are offered only places still available.";
    QMessageBox::information(this,tr("About Cresta"), tr(qPrintable(message)));
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
    for (int i = 0; i < table->findItems(pattern,match).count(); i++) {
        if (table->findItems(pattern,match)[i]->column() == column) result.append(table->findItems(pattern,match)[i]);
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
}
