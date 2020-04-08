#include <QAudioDeviceInfo>

#include "alert.h"


Alert::Alert(QWidget *parent)
    : QMainWindow(parent)
{
    reconnectTimer =new QTimer;
    connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnectBroker()));
    reconnectTimer->start(3000);
    QWidget *centraWidget =new QWidget;
    mainLayout=new QVBoxLayout();
    playLayout =new QHBoxLayout();
    connectLayout =new QHBoxLayout();
    pushLayout=new QHBoxLayout();
    subLayout=new QHBoxLayout();

    m_deviceBox = new QComboBox();
    const QAudioDeviceInfo &defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    m_deviceBox->addItem(defaultDeviceInfo.deviceName(), qVariantFromValue(defaultDeviceInfo));
    m_hostLable= new QLabel("host:");
    m_portLable= new QLabel("port:");
    m_pushTopicLable= new QLabel("topic:");
    m_subTopicLable= new QLabel("topic:");
    m_autoReconnectLable= new QLabel("auto");
    m_autoResubLable= new QLabel("auto");

    m_hostEdit= new QLineEdit("39.99.185.210");
    m_portEdit= new QLineEdit("1883");
    m_pushTopicEdit= new QLineEdit("alert/01");
    m_subTopicEdit= new QLineEdit("alert/+");
    m_pushMsgEdit= new QLineEdit("hello");
    m_subMsgPlainTextEdit= new QPlainTextEdit();

    m_playButton = new QPushButton(tr("PLAY"));
    m_stopButton = new QPushButton(tr("STOP"));
    m_connectButton = new QPushButton(tr("CONNECT"));
    m_pushButton = new QPushButton(tr("PUSH"));
    m_pushButton->setStyleSheet("background: #6666FF");
    m_subButton = new QPushButton(tr("SUB"));
    m_autoReconnectCheckBox =new QCheckBox();
    m_autoReconnectCheckBox->setChecked(true);
    m_autoResubCheckBox =new QCheckBox();
    m_autoResubCheckBox->setChecked(true);

    playLayout->addWidget(m_deviceBox);
    playLayout->addWidget(m_playButton);
    playLayout->addWidget(m_stopButton);

    connectLayout->addWidget(m_hostLable);
    connectLayout->addWidget(m_hostEdit);
    connectLayout->addWidget(m_portLable);
    connectLayout->addWidget(m_portEdit);
    connectLayout->addWidget(m_autoReconnectLable);
    connectLayout->addWidget(m_autoReconnectCheckBox);
    connectLayout->addWidget(m_connectButton);

    pushLayout->addWidget(m_pushTopicLable);
    pushLayout->addWidget(m_pushTopicEdit);
    pushLayout->addWidget(m_pushMsgEdit);
    pushLayout->addWidget(m_pushButton);

    subLayout->addWidget(m_subTopicLable);
    subLayout->addWidget(m_subTopicEdit);
    subLayout->addWidget(m_autoResubLable);
    subLayout->addWidget(m_autoResubCheckBox);
    subLayout->addWidget(m_subButton);

    mainLayout->addLayout(playLayout);
    mainLayout->addLayout(connectLayout);
    mainLayout->addLayout(subLayout);
    mainLayout->addLayout(pushLayout);
    mainLayout->addWidget(m_subMsgPlainTextEdit);
    centraWidget->setLayout(mainLayout);
    setCentralWidget(centraWidget);
    resize(1280,768);
    player = new QMediaPlayer;

    QString appPath= QCoreApplication::applicationDirPath();
    playList =new QMediaPlaylist;
    playList->addMedia(QUrl::fromLocalFile(appPath.append("/alert.mp3")));
    playList->setCurrentIndex(1);
    playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    player->setPlaylist(playList);
    player->setVolume(100);
    m_client = new QMqttClient(this);
    m_client->setKeepAlive(5);
    connect(m_playButton, SIGNAL(clicked()),this, SLOT(play()));
    connect(m_stopButton, SIGNAL(clicked()),this, SLOT(stop()));
    connect(m_connectButton, SIGNAL(clicked()),this, SLOT(connectBroker()));
    connect(m_pushButton, SIGNAL(clicked()), SLOT(push()));
    connect(m_subButton, SIGNAL(clicked()), SLOT(sub()));
    connect(m_client, &QMqttClient::stateChanged, this, &Alert::clientStateChange);
    connect(m_client, &QMqttClient::messageReceived, this, &Alert::getMessage);

}

void Alert::getMessage(const QByteArray &message, const QMqttTopicName &topic){
    const QString content = QDateTime::currentDateTime().toString()
                        + QLatin1String(" Received Topic: ")
                        + topic.name()
                        + QLatin1String(" Message: ")
                        + message
                        + QLatin1Char('\n');
    m_subMsgPlainTextEdit->insertPlainText(content);
    if (message =="alert"){
        player->play();
    }
}


void Alert::play()
{
    m_subMsgPlainTextEdit->insertPlainText("play");
    player->play();
}
void Alert::stop()
{
    player->stop();
}

void Alert::connectBroker()
{
    m_client->setHostname(m_hostEdit->text());
    m_client->setPort(m_portEdit->text().toInt());
    m_client->connectToHost();
}
void Alert::reconnectBroker(){
   QMqttClient::ClientState connectState= m_client->state();
   switch (connectState) {
   case QMqttClient::ClientState::Disconnected:
       m_client->setHostname(m_hostEdit->text());
       m_client->setPort(m_portEdit->text().toInt());
       m_client->connectToHost();
       break;
   case QMqttClient::ClientState:: Connecting:
       break;
   case QMqttClient::ClientState:: Connected:
       break;
   }
   if (m_subscription !=nullptr){
       QMqttSubscription::SubscriptionState subState =m_subscription->state();
       switch (subState) {
       case QMqttSubscription::Unsubscribed:
           m_subButton->setStyleSheet("background: #DDDDDD");
           m_subButton->setText("SUB");
           break;
       case QMqttSubscription::SubscriptionPending:
           m_subButton->setStyleSheet("background: #FFFF00");
           m_subButton->setText("Pending");
           break;
       case QMqttSubscription::Subscribed:
           m_subButton->setStyleSheet("background: #33FF33");
           m_subButton->setText("SUBED");
           break;
       case QMqttSubscription::Error:
           m_subButton->setStyleSheet("background: #FFFF00");
           m_subButton->setText("Error");
           break;
    //   default:
    //       m_subButton->setStyleSheet("background: #FFFF00");
    //       m_subButton->setText("--Unknown--");
    //       break;
       }
   }


}
void Alert::push()
{
    m_client->publish(m_pushTopicEdit->text(), m_pushMsgEdit->text().toUtf8());

}
void Alert::sub()
{
    m_subscription=m_client->subscribe(m_subTopicEdit->text());

    connect(m_subscription, &QMqttSubscription::stateChanged, this, &Alert::updateStatus);
}

void Alert::clientStateChange(QMqttClient::ClientState state){
    switch (state) {
    case QMqttClient::ClientState::Disconnected:
        m_connectButton->setStyleSheet("background: #DDDDDD");
        m_connectButton->setText("Connect");
        if (m_subscription != nullptr){
            m_subscription->unsubscribe();
        }
        break;
    case QMqttClient::ClientState:: Connecting:
        m_connectButton->setText("Connect...");
        break;
    case QMqttClient::ClientState:: Connected:
        m_connectButton->setStyleSheet("background: #33FF33");
        m_connectButton->setText("Connected");

        if (m_subscription != nullptr){
            m_subscription=m_client->subscribe(m_subTopicEdit->text());
            connect(m_subscription, &QMqttSubscription::stateChanged, this, &Alert::updateStatus);
        }
        break;
    }

}
void Alert::updateStatus(QMqttSubscription::SubscriptionState state)
{
    switch (state) {
    case QMqttSubscription::Unsubscribed:
        m_subButton->setStyleSheet("background: #DDDDDD");
        m_subButton->setText("SUB");
        break;
    case QMqttSubscription::SubscriptionPending:
        m_subButton->setStyleSheet("background: #FFFF00");
        m_subButton->setText("Pending");
        break;
    case QMqttSubscription::Subscribed:
        m_subButton->setStyleSheet("background: #33FF33");
        m_subButton->setText("SUBED");
        break;
    case QMqttSubscription::Error:
        m_subButton->setStyleSheet("background: #FFFF00");
        m_subButton->setText("Error");
        break;
    default:
        m_subButton->setStyleSheet("background: #FFFF00");
        m_subButton->setText("--Unknown--");
        break;
    }
}
Alert::~Alert()
{

}
