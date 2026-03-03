#pragma once

#include <shared_mutex>
#include <stdexcept>
#include <thread>

#include <sqlite_orm/sqlite_orm.h>

namespace DataBase {
template<typename... Entities>
class SafeSqliteOrm{
    using Storage = decltype(sqlite_orm::make_storage("", sqlite_orm::make_table<Entities...>()));

  public:
    // 单例获取（线程安全）
    static SafeSqliteOrm& instance(const std::string& dbPath = "database.db") {
        static SafeSqliteOrm instance(dbPath);
        return instance;
    }

    // 禁止拷贝/移动
    SafeSqliteOrm(const SafeSqliteOrm&)             = delete;
    SafeSqliteOrm& operator= (const SafeSqliteOrm&) = delete;
    SafeSqliteOrm(SafeSqliteOrm&&)                  = delete;
    SafeSqliteOrm& operator= (SafeSqliteOrm&&)      = delete;

    // ========== 读操作：共享锁（多线程并发读） ==========
    template<typename Func>
    auto read(Func&& func, int retryCount = 3) {
        std::shared_lock<std::shared_mutex> lock(sharedMutex_);
        int attempt = 0;
        while(attempt < retryCount) {
            try {
                return func(storage_);
            }
            catch(const std::exception& e) {
                attempt++;
                if(attempt >= retryCount) {
                    throw std::runtime_error("Read locked after " + std::to_string(retryCount) +
                                             " retries: " + e.what());
                }
            }
        }
        throw std::runtime_error("Read operation failed with unknown reason");
    }

    // ========== 写操作：独占锁（原子性） ==========
    template<typename Func>
    auto write(Func&& func, int retryCount = 3) {
        std::unique_lock<std::shared_mutex> lock(sharedMutex_);
        int attempt = 0;
        while(attempt < retryCount) {
            try {
                // 自动事务：成功提交，失败回滚
                return storage_.transaction([&]() { return func(storage_); });
            }
            catch(const std::exception& e) {
                attempt++;
                if(attempt >= retryCount) {
                    throw std::runtime_error("Write locked after " + std::to_string(retryCount) +
                                             " retries: " + e.what());
                }
            }
        }
        throw std::runtime_error("Write operation failed with unknown reason");
    }

    // 获取原始storage（谨慎使用）
    auto& get_storage() { return storage_; }

    // 手动检查当前journal模式（调试用）
    std::string getJournalMode() {
        std::shared_lock<std::shared_mutex> lock(sharedMutex_);
        return storage_.pragma.journal_mode(); // 调用无参重载，返回字符串
    }

  private:
    // 私有构造函数：初始化数据库连接+配置WAL
    explicit SafeSqliteOrm(const std::string& dbPath) {
        // 第一步：创建storage实例（仅注册表+基础配置）
        storage_ = sqlite_orm::make_storage(dbPath,
                                            sqlite_orm::make_table<Entities...>() // 注册你的实体类
        );

        // 第二步：初始化表结构（创建不存在的表）
        storage_.sync_schema();

        // 第三步：设置WAL模式（调用void版本的journal_mode）
        storage_.pragma.journal_mode(sqlite_orm::journal_mode::WAL); // 无返回值，正确调用
    }

    // sqlite_orm的storage实例（全局唯一）
    Storage storage_;

    // 共享互斥锁：读共享、写独占
    std::shared_mutex sharedMutex_;
};
}
