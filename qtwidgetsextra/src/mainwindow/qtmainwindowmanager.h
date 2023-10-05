#pragma once
#include <QFrame>

#include <QtWidgetsExtra>

class QMainWindow;

class QListWidgetItem;

class QtMainWindowManagerPrivate;
class QTWIDGETSEXTRA_EXPORT QtMainWindowManager :
        public QFrame
{
    Q_OBJECT

public:
    QtMainWindowManager(QWidget* parent = 0, Qt::WindowFlags flags = Qt::ToolTip);
    ~QtMainWindowManager();

    void setMainWindow(QMainWindow* w);
    QMainWindow* mainWindow() const;

    void setHighlightEnabled(bool on);
    bool isHighlightEnaled() const;

protected:
    void paintEvent(QPaintEvent* e);
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);
    bool eventFilter(QObject* object, QEvent* e);

private:
    bool filterKeyEvent(QObject* object, QKeyEvent* e);
    bool execAction(int key);

private Q_SLOTS:
    void itemClicked(QListWidgetItem*);

private:
    friend class QtMainWindowManagerPrivate;
    QScopedPointer<class QtMainWindowManagerPrivate> d;
};
