
#include <QDateTime>
#include <QString>

namespace Common {
inline static QString to3Decimals(double num) {
    return QString::number(num, 'f', 3);
}

inline static constexpr int modelRoleConvertor(int role) {
    return role - Qt::UserRole - 1;
}

inline static bool isMarketOpen() {
    auto now = QDateTime::currentDateTime();

    int day  = now.date().dayOfWeek();
    if(day == Qt::Saturday || day == Qt::Sunday) {
        return false;
    }

    QTime currentTime = now.time();
    int minutes       = currentTime.hour() * 60 + currentTime.minute();

    // 上午时段：09:30 -- 11:30
    bool morning = (minutes >= 570 && minutes <= 690);
    // 下午时段：13:00 -- 15:00
    bool afternoon = (minutes >= 780 && minutes <= 900);

    return morning || afternoon;
}
}
