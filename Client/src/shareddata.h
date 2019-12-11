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
    static SharedData &instance() {
        static SharedData data;
        return data;
    }

    QString username;
    QMap<int, QString> seksiMap;
    QMap<int, QString> jabatanMap;
    QMap<QString, UserData> userMap;

private:
    SharedData() {}
};

#endif // SHAREDDATA_H
