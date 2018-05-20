#include "butlerdconnection.h"
#include <QtDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

ButlerdConnection::ButlerdConnection(QObject *parent) : QObject(parent)
{
    qDebug() << "ButlerdConnection was created.";
    m_parent = parent;
}

Q_INVOKABLE void ButlerdConnection::connect(const QString &username, const QString &password)
{
    qDebug() << "Should connect with username "
                   << username << " and password"
                   << password;

    QString program = "/home/amos/Dev/go/bin/butler";
    QStringList args;
    args << "--json";
    args << "--dbpath" << "/home/amos/.config/itch/db/butler.db";
    args << "daemon";

    m_proc = new QProcess(m_parent);

    QObject::connect(m_proc, SIGNAL(readyReadStandardError()), this, SLOT(butlerStderrReady()));
    QObject::connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(butlerStdoutReady()));

    qDebug() << "starting " << program;
    m_proc->start(program, args);
    m_proc->setStandardErrorFile("/dev/stderr");
    qDebug() << "waiting for started...";
    m_proc->waitForStarted();
    m_proc->setReadChannel(QProcess::StandardOutput);
}

QString GetRandomString(int randomStringLength)
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = qrand() % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}

void ButlerdConnection::butlerStdoutReady()
{
  QProcess *p = (QProcess *)sender();
  p->setReadChannel(QProcess::StandardOutput);
  if (!p->canReadLine()) { return; }
  QString line = p->readLine();
  qDebug() << "[butler out] " << line;

  auto doc = QJsonDocument::fromJson(line.toUtf8());
  auto type = doc["type"].toString();
  // can't use switch, what year is this
  if (type == "butlerd/secret-request") {
      auto minLength = doc["minLength"].toInt();
      qDebug() << "Should generate secret, min length = " << minLength;
      auto secret = GetRandomString(minLength);
      m_secret = secret;
      qDebug() << "Generated secret: " << secret;

      QJsonObject res;
      res.insert("type", "butlerd/secret-result");
      res.insert("secret", secret);
      this->sendStderr(res);
  } else if (type == "butlerd/listen-notification") {
      qDebug() << "Should connect to butlerd";
  }
}

void ButlerdConnection::sendStderr(const QJsonObject &obj) {
    QJsonDocument resDoc;
    resDoc.setObject(obj);
    auto resText = resDoc.toJson(QJsonDocument::Compact);
    qDebug() << "[sending over stderr] " << resText;
    m_proc->write(resText);
    m_proc->write("\n");
}

void ButlerdConnection::butlerStderrReady()
{
  QProcess *p = (QProcess *)sender();
  p->setReadChannel(QProcess::StandardError);
  if (!p->canReadLine()) { return; }
  QString line = p->readLine();
  qDebug() << "[butler stderr] " << line;
}

