#include "qtpathlineedit.h"
#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QDirModel>
#include <QCompleter>
#include <QMutex>
#include <QMutexLocker>

class QtPathLineEditPrivate
{
public:
    QStringList filter;
    int type;
};


QtPathLineEdit::QtPathLineEdit(QWidget *parent, Qt::WindowFlags flags) 
    : QtLineBoxEdit(parent, flags), d(new QtPathLineEditPrivate)
{
    d->type = static_cast<int>(FileOpenPath);
    setReadOnly(false);
    connect(this, &QtLineBoxEdit::clicked, this, &QtPathLineEdit::setPath);
    setFocusPolicy(Qt::StrongFocus);
}

QtPathLineEdit::~QtPathLineEdit(void) = default;

void QtPathLineEdit::setFilter(const QStringList& filter)
{

    d->filter = filter;
}

QStringList QtPathLineEdit::filter() const
{

    return d->filter;
}

void QtPathLineEdit::setType(PathType type)
{

    d->type = type;
}

QtPathLineEdit::PathType QtPathLineEdit::type() const
{

    return static_cast<PathType>(d->type);
}


void QtPathLineEdit::setPath()
{

    setFocus();
    QString path = text();
    if (path.isEmpty())
        path = qApp->property("lastDir").toString();

    switch(static_cast<PathType>(d->type))
    {
    case DirPath:
        path = QFileDialog::getExistingDirectory(this, tr("Select Directory"), path);
        break;
    case FileOpenPath:
        path = QFileDialog::getOpenFileName(this, tr("Select File"), path, d->filter.join(";;"));
        break;
    case FileSavePath:
        path = QFileDialog::getSaveFileName(this, tr("Select File"), path, d->filter.join(";;"));
        break;
    }
    if (!path.isEmpty()) {
        setText(path);
        qApp->setProperty("lastDir", path);
    }
}



class QtPathLineDelegatePrivate
{
public:
    QStringList pathFilter;
    QtPathLineEdit::PathType pathType;
    bool completionEnabled;
    int column, role;
    QModelIndex curIdx;
};

QtPathLineDelegate::QtPathLineDelegate(QObject *parent)
    : QItemDelegate(parent)
    , d(new QtPathLineDelegatePrivate)
{
    d->pathFilter << tr("All files (*.*)");
    d->pathType = QtPathLineEdit::FileOpenPath;
    d->completionEnabled = true;
    d->column = 0;
    d->role = Qt::EditRole;
}

QtPathLineDelegate::~QtPathLineDelegate() = default;

void QtPathLineDelegate::setItemRole(int role)
{

    d->role = role;
}

int QtPathLineDelegate::itemRole() const
{

    return d->role;
}

void QtPathLineDelegate::setTypeColumn(int c)
{

    d->column = c;
}

int QtPathLineDelegate::typeColumn() const
{

    return d->column;
}

void QtPathLineDelegate::setFilter(const QStringList& filter)
{

    d->pathFilter = filter;
}

QStringList QtPathLineDelegate::filter() const
{

    return d->pathFilter;
}

void QtPathLineDelegate::setType(QtPathLineEdit::PathType type)
{

    d->pathType = type;
}

QtPathLineEdit::PathType QtPathLineDelegate::type() const
{

    return d->pathType;
}

void QtPathLineDelegate::setCompletionEnabled(bool on)
{

    d->completionEnabled = on;
}

bool QtPathLineDelegate::isCompletionEnabled() const
{

    return d->completionEnabled;
}

QWidget *QtPathLineDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */,
                                          const QModelIndex & index) const
{

    const QAbstractItemModel * model = index.model();
    QModelIndex idx = model->index(index.row(), d->column);

    QtLineBoxEdit* pathEdit = new QtLineBoxEdit(parent, Qt::FramelessWindowHint);
    pathEdit->setProperty( "pathType", (d->column == -1 ? d->pathType : idx.data(d->role).toInt()) );

    if (d->completionEnabled)
    {
        QDir::Filters dirFilters =
                (d->pathType == QtPathLineEdit::DirPath ?
                     QDir::Dirs|QDir::NoDotAndDotDot        :
                     QDir::AllEntries|QDir::NoDotAndDotDot);

        QCompleter *completer = new QCompleter(pathEdit);
        completer->setModel(new QDirModel(QStringList(), dirFilters, QDir::NoSort, completer));
        pathEdit->setCompleter(completer);
    }
    connect(pathEdit, &QtPathLineEdit::clicked, this, &QtPathLineDelegate::setPath);
    return pathEdit;
}

void QtPathLineDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

    QString path = index.model()->data(index, d->role).toString();
    QtPathLineEdit *pathEdit = static_cast<QtPathLineEdit*>(editor);
    pathEdit->setText(path);
}

void QtPathLineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QtLineBoxEdit *pathEdit = static_cast<QtLineBoxEdit*>(editor);
    model->setData(index, pathEdit->text(), Qt::EditRole);
}

void QtPathLineDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                              const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}


void QtPathLineDelegate::setPath()
{
    blockSignals(true);

    QtLineBoxEdit *edit = qobject_cast<QtLineBoxEdit*>(sender());
    QString path = edit->text();
    int type = (d->column == -1 ? d->pathType : edit->property("pathType").toInt());
    switch(static_cast<QtPathLineEdit::PathType>(type))
    {
    case QtPathLineEdit::DirPath:
        path = QFileDialog::getExistingDirectory(edit,
                                                 tr("Select Directory"),
                                                 path);
        break;
    case QtPathLineEdit::FileOpenPath:
        path = QFileDialog::getOpenFileName(edit,
                                            tr("Select File"),
                                            path,
                                            d->pathFilter.join(";;"));
        break;
    case QtPathLineEdit::FileSavePath:
        path = QFileDialog::getSaveFileName(edit,
                                            tr("Select File"),
                                            path,
                                            d->pathFilter.join(";;"));
        break;
    }

    if (path.isEmpty())
        return;


    if (edit)
        edit->setText(path);

    blockSignals(false);
}
