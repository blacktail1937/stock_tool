#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QSortFilterProxyModel>

#include "database/stocktable.h"

class StockFilterProxyModel;
class StockBaseInfoListModel: public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(StockFilterProxyModel* filterModel READ filterModel CONSTANT)

  public:
    enum Roles
    {
        CodeRole = Qt::UserRole + 1,
        NameRole,
    };

    explicit StockBaseInfoListModel(QObject* parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void updateFilter(const QString& text);

    StockFilterProxyModel* filterModel() const { return filter_; }

  public slots:
    void load(QList<QtModel::StockBaseInfoModel> model);

  private:
    QList<QtModel::StockBaseInfoModel> stocks_;
    // QList<QtModel::StockBaseInfoModel> stocksFilter_;
    StockFilterProxyModel* filter_;
};

class StockFilterProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit StockFilterProxyModel(QObject* parent = nullptr) :
        QSortFilterProxyModel(parent) {
        setFilterRole(Qt::DisplayRole);
        setFilterCaseSensitivity(Qt::CaseInsensitive);
    }

  protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override {
        QString filterText = filterRegularExpression().pattern();
        if(filterText.isEmpty()) {
            return true;
        }

        auto index = sourceModel()->index(row, 0, parent);

        auto code  = sourceModel()->data(index, StockBaseInfoListModel::CodeRole).toString();
        auto name  = sourceModel()->data(index, StockBaseInfoListModel::NameRole).toString();

        return code.contains(filterText, Qt::CaseInsensitive) ||
               name.contains(filterText, Qt::CaseInsensitive);
    }
};
