#include "ordertab.h"
#include "ui_ordertab.h"
#include "mainview.h"
#include <QtSql>
#include <QString>
#include "agendasettings.h"
#include <string>
#include "orderitemsettings.h"
#include "QSqlQueryModelImpl.h"
#include <QMenu>
#include "propertysettings.h"
#include "patterneditorreportitemsettings.h"

OrderTab::OrderTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderTab)
{
  ui->setupUi(this);

  mQueryModel = 0;
  mQueryModelReportTemplate = 0;
}

OrderTab::~OrderTab()
{
  if (0 != mQueryModel)
  {
    delete mQueryModel;
    mQueryModel = 0;
  }

  if (0 != mQueryModelReportTemplate)
  {
    delete mQueryModelReportTemplate;
    mQueryModelReportTemplate = 0;
  }

  delete ui;
}

void OrderTab::refreshOnSelected ()
{
  if (!this->isEnabled())
  {
    QMessageBox::information(this, "Fehler", "Alle Einstellungen deaktiviert.\nLegen Sie zunächst ein Wirtschaftsjahr und eine Eigentümerversammlung an");
    return;
  }

  updateAgendaTable();
  updateReportTemplateTable();

  ui->tableAgenda->setColumnWidth(0, 40);
  ui->tableAgenda->setColumnWidth(1, 250);

  int stretch = ui->tableAgenda->width() - ui->tableAgenda->columnWidth(0) /*- ui->tableAgenda->columnWidth(1)*/ - ui->tableAgenda->columnWidth(2);
  if (stretch > 0)
  {
    ui->tableAgenda->setColumnWidth(1, ui->tableAgenda->columnWidth(1) + stretch);
  }
}

void OrderTab::setParameter (settingsType aParameter, QString aSetting)
{
  switch (aParameter)
  {
    case eDbConnectionName:
      mUser = aSetting;
      break;
  }
}

void OrderTab::updateAgendaTable ()
{
  QSqlQuery query (QSqlDatabase::database(mUser));
  query.prepare("SELECT top_id, top_header, LeerzeilenProtokoll FROM Tagesordnungspunkte WHERE obj_id = :id AND wi_jahr = :year AND etv_nr = :etvnr");
  query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
  query.bindValue(":year", Global::getInstance()->getCurrentYear());
  query.bindValue(":etvnr", Global::getInstance()->getCurrentEtvNumber());
  query.exec();

  mQueryModel = new QSqlQueryModelImpl ();
  mQueryModel->setQuery(query);
  mQueryModel->setHeaderData(0, Qt::Horizontal, tr("Nr."));
  mQueryModel->setHeaderData(1, Qt::Horizontal, tr("Bezeichnung"));
  mQueryModel->setHeaderData(2, Qt::Horizontal, tr("Leerzeilen Protokollvordruck"));

  ui->tableAgenda->setModel(mQueryModel);
  ui->tableAgenda->horizontalHeader()->resizeSection(0, 50);
  ui->tableAgenda->show();
}

void OrderTab::updateReportTemplateTable ()
{
  QSqlQuery query (QSqlDatabase::database(mUser));
  query.prepare("SELECT Protokollueberschrift FROM Eigentuemerversammlungen WHERE obj_id = :id AND wi_jahr = :year AND etv_nr = :etvnr");
  query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
  query.bindValue(":year", Global::getInstance()->getCurrentYear());
  query.bindValue(":etvnr", Global::getInstance()->getCurrentEtvNumber());
  query.exec();

  mQueryModelReportTemplate = new QSqlQueryModelImpl ();
  mQueryModelReportTemplate->setQuery(query);
  //mQueryModelReportTemplate->setHeaderData(0, Qt::Horizontal, tr("Bezeichnung"));

  ui->tableReportTemplate->setModel(mQueryModelReportTemplate);
  ui->tableReportTemplate->show();
}

void OrderTab::on_tableAgenda_doubleClicked(const QModelIndex &index)
{
  if (ui->tableAgenda->selectionModel()->selectedRows().count() == 1)
  {
    int top_id = ui->tableAgenda->model()->index(index.row(),0).data().toInt();
    changeAgendaItemSettings (top_id);
  }
}

void OrderTab::changeAgendaItemSettings (int aId)
{
  int selectedRow = ui->tableAgenda->selectionModel()->selection().indexes().value(0).row();
  OrderItemSettings itemSettings (this);

  QSqlQuery query (QSqlDatabase::database(mUser));
  query.prepare("SELECT LeerzeilenProtokoll FROM Tagesordnungspunkte WHERE obj_id = :id AND wi_jahr = :year AND etv_nr = :etvnr AND top_id = :top_id");
  query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
  query.bindValue(":year", Global::getInstance()->getCurrentYear());
  query.bindValue(":etvnr", Global::getInstance()->getCurrentEtvNumber());
  query.bindValue(":top_id", aId);
  query.exec();

  while (query.next())
  {
    itemSettings.setValue(query.value(0).toInt());
  }

  //abort -> do not save settings
  if (itemSettings.exec() != QDialog::Accepted)
  {
    return;
  }
  else
  {
    QSqlQuery query (QSqlDatabase::database(mUser));
    //set values
    query.prepare("UPDATE Tagesordnungspunkte SET LeerzeilenProtokoll =:lines WHERE obj_id = :id AND wi_jahr = :year AND top_id = :topid AND etv_nr = :etvnum");
    query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
    query.bindValue(":year", Global::getInstance()->getCurrentYear());
    query.bindValue(":etvnum", Global::getInstance()->getCurrentEtvNumber());
    query.bindValue(":topid", aId);
    query.bindValue(":lines", itemSettings.getValue());
    query.exec();

    updateAgendaTable();
    ui->tableAgenda->setFocus();
    ui->tableAgenda->selectRow(selectedRow);
  }
}

void OrderTab::on_tableReportTemplate_doubleClicked(const QModelIndex &index)
{
  PatternEditorReportItemSettings itemSettings (this);

  QSqlQuery query (QSqlDatabase::database(mUser));
  query.prepare("SELECT Protokollueberschrift, Protokollvorlage FROM Eigentuemerversammlungen WHERE obj_id = :id AND wi_jahr = :year AND etv_nr = :etvnr");
  query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
  query.bindValue(":year", Global::getInstance()->getCurrentYear());
  query.bindValue(":etvnr", Global::getInstance()->getCurrentEtvNumber());
  query.exec();

  while (query.next())
  {
    itemSettings.setPatternName(query.value(0).toString());
    itemSettings.setBodyText(query.value(1).toString());
  }

  //abort -> do not save settings
  if (itemSettings.exec() != QDialog::Accepted)
  {
    return;
  }
  else
  {
    QSqlQuery query (QSqlDatabase::database(mUser));
    //set values
    query.prepare("UPDATE Eigentuemerversammlungen SET Protokollvorlage =:newBodyText WHERE obj_id = :id AND wi_jahr = :year AND etv_nr = :etvnr");
    query.bindValue(":id", Global::getInstance()->getCurrentPropertyId());
    query.bindValue(":year", Global::getInstance()->getCurrentYear());
    query.bindValue(":etvnr", Global::getInstance()->getCurrentEtvNumber());
    query.bindValue(":newBodyText", itemSettings.getBodyText());
    query.exec();
  }
}

void OrderTab::on_editButton_clicked()
{
  if (0 != ui->tableAgenda->selectionModel())
  {
    if (ui->tableAgenda->selectionModel()->selectedRows().count() == 1)
    {
      int selectedRow = ui->tableAgenda->selectionModel()->selection().indexes().value(0).row();
      int id = ui->tableAgenda->model()->index(selectedRow,0).data().toInt();

      changeAgendaItemSettings(id);
    }
    else
    {
      QMessageBox::information(this, "Fehler", "Es wurde kein Eintrag ausgewählt.");
    }
  }
  else
    QMessageBox::information(this, "Fehler", "Interner Fehler");
}
