#ifndef ALERT_H
#define ALERT_H
#include <QCoreApplication>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>

class Alert : public QMainWindow
{
    Q_OBJECT

public:
    Alert(QWidget *parent = 0);
    ~Alert();
private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *playLayout;
    QHBoxLayout *connectLayout;
    QHBoxLayout *pushLayout;
    QHBoxLayout *subLayout;
    QComboBox *m_deviceBox;

    QLabel *m_hostLable;
    QLabel *m_portLable;
    QLabel *m_pushTopicLable;
    QLabel *m_subTopicLable;

    QLineEdit *m_hostEdit;
    QLineEdit *m_portEdit;
    QLineEdit *m_pushTopicEdit;
    QLineEdit *m_subTopicEdit;

    QLineEdit *m_pushMsgEdit;
    QPlainTextEdit *m_subMsgPlainTextEdit;

    QPushButton *m_playButton;
    QPushButton *m_stopButton;
    QPushButton *m_connectButton;
    QPushButton *m_pushButton;
    QPushButton *m_subButton;

    QMediaPlayer *player;
    QMediaPlaylist *playList;
    QMqttClient *m_client;
    QMqttSubscription *m_subscription;
private slots:
    void play();
    void stop();
    void start();
    void push();
    void sub();
    void getPing();
    void clientStateChange(QMqttClient::ClientState state);
    void getMessage(const QByteArray &message, const QMqttTopicName &topic);
    void updateStatus(QMqttSubscription::SubscriptionState state);
};

#endif // ALERT_H
