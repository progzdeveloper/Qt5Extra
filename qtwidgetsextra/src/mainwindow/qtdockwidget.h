#pragma once
#include <QDockWidget>

#include <QtWidgetsExtra>

class QEvent;

class QtDockWidgetPrivate;
class QTWIDGETSEXTRA_EXPORT QtDockWidget :
        public QDockWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoHide READ isAutoHide WRITE setAutoHide)

public:
    explicit QtDockWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    explicit QtDockWidget(const QString& title, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~QtDockWidget();

    bool isAutoHide() const;

public Q_SLOTS:
    void setAutoHide(bool on = true);
    void setFloating(bool on) { QDockWidget::setFloating(on); }

Q_SIGNALS:
    void autoHideChanged(bool on);

protected:
    void leaveEvent(QEvent * e);
    void focusOutEvent(QFocusEvent *e);

private:
    friend class QtDockWidgetTitleBar;
    friend class QtDockWidgetPrivate;
    QScopedPointer<class QtDockWidgetPrivate> d;
};
