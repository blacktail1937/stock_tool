#include "stockbaseinfolistmodel.h"
#include "util/common.h"

StockBaseInfoListModel::StockBaseInfoListModel(QObject* parent) :
    QAbstractListModel(parent) {
    filter_ = new StockFilterProxyModel(this);
    filter_->setSourceModel(this);
}

int StockBaseInfoListModel::rowCount(const QModelIndex& parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if(parent.isValid()) {
        return 0;
    }

    return stocks_.size();
}

QVariant StockBaseInfoListModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid() || index.row() >= stocks_.size()) {
        return QVariant();
    }

    const auto& stock = stocks_.at(index.row());

    using namespace Common;
    if(role == Qt::DisplayRole) {
        // switch(index.column()) {
        // case modelRoleConvertor(Roles::CodeRole):
        // case modelRoleConvertor(Roles::NameRole):
        return QString("%1-%2").arg(stock.code, stock.name);
        // }
    }
    else {
        switch(role) {
        case Roles::CodeRole:
            return stock.code;
        case Roles::NameRole:
            return stock.name;
        }
    }

    return QVariant();
}

QHash<int, QByteArray> StockBaseInfoListModel::roleNames() const {
    return {
        { Roles::CodeRole,    "code" },
        { Roles::NameRole,    "name" },
        { Qt::DisplayRole, "display" }
    };
}

void StockBaseInfoListModel::updateFilter(const QString& text) {
    filter_->setFilterFixedString(text);
}

void StockBaseInfoListModel::load(QList<QtModel::StockBaseInfoModel> model) {
    beginResetModel();
    stocks_ = model;
    endResetModel();
}

