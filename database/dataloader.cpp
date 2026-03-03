#include "python/pythonwrapper.h"

#include "databasemanager.h"
#include "dataloader.h"
#include "util/common.h"
#include "util/stockcalcuator.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <random>

// #include <QtConcurrent/QtConcurrent>

using namespace Database;
using namespace std::chrono;

struct Elapsed {
    Elapsed() { start_ = steady_clock::now(); }

    ~Elapsed() {
        auto now = steady_clock::now();
        qDebug() << "elapsed:" << duration_cast<milliseconds>(now - start_).count();
    }

  private:
    time_point<steady_clock> start_;
};

inline static std::atomic_bool running = true;

DataLoader::DataLoader(QObject* parent) :
    QObject(parent) {
    ::initialize();

    QString scriptPath = QString("%1/%2").arg(QDir::currentPath(), "scripts");
    scriptPath         = QDir::cleanPath(scriptPath);
    qDebug() << "script path: " << scriptPath;

    // scriptPath = "./scripts";

    Python::initialize(scriptPath);
    Database::initialize();
}

DataLoader::~DataLoader() {
    running = false;
    if(routine_.isRunning()) {
        routine_.cancel();
    }
}

void DataLoader::loadDataAll() {
    auto _ = QtConcurrent::run([this] {
        Elapsed e;
        // 加载股票基本信息 代码 名称 市场
        loadAStocksBaseInfo();

        // 加载买卖信息
        loadStockTradeRecords();

        // 股票汇总信息计算
        buildSummary();

        // 开启定时更新
        updateRoutine();
    });
}

void DataLoader::loadDataViaAdd(QString code,
                                int tradeType,
                                QString date,
                                double price,
                                int holdings,
                                QString tradeID) {
    auto it = stockInfo_.find(code);
    if(it == stockInfo_.end()) {
        qWarning() << "添加失败，股票" << code << "不存在";
        return;
    }

    const auto& info = it.value();

    StockTradeRecord record;

    if(tradeType == 0) {
        StockTable::StockPurchaseHistory stock;
        stock.code         = info.code;
        stock.name         = info.name;
        stock.market       = info.market;
        stock.border       = info.border;

        stock.tradeDate    = date.toStdString();
        stock.tradeID      = tradeID.toStdString();

        stock.price        = price;
        stock.holdings     = holdings;

        stock.brokerageFee = StockCalcuator::brokerageFee(holdings, price);
        stock.transferFee  = StockCalcuator::transferFee(holdings, price);
        stock.cost = StockCalcuator::stockTotalPurchaseCost(holdings, price, stock.market == "沪A");

        try {
            stock.id = ::insertRecord<StockTable::StockPurchaseHistory>(stock);
            qDebug() << "插入数据库记录id:" << stock.id;
        }
        catch(std::exception& e) {
            qWarning() << "插入卖出记录[" << stock << "]失败:" << e.what();
            return;
        }

        addRecord(stock);

        qDebug() << "添加股票=>" << stock;
    }
    else {
        StockTable::StockSaleHistory stock;
        stock.code   = info.code;
        stock.name   = info.name;
        stock.market = info.market;
        stock.border = info.border;

        // TODO

        try {
            stock.id = ::insertRecord<StockTable::StockSaleHistory>(stock);
            qDebug() << "插入数据库记录id:" << stock.id;
        }
        catch(std::exception& e) {
            qWarning() << "插入买入记录[" << stock << "]失败:" << e.what();
            return;
        }

        addRecord(stock);

        qDebug() << "添加股票=>" << stock;
    }
}

void DataLoader::loadDataViaImport(QString content) {
    auto _ = QtConcurrent::run([this, content] {
        Elapsed e;

        auto list1 = content.split('\n');

        if(list1.empty()) {
            return;
        }

        QMap<QString, int> indexRecord;
        // 成交日期 0 0
        QString tradeDate("成交日期");
        // 成交时间 1 1
        QString tradeTime("成交时间");
        // 证券代码 2 2
        QString code("证券代码");
        // 证券名称 3 3
        QString name("证券名称");
        // 操作 4 4
        QString tradeType("操作");
        // 成交数量 5 5
        QString holdings("成交数量");
        // 成交均价 6 6
        QString price("成交均价");
        // 成交编号 7 10
        QString tradeID("成交编号");
        // 手续费 8 11
        QString brokerageFee("手续费");
        // 印花税 9 12
        QString stampTax("印花税");
        // 发生金额 10 14
        QString totalCost("发生金额");
        // 交易市场 11 16
        QString market("交易市场");
        // 过户费 12 17
        QString transferFee("过户费");

        QString stockCode;

        for(int i = 0; i < list1.size(); i++) {
            auto data = list1[i];
            if(i == 0) { // 处理标题行
                auto titles = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                if(titles.empty()) {
                    qWarning() << "导入失败，标题行无法分割";
                    break;
                }

                for(int j = 0; j < titles.size(); j++) {
                    const auto& s = titles[j];
                    if(s == tradeDate || s == tradeTime || s == code || s == name ||
                       s == tradeType || s == holdings || s == price || s == brokerageFee ||
                       s == stampTax || s == transferFee || s == totalCost || s == market ||
                       s == tradeID) {
                        indexRecord.insert(s, j);
                    }
                }
            }
            else {
                auto stockData = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if(stockData.empty()) {
                    qWarning() << "无法分割第" << i << "条股票信息";
                    continue;
                }

                auto toStdString = [](const QString& s) -> std::string {
                    return s.toUtf8().toStdString();
                };

                int index    = indexRecord[tradeType];
                QString type = stockData[index];
                if(type == "买入") {
                    StockTable::StockPurchaseHistory stock;

                    stock.tradeID = toStdString(stockData[indexRecord[tradeID]]);

                    stock.tradeDate =
                        toStdString(QString("%1 %2").arg(stockData[indexRecord[tradeDate]],
                                                         stockData[indexRecord[tradeTime]]));
                    stock.code = toStdString(stockData[indexRecord[code]]);
                    stock.name = toStdString(stockData[indexRecord[name]]);
                    stock.market =
                        toStdString(stockData[indexRecord[market]] == "1" ? "深A" : "沪A");
                    stock.border       = toStdString("主板");

                    stock.price        = stockData[indexRecord[price]].toDouble();
                    stock.holdings     = stockData[indexRecord[holdings]].toDouble();

                    stock.brokerageFee = stockData[indexRecord[brokerageFee]].toDouble();
                    stock.transferFee  = stockData[indexRecord[transferFee]].toDouble();
                    stock.cost         = std::abs(stockData[indexRecord[totalCost]].toDouble());

                    try {
                        stock.id = ::insertRecord<StockTable::StockPurchaseHistory>(stock);
                        qDebug() << "插入数据库记录id:" << stock.id;
                    }
                    catch(std::exception& e) {
                        qWarning() << "插入第" << i << "条记录[" << stock << "]失败:" << e.what();
                        continue;
                    }

                    addRecord(stock);

                    // if(stockCode.length() <= 0) {
                    //     stockCode = QString::fromStdString(stock.code);
                    // }
                    qDebug() << "批量导入=>" << stock;
                }
                else if(type == "卖出") {
                    StockTable::StockSaleHistory stock;

                    // 暂时未知卖出信息 TODO
                }
                else {
                    qWarning() << "未知操作类型:" << type << ", 发生在导入第" << i << "条股票信息";
                    continue;
                }
            }
        }

        // sortRecord(stockCode);

        // for(auto&& [_, v2]: stockTradeRecords_.asKeyValueRange()) {
        //     for(auto& s: v2) {
        //         auto tradeType2 = std::visit([](auto&& arg) { return arg.tradeType; }, s);
        //         if(tradeType2 == 0) {
        //             const auto& stock = std::get<StockTable::StockPurchaseHistory>(s);
        //             qDebug() << stock.tradeID;
        //         }
        //         else {
        //             const auto& stock = std::get<StockTable::StockSaleHistory>(s);
        //             qDebug() << stock.tradeID;
        //         }
        //     }
        // }

        // qDebug() << "--------------------------------------------------";

        // for(auto&& [_, v2]: stockTradeRecords_.asKeyValueRange()) {
        //     for(auto& s: v2) {
        //         if(auto* stock = std::get_if<StockTable::StockPurchaseHistory>(&s)) {
        //             qDebug() << stock->tradeID;
        //         }
        //         else if(auto* stock2 = std::get_if<StockTable::StockSaleHistory>(&s)) {
        //             qDebug() << stock2->tradeID;
        //         }
        //     }
        // }

        qDebug() << "导入数据成功!";
    });
}

void DataLoader::loadTradeTableModel(QString code) {
    if(!stockTradeRecords_.contains(code)) {
        qWarning() << "请求的股票[" << code << "]不存在记录";
        return;
    }

    QList<QtModel::StockPurchaseModel> purchaseModels;
    QList<QtModel::StockSaleModel> saleModels;

    auto& v = stockTradeRecords_[code];
    for(auto& s: v) {
        // 买
        if(auto* stock = std::get_if<StockTable::StockPurchaseHistory>(&s)) {
            QtModel::StockPurchaseModel model;
            model.from(*stock);
            purchaseModels.append(model);
        }
        else if(auto* stock2 = std::get_if<StockTable::StockSaleHistory>(&s)) {
            QtModel::StockSaleModel model;
            model.from(*stock2);
            saleModels.append(model);
        }
    }

    if(!purchaseModels.empty()) {
        emit updatePurchaseTable(code, purchaseModels);
    }
    if(!saleModels.empty()) {
        emit updateSaleTable(code, saleModels);
    }
}

void DataLoader::loadDetail(QString code) {
    if(!stockSummary_.contains(code)) {
        qWarning() << "加载细节信息，请求的股票[" << code << "]不存在记录";
        return;
    }

    emit updateStockDetail(code, stockSummary_[code]);
}

void DataLoader::addRecord(const StockTradeRecord& record, bool sort) {
    auto code = QString::fromStdString(
        std::visit([](auto&& arg) -> std::string { return arg.code; }, record));

    if(stockTradeRecords_.contains(code)) {
        auto& v = stockTradeRecords_[code];

        if(sort) {
            auto it = std::ranges::upper_bound(v, record, stockTradeRecordCompare);

            v.insert(it, record);
        }
        else {
            v.push_back(record);
        }
    }
    else {
        stockTradeRecords_.insert(code, { record });
    }
}

void DataLoader::sortRecord(const QString& code) {
    if(!stockTradeRecords_.contains(code)) {
        qWarning() << "排序代码" << code << "无记录";
        return;
    }

    auto& v = stockTradeRecords_[code];
    std::ranges::sort(v, stockTradeRecordCompare);
}

void DataLoader::loadAStocksBaseInfo() {
    auto _ = QtConcurrent::run([this] {
        try {
            auto count = ::countRecords<StockTable::AStocksBaseInfo>();
            if(count && count.value() <= 0) {
                qDebug() << "没有找到股票基本数据，开始读取...";
                using stockVector = std::vector<std::unordered_map<std::string, std::string>>;

                auto result = Python::call<stockVector>("akshare_stock", "get_stock_list_akshare");
                if(result) {
                    std::vector<StockTable::AStocksBaseInfo> s1;

                    for(auto&& stock: result.value()) {
                        auto code   = stock.at("code");
                        auto name   = stock.at("name");
                        auto market = stock.at("market");
                        auto border = stock.at("border");

                        s1.push_back(
                            StockTable::AStocksBaseInfo { -1, code, name, market, border });
                    }

                    qDebug() << "共读取了" << s1.size() << "股票信息";
                    ::insertRecords(s1);
                }
            }

            auto stocks = ::getRecords<StockTable::AStocksBaseInfo>();

            if(!stocks) {
                qDebug() << "数据库加载=>数据库没有股票基本记录";
                // 自动刷新 TODO
                return;
            }

            QList<QtModel::StockBaseInfoModel> models;

            std::ranges::for_each(stocks.value(), [this, &models](auto&& stock) {
                stockInfo_.insert(QString::fromStdString(stock.code), stock);
                models.append(
                    { QString::fromStdString(stock.code), QString::fromStdString(stock.name) });
            });

            emit updateStockBaseInfo(models);

            qDebug() << "数据库加载=>共加载了" << stockInfo_.size() << "条股票基本信息";
        }
        catch(std::exception& e) {
            qWarning() << "数据库加载=>加载股票基础信息失败:" << e.what();
        }
    });
    _.waitForFinished();
}

void DataLoader::loadStockTradeRecords() {
    try {
        auto result = ::getRecords<StockTable::StockPurchaseHistory>();
        if(!result && result.value().empty()) {
            qDebug() << "数据库加载=>数据库暂时没有买入记录";
            return;
        }

        auto&& stocks = result.value();
        std::ranges::for_each(stocks, [this](auto&& stock) {
            addRecord(stock);
            qDebug() << "数据库加载=>" << stock;
        });
        qDebug() << "数据库加载=>共加载了" << stocks.size() << "条买入记录";
    }
    catch(std::exception& e) {
        qWarning() << "数据库加载=>加载股票买入信息失败:" << e.what();
    }

    try {
        auto result = ::getRecords<StockTable::StockSaleHistory>();
        if(!result && result.value().empty()) {
            qDebug() << "数据库加载=>数据库暂时没有卖出记录";
            return;
        }

        auto&& stocks = result.value();
        std::ranges::for_each(stocks, [this](auto&& stock) {
            addRecord(stock);
            qDebug() << "数据库加载=>" << stock;
        });
        qDebug() << "数据库加载=>共加载了" << stocks.size() << "条卖出记录";
    }
    catch(std::exception& e) {
        qWarning() << "数据库加载=>加载股票卖出信息失败:" << e.what();
    }
}

void DataLoader::buildSummary() {
    Elapsed e;
    QList<QtModel::StockSummaryModel> models;

    for(auto&& [k, v]: stockTradeRecords_.asKeyValueRange()) {
        auto model = buildSummarySingle(k);
        if(model) {
            if(stockSummary_.contains(k)) {
                stockSummary_[k] = model.value();
            }
            else {
                stockSummary_.insert(k, model.value());
            }

            emit updateStockDetail(k, model.value());
            models.append(model.value());
        }
    }

    emit updateSummaryTable(models);
}

std::optional<QtModel::StockSummaryModel> DataLoader::buildSummarySingle(QString code) {
    auto it = stockTradeRecords_.find(code);
    if(it == stockTradeRecords_.end()) {
        qWarning() << "无" << code << "买卖记录";
        return {};
    }

    QtModel::StockSummaryModel model;

    // try {
    //     using cppMap = std::unordered_map<std::string, std::string>;

    //     auto result =
    //         Python::call<cppMap>("akshare_stock", "get_stock_price_xq",
    //         code.toUtf8().constData());
    //     if(result) {
    //         model.currentStockPrice = QString::fromStdString(result.value()["现价"]).toDouble();
    //     }
    // }
    // catch(std::exception& e) {
    //     model.currentStockPrice = 0.;
    //     qWarning() << "获取" << code << "股票当前价格失败:" << e.what();
    // }
    auto p = getStockPrice(code);
    if(p) {
        model.currentStockPrice = p.value();

#if 0
        std::random_device rd;
        std::mt19937_64 engine(rd());

        double upper  = 1.;
        double bottom = 0.5;
        std::uniform_real_distribution dist(bottom, upper);

        std::uniform_int_distribution dist2(0, 2);
        if(auto c = dist2(engine); c == 1) {
            model.currentStockPrice -= dist(engine);
        }
        else {
            model.currentStockPrice += dist(engine);
        }
#endif
    }
    else {
        model.currentStockPrice = 0.;
        qWarning() << "获取" << code << "股票当前价格失败";
    }

    const auto& v = it.value();
    for(auto& s: v) {
        if(auto* purchase = std::get_if<StockTable::StockPurchaseHistory>(&s)) {
            if(model.market.length() <= 0) {
                model.market = QString::fromStdString(purchase->market);
            }
            if(model.code.length() <= 0) {
                model.code = QString::fromStdString(purchase->code);
            }
            if(model.name.length() <= 0) {
                model.name = QString::fromStdString(purchase->name);
            }

            model.holdings += purchase->holdings;
            model.totalCost += purchase->cost;

            // 手续费 累计
            model.brokerageFee += purchase->brokerageFee;
            model.transferFee += purchase->transferFee;
        }
        else if(auto* sale = std::get_if<StockTable::StockSaleHistory>(&s)) {
            if(model.market.length() <= 0) {
                model.market = QString::fromStdString(sale->market);
            }
            if(model.code.length() <= 0) {
                model.code = QString::fromStdString(sale->code);
            }
            if(model.name.length() <= 0) {
                model.name = QString::fromStdString(sale->name);
            }

            model.holdings += sale->holdings;
            model.totalCost += sale->cost;
        }
    }

    // 均价
    model.purchaseCost = model.totalCost / model.holdings;

    if(model.currentStockPrice > std::numeric_limits<double>::epsilon()) {
        bool hasTransferFee = model.market == "沪A";
        // 持仓收益
        model.stockReturns     = StockCalcuator::stockReturns(model.holdings,
                                                          model.purchaseCost,
                                                          model.currentStockPrice,
                                                          hasTransferFee);
        model.stockReturnsRate = model.stockReturns / model.totalCost * 100.;

        // 卖出利润
        model.stockSaleProceeds    = StockCalcuator::stockSaleProceeds(model.holdings,
                                                                    model.currentStockPrice,
                                                                    hasTransferFee);
        model.stockSaleProfits     = StockCalcuator::stockSaleProfits(model.holdings,
                                                                  model.purchaseCost,
                                                                  model.currentStockPrice,
                                                                  hasTransferFee);
        model.stockSaleProfitsRate = model.stockSaleProfits / model.totalCost * 100.;

        // 手续费
        model.sellingBrokerageFee =
            StockCalcuator::brokerageFee(model.holdings, model.currentStockPrice);
        model.sellingTransferFee =
            StockCalcuator::transferFee(model.holdings, model.currentStockPrice);
        model.securitiesStampTax =
            StockCalcuator::securitiesStampTax(model.holdings, model.currentStockPrice);

        // 保本
        model.breakevenStockPrice =
            StockCalcuator::breakevenStockPrice(model.holdings, model.totalCost, hasTransferFee);
        model.breakevenBasedProfits    = StockCalcuator::stockSaleProfits(model.holdings,
                                                                       model.purchaseCost,
                                                                       model.breakevenStockPrice,
                                                                       hasTransferFee);
        model.breakevenBaseProfitsRate = model.breakevenBasedProfits / model.totalCost * 100.;
    }

    return model;
}

std::optional<double> DataLoader::getStockPrice(QString code) {
    try {
        using cppMap = std::unordered_map<std::string, std::string>;

        auto result =
            Python::call<cppMap>("akshare_stock", "get_stock_price_xq", code.toUtf8().constData());
        if(result) {
            return QString::fromStdString(result.value()["现价"]).toDouble();
        }
    }
    catch(std::exception& e) {
        qWarning() << "获取" << code << "股票当前价格失败:" << e.what();
    }

    return {};
}

void DataLoader::updateRoutine() {
    routine_ = QtConcurrent::run([this] {
        while(running) {
            if(!Common::isMarketOpen()) {
                return;
            }

            buildSummary();
            QThread::sleep(100ms);
        }
    });
}

QVariant DataLoaderQmlBridge::getDetail(QString code, int index) const {
    using namespace Common;

    if(code_ == code) {
        switch(index) {
        case Holdings:
            return model_.holdings;
        case Cost:
            return to3Decimals(model_.purchaseCost);
        case CurPrice:
            return to3Decimals(model_.currentStockPrice);
        case TotalCost:
            return to3Decimals(model_.totalCost);
        case StockReturns:
            return to3Decimals(model_.stockReturns);
        case StockReturnsRate:
            return to3Decimals(model_.stockReturnsRate);
        case BrokerageFee:
            return to3Decimals(model_.brokerageFee);
        case TransferFee:
            return to3Decimals(model_.transferFee);
        case SellingBrokerageFee:
            return to3Decimals(model_.sellingBrokerageFee);
        case SellingTransferFee:
            return to3Decimals(model_.sellingTransferFee);
        case SecuritiesStampTax:
            return to3Decimals(model_.securitiesStampTax);
        case StockSaleProceeds:
            return to3Decimals(model_.stockSaleProceeds);
        case StockSaleProfits:
            return to3Decimals(model_.stockSaleProfits);
        case StockSaleProfitsRate:
            return to3Decimals(model_.stockSaleProfitsRate);
        case BreakevenStockPrice:
            return to3Decimals(model_.breakevenStockPrice);
        case BreakevenBasedProfits:
            return to3Decimals(model_.breakevenBasedProfits);
        case BreakevenBaseProfitsRate:
            return to3Decimals(model_.breakevenBaseProfitsRate);
        }
    }

    return {};
}
