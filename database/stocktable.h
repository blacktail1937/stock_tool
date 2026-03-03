#pragma once

#include <string>

#include <QDebug>
#include <QString>

namespace StockTable {
// A股基本信息
struct AStocksBaseInfo {
    int id; // 主键自增
    std::string code;
    std::string name;
    std::string market;
    std::string border;
};

// 交易记录 单笔买入
struct StockPurchaseHistory {
    int id { -1 };         // 主键自增

    std::string tradeDate; // 成交日期 YYYYMMDD
    int tradeType { 0 };   // 买 0 卖1

    std::string code;
    std::string name;
    std::string market;
    std::string border;

    double price;        // 成本价
    int holdings;        // 持仓数量

    double brokerageFee; // 券商佣金
    double transferFee;  // 过户费
    double cost;         // 总成本 (买入成本=股票成本+手续费)

    std::string tradeID; // 成交编号 证券交易所生成 唯一 在当天成交的编号一定有大小顺序

    friend QDebug operator<< (QDebug debug, const StockPurchaseHistory& record) {
        QDebugStateSaver s(debug);

        debug << QString("id:%1, 时间:%2, 操作:%3, 代码:%4, 名称:%5, 市场:%6, 板块:%7, 价格:%8, "
                         "数量:%9, 佣金:%10, 过户费:%11, 成本:%12, 成交编号:%13")
                     .arg(record.id)
                     .arg(record.tradeDate)
                     .arg(record.tradeType)
                     .arg(record.code)
                     .arg(record.name)
                     .arg(record.market)
                     .arg(record.border)
                     .arg(record.price)
                     .arg(record.holdings)
                     .arg(record.brokerageFee)
                     .arg(record.transferFee)
                     .arg(record.cost)
                     .arg(record.tradeID);

        return debug;
    }
};

// 交易记录 单笔卖出
struct StockSaleHistory {
    int id { -1 };         // 主键自增

    std::string tradeDate; // 成交日期 YYYYMMDD
    int tradeType { 1 };   // 买 0 卖1

    std::string code;
    std::string name;
    std::string market;
    std::string border;

    double cost;              // 成本价
    double price;             // 成交价
    int holdings;             // 持仓数量

    double brokerageFee;      // 券商佣金
    double transferFee;       // 过户费
    double securitiesStampTax; // 印花税 卖出才有
    double proceeds;           // 总收入 (卖出收入=股票收入-手续费)

    double profits {};         // 卖出后利润
    double profitsRate {};     // 卖出收益率

    std::string tradeID;       // 成交编号 证券交易所生成 唯一 在当天成交的编号一定有大小顺序

    friend QDebug operator<< (QDebug debug, const StockSaleHistory& record) {
        QDebugStateSaver s(debug);

        debug << QString(
                     "id:%1, 时间:%2, 操作:%3, 代码:%4, 名称:%5, 市场:%6, 板块:%7, 成本:%14, "
                     "成交价:%8, "
                     "数量:%9, 佣金:%10, 过户费:%11, 收入:%12, 利润:%15, 回报率:%16, 成交编号:%13")
                     .arg(record.id)
                     .arg(record.tradeDate)
                     .arg(record.tradeType)
                     .arg(record.code)
                     .arg(record.name)
                     .arg(record.market)
                     .arg(record.border)
                     .arg(record.price)
                     .arg(record.holdings)
                     .arg(record.brokerageFee)
                     .arg(record.transferFee)
                     .arg(record.proceeds)
                     .arg(record.tradeID)
                     .arg(record.cost)
                     .arg(record.profits)
                     .arg(record.profitsRate);

        return debug;
    }
};
}

#include <QString>

namespace QtModel {
// 股票信息汇总表model
struct StockSummaryModel {
    QString market;             // 市场 沪A 深A
    QString code;               // 股票代码
    QString name;               // 股票名

    int holdings {};            // 持股总数
    double purchaseCost {};     // 持股成本
    double currentStockPrice {}; // 当前股价
    double brokerageFee {};      // 累计券商佣金
    double transferFee {};       // 累计过户费
    // double stockCost {};         // 股票成本
    double totalCost {};        // 实际成本(持股成本+手续费)
    double stockReturns {};     // 持仓盈亏
    double stockReturnsRate {}; // 持仓收益率
    double sellingBrokerageFee; // 卖出券商佣金
    double sellingTransferFee;  // 卖出过户费
    double securitiesStampTax;  // 卖出印花税
    double stockSaleProceeds {};    // 卖出后收入(收入-手续费)
    double stockSaleProfits {};     // 卖出后利润
    double stockSaleProfitsRate {}; // 卖出收益率
    double breakevenStockPrice {};  // 保本股价
    double
        breakevenBasedProfits {}; // 保本利润>=0，因为股价最小波动单位是分，所以可能出现盈利几块/毛/分钱的情况
    double breakevenBaseProfitsRate {}; // 回报率
};

// 股票买入信息
struct StockPurchaseModel {
    int id;             // 数据库唯一标识

    QString tradeDate;  // 交易时间 YYYYMMDD

    double price;       // 成交价
    int holdings;       // 数量
    double brokerageFee; // 券商佣金
    double transferFee;  // 过户费
    double cost;         // 股票总成本

    void from(const StockTable::StockPurchaseHistory& stock) {
        id           = stock.id;
        price        = stock.price;
        holdings     = stock.holdings;
        brokerageFee = stock.brokerageFee;
        transferFee  = stock.transferFee;
        cost         = stock.cost;
        tradeDate    = QString::fromStdString(stock.tradeDate);
    }
};

// 股票卖出信息
struct StockSaleModel {
    int id;                   // 数据库唯一标识

    QString tradeDate;        // 交易时间 YYYYMMDD

    double price;             // 成交价
    int holdings;             // 数量
    double brokerageFee;      // 券商佣金
    double transferFee;       // 过户费
    double securitiesStampTax; // 印花税
    double proceeds;           // 股票总收入

    double profits {};         // 卖出后利润
    double profitsRate {};     // 卖出收益率

    void from(const StockTable::StockSaleHistory& stock) {
        id                 = stock.id;

        tradeDate          = QString::fromStdString(stock.tradeDate);

        price              = stock.price;
        holdings           = stock.holdings;
        brokerageFee       = stock.brokerageFee;
        transferFee        = stock.transferFee;
        securitiesStampTax = stock.securitiesStampTax;
        proceeds           = stock.proceeds;

        profits            = stock.profits;
        profitsRate        = stock.profitsRate;
    }
};

// 股票基本信息
struct StockBaseInfoModel {
    QString code;
    QString name;
};
}
