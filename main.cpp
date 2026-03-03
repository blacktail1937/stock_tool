// #include <pybind11/embed.h>
#define PYBIND11_NO_ASSERT_GIL_HELD_INCREF_DECREF

#include "python/pythonwrapper.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <string>

#include "database/databasemanager.h"
#include "util/stockcalcuator.h"

#ifdef _WIN32
    #include <windows.h>
#endif

void print_stocks(const py::list& stocks) {
    if(stocks.is_none() || stocks.size() == 0) {
        qDebug().noquote() << "没有找到主板股票数据";
        return;
    }

    qDebug().noquote() << "\n========== 主板股票列表 ==========";
    qDebug().noquote() << "总计:" << stocks.size() << "只股票\n";

    // Qt 的 qDebug 不支持 setw，所以需要手动格式化
    QString header;
    QTextStream headerStream(&header);
    headerStream << QStringLiteral("代码").leftJustified(10)
                 << QStringLiteral("名称").leftJustified(16)
                 << QStringLiteral("市场").leftJustified(16) << QStringLiteral("交易所");
    qDebug().noquote() << header;
    qDebug().noquote() << QString(50, '-');

    int count = 0;
    for(py::handle item: stocks) {
        if(count++ >= 20) {
            qDebug().noquote() << QString("... 还有 %1 只股票未显示").arg(stocks.size() - 20);
            break;
        }

        py::dict stock = item.cast<py::dict>();

        QString code   = QString::fromStdString(stock["代码"].cast<std::string>());
        QString name   = QString::fromStdString(stock["名称"].cast<std::string>())
                           .simplified()
                           .replace(QRegularExpression("\\s"), "");
        QString market   = QString::fromStdString(stock["市场"].cast<std::string>());
        QString exchange = QString::fromStdString(stock["交易所"].cast<std::string>());

        QString line;
        QTextStream lineStream(&line);
        lineStream << code.leftJustified(10) << name.leftJustified(16) << market.leftJustified(16)
                   << exchange;
        qDebug().noquote() << line;
    }
}

// void runDatabase() {
//     using namespace sqlite_orm;
//     using namespace StockTable;
//     using namespace std::chrono;

//     struct Elapsed {
//         Elapsed() { start_ = steady_clock::now(); }

//         ~Elapsed() {
//             auto now = steady_clock::now();
//             qDebug() << "elapsed:" << duration_cast<milliseconds>(now - start_).count();
//         }

//       private:
//         time_point<steady_clock> start_;
//     };

//     QString scriptPath = QString("%1/%2").arg(QDir::currentPath(), "scripts");
//     scriptPath         = QDir::cleanPath(scriptPath);
//     qDebug() << "script path: " << scriptPath;

//     // scriptPath = "./scripts";

//     Python::initialize(scriptPath);

//     auto _ = QtConcurrent::run([] {
//         // return;

//         // py::gil_scoped_acquire r;
//         qDebug() << "GIL STATE:" << PyGILState_Check();

//         auto result = Python::call<std::vector<std::unordered_map<std::string, std::string>>>(
//             "akshare_stock",
//             "get_stock_list_akshare");

//         if(result) {
//             try {
//                 Database::initialize();

//                 qDebug() << "thread safe:" << sqlite_orm::threadsafe();
//                 const auto& values = result.value();

//                 qDebug() << "总共 " << values.size() << " 条记录";

//                 // storage.begin_transaction();
//                 // storage.remove_all<AStocksBaseInfo>();
//                 // storage.commit();

//                 // storage.begin_transaction();

//                 // int count = 1;

//                 std::vector<AStocksBaseInfo> s1;
//                 std::vector<AStocksBaseInfo> s2;

//                 for(auto&& m: values) {
//                     auto code   = m.at("code");
//                     auto name   = m.at("name");
//                     auto market = m.at("market");
//                     auto border = m.at("border");

//                     s1.push_back(std::move(AStocksBaseInfo { -1, code, name, market, border }));
//                     s2.push_back(std::move(AStocksBaseInfo { -1, code, name, market, border }));
//                 }
//                 // for(py::handle item: values) {
//                 //     auto stock  = item.cast<py::dict>();

//                 //     auto code   = stock["code"].cast<std::string>();
//                 //     auto name   = stock["name"].cast<std::string>();
//                 //     auto market = stock["market"].cast<std::string>();
//                 //     auto border = stock["border"].cast<std::string>();

//                 //     // for(auto&& s: stock) {
//                 //     //     qDebug() << s.first.cast<std::string>();
//                 //     // }

//                 //     s1.push_back(std::move(AStocksBaseInfo { -1, code, name, market,
//                 //     border
//                 //     })); s2.push_back(std::move(AStocksBaseInfo { -1, code, name, market,
//                 //     border }));
//                 // }

//                 qDebug() << "GIL STATE:" << PyGILState_Check();
//                 // return;

//                 auto _1 = QtConcurrent::run([s1] {
//                     // return;
//                     qDebug() << "s1";
//                     {
//                         Elapsed e;
//                         try {
//                             for(auto&& s: s1) {
//                                 Database::insertRecord(s);
//                             }
//                             // StockTable::insertRecords(s1);
//                         }
//                         catch(std::exception& e) {
//                             qWarning() << "s1 sqlite_orm:" << e.what();
//                         }
//                     }
//                 });

//                 auto _2 = QtConcurrent::run([s2] {
//                     // return;
//                     qDebug() << "s2";
//                     {
//                         Elapsed e;
//                         try {
//                             for(auto&& s: s2) {
//                                 Database::insertRecord(s);
//                             }
//                             // StockTable::insertRecords(s2);
//                         }
//                         catch(std::exception& e) {
//                             qWarning() << "s2 sqlite_orm:" << e.what();
//                         }
//                     }
//                 });
//                 _2.cancel();

//                 // StockTable::insertRecords(s);

//                 // StockTable::initialize();
//                 auto task = [] {
//                     std::this_thread::sleep_for(1s);
//                     int count = 1;
//                     qDebug() << QThread::currentThreadId();
//                     while(true) {
//                         if(count >= 40) {
//                             break;
//                         }
//                         try {
//                             qDebug() << count << " 1 " << QThread::currentThreadId();

//                             auto stocks = Database::getRecords<AStocksBaseInfo>(
//                                 where(c(&AStocksBaseInfo::code) == "000776"));
//                             qDebug() << count << "stocks.size: " << stocks.size();
//                             if(stocks.size() > 0) {
//                                 // qDebug() << count << "2 ";
//                                 auto stock = stocks.front();
//                                 qDebug() << QString("%6
//                                 id:%1,code:%2,name:%3,market:%4,border:%5")
//                                                 .arg(stock.id)
//                                                 .arg(stock.code)
//                                                 .arg(stock.name)
//                                                 .arg(stock.market)
//                                                 .arg(stock.border)
//                                                 .arg(count)
//                                          << QThread::currentThreadId();
//                             }
//                             // qDebug() << "3 ";
//                         }
//                         catch(std::exception& e) {
//                             qWarning()
//                                 << count << " task:" << e.what() << QThread::currentThreadId();
//                         }
//                         count++;

//                         std::this_thread::sleep_for(500ms);
//                     }
//                 };
//                 _1 = QtConcurrent::run(task);
//                 _2 = QtConcurrent::run(task);
//             }
//             catch(const py::error_already_set& e) {
//                 qWarning() << e.what();
//             }
//             catch(const std::exception& e) {
//                 qWarning() << "sqlite_orm:" << e.what();
//             }
//         }
//     });
// }

// void runPython() {
//     using namespace std::chrono;

//     struct Elapsed {
//         Elapsed() { start_ = steady_clock::now(); }

//         ~Elapsed() {
//             auto now = steady_clock::now();
//             qDebug() << "elapsed:" << duration_cast<milliseconds>(now - start_).count();
//         }

//       private:
//         time_point<steady_clock> start_;
//     };

//     QString scriptPath = QString("%1/%2").arg(QDir::currentPath(), "scripts");
//     scriptPath         = QDir::cleanPath(scriptPath);
//     qDebug() << "script path: " << scriptPath;

//     // scriptPath = "./scripts";

//     Python::initialize(scriptPath);

//     auto task = [](QString code) {
//         int count = 1;
//         while(true) {
//             if(count > 1) {
//                 break;
//             }
//             try {
//                 Elapsed s;
//                 auto result =
//                     Python::call<std::map<std::string, std::string>>("akshare_stock",
//                                                                      "get_stock_price_xq",
//                                                                      code.toUtf8().constData());
//                 if(result) {
//                     for(auto&& r: result.value()) {
//                         qDebug() << QString::fromUtf8(r.first) << "-" <<
//                         QString::fromUtf8(r.second)
//                                  << "\n"
//                                  << QString("第%1次获取").arg(count);
//                     }
//                 }

//                 qDebug() << "\n\n";
//             }
//             catch(const py::error_already_set& e) {
//                 qWarning() << e.what();
//             }
//             catch(const std::exception& e) {
//                 qWarning() << "sqlite_orm:" << e.what();
//             }
//             count++;

//             using namespace std::chrono;
//             std::this_thread::sleep_for(2s);
//         }

//         qDebug() << "qt thread quit!";
//     };

//     {
//         std::thread t(task, "000776");
//         t.join();
//     }
// }

#include <chrono>
int main(int argc, char* argv[]) {
#ifdef _WIN32
    // 设置控制台输出为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
#endif
#if 0
    double total  = 15956.15;
    int holdings  = 3000;

    auto p1       = StockCalcuator::breakevenStockPrice(holdings, total);
    auto proceeds = StockCalcuator::stockSaleProceeds(holdings, p1);
    auto cost1    = StockCalcuator::stockTotalPurchaseCost(1000, 5.42) +
                 StockCalcuator::stockTotalPurchaseCost(1000, 5.29);
    auto avgCost1 = cost1 / 2000;

    qDebug() << "保本股价:" << p1;
    qDebug() << "保本收入:" << proceeds;
    qDebug() << "保本利润:" << proceeds - total;
    qDebug() << "成本1:"
             << StockCalcuator::stockTotalPurchaseCost(1000, 5.42) +
                    StockCalcuator::stockTotalPurchaseCost(1000, 5.29) +
                    StockCalcuator::stockTotalPurchaseCost(1000, 5.25);
    qDebug() << "成本2:" << StockCalcuator::stockTotalPurchaseCost(3000, 5.33);
    qDebug() << "平均成本1:" << avgCost1;
#endif
    // runDatabase();
    // runPython();

    // QString data("20260227    14:40:13    000776    广发证券    买入    500.000    20.900    "
    //              "10450.000    1100    247182    0101000074932821    5.000    0.000    0.000    "
    //              "-10455.000    证券买入    1    0.000    0    0        ");
    // auto list = data.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    // for(auto&& l: list) {
    //     qDebug() << l;
    // }

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);
    engine.loadFromModule("stock_tool", "Main");

    ::_exit(app.exec());
}
