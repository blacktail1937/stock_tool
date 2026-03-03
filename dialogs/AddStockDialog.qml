// dialogs/TradeDialog.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Dialog {
    id: root

    title: "股票交易"
    modal: true
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    signal tradeConfirmed(string code, string name, int tradeType, string tradeDate, double price, int quantity)

    property bool isValid: false
    property var selectedStock: null
    property int tradeType: 0 // 0:买入 1:卖出

    width: 500
    height: 600
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    // 简化的股票模型 - 直接使用 ComboBox 能识别的格式
    ListModel {
        id: stockModel
        ListElement {
            text: "000001 - 平安银行 (深A)"
        }
        ListElement {
            text: "000002 - 万科A (深A)"
        }
        ListElement {
            text: "600519 - 贵州茅台 (沪A)"
        }
        ListElement {
            text: "300750 - 宁德时代 (创业板)"
        }
        ListElement {
            text: "601318 - 中国平安 (沪A)"
        }
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

        if (!dateInput.text || !/^\d{8}$/.test(dateInput.text)) {
            valid = false
            errorMsg = errorMsg || "请输入有效的日期(YYYYMMDD)"
        }

        errorText.text = errorMsg
        root.isValid = valid
        return valid
    }

    // 清除选中
    function clearSelection() {
        selectedStock = null
        stockCombo.currentIndex = -1
        priceInput.text = ""
        quantityInput.text = ""
        dateInput.text = ""
        tradeType = 0
        buyRadio.checked = true
        validateInput()
    }

    contentItem: ColumnLayout {
        spacing: 15
        anchors.margins: 15

        // 交易类型
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Label {
                text: "交易类型:"
                font.bold: true
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
            Layout.fillWidth: true

            Label {
                text: "交易日期:"
                font.bold: true
                Layout.preferredWidth: 80
            }

            TextField {
                id: dateInput
                Layout.fillWidth: true
                placeholderText: "YYYYMMDD"
                inputMask: "00000000"
                maximumLength: 8
                onTextChanged: validateInput()

                Component.onCompleted: {
                    var today = new Date()
                    var year = today.getFullYear()
                    var month = (today.getMonth() + 1).toString().padStart(2,
                                                                           '0')
                    var day = today.getDate().toString().padStart(2, '0')
                    text = year + month + day
                }
            }

            Button {
                text: "今天"
                onClicked: {
                    var today = new Date()
                    var year = today.getFullYear()
                    var month = (today.getMonth() + 1).toString().padStart(2,
                                                                           '0')
                    var day = today.getDate().toString().padStart(2, '0')
                    dateInput.text = year + month + day
                }
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

            // 最基本的设置
            model: stockModel
            editable: true

            // 当选中的时候
            onCurrentIndexChanged: {
                if (currentIndex >= 0) {
                    // 从选中的文本解析数据
                    var text = stockModel.get(currentIndex).text
                    var parts = text.split(" - ")
                    selectedStock = {
                        "code": parts[0],
                        "name": parts[1].split(" ")[0],
                        "market": parts[1].split(
                            " ")[1] ? parts[1].split(" ")[1].replace(/[\(\)]/g,
                                                                     '') : ""
                    }
                } else {
                    selectedStock = null
                }
                validateInput()
            }

            // 输入时过滤（简单实现）
            onEditTextChanged: {

                // 这里先不做复杂过滤，确保基本功能正常
            }
        }

        // 价格输入
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "交易价格:"
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
                text: "交易数量:"
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
            visible: quantityInput.text.length > 0
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
                                            dateInput.text,
                                            parseFloat(priceInput.text),
                                            parseInt(quantityInput.text))
                        root.close()
                    }
                }
            }
        }
    }

    onClosed: clearSelection()
}
