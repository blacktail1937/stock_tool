#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QtConcurrent/QtConcurrent>

#include <variant>

#include "stocktable.h"

namespace Database {
class DataLoader: public QObject
{
    Q_OBJECT
    QML_ELEMENT

    using StockTradeRecord =
        std::variant<StockTable::StockPurchaseHistory, StockTable::StockSaleHistory>;

  public:
    explicit DataLoader(QObject* parent = nullptr);
    ~DataLoader();

  signals:
    void updateSummaryTable(QList<QtModel::StockSummaryModel> models);
    // void updateSummaryTable(int row, double price);
    void updatePurchaseTable(QString code, QList<QtModel::StockPurchaseModel> models);
    void updateSaleTable(QString code, QList<QtModel::StockSaleModel> models);
    void updateStockDetail(QString code, QtModel::StockSummaryModel model);
    void updateStockBaseInfo(QList<QtModel::StockBaseInfoModel> model);
  public slots:
    void loadDataAll();
    void loadDataViaAdd(QString code,
                        int tradeType,
                        QString date,
                        double price,
                        int holdings,
                        QString tradeID);
    void loadDataViaImport(QString content);

    // stock detail window
    void loadTradeTableModel(QString code);
    void loadDetail(QString code);

  protected:
    Q_DISABLE_COPY_MOVE(DataLoader)
  private:
    // 每次插入都会排序 批量插入时效率很低
    void addRecord(const StockTradeRecord& record, bool sort = true);
    void sortRecord(const QString& code);
    // 加载沪深股票信息
    void loadAStocksBaseInfo();
    // 加载股票交易记录
    void loadStockTradeRecords();
    // 股票信息汇总 全量计算
    void buildSummary();
    std::optional<QtModel::StockSummaryModel> buildSummarySingle(QString code);
    std::optional<double> getStockPrice(QString code);

    // 定时读取
    void updateRoutine();

  private:
    // 股票信息
    QMap<QString, StockTable::AStocksBaseInfo> stockInfo_;

    // 股票汇总
    QMap<QString, QtModel::StockSummaryModel> stockSummary_;

    // 股票买卖记录
    QMap<QString, QList<StockTradeRecord>> stockTradeRecords_;
    inline static auto stockTradeRecordCompare = [](const StockTradeRecord& a,
                                                    const StockTradeRecord& b) {
        auto dateA = std::visit([](auto&& arg) { return arg.tradeDate; }, a);
        auto dateB = std::visit([](auto&& arg) { return arg.tradeDate; }, b);

        if(dateA != dateB) {
            return dateA < dateB;
        }

        auto tradeIDA = std::visit([](auto&& arg) { return arg.tradeID; }, a);
        auto tradeIDB = std::visit([](auto&& arg) { return arg.tradeID; }, b);

        return tradeIDA < tradeIDB; // 成交编号一定不同，无条件信任交易所
    };

    // routine
    QFuture<void> routine_;
};

class DataLoaderQmlBridge: public QObject
{
    Q_OBJECT
    QML_ELEMENT

  public:
    enum Type
    {
        Holdings,
        Cost,
        CurPrice,

        TotalCost,
        StockReturns,     // 持仓盈亏
        StockReturnsRate, // 持仓收益率

        BrokerageFee,
        TransferFee,

        // 卖
        SellingBrokerageFee, // 卖出券商佣金
        SellingTransferFee,  // 卖出过户费
        SecuritiesStampTax,  // 卖出印花税
        StockSaleProceeds,
        StockSaleProfits,
        StockSaleProfitsRate,
        // 保本
        BreakevenStockPrice,
        BreakevenBasedProfits,
        BreakevenBaseProfitsRate
    };
    Q_ENUM(Type)

    explicit DataLoaderQmlBridge(QObject* parent = nullptr) :
        QObject(parent) {}

    ~DataLoaderQmlBridge() = default;

    QString code() const { return code_; }

    void setCode(const QString& newCode) {
        if(code_ == newCode) {
            return;
        }
        code_ = newCode;
        emit codeChanged();
    }

  public slots:

    void setModel(QString code, QtModel::StockSummaryModel model) {
        if(code_ == code) {
            model_ = model;

            emit update();
        }
    }

    QVariant getDetail(QString code, int index) const;

  signals:
    void codeChanged();
    void update();

  protected:
    Q_DISABLE_COPY_MOVE(DataLoaderQmlBridge)
  private:
    QString code_;
    QtModel::StockSummaryModel model_;
    Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged FINAL)
};
}
