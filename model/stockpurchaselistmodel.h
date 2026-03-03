#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QQmlEngine>

#include "database/stocktable.h"

class StockPurchaseListModel: public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

  public:
    enum Roles
    {
        TradeDataRole,
        PriceRole,
        StockHoldingsRole,
        BrokerageFeeRole,
        TransferFeeRole,
        CostRole,

        // 功能
        ForegroundRole,
        TextAlignmentRole,
    };
    Q_ENUM(Roles)

    explicit StockPurchaseListModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  public slots:
    void upateRecords(QList<QtModel::StockPurchaseModel> models);

  private:
    QList<QtModel::StockPurchaseModel> stocks_;
    QStringList headers_;
};

