#pragma once

#include "platform/InstrumentPlatform.h"
#include <QAbstractListModel>
#include <array>

class FantomSceneModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool edited READ edited NOTIFY editedChanged)
public:
    enum Role { NumberRole=Qt::UserRole+1, ToneMsbRole, ToneLsbRole, ToneProgramRole,
                ChannelRole, ReceiveRole, MuteRole, LevelRole, PanRole, ChorusRole,
                ReverbRole, OutputRole, KeyboardRole, KeyLowRole, KeyHighRole,
                VelocityLowRole, VelocityHighRole, EqEnabledRole };
    Q_ENUM(Role)

    explicit FantomSceneModel(InstrumentPlatform *platform, QObject *parent=nullptr);
    int rowCount(const QModelIndex &parent={}) const override;
    QVariant data(const QModelIndex &index,int role) const override;
    bool setData(const QModelIndex &index,const QVariant &value,int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int,QByteArray> roleNames() const override;
    QString name() const { return m_name; }
    QString status() const { return m_status; }
    bool busy() const { return m_busy; }
    bool edited() const { return m_edited; }

    Q_INVOKABLE bool pull();
    Q_INVOKABLE bool push();
    Q_INVOKABLE bool setZoneValue(int zone,const QString &property,int value);
    Q_INVOKABLE bool assignTone(int zone,int msb,int lsb,int program);

signals:
    void nameChanged(); void statusChanged(); void busyChanged(); void editedChanged();

private:
    struct Zone { QByteArray main=QByteArray(73,'\0'); QByteArray eq=QByteArray(9,'\0'); QByteArray control=QByteArray(112,'\0'); };
    static int byte(const QByteArray &,int);
    bool liveWrite(int zone,InstrumentPlatform::StudioBlock block,int offset,int value);
    void setStatus(QString value);
    void setBusy(bool value);
    InstrumentPlatform *m_platform=nullptr;
    QByteArray m_common=QByteArray(150,'\0');
    std::array<Zone,16> m_zones;
    QString m_name=QStringLiteral("No Scene pulled");
    QString m_status=QStringLiteral("Connect a FANTOM-06/07/08 and pull its Temporary Scene");
    bool m_busy=false;
    bool m_edited=false;
};
