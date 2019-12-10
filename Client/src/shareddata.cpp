#include "shareddata.h"

QString SharedData::mUsername;
QMap<int, QString> SharedData::mSeksiMap;
QMap<int, QString> SharedData::mJabatanMap;
QMap<QString, UserData> SharedData::mUserMap;
SharedData::SharedData() {}

QString &SharedData::username()
{
    return mUsername;
}

QMap<int, QString> &SharedData::seksiMap()
{
    return mSeksiMap;
}

QMap<int, QString> &SharedData::jabatanMap()
{
    return mJabatanMap;
}

QMap<QString, UserData> &SharedData::userMap()
{
    return mUserMap;
}
