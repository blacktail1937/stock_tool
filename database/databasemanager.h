// database_manager.h
#pragma once

#include "stocktable.h"

#include <sqlite_orm/sqlite_orm.h>

#include <functional>
#include <future>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>
#include <type_traits>

#include <QDebug>

namespace Database {
namespace internal {
using namespace StockTable;
// 数据库句柄
inline static auto handle = sqlite_orm::make_storage(
    "astock.sqlite",
    sqlite_orm::make_table("AStocksBaseInfo",
                           sqlite_orm::make_column("id",
                                                   &AStocksBaseInfo::id,
                                                   sqlite_orm::primary_key().autoincrement()),
                           sqlite_orm::make_column("code", &AStocksBaseInfo::code),
                           sqlite_orm::make_column("name", &AStocksBaseInfo::name),
                           sqlite_orm::make_column("market", &AStocksBaseInfo::market),
                           sqlite_orm::make_column("border", &AStocksBaseInfo::border)),
    sqlite_orm::make_table(
        "StockPurchaseHistory",
        sqlite_orm::make_column("id",
                                &StockPurchaseHistory::id,
                                sqlite_orm::primary_key().autoincrement()),
        sqlite_orm::make_column("tradeDate", &StockPurchaseHistory::tradeDate),
        sqlite_orm::make_column("tradeType", &StockPurchaseHistory::tradeType),
        sqlite_orm::make_column("code", &StockPurchaseHistory::code),
        sqlite_orm::make_column("name", &StockPurchaseHistory::name),
        sqlite_orm::make_column("market", &StockPurchaseHistory::market),
        sqlite_orm::make_column("border", &StockPurchaseHistory::border),
        sqlite_orm::make_column("price", &StockPurchaseHistory::price),
        sqlite_orm::make_column("holdings", &StockPurchaseHistory::holdings),
        sqlite_orm::make_column("brokerageFee", &StockPurchaseHistory::brokerageFee),
        sqlite_orm::make_column("transferFee", &StockPurchaseHistory::transferFee),
        sqlite_orm::make_column("cost", &StockPurchaseHistory::cost),
        sqlite_orm::make_column("tradeID", &StockPurchaseHistory::tradeID)),
    sqlite_orm::make_table(
        "StockSaleHistory",
        sqlite_orm::make_column("id",
                                &StockSaleHistory::id,
                                sqlite_orm::primary_key().autoincrement()),
        sqlite_orm::make_column("tradeDate", &StockSaleHistory::tradeDate),
        sqlite_orm::make_column("tradeType", &StockSaleHistory::tradeType),
        sqlite_orm::make_column("code", &StockSaleHistory::code),
        sqlite_orm::make_column("name", &StockSaleHistory::name),
        sqlite_orm::make_column("market", &StockSaleHistory::market),
        sqlite_orm::make_column("border", &StockSaleHistory::border),
        sqlite_orm::make_column("cost", &StockSaleHistory::cost),
        sqlite_orm::make_column("price", &StockSaleHistory::price),
        sqlite_orm::make_column("holdings", &StockSaleHistory::holdings),
        sqlite_orm::make_column("brokerageFee", &StockSaleHistory::brokerageFee),
        sqlite_orm::make_column("transferFee", &StockSaleHistory::transferFee),
        sqlite_orm::make_column("securitiesStampTax", &StockSaleHistory::securitiesStampTax),
        sqlite_orm::make_column("proceeds", &StockSaleHistory::proceeds),
        sqlite_orm::make_column("profits", &StockSaleHistory::profits),
        sqlite_orm::make_column("profitsRate", &StockSaleHistory::profitsRate),
        sqlite_orm::make_column("tradeID", &StockSaleHistory::tradeID)));

inline static void initialize() {
    handle.pragma.journal_mode(sqlite_orm::journal_mode::WAL);
    // handle.busy_timeout(30000);
    handle.sync_schema();
}

class DatabaseManager
{
  public:
    inline static DatabaseManager& GetInstance() {
        static DatabaseManager instance;
        return instance;
    }

    template<typename Func>
    auto read(Func&& func) -> std::optional<decltype(func(handle))> {
        decltype(func(handle)) returns;

        int maxRetry = 3;
        for(int i = 0; i < maxRetry; i++) {
            try {
                std::shared_lock<std::shared_mutex> read_lock(rwMutex_);
                returns = func(handle);
                break;
            }
            catch(std::exception& e) {
                qWarning() << "read:" << e.what();
            }
            qWarning() << "retry:" << i << " sleep";
            using namespace std::chrono;
            std::this_thread::sleep_for(100ms);
        }

        return returns;
    }

    template<typename Func, typename Record>
    auto write(Func&& func, const Record& record) {
        using ReturnType = decltype(func(handle, record));

        auto promise     = std::make_shared<std::promise<ReturnType>>();
        auto future      = promise->get_future();

        {
            std::lock_guard<std::mutex> lock(writeQueueMutex_);
            writeQueue_.emplace([func, record, promise]() mutable {
                try {
                    if constexpr(std::is_void_v<ReturnType>) {
                        func(handle, record);
                        promise->set_value();
                    }
                    else {
                        promise->set_value(func(handle, record));
                    }
                }
                catch(...) {
                    promise->set_exception(std::current_exception());
                }
            });
        }

        writeQueueCv_.notify_one();
        return future.get();
    }

    // 写操作工作线程（串行处理所有写请求）
    void processWriteQueue() {
        while(!stopFlag_) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(writeQueueMutex_);
                writeQueueCv_.wait(lock, [this] { return !writeQueue_.empty() || stopFlag_; });

                if(stopFlag_ && writeQueue_.empty()) {
                    break;
                }

                task = std::move(writeQueue_.front());
                writeQueue_.pop();
            }

            // 写操作需要独占锁
            {
                std::unique_lock<std::shared_mutex> write_lock(rwMutex_);
                try {
                    task();
                }
                catch(const std::exception& e) {
                    qWarning() << "写操作异常: " << e.what();
                }
            }
        }
    }

  protected:
    DatabaseManager() {
        //
        writeWorker_ = std::thread(&DatabaseManager::processWriteQueue, this);
    }

    ~DatabaseManager() {
        stopFlag_ = true;

        writeQueueCv_.notify_all();
        if(writeWorker_.joinable()) {
            writeWorker_.join();
        }
    }

    Q_DISABLE_COPY_MOVE(DatabaseManager)

  private:
    std::queue<std::function<void()>> writeQueue_;
    std::mutex writeQueueMutex_;
    std::condition_variable writeQueueCv_;
    std::atomic_bool stopFlag_;
    std::shared_mutex rwMutex_;

    std::thread writeWorker_;
};
}

inline static void initialize() {
    internal::initialize();
}

template<typename T>
inline static std::optional<int64_t> countRecords() {
    return internal::DatabaseManager::GetInstance().read(
        [](auto&& storage) { return storage.template count<T>(); });
}

template<typename T>
inline static std::optional<std::vector<T>> getRecords() {
    return internal::DatabaseManager::GetInstance().read(
        [](auto& storage) { return storage.template get_all<T>(); });
}

template<typename T, typename Condition>
inline static std::optional<std::vector<T>> getRecords(Condition c) {
    return internal::DatabaseManager::GetInstance().read(
        [c](auto& storage) { return storage.template get_all<T>(c); });
}

template<typename T>
inline static int64_t insertRecord(const T& record) {
    return internal::DatabaseManager::GetInstance().write(
        [](auto& storage, const T& rec) {
            //
            return storage.template insert<T>(rec);
        },
        record);
}

template<typename T>
inline static void insertRecords(const std::vector<T>& records) {
    internal::DatabaseManager::GetInstance().write(
        [](auto& storage, const std::vector<T>& recs) {
            storage.begin_transaction();
            try {
                for(const auto& rec: recs) {
                    storage.template insert<T>(rec);
                }
                storage.commit();
            }
            catch(...) {
                storage.rollback();
                throw;
            }
        },
        records);
}

template<typename T>
inline static void updateRecord(const T& record) {
    internal::DatabaseManager::GetInstance().write(
        [](auto& storage, const T& rec) { storage.template update<T>(rec); },
        record);
}

template<typename T>
inline static void removeRecord(const T& record) {
    internal::DatabaseManager::GetInstance().write(
        [](auto& storage, const T& rec) { storage.template remove<T>(rec); },
        record);
}
}

