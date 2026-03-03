// .h
#pragma once

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

#include <array>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace PythonConcept {
// 检查多个类型其一
template<typename T, typename... Args>
inline constexpr bool kIsOneOf = (std::is_same_v<std::decay_t<T>, Args> || ...);

// void
template<typename T>
inline constexpr bool kIsVoid = std::is_void_v<std::decay_t<T>>;
// nullptr
template<typename T>
inline constexpr bool kIsNullptr = std::is_same_v<std::decay_t<T>, std::nullptr_t>;
// 算数类型
template<typename T>
inline constexpr bool kIsArithmetic = std::is_arithmetic_v<std::decay_t<T>>;
// 字符串类型
template<typename T>
inline constexpr bool kIsString = kIsOneOf<T, std::string, std::string_view, const char*, char*>;

// pybind11类型
template<typename T>
inline constexpr bool kIsPybindObj = kIsOneOf<T,
                                              py::handle,
                                              py::object,
                                              py::bool_,
                                              py::int_,
                                              py::float_,
                                              py::str,
                                              py::bytes,
                                              py::tuple,
                                              py::list,
                                              py::dict,
                                              py::slice,
                                              py::none,
                                              py::capsule,
                                              py::iterable,
                                              py::function,
                                              py::buffer>;

// numpy类型
template<typename T>
inline constexpr bool kIsNumpyArray = kIsOneOf<T,
                                               py::array,
                                               py::array_t<char>,
                                               py::array_t<int8_t>,
                                               py::array_t<uint8_t>,
                                               py::array_t<int16_t>,
                                               py::array_t<uint16_t>,
                                               py::array_t<int32_t>,
                                               py::array_t<uint32_t>,
                                               py::array_t<float>,
                                               py::array_t<double>,
                                               py::array_t<bool>>;

// ============================================================================
// STL 容器识别（无递归）
// ============================================================================

template<typename T>
struct IsStlContainer: std::false_type {
};

template<typename... Args>
struct IsStlContainer<std::vector<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::list<Args...>>: std::true_type {
};

template<typename T, size_t N>
struct IsStlContainer<std::array<T, N>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::map<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::unordered_map<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::set<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::unordered_set<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::pair<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::tuple<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::optional<Args...>>: std::true_type {
};

template<typename... Args>
struct IsStlContainer<std::variant<Args...>>: std::true_type {
};

template<typename T>
inline constexpr bool kIsStlContainer = IsStlContainer<std::decay_t<T>>::value;

// 元素类型提取（添加 pair 和 tuple 的特化）
template<typename T>
struct ElementType {
    using type = T;
};

template<typename T>
struct ElementType<std::vector<T>> {
    using type = T;
};

template<typename T>
struct ElementType<std::list<T>> {
    using type = T;
};

template<typename T, size_t N>
struct ElementType<std::array<T, N>> {
    using type = T;
};

template<typename K, typename V>
struct ElementType<std::map<K, V>> {
    using type = std::pair<K, V>;
};

template<typename K, typename V>
struct ElementType<std::unordered_map<K, V>> {
    using type = std::pair<K, V>;
};

template<typename T>
struct ElementType<std::set<T>> {
    using type = T;
};

template<typename T>
struct ElementType<std::unordered_set<T>> {
    using type = T;
};

template<typename T>
struct ElementType<std::optional<T>> {
    using type = T;
};

// pair 和 variant 作为叶子节点，不继续提取元素
template<typename T1, typename T2>
struct ElementType<std::pair<T1, T2>> {
    using type = std::pair<T1, T2>;
};

template<typename... Ts>
struct ElementType<std::variant<Ts...>> {
    using type = std::variant<Ts...>;
};

// 主模板：基础类型
template<typename T>
struct ArgChecker {
    inline static constexpr bool value =
        kIsArithmetic<T> || kIsString<T> || kIsNullptr<T> || kIsPybindObj<T> || kIsNumpyArray<T>;
};

// 特化：容器类型（递归检查元素）
template<typename T>
    requires kIsStlContainer<T> &&
             (!std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>> ||
              !requires { typename T::first_type; })
struct ArgChecker<T> {
    inline static constexpr bool value = ArgChecker<ElementType<T>>::value;
};

// pair 特化：检查两个元素
template<typename T1, typename T2>
struct ArgChecker<std::pair<T1, T2>> {
    inline static constexpr bool value = ArgChecker<T1>::value && ArgChecker<T2>::value;
};

// tuple 特化：检查所有元素
template<typename... Ts>
struct ArgChecker<std::tuple<Ts...>> {
    inline static constexpr bool value = (ArgChecker<Ts>::value && ...);
};

// variant 特化：检查所有备选类型
template<typename... Ts>
struct ArgChecker<std::variant<Ts...>> {
    inline static constexpr bool value = (ArgChecker<Ts>::value && ...);
};

// 辅助常量
template<typename T>
inline constexpr bool kIsValidPyArg = ArgChecker<T>::value;

// 函数参数
template<typename T>
concept Pybind11Arg = kIsValidPyArg<T> || kIsVoid<T>;

// 函数返回值
template<typename T>
concept Pybind11ReturnType =
    !kIsVoid<T> && (kIsArithmetic<T> || kIsString<T> || kIsNullptr<T> || kIsPybindObj<T> ||
                    kIsNumpyArray<T> || kIsStlContainer<T>);
template<typename T>
concept Pybind11ReturnVoid = kIsVoid<T>;
}

namespace Python {
namespace internal {
class PythonWrapper
{
  public:
    inline static PythonWrapper& GetInstance() {
        // std::call_once(once_, [] { instance_.reset(new PythonWrapper); });
        // return *instance_;
        static PythonWrapper instance;
        return instance;
    }

    ~PythonWrapper() {
        //
        qDebug() << "~PythonWrapper";
        shutdown();
    }

    void appendPath(const QString& path) const {
        // GIL gil;
        try {
            py::gil_scoped_acquire a;

            auto sys = py::module_::import("sys");
            sys.attr("path").attr("append")(path.toUtf8().constData());
            qDebug() << QString("Python version: %1").arg(sys.attr("version").cast<std::string>());
        }
        catch(const py::error_already_set& e) {
            qWarning() << "Failed to append Python path:" << e.what();
        }
    }

    std::optional<py::object> callFunction(const QString& module, const QString& func) const;

    template<PythonConcept::Pybind11ReturnType ReturnType, PythonConcept::Pybind11Arg... Args>
    [[nodiscard]] std::optional<ReturnType> call(const QString& module,
                                                 const QString& func,
                                                 Args&&... args);

    template<PythonConcept::Pybind11Arg... Args>
    void call(const QString& module, const QString& func, Args&&... args);

    void shutdown() {
        if(finish_) {
            return;
        }
        if(release_) {
            release_.reset();
            // release_.release();
        }

        try {
            // 强制 Python 模块尝试清理自己
            py::exec("import sys; sys.modules.clear()");
            auto gc = py::module_::import("gc");
            gc.attr("collect")();

            py::detail::get_internals().registered_types_cpp.clear();
            PyErr_Clear();
        }
        catch(const py::error_already_set& e) {
            qWarning() << e.what();
        }

        py::finalize_interpreter();
        finish_ = true;

        // qDebug() << "Python Interpreter finalized safely.";
    }

  private:
    template<typename... Args>
    py::object doCall(const QString& module, const QString& func, Args&&... args);

  protected:
    PythonWrapper() {
        py::initialize_interpreter();
        release_.reset(new py::gil_scoped_release);
        // qDebug() << "GIL STATE:" << PyGILState_Check();
    }

    Q_DISABLE_COPY_MOVE(PythonWrapper)

  private:
    // py::scoped_interpreter guard_ {};
    // mutable QMutex mutex_; // 添加互斥锁保护Python调用
    std::atomic_bool finish_ = false;
    // inline static std::once_flag once_;
    // inline static std::unique_ptr<PythonWrapper> instance_;

    std::unique_ptr<py::gil_scoped_release> release_;
};

template<PythonConcept::Pybind11ReturnType ReturnType, PythonConcept::Pybind11Arg... Args>
std::optional<ReturnType> PythonWrapper::call(const QString& module,
                                              const QString& func,
                                              Args&&... args) {
    try {
        py::gil_scoped_acquire a;

        auto result = doCall(module, func, std::forward<Args>(args)...);
        if(result.is_none()) {
            return {};
        }

        if constexpr(std::is_same_v<ReturnType, py::object>) {
            return result;
        }
        else {
            return result.template cast<ReturnType>();
        }
    }
    catch(const py::error_already_set& e) {
        qWarning() << QString("Python exception in module: %1, function: %2, exception: %3")
                          .arg(module, func, e.what());
    }
    catch(const std::exception& e) {
        qWarning() << QString("Exception in call module: %1, function: %2, exception: %3")
                          .arg(module, func, e.what());
    }
    catch(...) {
        qFatal() << QString("Unkown exception in call module: %1, function: %2").arg(module, func);
    }

    return {};
}

template<PythonConcept::Pybind11Arg... Args>
void PythonWrapper::call(const QString& module, const QString& func, Args&&... args) {
    try {
        py::gil_scoped_acquire a;
        (void)doCall(module, func, std::forward<Args>(args)...);
    }
    catch(const py::error_already_set& e) {
        qWarning() << QString("Python exception in module: %1, function: %2, exception: %3")
                          .arg(module, func, e.what());
    }
    catch(const std::exception& e) {
        qWarning() << QString("Exception in call module: %1, function: %2, exception: %3")
                          .arg(module, func, e.what());
    }
    catch(...) {
        qFatal() << QString("Unkown exception in call module: %1, function: %2").arg(module, func);
    }
}

template<typename... Args>
py::object PythonWrapper::doCall(const QString& module, const QString& func, Args&&... args) {
    // py::gil_scoped_acquire r;可以保留，也可以注释，只要上层调用时正确获取了gil
    auto m            = py::module_::import(module.toUtf8().constData());
    py::object result = m.attr(func.toUtf8().constData())(std::forward<Args>(args)...);

    return result;
}
}

inline static void initialize(const QString& path) {
    internal::PythonWrapper::GetInstance().appendPath(path);
}

// inline static void finalize() {
//     internal::PythonWrapper::GetInstance().shutdown();
// }

template<PythonConcept::Pybind11ReturnType ReturnType, PythonConcept::Pybind11Arg... Args>
[[nodiscard]] inline static std::optional<ReturnType> call(const QString& module,
                                                           const QString& func,
                                                           Args&&... args) {
    return internal::PythonWrapper::GetInstance().call<ReturnType>(module,
                                                                   func,
                                                                   std::forward<Args>(args)...);
}

template<PythonConcept::Pybind11Arg... Args>
inline static void call(const QString& module, const QString& func, Args&&... args) {
    internal::PythonWrapper::GetInstance().call(module, func, std::forward<Args>(args)...);
}
}
