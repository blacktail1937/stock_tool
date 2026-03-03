#include "stocksummarytablemodel.h"
#include <QColor>

#include "util/common.h"

StockSummaryTableModel::StockSummaryTableModel(QObject* parent) :
    QAbstractTableModel { parent } {
    headers_.append("市场");
    headers_.append("代码");
    headers_.append("名称");
    headers_.append("持股数");
    headers_.append("持仓成本");
    headers_.append("当前股价");
    headers_.append("实际总成本");
    headers_.append("持仓盈亏");
    headers_.append("持仓收益率");
    headers_.append("卖出实际收入");
    headers_.append("卖出收益");
    headers_.append("卖出收益率");
    // headers_.append("保本股价");
    // headers_.append("保本利润");

    // double perCost             = 5.42;
    // double curPrice            = 5.28;
    // int stockHoldings          = 1000;
    // double totalCost           = StockCalcuator::stockTotalPurchaseCost(stockHoldings, perCost);
    // double stockReturns        = StockCalcuator::stockReturns(stockHoldings, perCost, curPrice);
    // double saleProceeds        = StockCalcuator::stockSaleProceeds(stockHoldings, curPrice);
    // // double breakevenStockPrice = StockCalcuator::breakevenStockPrice(stockHoldings,
    // totalCost);
    // // double saleProfits         = StockCalcuator::stockSaleProfits(stockHoldings, perCost,
    // // curPrice);

    // stocks_.append({ "沪A",
    //                  "601988",
    //                  "中国银行",
    //                  stockHoldings,
    //                  perCost,
    //                  curPrice,
    //                  totalCost,
    //                  stockReturns,
    //                  stockReturns / totalCost * 100.,
    //                  saleProceeds,
    //                  saleProceeds - totalCost,
    //                  (saleProceeds - totalCost) / totalCost * 100. });

    // stocks_.append({ "沪A",
    //                  "000776",
    //                  "广发证券",
    //                  stockHoldings,
    //                  perCost,
    //                  curPrice,
    //                  totalCost,
    //                  stockReturns,
    //                  stockReturns / totalCost * 100.,
    //                  saleProceeds,
    //                  saleProceeds - totalCost,
    //                  (saleProceeds - totalCost) / totalCost * 100. });
}

int StockSummaryTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : stocks_.size();
}

int StockSummaryTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : headers_.size();
}

QVariant StockSummaryTableModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid() || index.row() >= stocks_.size()) {
        return {};
    }

    const auto& stock = stocks_.at(index.row());
    int column        = index.column();

    using namespace Common;

    if(role == Qt::DisplayRole) {
        switch(column) {
        case modelRoleConvertor(Roles::MarketRole):
            return stock.market;
        case modelRoleConvertor(Roles::CodeRole):
            return stock.code;
        case modelRoleConvertor(Roles::NameRole):
            return stock.name;
        case modelRoleConvertor(Roles::StockPriceRole):
            return to3Decimals(stock.currentStockPrice);
        case modelRoleConvertor(Roles::StockHoldingsRole):
            return stock.holdings;
        case modelRoleConvertor(Roles::PurchaseRole):
            return to3Decimals(stock.purchaseCost);
        case modelRoleConvertor(Roles::TotalCostRole):
            return to3Decimals(stock.totalCost);
        case modelRoleConvertor(Roles::StockReturnsRole):
            return to3Decimals(stock.stockReturns);
        case modelRoleConvertor(Roles::StockReturnsRateRole):
            return to3Decimals(stock.stockReturnsRate) + "%";
        case modelRoleConvertor(Roles::StockSaleProceedsRole):
            return to3Decimals(stock.stockSaleProceeds);
        case modelRoleConvertor(Roles::StockSaleProfitsRole):
            return to3Decimals(stock.stockSaleProfits);
        case modelRoleConvertor(Roles::StockSaleProfitsRateRole):
            return to3Decimals(stock.stockSaleProfitsRate) + "%";
        }
    }
    else {
        switch(role) {
        case Qt::ForegroundRole: {
            // qDebug() << "row:" << index.row() << ",column:" << column;
            auto&& green = QColor("green");
            auto&& red   = QColor("red");
            if(column == modelRoleConvertor(Roles::StockReturnsRole) ||
               column == modelRoleConvertor(Roles::StockReturnsRateRole)) {
                return stock.stockReturns < 0 ? green : red;
            }
            else if(column == modelRoleConvertor(Roles::StockSaleProfitsRole) ||
                    column == modelRoleConvertor(Roles::StockSaleProfitsRateRole)) {
                return stock.stockSaleProfits < 0 ? green : red;
            }
            else if(column == modelRoleConvertor(Roles::StockPriceRole)) {
                return QColor("purple");
            }
            // else if(column == Roles::BreakevenStockPriceRole) {
            //     return QColor(Qt::cyan);
            // }
            // else if(column == Roles::BreakEvenBaseProfitsRole) {
            //     return red;
            // }

            return QColor(0x333333);
        }
        case Qt::TextAlignmentRole: {
            if(column > modelRoleConvertor(Roles::NameRole)) {
                return QVariant { Qt::AlignRight | Qt::AlignVCenter };
            }
            else {
                return QVariant { Qt::AlignCenter };
            }
        }
            // 自定义
        case Roles::CodeRole:
            return stock.code;
        case Roles::NameRole:
            return stock.name;
        }
    }

    return {};
}

QModelIndex StockSummaryTableModel::index(int row, int column, const QModelIndex& parent) const {
    if(parent.isValid() || row < 0 || row >= stocks_.size() || column < 0 ||
       column >= headers_.size()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QVariant StockSummaryTableModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if(section >= 0 && section < headers_.size()) {
            return headers_[section];
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QHash<int, QByteArray> StockSummaryTableModel::roleNames() const {
    return {
        {               Roles::MarketRole,               "market" },
        {                 Roles::CodeRole,                 "code" },
        {                 Roles::NameRole,                 "name" },
        {        Roles::StockHoldingsRole,        "stockHoldings" },
        {             Roles::PurchaseRole,         "purchaseCost" },
        {           Roles::StockPriceRole,    "currentStockPrice" },
        {            Roles::TotalCostRole,            "totalCost" },
        {         Roles::StockReturnsRole,         "stockReturns" },
        {     Roles::StockReturnsRateRole,     "stockReturnsRate" },
        {    Roles::StockSaleProceedsRole,    "stockSaleProceeds" },
        {     Roles::StockSaleProfitsRole,     "stockSaleProfits" },
        { Roles::StockSaleProfitsRateRole, "stockSaleProfitsRate" },
        {              Qt::ForegroundRole,           "foreground" },
        {           Qt::TextAlignmentRole,        "textAlignment" },
        {                 Qt::DisplayRole,              "display" }
    };
}

void StockSummaryTableModel::load(QList<QtModel::StockSummaryModel> model) {
    // 1. 如果数量发生了变化（比如新增了股票），必须重置一次布局
    if(stocks_.size() != model.size()) {
        beginResetModel();
        stocks_ = model;
        // stocks_.detach(); // 如果是传值进来的，这句通常不需要，但求稳可以加上
        endResetModel();
    }
    else {
        // 2. 如果数量没变，只是数值变了，使用你写的“轻量刷新”
        for(int i = 0; i < model.size(); ++i) {
            // 这里可以加一个简单的判断，如果数据真的变了再赋值和发信号
            // if (stocks_[i] != model[i]) {
            stocks_[i]              = model[i];

            QModelIndex topLeft     = index(i, 0);
            QModelIndex bottomRight = index(i, headers_.size() - 1);

            // 关键：通知 View 只重画这一行
            // qDebug() << i << "行更新:" << stocks_[i].currentStockPrice;
            emit dataChanged(topLeft, bottomRight);
            // }
        }
    }

    emit totalCostChanged();
    emit totalStockReturnsChanged();
    emit totalSellingProfitsChanged();
}

// void StockSummaryTableModel::load(int row, double price) {
//     if(row < 0 || row > stocks_.size()) {
//         return;
//     }

//     stocks_[row].currentStockPrice = price;

//     QModelIndex idx = index(row, 0);
//     emit dataChanged(idx, index(row, columnCount() - 1));
// }
