#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include <QMap>

struct UserData {
    QString username;
    QString nama;
    int seksi;
    int jabatan;
    bool online;
};

class SharedData
{
public:
    SharedData();

    static QString &username();
    static QMap<int, QString> &seksiMap();
    static QMap<int, QString> &jabatanMap();
    static QMap<QString, UserData> &userMap();

private:
    static QString mUsername;
    static QMap<int, QString> mSeksiMap;
    static QMap<int, QString> mJabatanMap;
    static QMap<QString, UserData> mUserMap;
};

#endif // SHAREDDATA_H
