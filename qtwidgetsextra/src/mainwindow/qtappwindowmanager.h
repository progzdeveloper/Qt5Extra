#pragma once
#include <QFrame>

#include <QtWidgetsExtra>

class QMainWindow;

class QListWidgetItem;

class QtAppWindowManagerPrivate;
class QTWIDGETSEXTRA_EXPORT QtAppWindowManager :
        public QFrame
{
    Q_OBJECT

public:
    QtAppWindowManager(QWidget* parent = 0, Qt::WindowFlags flags = Qt::ToolTip);
    ~QtAppWindowManager();

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

private slots:
    void itemClicked(QListWidgetItem*);

private:
    friend class QtAppWindowManagerPrivate;
    QScopedPointer<class QtAppWindowManagerPrivate> d;
};
