#include "qsqlquerymodelrichtext.h"
#include <QTextDocument>

#define INVALID -1

QSqlQueryModelRichtext::QSqlQueryModelRichtext(QObject *parent)
{
  mDbConnectionName = "";
  mPropertyId = INVALID;
  mYear = INVALID;
  mAgendaNum = INVALID;
}

void QSqlQueryModelRichtext::setDbConnectionName (QString &dbName)
{
  mDbConnectionName = dbName;
}

void QSqlQueryModelRichtext::setPropertyId (int propId)
{
  mPropertyId = propId;
}

void QSqlQueryModelRichtext::setYear (int year)
{
  mYear = year;
}

void QSqlQueryModelRichtext::setAgendaNum (int agendaNum)
{
  mAgendaNum = agendaNum;
}

QVariant QSqlQueryModelRichtext::data(const QModelIndex &index, int role) const
{
  //remove html syntax from database entry
  //return as plain string
  // - just for tableview purpose

  QVariant value = QSqlQueryModel::data(index, role);

  if (value.isValid() && role == Qt::DisplayRole)
  {
    QTextDocument doc;
    doc.setHtml(value.toString());

    return doc.toPlainText();
  }
  return value;
}
