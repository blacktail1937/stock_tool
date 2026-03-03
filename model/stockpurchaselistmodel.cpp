#include "stockpurchaselistmodel.h"

#include "util/common.h"

StockPurchaseListModel::StockPurchaseListModel(QObject* parent) :
    QAbstractListModel(parent) {
    headers_.append("时间");
    headers_.append("价格");
    headers_.append("数量");
    headers_.append("佣金");
    headers_.append("过户费");
    headers_.append("成本");

    // stocks_.append({ 1000, "1234", 1234, 1234, 1234, 1234, 1234 });
}

QVariant StockPurchaseListModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const {
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

int StockPurchaseListModel::rowCount(const QModelIndex& parent) const {
    if(parent.isValid()) {
        return 0;
    }

    return stocks_.size();
}

int StockPurchaseListModel::columnCount(const QModelIndex& parent) const {
    if(parent.isValid()) {
        return 0;
    }

    return headers_.size();
}

QVariant StockPurchaseListModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid() || index.row() > stocks_.size()) {
        return QVariant();
    }

    const auto& stock = stocks_.at(index.row());
    // int col           = index.column();

    using namespace Common;

    switch(role) {
    case Roles::TradeDataRole: // 这是枚举的第一位，值为0
        return stock.tradeDate;
    case Roles::PriceRole:
        return to3Decimals(stock.price);
    case Roles::StockHoldingsRole:
        return stock.holdings;
    case Roles::BrokerageFeeRole:
        return to3Decimals(stock.brokerageFee);
    case Roles::TransferFeeRole:
        return to3Decimals(stock.transferFee);
    case Roles::CostRole:
        return to3Decimals(stock.cost);
    }

    return QVariant();
}

void StockPurchaseListModel::upateRecords(QList<QtModel::StockPurchaseModel> models) {
    beginResetModel();
    auto _  = qScopeGuard([this] { endResetModel(); });

    stocks_ = models;
    stocks_.detach();
}
