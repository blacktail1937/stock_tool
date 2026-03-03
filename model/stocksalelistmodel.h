#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QQmlEngine>

#include "database/stocktable.h"

class StockSaleListModel: public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

  public:
    enum Roles
    {
        TradeDateRole,
        PriceRole,
        StockHoldingsRole,
        BrokerageFeeRole,
        TransferFeeRole,
        SecuritiesStampTaxRole,
        ProceedsRole,

        // 功能
        ForegroundRole,
        TextAlignmentRole,
    };
    Q_ENUM(Roles)

    explicit StockSaleListModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  public slots:
    void updateRecords(QList<QtModel::StockSaleModel> models);

  private:
    QList<QtModel::StockSaleModel> stocks_;
    QStringList headers_;
};

