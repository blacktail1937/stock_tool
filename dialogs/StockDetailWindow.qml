// StockDetailWindow.qml
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import stock_tool

Window {
    id: root
    title: stockName + " - " + stockCode
    width: 900
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowCloseButtonHint

    //唤起添加界面
    signal callAddDialog
    //请求买卖记录
    signal requestRecord(string code)

    // ========== 从 C++ 更新的数据接口 ==========
    // 股票基本信息
    property string stockCode
    property string stockName
    //成本
    property double currentCost
    //现价
    property double currentPrice
    //持仓数量
    property int stockHoldings

    // 保本信息
    property double breakevenPrice: 0 //价格
    property double breakevenProfit: 0 //利润
    property double breakevenProfitRate: 0 //利润率

    // 汇总数据
    property double totalCost: 0 // 成本

    property double totalCommission: 0 // 佣金累计
    property double totalTransferFee: 0 // 过户费累计

    property double stockReturns: 0 //持仓利润
    property double stockReturnsRate: 0 //回报率

    property double saleProfits: 0 //卖出利润
    property double saleProfitsRate: 0 //卖出回报率
    property double totalSellIncome: 0 // 卖出总收入
    property double totalSellCommission: 0 // 卖出佣金
    property double totalSellTransferFee: 0 // 卖出过户费
    property double totalStampTax: 0 // 印花税

    DataLoaderQmlBridge {
        id: bridge
        code: root.stockCode

        onUpdate: {
            //成本
            currentCost = getDetail(stockCode, DataLoaderQmlBridge.Cost)
            //现价
            currentPrice = getDetail(stockCode, DataLoaderQmlBridge.CurPrice)
            //持仓数量
            stockHoldings = getDetail(stockCode, DataLoaderQmlBridge.Holdings)

            // return

            // 保本信息
            breakevenPrice = getDetail(
                        stockCode, DataLoaderQmlBridge.BreakevenStockPrice) //价格
            breakevenProfit = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.BreakevenBasedProfits) //利润
            breakevenProfitRate = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.BreakevenBaseProfitsRate) //利润率

            // 汇总数据
            totalCost = getDetail(stockCode,
                                  DataLoaderQmlBridge.TotalCost) // 成本

            totalCommission = getDetail(
                        stockCode, DataLoaderQmlBridge.BrokerageFee) // 佣金累计
            totalTransferFee = getDetail(
                        stockCode, DataLoaderQmlBridge.TransferFee) // 过户费累计

            stockReturns = getDetail(stockCode,
                                     DataLoaderQmlBridge.StockReturns) //持仓利润
            stockReturnsRate = getDetail(
                        stockCode, DataLoaderQmlBridge.StockReturnsRate) //回报率

            saleProfits = getDetail(stockCode,
                                    DataLoaderQmlBridge.StockSaleProfits) //卖出利润
            saleProfitsRate = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.StockSaleProfitsRate) //卖出回报率
            totalSellIncome = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.StockSaleProceeds) // 卖出总收入
            totalSellCommission = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.SellingBrokerageFee) // 卖出佣金
            totalSellTransferFee = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.SellingTransferFee) // 卖出过户费
            totalStampTax = getDetail(
                        stockCode,
                        DataLoaderQmlBridge.SecuritiesStampTax) // 印花税
        }
    }

    // onClosing: console.log("Im closing")
    StockPurchaseListModel {
        id: purchaseModel
    }

    StockSaleListModel {
        id: saleModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // ===== 第一个块：顶部信息行 =====
        RowLayout {
            Layout.preferredHeight: 70
            Layout.maximumHeight: 70
            // Layout.fillHeight: false
            Layout.fillWidth: true
            spacing: 10

            // 数量
            ColumnLayout {
                spacing: 2
                Text {
                    text: "数量"
                    color: "#666666"
                    font.pixelSize: 12
                }
                Text {
                    text: stockHoldings
                    color: "black"
                    font.pixelSize: 28
                    font.bold: true
                }
            }

            // 分割线
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                color: "#d0d0d0"
            }

            // 成本
            RowLayout {
                ColumnLayout {
                    // Layout.preferredWidth: 100
                    spacing: 2
                    Text {
                        text: "成本"
                        color: "#666666"
                        font.pixelSize: 12
                    }
                    Text {
                        text: currentCost
                        color: "cyan"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }
                // 保本信息
                Rectangle {
                    Layout.fillHeight: true
                    // implicitWidth: innerLayout.implicitWidth + 20
                    Layout.preferredWidth: 80
                    color: "#f0f8ff"
                    radius: 5
                    border.color: "#c0c0c0"
                    border.width: 1

                    GridLayout {
                        columns: 2
                        columnSpacing: 5
                        // rows: 2
                        rowSpacing: 10
                        anchors.fill: parent
                        anchors.margins: 5

                        // spacing: 20
                        Text {
                            text: "保本价"
                            font.bold: true
                            color: "#333333"
                        }
                        Text {
                            text: "收益"
                            font.bold: true
                            color: "#333333"
                        }
                        Text {
                            text: breakevenPrice
                            font.bold: true
                            color: "red"
                        }
                        Text {
                            text: breakevenProfit
                            font.bold: true
                            color: "red"
                        }

                        // Text {
                        //     text: "收益率"
                        //     font.bold: true
                        //     color: "#333333"
                        // }
                        // Text {
                        //     text: breakevenProfitRate
                        //     font.bold: true
                        //     color: "red"
                        // }
                    }
                }
            }

            // 分割线
            Rectangle {
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                color: "#d0d0d0"
            }
            // 现价
            RowLayout {

                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "现价"
                        color: "#666666"
                        font.pixelSize: 12
                    }
                    Text {
                        text: currentPrice
                        color: "purple"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    implicitWidth: innerLayout.implicitWidth + 20
                    color: "#f0f8ff"
                    radius: 5
                    border.color: "#c0c0c0"
                    border.width: 1

                    GridLayout {
                        id: innerLayout

                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 5
                        anchors.fill: parent
                        anchors.margins: 5

                        // 第一行
                        Text {
                            text: "收益"
                            font.bold: true
                            color: "#666666"
                        }
                        Text {
                            text: "收益率"
                            font.bold: true
                            color: "#666666"
                        }
                        Text {
                            text: "收入"
                            font.bold: true
                            color: "#666666"
                        }
                        Text {
                            text: saleProfits
                            color: getColor(saleProfits)
                        }
                        Text {
                            text: saleProfitsRate
                            color: getColor(saleProfitsRate)
                        }
                        Text {
                            text: totalSellIncome
                            color: getColor(saleProfits)
                        }
                    }
                }
            }

            // // 间隔
            // Item {
            //     width: 10
            // }

            // 弹性空间
            Item {
                Layout.fillWidth: true
            }

            // 添加按钮
            Button {
                text: "添加"
                implicitWidth: 80
                implicitHeight: 36
                highlighted: true

                contentItem: Text {
                    text: "添加"
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.hovered ? "#1976D2" : "#2196F3"
                    radius: 4
                }

                onClicked: {
                    console.log("添加交易")
                    root.callAddDialog()
                }
            }
        }

        // 分割线
        Rectangle {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "#d0d0d0"
        }

        // ===== 第二个块：汇总表格行 =====
        RowLayout {
            Layout.preferredHeight: 140
            Layout.maximumHeight: 140
            // Layout.fillHeight: false
            Layout.fillWidth: true
            spacing: 20

            // 交易汇总
            GroupBox {
                title: "交易汇总"
                Layout.fillWidth: true
                Layout.preferredHeight: 120

                GridLayout {
                    columns: 6
                    columnSpacing: 20
                    rowSpacing: 10
                    anchors.fill: parent
                    anchors.margins: 10

                    // 第一行
                    Text {
                        text: "持仓收益"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: stockReturns
                        color: getColor(stockReturns)
                    }
                    Text {
                        text: "收益率"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: stockReturnsRate
                        color: getColor(stockReturnsRate)
                    }
                    Text {
                        text: "成本"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalCost
                        color: "#333333"
                    }

                    // 第二行
                    Text {
                        text: "佣金"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalCommission
                        color: "#333333"
                    }
                    Text {
                        text: "过户费"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalTransferFee
                        color: "#333333"
                    }

                    Item {}
                    Item {}

                    Text {
                        text: "佣金(卖)"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalSellCommission
                        color: "#333333"
                    }
                    Text {
                        text: "过户费(卖)"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalSellTransferFee
                        color: "#333333"
                    }

                    // 第三行
                    Text {
                        text: "印花税(卖)"
                        font.bold: true
                        color: "#666666"
                    }
                    Text {
                        text: totalStampTax
                        color: "#333333"
                    }
                }
            }

            // 卖出提示
            // GroupBox {
            //     title: "卖出提示（以现价全部卖出）"
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 120

            //     GridLayout {
            //         columns: 6
            //         columnSpacing: 20
            //         rowSpacing: 10
            //         anchors.fill: parent
            //         anchors.margins: 10

            //         // 第一行
            //         Text {
            //             text: "收益"
            //             font.bold: true
            //             color: "#666666"
            //         }
            //         Text {
            //             text: saleProfits
            //             color: getColor(saleProfits)
            //         }
            //         Text {
            //             text: "收益率"
            //             font.bold: true
            //             color: "#666666"
            //         }
            //         Text {
            //             text: saleProfitsRate
            //             color: getColor(saleProfitsRate)
            //         }

            //         // 第二行
            //         Text {
            //             text: "收入"
            //             font.bold: true
            //             color: "#666666"
            //         }
            //         Text {
            //             text: totalSellIncome
            //             color: "#333333"
            //         }
            //     }
            // }
        }

        // 分割线
        Rectangle {
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "#d0d0d0"
        }

        // ===== 第三个块：买入卖出记录 =====
        SplitView {
            Layout.fillHeight: true // 填充剩余空间
            Layout.fillWidth: true
            orientation: Qt.Vertical

            // 买入记录表格
            GroupBox {
                title: "买入记录 (" + purchaseView.count + ")"
                SplitView.minimumHeight: 150
                SplitView.fillHeight: true

                Item {
                    anchors.fill: parent

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "white"
                    //     opacity: 0.5
                    //     z: 1

                    //     BusyIndicator {
                    //         running: false
                    //         anchors.centerIn: parent
                    //     }
                    // }
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        // 表头
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Repeater {
                                model: purchaseModel.columnCount()
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "#e0e0e0"
                                    Text {
                                        anchors.centerIn: parent
                                        text: purchaseModel.headerData(
                                                  index, Qt.Horizontal)
                                        font.bold: true
                                        font.pixelSize: 12
                                        color: "#333333"
                                    }
                                }
                            }
                        }

                        // 数据行
                        ListView {
                            id: purchaseView

                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: purchaseModel
                            clip: true
                            spacing: 1
                            delegate: RowLayout {
                                id: delegateRoot
                                width: ListView.view.width
                                property int rowIndex: index
                                spacing: 5

                                Repeater {
                                    model: purchaseView.model.columnCount()

                                    delegate: Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 30
                                        color: delegateRoot.rowIndex % 2
                                               === 0 ? "#ffffff" : "#f8f8f8"
                                        Text {
                                            anchors.centerIn: parent
                                            text: {
                                                // console.log("行:",
                                                //             delegateRoot.rowIndex,
                                                //             "列:", index)
                                                // console.log("目前有数据",
                                                //             purchaseView.model.rowCount(
                                                //                 ))
                                                return purchaseView.model.data(
                                                            purchaseView.model.index(
                                                                delegateRoot.rowIndex,
                                                                index), index)
                                            }
                                            font.pixelSize: 12
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 空状态提示
                    Rectangle {
                        anchors.fill: parent
                        color: "#f8f8f8"
                        visible: purchaseView.count === 0
                        Text {
                            anchors.centerIn: parent
                            text: "暂无买入记录"
                            font.pixelSize: 28
                            color: "#999999"
                            font.italic: true
                        }
                    }
                }
            }

            // 卖出记录表格
            GroupBox {
                title: "卖出记录 (" + saleView.count + ")"
                SplitView.minimumHeight: 150
                SplitView.fillHeight: true

                Item {
                    anchors.fill: parent

                    // Rectangle {
                    //     anchors.fill: parent
                    //     color: "white"
                    //     opacity: 0.5
                    //     z: 1

                    //     BusyIndicator {
                    //         running: false
                    //         anchors.centerIn: parent
                    //     }
                    // }
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        // 表头
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Repeater {
                                model: saleModel.columnCount()
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    color: "#e0e0e0"
                                    Text {
                                        anchors.centerIn: parent
                                        text: saleModel.headerData(
                                                  index, Qt.Horizontal)
                                        font.bold: true
                                        font.pixelSize: 12
                                        color: "#333333"
                                    }
                                }
                            }
                        }

                        // 数据行
                        ListView {
                            id: saleView

                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: saleModel
                            clip: true
                            spacing: 1
                            delegate: RowLayout {
                                id: delegateRoot2

                                width: ListView.view.width
                                property int rowIndex: index
                                spacing: 5

                                Repeater {
                                    model: saleView.model.columnCount()

                                    delegate: Rectangle {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 30
                                        color: delegateRoot2.rowIndex % 2
                                               === 0 ? "#ffffff" : "#333333"
                                        Text {
                                            anchors.centerIn: parent
                                            text: saleView.model.data(
                                                      saleView.model.index(
                                                          delegateRoot2.rowIndex,
                                                          index), index)
                                            font.pixelSize: 12
                                        }
                                    }
                                }
                            }
                        }
                        // RowLayout {
                        //     Layout.preferredHeight: 12
                        //     Layout.fillHeight: false
                        //     Layout.fillWidth: true

                        //     // visible: saleModel.rowCount() > 0
                        //     Rectangle {
                        //         Layout.fillHeight: true
                        //         Layout.fillWidth: false
                        //         // width: ListView.view.width
                        //         Text {
                        //             anchors.centerIn: parent
                        //             text: "利润总计:"
                        //             font.pixelSize: 12
                        //             font.bold: true
                        //         }
                        //     }

                        //     Item {
                        //         Layout.fillWidth: true
                        //     }

                        //     Rectangle {
                        //         Layout.fillHeight: true
                        //         Layout.fillWidth: false
                        //         // width: ListView.view.width
                        //         Text {
                        //             anchors.centerIn: parent
                        //             text: "利润总计:"
                        //             font.pixelSize: 12
                        //             color: "red"
                        //         }
                        //     }
                        // }
                    }

                    // 空状态提示
                    Rectangle {
                        anchors.fill: parent
                        color: "#f8f8f8"
                        visible: saleView.count === 0
                        Text {
                            anchors.centerIn: parent
                            text: "暂无卖出记录"
                            font.pixelSize: 28
                            color: "#999999"
                            font.italic: true
                        }
                    }
                }
            }
        }
    }

    // 信号于槽连接
    function dateLoaderConnection(dataLoader) {
        if (dataLoader === null)
            return

        console.log("绑定dataLoader")

        root.requestRecord.connect(code => {
                                       dataLoader.loadTradeTableModel(code)
                                       dataLoader.loadDetail(code)
                                   })
        dataLoader.updatePurchaseTable.connect((code, models) => {
                                                   if (code === root.stockCode) {
                                                       console.log("更新股票",
                                                                   code, "的买入表")
                                                       purchaseModel.upateRecords(
                                                           models)
                                                   }
                                               })
        dataLoader.updateSaleTable.connect((code, models) => {
                                               if (code === root.stockCode) {
                                                   saleModel.upateRecords(
                                                       models)
                                               }
                                           })
        dataLoader.updateStockDetail.connect((code, model) => {
                                                 // console.log("更新股票细节")
                                                 if (code === root.stockCode) {
                                                     bridge.setModel(code,
                                                                     model)
                                                 }
                                             })
    }

    function getColor(val) {
        if (val < 0)
            return "green"
        else if (val > 0)
            return "red"
        return "#333333"
    }
}
