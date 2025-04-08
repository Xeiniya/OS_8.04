#include <mutex>
#include <atomic>
#include <iostream>
#include <memory>

// Политики времени создания
class CreateOnFirstUse {};
class CreateOnInit {};

// Политики уничтожения
class DestroyOnExit {};
class NeverDestroy {};

// Политики синхронизации в многопоточной среде
class NoThreadSafety {
public:
    void lock() const {}
    void unlock() const {}
};

class ClassLevelLockable {
private:
    mutable std::mutex mtx;
public:
    void lock() const { mtx.lock(); }
    void unlock() const { mtx.unlock(); }
};

class DoubleCheckedLockable {
private:
    mutable std::mutex mtx;
    mutable std::atomic<bool> initialized{false};
public:
    bool isInitialized() const { return initialized; }
    void setInitialized() const { initialized = true; }
    void lock() const { mtx.lock(); }
    void unlock() const { mtx.unlock(); }
};

// Обертка для автоматической блокировки
template <typename T>
class LockGuard {
private:
    const T& lockable;
public:
    explicit LockGuard(const T& obj) : lockable(obj) {
        lockable.lock();
    }
    ~LockGuard() {
        lockable.unlock();
    }
};

// Основной шаблон синглтона с ортогональными стратегиями
template <
    typename T,
    template <typename> class CreationPolicy = CreateOnFirstUse,
    typename ThreadingModel = ClassLevelLockable,
    template <typename> class LifetimePolicy = DestroyOnExit
>
class Singleton {
private:
    static T* instance;
    static ThreadingModel mutex;
    
    // Защита от копирования и присваивания
    Singleton() = delete;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    
    // Реализации политик

    // Создание на первом использовании
    template <typename U>
    static T* createInstance(CreateOnFirstUse<U>) {
        static T obj;
        return &obj;
    }
    
    // Создание при инициализации
    template <typename U>
    static T* createInstance(CreateOnInit<U>) {
        static T* ptr = new T();
        return ptr;
    }
    
    // Фикция для компиляции - использована общая стратегия создания
    static T* createInstance() {
        return createInstance(CreationPolicy<T>());
    }
    
    // Политика уничтожения при завершении программы
    template <typename U>
    static void destroyInstance(DestroyOnExit<U>) {
        if (instance != nullptr && !std::is_same<CreationPolicy<U>, CreateOnFirstUse<U>>::value) {
            delete instance;
            instance = nullptr;
        }
    }
    
    // Политика никогда не уничтожать
    template <typename U>
    static void destroyInstance(NeverDestroy<U>) {
        // Ничего не делать
    }
    
    // Класс для автоматического уничтожения при выходе
    class Destroyer {
    public:
        ~Destroyer() {
            destroyInstance(LifetimePolicy<T>());
        }
    };
    
    static Destroyer destroyer;
    
public:
    // Получение экземпляра с обычной блокировкой
    static T& getInstance() {
        if (instance == nullptr) {
            LockGuard<ThreadingModel> guard(mutex);
            if (instance == nullptr) {
                instance = createInstance();
            }
        }
        return *instance;
    }
    
    // Специализированная реализация для DoubleCheckedLockable
    static T& getInstanceDoubleChecked(const DoubleCheckedLockable& model) {
        if (!model.isInitialized()) {
            LockGuard<DoubleCheckedLockable> guard(model);
            if (!model.isInitialized()) {
                if (instance == nullptr) {
                    instance = createInstance();
                }
                model.setInitialized();
            }
        }
        return *instance;
    }
    
    // Общий метод получения экземпляра с учетом стратегии синхронизации
    static T& getInstanceSpecialized() {
        if constexpr (std::is_same<ThreadingModel, DoubleCheckedLockable>::value) {
            return getInstanceDoubleChecked(mutex);
        } else {
            return getInstance();
        }
    }
};

// Инициализация статических членов
template <typename T, template <typename> class C, typename TM, template <typename> class L>
T* Singleton<T, C, TM, L>::instance = nullptr;

template <typename T, template <typename> class C, typename TM, template <typename> class L>
TM Singleton<T, C, TM, L>::mutex;

template <typename T, template <typename> class C, typename TM, template <typename> class L>
typename Singleton<T, C, TM, L>::Destroyer Singleton<T, C, TM, L>::destroyer;

// Пример использования

// Тестовый класс, который будет использоваться как Singleton
class Logger {
private:
    Logger() { std::cout << "Logger created\n"; }
    ~Logger() { std::cout << "Logger destroyed\n"; }
    
    friend class Singleton<Logger, CreateOnFirstUse, ClassLevelLockable, DestroyOnExit>;
    friend class Singleton<Logger, CreateOnFirstUse, DoubleCheckedLockable, DestroyOnExit>;
    friend class Singleton<Logger, CreateOnFirstUse, NoThreadSafety, DestroyOnExit>;

public:
    void log(const std::string& message) {
        std::cout << "LOG: " << message << std::endl;
    }
};

// Определение конкретных типов синглтонов с разными стратегиями
using ThreadSafeLogger = Singleton<Logger, CreateOnFirstUse, ClassLevelLockable, DestroyOnExit>;
using DCLLogger = Singleton<Logger, CreateOnFirstUse, DoubleCheckedLockable, DestroyOnExit>;
using UnsafeLogger = Singleton<Logger, CreateOnFirstUse, NoThreadSafety, DestroyOnExit>;

// Демонстрация использования
int main() {
    std::cout << "=== Использование Thread-Safe синглтона ===\n";
    ThreadSafeLogger::getInstanceSpecialized().log("Сообщение из thread-safe логгера");
    
    std::cout << "\n=== Использование Double-Checked Locking синглтона ===\n";
    DCLLogger::getInstanceSpecialized().log("Сообщение из DCL логгера");
    
    std::cout << "\n=== Использование простого синглтона без потокобезопасности ===\n";
    UnsafeLogger::getInstanceSpecialized().log("Сообщение из небезопасного логгера");
    
    std::cout << "\n=== Завершение программы ===\n";
    return 0;
}
