#include "qtgraphicseffectpipeline.h"
#include <QTimer>
#include <QEvent>
#include <QPainter>
#include <QPixmap>
#include <QList>

namespace
{
    class _GraphicsEffect : public QGraphicsEffect
    {
    public:
        using QGraphicsEffect::draw;
    };
}

class QtGraphicsEffectPipelinePrivate
{
public:
    QtGraphicsEffectPipeline* q = Q_NULLPTR;
    QList<QGraphicsEffect*> effects;
    QtGraphicsEffectPipeline::RenderMode mode = QtGraphicsEffectPipeline::RenderDirect;

    QtGraphicsEffectPipelinePrivate(QtGraphicsEffectPipeline* effect)
        : q(effect)
    {}

    void onChildAdded(QGraphicsEffect* effect)
    {
        if (!effect || effects.contains(effect))
            return;

        effects.push_back(effect);
        q->update();
    }

    void onChildRemoved(QGraphicsEffect* effect)
    {
        if (!effect)
            return;

        effects.removeAll(effect);
        q->update();
    }

    void renderDirect(QPainter* painter)
    {
        for (auto effect : qAsConst(effects))
            static_cast<_GraphicsEffect*>(effect)->draw(painter);
    }

    void renderCached(QPainter* painter)
    {
        QPoint offset;
        QPixmap pixmap = q->sourcePixmap(Qt::DeviceCoordinates, &offset);
        if (pixmap.isNull())
            return q->drawSource(painter);

        QPainter bufferPainter;
        bufferPainter.begin(&pixmap);
        renderDirect(&bufferPainter);
        bufferPainter.end();

        const QTransform restoreTransform = painter->worldTransform();
        painter->setWorldTransform({});
        painter->drawPixmap(offset, pixmap);
        painter->setWorldTransform(restoreTransform);
    }
};


QtGraphicsEffectPipeline::QtGraphicsEffectPipeline(QObject *parent)
    : QGraphicsEffect(parent)
    , d(new QtGraphicsEffectPipelinePrivate(this))
{
}

QtGraphicsEffectPipeline::~QtGraphicsEffectPipeline() = default;

void QtGraphicsEffectPipeline::setRenderMode(RenderMode mode)
{
    if (d->mode == mode)
        return;

    d->mode = mode;
    update();
}

QtGraphicsEffectPipeline::RenderMode QtGraphicsEffectPipeline::renderMode() const
{
    return d->mode;
}

void QtGraphicsEffectPipeline::draw(QPainter *painter)
{
    if (isEmpty())
        return drawSource(painter);

    switch(d->mode)
    {
    case RenderDirect:
        d->renderDirect(painter);
        break;
    case RenderCached:
        d->renderCached(painter);
        break;
    default:
        break;
    }
}

bool QtGraphicsEffectPipeline::raise(QGraphicsEffect *e)
{
    const int i = d->effects.indexOf(e);
    if (i < 1)
        return false;

    d->effects.move(i, 0);
    update();
    return true;
}

bool QtGraphicsEffectPipeline::lower(QGraphicsEffect *e)
{
    const int i = d->effects.indexOf(e);
    if (i < 0 || i == (d->effects.size() - 1))
        return false;

    d->effects.move(i, d->effects.size() - 1);
    update();
    return true;
}

bool QtGraphicsEffectPipeline::isEmpty() const
{
    return d->effects.isEmpty();
}

bool QtGraphicsEffectPipeline::contains(QGraphicsEffect *e) const
{
    return d->effects.contains(e);
}

bool QtGraphicsEffectPipeline::event(QEvent* e)
{
    QObject* child = nullptr;
    switch(e->type())
    {
    case QEvent::ChildAdded:
        childAddedEvent(static_cast<QChildEvent*>(e));
        break;
    case QEvent::ChildRemoved:
        childRemovedEvent(static_cast<QChildEvent*>(e));
        break;
    default:
        break;
    }
    return QObject::event(e);
}

void QtGraphicsEffectPipeline::childAddedEvent(QChildEvent *e)
{
    if (QObject* object = e->child())
        QTimer::singleShot(0, this, [this, object]() { d->onChildAdded(qobject_cast<QGraphicsEffect*>(object)); });
}

void QtGraphicsEffectPipeline::childRemovedEvent(QChildEvent *e)
{
    if (QObject* object = e->child())
        QTimer::singleShot(0, this, [this, object]() { d->onChildRemoved(qobject_cast<QGraphicsEffect*>(object)); });
}
