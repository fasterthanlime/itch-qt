#ifndef BUTLERDCONNECTION_H
#define BUTLERDCONNECTION_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QTcpSocket>

class ButlerdConnection : public QObject
{
    Q_OBJECT
public:
    explicit ButlerdConnection(QObject *parent = nullptr);

    Q_INVOKABLE void connect(const QString &username, const QString &password);

signals:

public slots:
    void butlerStdoutReady();
    void butlerStderrReady();
    void socketConnected();

private:
    QObject *m_parent;
    QProcess *m_proc;
    QString m_secret;
    QTcpSocket *m_socket;

    void sendStderr(const QJsonObject &obj);
};

#endif // BUTLERDCONNECTION_H
