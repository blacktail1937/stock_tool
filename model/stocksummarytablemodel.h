#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QQmlEngine>

#include "database/stocktable.h"

class StockSummaryTableModel: public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(double totalCost READ totalCost NOTIFY totalCostChanged)                  // 成本
    Q_PROPERTY(
        double totalStockReturns READ totalStockReturns NOTIFY totalStockReturnsChanged) // 盈亏
    Q_PROPERTY(double totalSellingProfits READ totalSellingProfits NOTIFY
                   totalSellingProfitsChanged)                                           // 卖出盈亏
  public:
    enum Roles
    {
        MarketRole = Qt::UserRole + 1,
        CodeRole,
        NameRole,
        StockHoldingsRole,
        PurchaseRole,
        StockPriceRole,
        TotalCostRole,
        StockReturnsRole,
        StockReturnsRateRole,
        StockSaleProceedsRole,
        StockSaleProfitsRole,
        StockSaleProfitsRateRole,
        // BreakevenStockPriceRole,
        // BreakEvenBaseProfitsRole,

        // 功能
        // ForegroundRole,
        // TextAlignmentRole,
    };
    Q_ENUM(Roles)

    explicit StockSummaryTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row,
                      int column,
                      const QModelIndex& parent = QModelIndex()) const override;

    // 提供表头数据
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE double totalCost() const {
        double sum = 0.;

        for(auto&& s: stocks_) {
            sum += s.totalCost;
        }

        return sum;
    }

    Q_INVOKABLE double totalStockReturns() const {
        double sum = 0.;

        for(auto&& s: stocks_) {
            sum += s.stockReturns;
        }

        return sum;
    }

    Q_INVOKABLE double totalSellingProfits() const {
        double sum = 0.;

        for(auto&& s: stocks_) {
            sum += s.stockSaleProfits;
        }

        return sum;
    }

  signals:
    void totalCostChanged();
    void totalStockReturnsChanged();
    void totalSellingProfitsChanged();
  public slots:
    void load(QList<QtModel::StockSummaryModel> model);
    // void load(int row, double price);

  private:
    QList<QtModel::StockSummaryModel> stocks_;
    QStringList headers_;
};

