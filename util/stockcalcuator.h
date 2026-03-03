#pragma once

#include <cmath>
#include <QDebug>

namespace StockCalcuator {
namespace internal {
inline static double roundToCent(double v) {
    return std::round(v * 100.) / 100.;
}

inline static double ceilToCent(double v) {
    return std::ceil(v * 100.) / 100.;
}
}

// 券商佣金 万3最低5
constexpr double brokerageFeeRate = 0.0003;
// 券商最低佣金
constexpr double brokerageMinFee = 5.;
// 过户费 万0.1
constexpr double transferFeeRate = 0.00001;
// 印花税 万5
constexpr double taxRate = 0.0005;

// 券商佣金
inline static double brokerageFee(int stockHoldings, double cost) {
    auto fee = internal::roundToCent(stockHoldings * cost * brokerageFeeRate);

    return std::fmax(brokerageMinFee, fee);
}

// 过户费
inline static double transferFee(int stockHoldings, double cost) {
    auto fee = internal::roundToCent(stockHoldings * cost * transferFeeRate);
    return fee;
}

// 印花税
inline static double securitiesStampTax(int stockHoldings, double cost) {
    auto fee = internal::roundToCent(stockHoldings * cost * taxRate);
    return fee;
}

// 股票价值=股票价格x数量
inline static double stockValues(int stockHoldings, double price) {
    auto cost = internal::roundToCent(stockHoldings * price);
    return cost;
}

// 持仓总成本
inline static double stockTotalPurchaseCost(int stockHoldings,
                                            double cost,
                                            bool hasTransferFee = true) {
    double f1 = brokerageFee(stockHoldings, cost);
    double f2 = (hasTransferFee ? transferFee(stockHoldings, cost) : 0.);

    return stockValues(stockHoldings, cost) + f1 + f2;
}

// 持仓盈亏
inline static double stockReturns(int stockHoldings,
                                  double cost,
                                  double price,
                                  bool hasTransferFee = true) {
    double totalCost  = stockTotalPurchaseCost(stockHoldings, cost, hasTransferFee);
    double stockValue = stockValues(stockHoldings, price);
    return stockValue - totalCost;
}

// 卖出总收入
inline static double stockSaleProceeds(int stockHoldings,
                                       double price,
                                       bool hasTransferFee = true) {
    double f1 = brokerageFee(stockHoldings, price);
    double f2 = (hasTransferFee ? transferFee(stockHoldings, price) : 0.);
    double f3 = securitiesStampTax(stockHoldings, price);

    // #ifndef QT_NO_DEBUG
    //     // 调试模式下打印或记录，确保变量被使用
    //     qDebug() << "f1:" << f1 << "f2:" << f2 << "f3:" << f3;
    // #endif

    return stockValues(stockHoldings, price) - f1 - f2 - f3;
}

// 卖出利润 适用于单笔交易
// 多笔交易 只能算出卖出收入再减去所有买入的成本
inline static double stockSaleProfits(int stockHoldings,
                                      double cost,
                                      double price,
                                      bool hasTransferFee = true) {
    double totalCost = stockTotalPurchaseCost(stockHoldings, cost, hasTransferFee);
    double proceeds  = stockSaleProceeds(stockHoldings, price, hasTransferFee);

    return proceeds - totalCost;
}

// 单笔交易保本股价
// 股票数量 股票总成本 是否有过户费
inline static double breakevenStockPrice(int stockHoldings,
                                         double totalCost,
                                         bool hasTransferFee = true) {
#if 0
    double threshold        = brokerageMinFee / brokerageFeeRate;
    double transferFeeRate2 = (hasTransferFee ? transferFeeRate : 0.);

    // double totalCost        = stockTotalPurchaseCost(stockHoldings, perCost);

    // 1.佣金固定5元
    double brokerageFee1 = 1. - taxRate - transferFeeRate2;
    double minPrice1     = (totalCost + brokerageMinFee) / brokerageFee1;

    // 2.按比例佣金
    double brokerageFee2 = 1. - taxRate - brokerageFeeRate - transferFeeRate2;
    double minPrice2     = totalCost / brokerageFee2;

    double realMinPrice {};

    do {
        if(minPrice1 <= threshold + 1e-9) {
            realMinPrice = minPrice1;
            break;
        }

        if(minPrice2 > threshold - 1e-9) {
            realMinPrice = minPrice2;
            break;
        }

        realMinPrice = minPrice2;
    } while(0);

    return internal::ceilToCent(realMinPrice / stockHoldings);
#else
    // 1. 初始猜测：假设没有费用的成本价
    double guessPrice = totalCost / stockHoldings;

    // 2. 迭代逼近
    // 因为保本价是“卖出所得等于买入总成本”的价格
    // 且费用函数包含 max(5, rate*amount) 这种非平滑逻辑
    // 迭代法是唯一能保证边界正确的方法
    int maxloopCount = 5;
    for(int i = 0; i < maxloopCount; ++i) {
        double proceeds = stockSaleProceeds(stockHoldings, guessPrice, hasTransferFee);
        double diff     = totalCost - proceeds; // 差额
        // if(std::abs(diff) < 1e-9) {
        //     break;
        // }
        guessPrice += diff / stockHoldings;     // 调整价格
    }

    // 3. 向上取整到分
    // 保本价定义：卖出的钱 >= 总成本，所以必须向上取整
    return internal::ceilToCent(guessPrice);
#endif
}
}
