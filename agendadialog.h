#ifndef AGENDADIALOG_H
#define AGENDADIALOG_H

#include <QDialog>

class QSqlQueryModelAgendaView;

namespace Ui {
  class AgendaDialog;
}

class AgendaDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AgendaDialog(QWidget *parent, QString &rDbConnName, int estateId, int agendaYear, int agendaNum);
  void updateAgendaTable();
  ~AgendaDialog();

private slots:
  void on_moveAgendaItemDown_clicked();
  void on_moveAgendaItemUp_clicked();
  void on_editEntry_clicked();
  void on_addEntry_clicked();
  void on_deleteEntry_clicked();
  void on_tableAgenda_doubleClicked(const QModelIndex &index);

private:
  void initAgendaTable ();
  void changeAgendaItemSettings (int aTopId);

  Ui::AgendaDialog *ui;
  QSqlQueryModelAgendaView* mQueryModelAgendaView;
  QString mDbConnName;
  int mEstateId;
  int mAgendaYear;
  int mAgendaNum;
};

#endif // AGENDADIALOG_H