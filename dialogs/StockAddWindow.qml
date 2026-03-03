import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import stock_tool
import "../component"

Window {
    id: root
    width: 450
    height: 550
    title: "股票记录"

    flags: Qt.Window | Qt.MSWindowsFixedSizeDialogHint
    property var parentWindow: null
    property int offset: 5

    property bool isValid: false
    property var selectedStock: null
    property int tradeType: 0 // 0:买入 1:卖出

    signal tradeConfirmed(string code, string name, int type, string date, double price, int holdings, string tradeID)

    StockBaseInfoListModel {
        id: stockModel
    }

    Connections {
        target: parentWindow
        function onXChanged() {
            updatePosition()
        }
        function onYChanged() {
            updatePosition()
        }
    }

    onVisibilityChanged: {
        if (visible && parentWindow) {
            updatePosition()
        }
    }

    // border.color: "red"
    Rectangle {
        anchors.fill: parent
        anchors.margins: 15

        ColumnLayout {
            anchors.margins: 15
            spacing: 15

            // 交易类型
            RowLayout {
                Layout.fillWidth: true
                spacing: 20

                Label {
                    text: "类型:"
                    font.bold: true
                    Layout.preferredWidth: 60
                }

                RadioButton {
                    id: buyRadio
                    text: "买入"
                    checked: true
                    onCheckedChanged: if (checked) {
                                          tradeType = 0
                                          validateInput()
                                      }
                }

                RadioButton {
                    id: sellRadio
                    text: "卖出"
                    onCheckedChanged: if (checked) {
                                          tradeType = 1
                                          validateInput()
                                      }
                }
            }

            // 日期选择
            RowLayout {
                Layout.fillHeight: false

                Label {
                    text: "日期:"
                    font.bold: true
                    Layout.preferredWidth: 80
                }

                DateTimePicker {
                    id: datePicker
                }
            }

            // 股票选择 - 最简单的 ComboBox
            Label {
                text: "选择股票:"
                font.bold: true
            }

            ComboBox {
                id: stockCombo
                Layout.fillWidth: true
                Layout.preferredHeight: 40

                editable: true
                selectTextByMouse: true

                model: stockModel.filterModel
                textRole: "display"

                currentIndex: 0

                // 解决输入没反应的关键：手动同步选中的数据
                onActivated: index => {
                                 let parts = editText.split("-")
                                 root.selectedStock = {
                                     "code": parts[0],
                                     "name": parts[1]
                                 }
                                 validateInput()
                             }

                // onEditTextChanged: {
                //     if (activeFocus) {
                //         stockModel.updateFilter(editText)

                //         if (editText.length > 0 && stockModel.rowCount() > 0) {
                //             if (!popup.opened)
                //                 popup.open()
                //         } else {
                //             popup.close()
                //         }
                //     }
                // }
            }

            // 价格输入
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "价格:"
                    font.bold: true
                    Layout.preferredWidth: 80
                }

                TextField {
                    id: priceInput
                    Layout.fillWidth: true
                    placeholderText: "请输入价格"
                    validator: DoubleValidator {
                        bottom: 0.01
                        decimals: 2
                    }
                    onTextChanged: validateInput()
                }
            }

            // 数量输入
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "数量:"
                    font.bold: true
                    Layout.preferredWidth: 80
                }

                TextField {
                    id: quantityInput
                    Layout.fillWidth: true
                    placeholderText: tradeType === 0 ? "100的整数倍" : "整数"
                    validator: IntValidator {
                        bottom: 1
                        top: 9999999
                    }
                    onTextChanged: validateInput()
                }
            }

            // 数量提示
            Label {
                text: tradeType === 0 ? "提示：买入数量必须是100的整数倍" : "提示：卖出数量请输入整数"
                color: "#666666"
                font.pixelSize: 10
                // visible: quantityInput.text.length > 0
            }

            // 成交编号
            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "成交编号:"
                    font.bold: true
                    Layout.preferredWidth: 80
                }

                TextField {
                    id: uidInput
                    Layout.fillWidth: true
                    placeholderText: "成交记录中的唯一标识"
                    validator: RegularExpressionValidator {
                        regularExpression: /^[0-9]{1,20}$/
                    }
                    onTextChanged: validateInput()
                }
            }

            // 错误提示
            Label {
                id: errorText
                Layout.fillWidth: true
                color: "red"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                visible: text !== ""
            }

            // 按钮
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "取消"
                    onClicked: root.close()
                }

                Button {
                    text: "确定"
                    enabled: root.isValid
                    highlighted: true
                    onClicked: {
                        if (root.isValid) {
                            root.tradeConfirmed(selectedStock.code,
                                                selectedStock.name, tradeType,
                                                datePicker.selectedDateTime,
                                                parseFloat(priceInput.text),
                                                parseInt(quantityInput.text),
                                                uidInput.text)
                            root.close()
                        }
                    }
                }
            }
        }
    }

    // 更新位置函数
    function updatePosition() {
        if (!mainWindow)
            return

        // 获取主窗口在屏幕上的位置
        var mainX = mainWindow.x
        var mainY = mainWindow.y
        var mainWidth = mainWindow.width

        // 设置窗口位置：主窗口右侧 + 偏移量，顶部对齐
        x = mainX + mainWidth + offset
        y = mainY

        // 如果超出屏幕右边界，放在左侧
        if (x + width > Screen.width) {
            x = mainX - width - offset
        }

        // 确保不超出屏幕上下边界
        if (y + height > Screen.height) {
            y = mainY + mainWindow.height - height
        }
        if (y < 0) {
            y = mainY
        }

        // console.log("new position:", x, y)
    }

    // 验证输入
    function validateInput() {
        var valid = true
        var errorMsg = ""

        if (!selectedStock) {
            valid = false
            errorMsg = "请选择股票"
        } else if (!priceInput.text || parseFloat(priceInput.text) <= 0) {
            valid = false
            errorMsg = "请输入有效的价格"
        } else if (!quantityInput.text || parseInt(quantityInput.text) <= 0) {
            valid = false
            errorMsg = "请输入有效的数量"
        } else {
            var quantity = parseInt(quantityInput.text)
            if (tradeType === 0 && quantity % 100 !== 0) {
                valid = false
                errorMsg = "买入数量必须是100的整数倍"
            }
        }

        // if (!dateInput.text || !/^\d{8}$/.test(dateInput.text)) {
        //     valid = false
        //     errorMsg = errorMsg || "请输入有效的日期(YYYYMMDD)"
        // }
        if (!uidInput.text) {
            valid = false
            errorMsg = errorMsg || "请输入有效的成交编号，可以在交易记录中查看"
        }

        errorText.text = errorMsg
        root.isValid = valid
        return valid
    }

    // 信号于槽连接
    function dateLoaderConnection(dataLoader) {
        if (dataLoader === null)
            return

        console.log("添加界面绑定dataLoader")
        dataLoader.updateStockBaseInfo.connect(stockModel.load)
    }
}
