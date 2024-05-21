#include <QGraphicsEffect>
#include <QtWidgetsExtra>

class QTWIDGETSEXTRA_EXPORT QtGraphicsEffectPipeline : public QGraphicsEffect
{
    Q_OBJECT

public:
    enum RenderMode
    {
        RenderDirect,
        RenderCached
    };
    Q_ENUM(RenderMode)

    explicit QtGraphicsEffectPipeline(QObject* parent = Q_NULLPTR);
    ~QtGraphicsEffectPipeline();

    void setRenderMode(RenderMode mode);
    RenderMode renderMode() const;

    void draw(QPainter* painter) Q_DECL_OVERRIDE;
    bool raise(QGraphicsEffect* e);
    bool lower(QGraphicsEffect* e);

    bool isEmpty() const;
    bool contains(QGraphicsEffect* e) const;

protected:
    bool event(QEvent* e) Q_DECL_OVERRIDE;
    void childAddedEvent(QChildEvent* e);
    void childRemovedEvent(QChildEvent* e);
private:
    friend class QtGraphicsEffectPipelinePrivate;
    QScopedPointer<class QtGraphicsEffectPipelinePrivate> d;
};
