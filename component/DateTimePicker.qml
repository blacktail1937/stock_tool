import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    //public
    readonly property string selectedDateTime: showTime ? Qt.formatDateTime(
                                                              _internalDate,
                                                              "yyyyMMdd hh:mm:ss") : Qt.formatDate(
                                                              _internalDate,
                                                              "yyyyMMdd")
    property bool showTime: true //隐藏时分秒

    //internal
    property date _internalDate: new Date()

    // --- 关键修改：让根 Item 跟随布局的大小 ---
    implicitWidth: mainLayout.implicitWidth
    implicitHeight: mainLayout.implicitHeight

    RowLayout {
        id: mainLayout

        anchors.fill: parent
        spacing: showTime ? 10 : 0

        // --- 年月日组合 ---
        RowLayout {
            spacing: 0
            TextField {
                id: dateEdit
                text: Qt.formatDate(_internalDate, "yyyyMMdd")
                implicitWidth: 100
                selectByMouse: true

                // 1. 物理长度限制：绝对不允许输入超过 8 位
                maximumLength: 8

                // 2. 输入过滤：只允许输入纯数字 (0-9)
                validator: RegularExpressionValidator {
                    regularExpression: /[0-9]+/
                }

                // 3. 逻辑检查：当用户停止编辑（回车或点击别处）
                onEditingFinished: {
                    // 检查：必须满 8 位 且 通过日期合法性校验
                    if (text.length === 8 && isValidDate(text)) {
                        let y = parseInt(text.substring(0, 4))
                        let m = parseInt(text.substring(4, 6)) - 1
                        let d = parseInt(text.substring(6, 8))

                        let copy = new Date(_internalDate)
                        copy.setFullYear(y)
                        copy.setMonth(m)
                        copy.setDate(d)

                        _internalDate = copy
                    } else {
                        // 只要不满足上述条件（比如只输了 7 位，或者输了 20241332）
                        // 立即重置为当天
                        let today = new Date()
                        _internalDate.setFullYear(today.getFullYear())
                        _internalDate.setMonth(today.getMonth())
                        _internalDate.setDate(today.getDate())

                        // 显式刷新 UI 文本，强制覆盖掉用户输入的错误内容
                        text = Qt.formatDate(_internalDate, "yyyyMMdd")
                    }
                }
            }

            Button {
                id: openBtn
                text: "📅"
                implicitWidth: 35
                implicitHeight: dateEdit.height
                onClicked: datePopup.open()

                // --- 弹窗定义在按钮内部，方便相对定位 ---
                Popup {
                    id: datePopup
                    x: -dateEdit.width
                    y: parent.height
                    width: 220 // 宽度微调
                    padding: 5 // 极小内边距

                    contentItem: ColumnLayout {
                        spacing: 2 // 控件间的垂直间距设为极小

                        // 1. 紧凑型导航栏
                        // --- 在 Popup 的 contentItem: ColumnLayout 内部替换顶部的 RowLayout ---
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 25
                            spacing: 2

                            // 往前一个月
                            Button {
                                text: "<"
                                flat: true
                                Layout.preferredWidth: 20
                                onClicked: _internalDate = safeSetMonth(
                                               _internalDate, -1)
                            }

                            // --- 年份输入框 ---
                            TextField {
                                id: yearInput
                                text: _internalDate.getFullYear()
                                font.pixelSize: 11
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                Layout.preferredWidth: 45
                                background: null // 去掉背景，更美观
                                selectByMouse: true
                                inputMask: "9999" // 限制 4 位数字

                                onEditingFinished: {
                                    let newYear = parseInt(text)
                                    if (newYear >= 1900 && newYear <= 2100) {
                                        _internalDate = safeSetYear(
                                                    _internalDate,
                                                    newYear - _internalDate.getFullYear(
                                                        ))
                                    } else {
                                        text = _internalDate.getFullYear(
                                                    ) // 输入非法则回滚
                                    }
                                }
                            }

                            Text {
                                text: "-"
                                font.pixelSize: 10
                                color: "#999"
                            }

                            // --- 月份输入框 ---
                            TextField {
                                id: monthInput
                                text: (_internalDate.getMonth() + 1).toString(
                                          ).padStart(2, '0')
                                font.pixelSize: 11
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                Layout.preferredWidth: 30
                                background: null
                                selectByMouse: true

                                onEditingFinished: {
                                    let newMonth = parseInt(text)
                                    if (newMonth >= 1 && newMonth <= 12) {
                                        // 注意：JS 的 Month 是从 0 开始的
                                        let targetDate = new Date(_internalDate.getFullYear(
                                                                      ),
                                                                  newMonth - 1,
                                                                  _internalDate.getDate(
                                                                      ))
                                        // 同样要处理月底溢出逻辑
                                        let lastDay = new Date(_internalDate.getFullYear(
                                                                   ), newMonth,
                                                               0).getDate()
                                        targetDate.setDate(
                                                    Math.min(
                                                        _internalDate.getDate(
                                                            ), lastDay))
                                        _internalDate = targetDate
                                    } else {
                                        text = (_internalDate.getMonth(
                                                    ) + 1).toString(
                                                    ).padStart(2, '0')
                                    }
                                }
                            }

                            // 往后一个月
                            Button {
                                text: ">"
                                flat: true
                                Layout.preferredWidth: 20
                                onClicked: _internalDate = safeSetMonth(
                                               _internalDate, 1)
                            }
                        }

                        // 2. 极矮的星期行
                        DayOfWeekRow {
                            locale: Qt.locale("zh_CN")
                            Layout.fillWidth: true
                            Layout.preferredHeight: 15 // 强制压缩高度
                            delegate: Text {
                                text: model.narrowName
                                font.pixelSize: 9
                                color: "#999"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        // 3. 紧凑型日期网格
                        MonthGrid {
                            id: grid
                            month: _internalDate.getMonth()
                            year: _internalDate.getFullYear()
                            locale: Qt.locale("zh_CN")
                            Layout.fillWidth: true

                            // 关键：这里控制每一行的高度
                            delegate: Control {
                                id: dayControl
                                implicitWidth: 30
                                implicitHeight: 14
                                hoverEnabled: true // 开启悬浮功能

                                // 增加一个内部状态：是否正在被点击（用于显示选中的那个瞬间）
                                property bool isBeingClicked: false

                                contentItem: Text {
                                    text: model.day
                                    font.pixelSize: 11
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    // 逻辑：如果是选中日期 OR 正在被点击，文字变白
                                    color: (model.date.toDateString(
                                                ) === _internalDate.toDateString()
                                            || dayControl.isBeingClicked) ? "white" : "black"
                                    opacity: model.month === grid.month ? 1 : 0.3
                                }

                                background: Rectangle {
                                    radius: 4
                                    // 1. 填充颜色逻辑
                                    color: {
                                        if (model.date.toDateString(
                                                    ) === _internalDate.toDateString()
                                                || dayControl.isBeingClicked) {
                                            return "#2196F3" // 选中或点击瞬时的深蓝色
                                        }
                                        if (dayControl.hovered) {
                                            return "#E3F2FD" // 鼠标悬浮时的浅蓝色背景
                                        }
                                        return "transparent"
                                    }

                                    // 2. 边框逻辑：悬浮时显示边框
                                    border.color: {
                                        if (dayControl.hovered)
                                            return "#2196F3"
                                        if (model.today)
                                            return "#FF5722" // 今天的特殊边框
                                        return "transparent"
                                    }
                                    border.width: (dayControl.hovered
                                                   || model.today) ? 1 : 0
                                }

                                // 3. 点击逻辑与延迟关闭
                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true // 必须也开启，否则会吞掉父层的hover
                                    onClicked: {
                                        dayControl.isBeingClicked = true // 触发“选中”视觉效果

                                        let copy = new Date(_internalDate)
                                        copy.setFullYear(
                                                    model.date.getFullYear())
                                        copy.setMonth(model.date.getMonth())
                                        copy.setDate(model.date.getDate())

                                        _internalDate = copy

                                        // 使用定时器延迟 150 毫秒关闭，让用户看清“点到了”
                                        closeTimer.start()
                                    }
                                }

                                Timer {
                                    id: closeTimer
                                    interval: 150 // 0.15秒
                                    onTriggered: {
                                        dayControl.isBeingClicked = false
                                        datePopup.close()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- 时分秒部分 (布局优化) ---
        RowLayout {
            visible: showTime

            spacing: 2
            TimeInputUnit {
                id: hourInput
                value: _internalDate.getHours()
                max: 23
                unit: "时"

                onValueChanged: {
                    let copy = new Date(_internalDate)
                    copy.setHours(value)
                    _internalDate = copy
                }
            }
            TimeInputUnit {
                id: minInput
                value: _internalDate.getMinutes()
                max: 59
                unit: "分"

                onValueChanged: {
                    let copy = new Date(_internalDate)
                    copy.setMinutes(value)
                    _internalDate = copy
                }
            }
            TimeInputUnit {
                id: secInput
                value: _internalDate.getSeconds()
                max: 59
                unit: "秒"

                onValueChanged: {
                    let copy = new Date(_internalDate)
                    copy.setSeconds(value)
                    _internalDate = copy
                }
            }
        }
    }

    // 时间单位组件实现
    component TimeInputUnit: RowLayout {
        property int value: 0
        property int max: 59
        property string unit: ""

        TextField {
            id: tField
            text: parent.value.toString().padStart(2, '0')
            implicitWidth: 40
            inputMask: "99" // 限制只能输两位数字
            horizontalAlignment: TextInput.AlignHCenter

            // 编辑完成时的检查
            onEditingFinished: {
                let v = parseInt(text)
                // 检查：如果是空、不是数字、或者超过最大值
                if (isNaN(v) || v > parent.max || v < 0) {
                    // 非法输入直接重置为当前时间的对应单位
                    let now = new Date()
                    if (parent.unit === "时")
                        parent.value = now.getHours()
                    else if (parent.unit === "分")
                        parent.value = now.getMinutes()
                    else
                        parent.value = now.getSeconds()
                } else {
                    parent.value = v
                }
                // 确保显示始终是两位数格式
                text = parent.value.toString().padStart(2, '0')
            }

            // 滚轮逻辑（保持不变）
            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                onWheel: wheel => {
                             console.log("滚轮在滚动", wheel.angleDelta.y,parent.parent.value)
                             if (wheel.angleDelta.y > 0)
                             parent.parent.value = (parent.parent.value + 1)
                             % (parent.parent.max + 1)
                             else
                             parent.parent.value
                             = (parent.parent.value - 1
                                < 0) ? parent.parent.max : parent.parent.value - 1
                         }
                onPressed: mouse => mouse.accepted = false
            }
        }
        Text {
            text: unit
            font.pixelSize: 11
            color: "#666"
        }
    }

    function isValidDate(dateStr) {
        if (dateStr.length !== 8)
            return false
        let y = parseInt(dateStr.substring(0, 4))
        let m = parseInt(dateStr.substring(4, 6)) - 1
        let d = parseInt(dateStr.substring(6, 8))

        // --- 增加业务合理性校验 ---
        // 如果年份小于 1900 或大于 2099，直接判定为“非法”
        if (y < 1900 || y > 2099)
            return false

        let testDate = new Date(y, m, d)
        return testDate.getFullYear() === y && testDate.getMonth() === m
                && testDate.getDate() === d
    }

    // --- 核心修复函数：安全地切换月份 ---
    function safeSetMonth(sourceDate, delta) {
        let year = sourceDate.getFullYear()
        let month = sourceDate.getMonth() + delta
        let day = sourceDate.getDate()

        // 1. 获取目标月份的实际最大天数
        // 原理：下个月的第 0 天就是本月的最后一天
        let lastDayOfTargetMonth = new Date(year, month + 1, 0).getDate()

        // 2. 如果当前天数超过了目标月的最大天数，则取最后一天
        let finalDay = Math.min(day, lastDayOfTargetMonth)

        // 3. 返回构造后的新日期
        return new Date(year, month, finalDay)
    }

    // --- 核心修复函数：安全地切换年份 ---
    function safeSetYear(sourceDate, delta) {
        let year = sourceDate.getFullYear() + delta
        let month = sourceDate.getMonth()
        let day = sourceDate.getDate()

        // 处理闰年平年转换（例如 2月29日 切换到平年变成 2月28日）
        let lastDayOfTargetMonth = new Date(year, month + 1, 0).getDate()
        let finalDay = Math.min(day, lastDayOfTargetMonth)

        return new Date(year, month, finalDay)
    }
}
