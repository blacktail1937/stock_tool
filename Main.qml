import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Window
import stock_tool
import "./dialogs"

ApplicationWindow {
    id: mainWindow

    visible: true
    // minimumWidth: 800
    minimumHeight: 800

    width: widthController.totalWidth + mainLayout.anchors.margins * 4

    height: 800
    title: "股票持仓汇总表"

    // c++
    StockSummaryTableModel {
        id: stockModel
    }

    DataLoader {
        id: dataLoader

        onUpdateSummaryTable: function (models) {
            // console.log("更新表格")
            stockModel.load(models)
        }
    }

    StockAddWindow {
        id: addWindow
        parentWindow: mainWindow

        onTradeConfirmed: function (code, name, type, date, price, holdings, tradeID) {
            console.log(code, name, type, date, price, holdings, tradeID)
            dataLoader.loadDataViaAdd(code, type, date, price,
                                      holdings, tradeID)
        }
    }

    StockImportWindow {
        id: importWindow
        parentWindow: mainWindow

        //窗口和c++通信
        Connections {
            target: importWindow

            function onImportConfirmed(content) {
                dataLoader.loadDataViaImport(content)
            }
        }
    }
    property var stockWindows: []

    // 主布局
    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ========== 标题行 + 按钮 ==========
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // 标题
            Label {
                text: "股票持仓汇总"
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignLeft
            }

            // 弹性空间，将按钮推到右边
            Item {
                Layout.fillWidth: true
            }

            // 状态信息
            // Label {
            //     text: dataLoader.statusMessage
            //     color: dataLoader.isLoading ? "#ff9900" : "#00aa00"
            //     visible: dataLoader.statusMessage !== ""
            //     font.pixelSize: 12
            // }

            // 刷新按钮
            Button {
                id: refreshButton
                text: "刷新"
                implicitWidth: 80
                implicitHeight: 32

                ToolTip {
                    // visible: enabled && hovered
                    text: "获取A股主板股票信息(代码、名称、市场)，用于添加交易时的自动填充"
                    delay: 500
                }

                enabled: false

                // 按钮样式
                background: Rectangle {
                    color: {
                        if (!parent.enabled)
                            return "#cccccc"
                        refreshButton.hovered ? "#e0e0e0" : "#f0f0f0"
                    }
                    border.color: "#c0c0c0"
                    border.width: 1
                    radius: 4
                }

                contentItem: Text {
                    text: parent.text
                    color: "#333333"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                }

                onClicked: {
                    console.log("刷新按钮点击")
                    // TODO: 调用 C++ 模型的刷新方法
                }
            }

            // 添加按钮
            Button {
                id: importButton
                text: "导入"
                implicitWidth: 80
                implicitHeight: 32

                ToolTip.visible: hovered
                ToolTip.text: "从交易软件导入记录，目前仅支持同花顺"
                ToolTip.delay: 500

                background: Rectangle {
                    color: addButton.hovered ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#c0c0c0"
                    border.width: 1
                    radius: 4
                }

                contentItem: Text {
                    text: parent.text
                    color: "#333333"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                }

                onClicked: {
                    console.log("导入按钮点击")
                    importWindow.show()
                }
            }

            // 添加按钮
            Button {
                id: addButton
                text: "添加"
                implicitWidth: 80
                implicitHeight: 32

                ToolTip.visible: hovered
                ToolTip.text: "添加股票交易记录"
                ToolTip.delay: 500

                background: Rectangle {
                    color: addButton.hovered ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#c0c0c0"
                    border.width: 1
                    radius: 4
                }

                contentItem: Text {
                    text: parent.text
                    color: "#333333"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                }

                onClicked: {
                    console.log("添加按钮点击")
                    // addDialog.open()
                    addWindow.show()
                }
            }
        }

        // 表格区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f5f5f5"
            border.color: "#d0d0d0"
            border.width: 1

            // 表格布局
            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 放在 ColumnLayout 内部或外面
                Item {
                    id: widthController

                    // 存储计算好的列宽
                    property var columnWidths: []
                    property int totalWidth: 0

                    // 辅助测量工具
                    TextMetrics {
                        id: tMetrics
                        font.pixelSize: 12
                        font.bold: true // 表头通常是粗体，按粗体算更保险
                    }

                    // 初始化时计算一次
                    Component.onCompleted: {
                        var tempWidths = []
                        for (var i = 0; i < stockModel.columnCount(); i++) {
                            // 1. 获取 C++ 传来的表头文本
                            var txt = stockModel.headerData(i, Qt.Horizontal)
                            tMetrics.text = txt

                            // 2. 计算宽度：文字宽度 * 1.5 + 左右边距(16)
                            var calculatedWidth = (tMetrics.width * 1.5) + 16

                            // 3. 设置一个最小宽度保护（防止文字太少列太窄）
                            let width = Math.max(calculatedWidth, 80)
                            totalWidth += width
                            tempWidths.push(width)
                        }
                        columnWidths = tempWidths
                    }
                }

                // 水平表头
                HorizontalHeaderView {
                    id: horizontalHeader
                    Layout.fillWidth: true
                    syncView: tableView
                    clip: true

                    // 模型使用 stockModel
                    model: stockModel

                    resizableColumns: true

                    // Connections {
                    //     target: horizontalHeader
                    //     // 注意信号参数：column (int), oldSize (real), newSize (real)
                    //     function onSectionResized(column, oldSize, newSize) {
                    //         // 更新存储的宽度
                    //         var temp = widthController.columnWidths
                    //         temp[column] = newSize
                    //         widthController.columnWidths = temp // 触发属性变更通知

                    //         // 强制同步表格布局
                    //         tableView.forceLayout()
                    //     }
                    // }

                    // 表头委托
                    delegate: Rectangle {
                        required property int column
                        property string headerText: stockModel.headerData(
                                                        column, Qt.Horizontal)

                        implicitHeight: 36
                        color: "#e0e0e0"

                        Text {
                            anchors.centerIn: parent
                            text: parent.headerText
                            font.bold: true
                            font.pixelSize: 12
                            color: "#333333"
                        }

                        // 右侧分隔线 + 拖拽区域
                        Rectangle {
                            anchors.right: parent.right
                            width: 8 // 增加感应宽度，手感更好
                            height: parent.height
                            color: "transparent" // 默认透明

                            // 视觉上的分割线
                            Rectangle {
                                width: 1
                                height: parent.height * 0.6
                                anchors.centerIn: parent
                                color: "#c0c0c0"
                            }

                            MouseArea {
                                id: resizeArea
                                anchors.fill: parent
                                cursorShape: Qt.SplitHCursor

                                // 重要：防止事件被上层 TableView 拦截
                                preventStealing: true

                                property real startX: 0
                                property real oldWidth: 0

                                onPressed: mouse => {
                                               startX = mouse.x
                                               oldWidth = widthController.columnWidths[column]
                                               // 强制捕获鼠标，直到释放
                                               mouse.accepted = true
                                           }

                                onPositionChanged: mouse => {
                                                       if (pressed) {
                                                           // 计算相对于按下时的偏移量
                                                           var delta = mouse.x - startX
                                                           var newW = Math.max(
                                                               50,
                                                               oldWidth + delta)

                                                           // 更新宽度数组
                                                           var temp = widthController.columnWidths
                                                           temp[column] = newW
                                                           widthController.columnWidths = temp

                                                           // 刷新布局
                                                           tableView.forceLayout()
                                                       }
                                                   }
                            }
                        }
                    }
                }

                // 表格视图
                TableView {
                    id: tableView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    model: stockModel
                    selectionModel: ItemSelectionModel {
                        model: stockModel
                    }
                    selectionBehavior: TableView.SelectRows
                    selectionMode: TableView.SingleSelection

                    columnSpacing: 1
                    rowSpacing: 1

                    alternatingRows: true

                    columnWidthProvider: function (column) {
                        return widthController.columnWidths[column] || 100
                    }

                    // 行高提供者
                    rowHeightProvider: function (row) {
                        return 32
                    }

                    // 单元格委托
                    delegate: Rectangle {
                        // implicitWidth: tableView.columnWidthProvider(column)
                        implicitHeight: 32

                        required property bool selected
                        required property int row
                        required property int column

                        // 背景色
                        color: {
                            if (tableView.selectionModel.hasSelection)
                                return "#c0e0ff"
                            else if (row === tableView.currentRow
                                     && column === tableView.currentColumn)
                                return "#e0f0ff"
                            else if (row % 2 === 0)
                                return "#ffffff"
                            else
                                return "#f8f8f8"
                        }

                        // 边框
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: "#d0d0d0"
                            border.width: 1
                        }

                        // 单元格内容
                        Text {
                            anchors {
                                fill: parent
                                leftMargin: 8
                                rightMargin: 8
                                verticalCenter: parent.verticalCenter
                            }

                            // 根据列索引决定显示内容
                            text: model.display

                            color: model.foreground

                            // 数字右对齐，其他居中
                            horizontalAlignment: model.textAlignment

                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 12
                            font.family: column === 1 ? "Consolas, monospace" : "Microsoft YaHei"
                            elide: Text.ElideRight
                        }

                        // 鼠标悬停效果
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#e8f0fe"
                            onExited: parent.color = row % 2 === 0 ? "#ffffff" : "#f8f8f8"

                            onClicked: {

                                // tableView.currentRow = row
                                // tableView.currentColumn = column
                                // console.log(StockSummaryTableModel.MarketRole)
                                // console.log(stockReturnsRate)
                                // console.log(Qt.UserRole)
                            }

                            onDoubleClicked: {
                                var code = model.code
                                var name = model.name

                                var stockDetailWindow = mainWindow.findStockWindow(
                                            code)
                                if (stockDetailWindow === null) {
                                    var component = Qt.createComponent(
                                                "dialogs/StockDetailWindow.qml")
                                    if (component.status === Component.Ready) {
                                        stockDetailWindow = component.createObject(
                                                    mainWindow)

                                        // signals connect============================================
                                        stockDetailWindow.callAddDialog.connect(
                                                    () => {
                                                        if (!addWindow.visible) {
                                                            // addWindow.transientParent = sideWindow
                                                            addWindow.show()
                                                        }
                                                        // sideWindow.raise()
                                                    })
                                        // sideWindow.closing.connect(() => {
                                        //                                mainWindow.removeStockWindow(
                                        //                                    sideWindow.stockCode)

                                        //                                // if (addWindow.visible
                                        //                                //     && addWindow.transientParent === sideWindow) {
                                        //                                // addWindow.close()
                                        //                                // }
                                        //                            })
                                        mainWindow.closing.connect(
                                                    stockDetailWindow.close)

                                        // signals connect============================================
                                        stockWindows.push(stockDetailWindow)

                                        // sideWindow.title = title
                                        stockDetailWindow.stockCode = code
                                        stockDetailWindow.stockName = name
                                        // sideWindow.mainWindow = mainWindow
                                        stockDetailWindow.dateLoaderConnection(
                                                    dataLoader)
                                        stockDetailWindow.requestRecord(code)
                                        stockDetailWindow.show()
                                    } else {
                                        console.error("创建窗口失败:",
                                                      component.errorString())
                                    }
                                } else {
                                    var pos = mapToGlobal(mouseX, mouseY)
                                    stockDetailWindow.x = pos.x
                                    stockDetailWindow.y = pos.y
                                    stockDetailWindow.requestActivate()
                                    stockDetailWindow.show()
                                }
                            }
                        }
                    }

                    // 选中单元格边框
                    onCurrentRowChanged: forceLayout()
                    onCurrentColumnChanged: forceLayout()

                    // 滚动条
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                    ScrollBar.horizontal: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }

        // 底部信息栏
        RowLayout {
            id: bottomLayout
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 20

            property double stockReturns: stockModel.totalStockReturns.toFixed(
                                              3)
            property double stockSaleProfits: stockModel.totalSellingProfits.toFixed(
                                                  3)

            // 统计信息
            Label {
                text: "总持仓: " + tableView.rows + " 只股票"
                color: "#666666"
            }

            Label {
                text: "持仓成本: " + stockModel.totalCost.toFixed(3)
                color: "#666666"
            }

            Label {
                text: "持仓盈亏: " + parent.stockReturns.toFixed(3)
                color: parent.stockReturns < 0 ? "green" : "red"
            }

            Label {
                text: "(如果)卖出盈亏: " + parent.stockSaleProfits.toFixed(3)
                color: parent.stockSaleProfits < 0 ? "green" : "red"
            }

            Item {
                Layout.fillWidth: true
            }

            // 颜色说明
            Row {
                spacing: 15
                Rectangle {
                    width: 16
                    height: 16
                    color: "red"
                }
                Label {
                    text: "盈利"
                    color: "#666666"
                }

                Rectangle {
                    width: 16
                    height: 16
                    color: "green"
                }
                Label {
                    text: "亏损"
                    color: "#666666"
                }
            }
        }
    }

    // 初始加载数据
    Component.onCompleted: {
        addWindow.dateLoaderConnection(dataLoader)
        dataLoader.loadDataAll()
    }

    function findStockWindow(code) {
        for (var i = 0; i < stockWindows.length; i++) {
            var win = stockWindows[i]
            if (win.stockCode === code) {
                try {
                    return stockWindows[i]
                } catch (e) {
                    // 窗口已无效，从列表中移除
                    console.log("移除无效窗口:", stockWindows[i])
                    break
                }
            }
        }

        return null
    }

    function removeStockWindow(code) {
        for (var i = 0; i < stockWindows.length; i++) {
            var win = stockWindows[i]
            if (win.stockCode === code) {
                stockWindows.splice(i, 1)
            }
        }
    }
}
