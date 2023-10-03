#include "qtcolorbutton.h"
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QCoreApplication>

#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QWidgetAction>
#include <QtWidgets/QColorDialog>

#include "../painting/qtcolorutils.h"
#include "qtcolorgrid.h"

namespace
{

const QtColorSet kStandardColorSet
{
    Qt::white,   Qt::black,
    Qt::red,     Qt::darkRed,
    Qt::green,   Qt::darkGreen,
    Qt::blue,    Qt::darkBlue,
    Qt::cyan,    Qt::darkCyan,
    Qt::magenta, Qt::darkMagenta,
    Qt::yellow,  Qt::darkYellow,
    Qt::gray,    Qt::darkGray, Qt::lightGray
};

}

class QtColorButtonPrivate
{
    Q_DECLARE_TR_FUNCTIONS(QtColorButtonPrivate)
public:
    QtColorButton* q_ptr;
    QSet<QColor> colors;
    QColor color;
    QMenu* menu;
    QtColorButton::PopupStyle popupStyle;
    int gridWidth;

    QtColorButtonPrivate(QtColorButton* q);
    void initUi();

    QAction* createAction(const QString& name);
    QAction* createAction(const QColor& c, const QString& name = QString());

    void createListMenu(const QStringList& names);
    void createListMenu(const QtColorSet& colors);
    void createListMenu(const QtColorSet& colors, const QStringList& names);

    void createGridMenu(const QStringList& names);
    void createGridMenu(const QtColorSet& colors);
    void createGridMenu(const QtColorSet& colors, const QStringList& names);
    void setupColorGrid(QtColorGrid* grid);

    void recreateMenu();
    void resetMenu();
};

QtColorButtonPrivate::QtColorButtonPrivate(QtColorButton * q)
    : q_ptr(q)
    , menu(Q_NULLPTR)
    , popupStyle(QtColorButton::GridPopup)
    , gridWidth(-1)
{
}

void QtColorButtonPrivate::initUi()
{
    q_ptr->setMenu(menu);
    q_ptr->setText(color.name());
    q_ptr->setIcon(colorPixmap(color, q_ptr->iconSize()));
    q_ptr->setPopupMode(QToolButton::MenuButtonPopup);
    q_ptr->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QObject::connect(q_ptr, &QtColorButton::clicked, q_ptr, &QtColorButton::pickColor);
}

QAction* QtColorButtonPrivate::createAction(const QString &name)
{
    const QColor c(name);
    return createAction(c, name);
}

QAction *QtColorButtonPrivate::createAction(const QColor &c, const QString &name)
{
    QAction* action = Q_NULLPTR;
    if (!c.isValid())
        return action;

    colors.insert(c);
    action = new QAction(menu);
    action->setData(c);
    action->setIcon(colorPixmap(c, q_ptr->iconSize()));
    action->setText(name.isEmpty() ? standardColorName(c) : name);

    QObject::connect(action, &QAction::triggered, q_ptr, &QtColorButton::colorSelected);

    return action;
}

void QtColorButtonPrivate::createListMenu(const QStringList& names)
{
    recreateMenu();
    for (const auto& name : names)
    {
        if (QAction* action = createAction(name))
            menu->addAction(action);
    }
}

void QtColorButtonPrivate::createListMenu(const QtColorSet& colors)
{
    recreateMenu();
    for (const auto& color : colors)
    {
        if (QAction* action = createAction(color))
            menu->addAction(action);
    }
}

void QtColorButtonPrivate::createListMenu(const QtColorSet& colors, const QStringList& names)
{
    recreateMenu();

    auto nameIt = names.begin();
    for (auto it = colors.begin(); it != colors.end(); ++it, ++nameIt)
    {
        QAction* action = createAction(*it, *nameIt);
        if (action)
            menu->addAction(action);
    }
}

void QtColorButtonPrivate::createGridMenu(const QStringList &names)
{
    recreateMenu();

    QtColorGrid* grid = new QtColorGrid(menu);

    QtColorSet colors;
    for (auto it = names.begin(); it != names.end(); ++it) {
        colors.push_back(QColor{ *it });
    }
    grid->setColors(colors);

    setupColorGrid(grid);
}

void QtColorButtonPrivate::createGridMenu(const QtColorSet &colors)
{
    recreateMenu();

    QtColorGrid* grid = new QtColorGrid(menu);
    grid->setColors(colors);

    setupColorGrid(grid);
}

void QtColorButtonPrivate::createGridMenu(const QtColorSet &colors, const QStringList &)
{
    recreateMenu();

    QtColorGrid* grid = new QtColorGrid(menu);
    grid->setColors(colors);

    setupColorGrid(grid);
}

void QtColorButtonPrivate::setupColorGrid(QtColorGrid *grid)
{
    const QSize s = q_ptr->iconSize();
    int width = gridWidth == -1 ? (8 * s.width() + s.width() - 3) : gridWidth;

    grid->setIconSize(s);
    grid->setFixedWidth(width);
    QObject::connect(grid, &QtColorGrid::colorChanged, q_ptr, &QtColorButton::setColor);

    QWidgetAction* action = new QWidgetAction(menu);
    action->setDefaultWidget(grid);
    menu->addAction(action);

    menu->setFixedWidth(width);
}

void QtColorButtonPrivate::recreateMenu()
{
    if (menu)
        delete menu;
    menu = new QMenu(q_ptr);
}

void QtColorButtonPrivate::resetMenu()
{
    QtColorSet colorset(colors.size());
    qCopy(colors.begin(), colors.end(), colorset.begin());
    switch (popupStyle) {
    case QtColorButton::GridPopup:
        createGridMenu(colorset);
        menu->addAction(tr("Select color..."), q_ptr, SLOT(pickColor()));
        break;
    case QtColorButton::ListPopup:
        createListMenu(colorset);
        break;
    }
    q_ptr->setMenu(menu);
}


QtColorButton::QtColorButton(QWidget* parent)
    : QtColorButton(kStandardColorSet, parent)
{}

QtColorButton::QtColorButton(const QStringList& colorNames, QWidget* parent)
    : QToolButton(parent)
    , d(new QtColorButtonPrivate(this))
{
    for (const auto& name : colorNames)
    {
        const QColor c(name);
        if (c.isValid())
            d->colors.insert(c);
    }
    d->resetMenu();
    d->initUi();
}

QtColorButton::QtColorButton(const QtColorSet& colorSet, QWidget* parent)
    : QToolButton(parent)
    , d(new QtColorButtonPrivate(this))
{
    for (const auto& color : colorSet)
        d->colors.insert(color);
    d->resetMenu();
    d->initUi();
}

QtColorButton::~QtColorButton() = default;

void QtColorButton::colorSelected()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
        setColor(action->data().value<QColor>().name());
}

void QtColorButton::pickColor()
{
    QColor c = QColorDialog::getColor(d->color, parentWidget(), tr("Select Color"), QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return;

    setColor(c);
    if (!d->colors.contains(c) && d->popupStyle != GridPopup)
        d->menu->addAction(d->createAction(c));
}

void QtColorButton::setColor(const QColor & color)
{
    d->color = color;
    setText(standardColorName(d->color));
    setIcon(colorPixmap(d->color, iconSize()));
    if (!d->colors.contains(d->color))
        d->menu->addAction(d->createAction(d->color));

    if (d->menu->isVisible())
        d->menu->hide();
    emit colorChanged(d->color);
}

QColor QtColorButton::color() const
{
    return d->color;
}

void QtColorButton::setColors(const QtColorSet & colorSet)
{
    d->colors.clear();
    for (auto it = colorSet.begin(); it != colorSet.end(); ++it)
        d->colors.insert(*it);

    d->resetMenu();
}

QtColorSet QtColorButton::colors() const
{
    QtColorSet colorSet;
    QList<QAction*> actions = d->menu->actions();
    for (auto it = actions.begin(); it != actions.end(); ++it)
        colorSet.push_back((*it)->data().value<QColor>());

    return colorSet;
}

void QtColorButton::setGridWidth(int width)
{
    d->gridWidth = width;
}

int QtColorButton::gridWidth() const
{
    return d->gridWidth;
}

void QtColorButton::setPopupStyle(QtColorButton::PopupStyle style)
{
    if (d->popupStyle != style) {
        d->menu->hide();
        d->popupStyle = style;
        d->resetMenu();
        Q_EMIT popupStyleChanged(d->popupStyle);
    }
}

QtColorButton::PopupStyle QtColorButton::popupStyle() const
{
    return d->popupStyle;
}

void QtColorButton::updateMenu()
{
    d->resetMenu();
}



