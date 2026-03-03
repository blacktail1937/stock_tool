// SideWindow.qml
import QtQuick
// import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Window {
    id: root

    // 属性：要依附的主窗口
    property var mainWindow: null
    property int offset: 5 // 与主窗口的间距

    // 窗口基本属性
    width: 400
    height: 600
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
    title: "侧边窗口{offset}"

    signal callAddDialog

    // 当窗口显示时，自动定位到主窗口右侧
    onVisibleChanged: {
        if (visible && mainWindow) {
            updatePosition()
        }
    }

    onClosing: function (e) {
        console.log(e)
    }

    // 当主窗口移动时，更新位置
    Connections {
        target: mainWindow
        function onXChanged() {// updatePosition()
        }
        function onYChanged() {// updatePosition()
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

        console.log("new position:", x, y)
    }

    // 窗口内容 - 随便加点东西
    Rectangle {
        anchors.fill: parent
        color: "#f5f5f5"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // 标题
            Label {
                text: "侧边窗口"
                font.bold: true
                font.pixelSize: 20
                color: "#333333"
                Layout.alignment: Qt.AlignHCenter
            }

            // 分隔线
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#d0d0d0"
            }

            // 一些演示内容
            GroupBox {
                title: "示例控件"
                Layout.fillWidth: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Label {
                        text: "这是一个独立的窗口"
                    }
                    Label {
                        text: "它紧挨着主窗口的右侧显示"
                    }
                    Label {
                        text: "你可以随意移动它"
                    }
                }
            }

            // 输入框示例
            GroupBox {
                title: "输入示例"
                Layout.fillWidth: true

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    TextField {
                        Layout.fillWidth: true
                        placeholderText: "输入一些内容..."
                    }

                    TextArea {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        placeholderText: "多行输入..."
                    }
                }
            }

            // 按钮区域
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 10
                spacing: 10

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "关闭"
                    onClicked: root.callAddDialog()
                }

                Button {
                    text: "确定"
                    highlighted: true
                    onClicked: {
                        console.log("确定按钮点击")
                        // TODO: 处理确定逻辑
                    }
                }
            }
        }
    }
}
