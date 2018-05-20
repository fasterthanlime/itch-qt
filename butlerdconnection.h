#ifndef BUTLERDCONNECTION_H
#define BUTLERDCONNECTION_H

#include <QObject>
#include <QProcess>

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

private:
    QObject *m_parent;
    QProcess *m_proc;
};

#endif // BUTLERDCONNECTION_H
