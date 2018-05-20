#include "butlerdconnection.h"
#include <QtDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>

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
      m_socket = new QTcpSocket();
      auto address = doc["address"].toString();
      auto tokens = address.split(QRegExp(":"));
      auto host = tokens[0];
      auto port = tokens[1].toInt();
      QObject::connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
      QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));

      m_socket->connectToHost(host, port);
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

void ButlerdConnection::sendSocket(const QJsonObject &obj) {
    QJsonDocument resDoc;
    resDoc.setObject(obj);
    auto resText = resDoc.toJson(QJsonDocument::Compact);
    qDebug() << "[sending over socket] " << resText;
    m_socket->write(resText);
    m_socket->write("\n");
}

void ButlerdConnection::butlerStderrReady()
{
  QProcess *p = (QProcess *)sender();
  p->setReadChannel(QProcess::StandardError);
  if (!p->canReadLine()) { return; }
  QString line = p->readLine();
  qDebug() << "[butler stderr] " << line;
}

void ButlerdConnection::socketConnected()
{
    qDebug() << "connected to butlerd!";
}

void ButlerdConnection::socketReadyRead()
{
    qDebug() << "socket ready read";
    if (!m_socket->canReadLine()) {
        return;
    }
    QString line = m_socket->readLine();
    qDebug() << "[socket] " << line;

    auto doc = QJsonDocument::fromJson(line.toUtf8());
    auto id = doc["id"].toInt();
    auto method = doc["method"].toString();
    auto params = doc["params"].toObject();
    if (id != 0) {
        // it's a response
        auto result = doc["result"];
        qDebug() << "Got result: " << result;

        if (id == 1) {
            // profile result
            auto profiles = result["profiles"].toArray();
            QStringList profileNames;
            foreach (const QJsonValue &profile, profiles) {
                auto profileName = profile["user"]["displayName"].toString();
                qDebug() << "Adding profile name " << profileName;
                profileNames << profileName;
            }
            this->gotProfiles(profileNames);
        }
    } else {
        // it's probably a request
        if (method == "Handshake") {
            auto message = params["message"].toString();
            QJsonObject pkt;
            QCryptographicHash hash(QCryptographicHash::Sha256);
            hash.addData(m_secret.toUtf8());
            hash.addData(message.toUtf8());
            auto hashResult = hash.result();
            auto signature = QString::fromLocal8Bit(hashResult.toHex());
            pkt.insert("jsonrpc", "2.0");
            pkt.insert("id", doc["id"]);
            QJsonObject result;
            result.insert("signature", signature);
            pkt.insert("result", result);
            sendSocket(pkt);

            this->testRequest();
        }
    }
}

void ButlerdConnection::testRequest()
{
    QJsonObject pkt;
    pkt.insert("jsonrpc", "2.0");
    pkt.insert("method", "Profile.List");
    QJsonObject params;
    pkt.insert("params", params);
    pkt.insert("id", 1);
    sendSocket(pkt);
}

