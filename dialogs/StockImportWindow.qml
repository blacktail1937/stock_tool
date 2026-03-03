import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Window {
    id: root
    width: 450
    height: 500
    title: "批量导入"

    // visible: true
    flags: Qt.Window | Qt.MSWindowsFixedSizeDialogHint

    signal importConfirmed(string content)

    property var parentWindow: null

    onVisibilityChanged: {
        textArea.clear()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 15

        // border.color: "red"
        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            // 1. 使用 ScrollView 作为外层容器
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                anchors.margins: 10

                clip: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                // 2. 内部放置 TextArea
                TextArea {
                    id: textArea
                    placeholderText: "目前仅支持同花顺成交记录导入"

                    // 允许自动换行
                    wrapMode: TextArea.Wrap

                    // 设置字体
                    font.pixelSize: 14

                    // 选填：设置一些内边距，让文字不贴边
                    leftPadding: 10
                    rightPadding: 10
                    topPadding: 10
                    bottomPadding: 10

                    // 建议：让 TextArea 背景透明，或者自定义背景
                    background: Rectangle {
                        color: "#f8f8f8"
                        border.color: "#ddd"
                    }
                }
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
                    text: "导入"
                    enabled: textArea.text.length > 0
                    highlighted: true
                    onClicked: {
                        importConfirmed(textArea.text)

                        // console.log(textArea.text)
                        root.close()
                    }
                }
            }
        }
    }
}
