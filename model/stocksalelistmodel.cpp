#include "stocksalelistmodel.h"

#include "util/common.h"

StockSaleListModel::StockSaleListModel(QObject* parent) :
    QAbstractListModel(parent) {
    headers_.append("时间");
    headers_.append("价格");
    headers_.append("数量");
    headers_.append("佣金");
    headers_.append("过户费");
    headers_.append("印花税");
    headers_.append("利润");
}

QVariant StockSaleListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role != Qt::DisplayRole) {
        return QVariant();
    }

    if(orientation == Qt::Horizontal) {
        if(section >= 0 && section < headers_.size()) {
            return headers_.at(section);
        }
    }

    return {};
}

int StockSaleListModel::rowCount(const QModelIndex& parent) const {
    if(parent.isValid()) {
        return 0;
    }

    return stocks_.size();
}

int StockSaleListModel::columnCount(const QModelIndex& parent) const {
    if(parent.isValid()) {
        return 0;
    }

    return headers_.size();
}

QVariant StockSaleListModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid() || index.row() > stocks_.size()) {
        return QVariant();
    }

    const auto& stock = stocks_.at(index.row());

    using namespace Common;

    switch(role) {
    case Roles::TradeDateRole:
        return stock.tradeDate;
    case Roles::PriceRole:
        return to3Decimals(stock.price);
    case Roles::StockHoldingsRole:
        return stock.holdings;
    case Roles::BrokerageFeeRole:
        return to3Decimals(stock.brokerageFee);
    case Roles::TransferFeeRole:
        return to3Decimals(stock.transferFee);
    case Roles::SecuritiesStampTaxRole:
        return to3Decimals(stock.securitiesStampTax);
    case Roles::ProceedsRole:
        return to3Decimals(stock.proceeds);
    }

    return QVariant();
}

void StockSaleListModel::updateRecords(QList<QtModel::StockSaleModel> models) {
    beginResetModel();
    auto _  = qScopeGuard([this] { endResetModel(); });

    stocks_ = models;
    stocks_.detach();
}
