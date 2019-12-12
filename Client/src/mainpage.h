#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>

class QTabWidget;
class QListView;
class QTreeView;
class QStandardItem;
class MainPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainPage(QWidget *parent = nullptr);

    void loadData();
    void loadPendingChat();
    void refreshUserView();

private slots:
    void onEngineGotData(const QVariantMap &data);
    void onUserItemActivated(const QModelIndex &index);

private:
    QTabWidget *mTab;
    QListView *mPengumumanView;
    QListView *mLobbyView;
    QTreeView *mUserView;
    QHash<QString, QStandardItem *> mUserItemHash;
};

#endif // MAINPAGE_H
