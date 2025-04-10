# Ортогональные стратегии для реализации паттерна Singleton

Данная библиотека представляет собой мощную и гибкую реализацию паттерна проектирования Singleton, основанную на идеях Андрея Александреску из его книги "Modern C++ Design" и библиотеки Loki. Особенностью этой реализации является использование ортогональных стратегий (политик), которые позволяют комбинировать различные аспекты поведения Singleton в зависимости от конкретных требований проекта.

## Основные возможности

- **Гибкая настройка** поведения через ортогональные политики
- **Потокобезопасность** с несколькими стратегиями синхронизации
- **Контроль создания экземпляра** (ленивая инициализация или немедленное создание)
- **Управление жизненным циклом** через различные стратегии уничтожения
- **Статическая типобезопасность** благодаря шаблонной архитектуре

## Ортогональные политики

Библиотека предлагает несколько независимых политик, которые можно комбинировать для создания различных вариантов Singleton:

### Политики времени создания

- **CreateOnFirstUse** - экземпляр создается при первом обращении к нему (ленивая инициализация)
- **CreateOnInit** - экземпляр создается сразу при инициализации

### Политики синхронизации в многопоточной среде

- **NoThreadSafety** - без синхронизации, для однопоточных приложений
- **ClassLevelLockable** - блокировка на уровне класса при доступе к экземпляру
- **DoubleCheckedLockable** - Double-Checked Locking, оптимизированная стратегия для многопоточной среды

### Политики уничтожения

- **DestroyOnExit** - экземпляр уничтожается при завершении программы
- **NeverDestroy** - экземпляр никогда не уничтожается (полезно для объектов, используемых до конца работы программы)

## Использование

### Базовый пример

```cpp
// Определение класса, который будет использоваться как Singleton
class Logger {
private:
    Logger() { std::cout << "Logger created\n"; }
    ~Logger() { std::cout << "Logger destroyed\n"; }
    
    // Предоставляем доступ к приватным конструкторам для Singleton
    friend class Singleton<Logger, CreateOnFirstUse, ClassLevelLockable, DestroyOnExit>;

public:
    void log(const std::string& message) {
        std::cout << "LOG: " << message << std::endl;
    }
};

// Определение типа синглтона с выбранными стратегиями
using ThreadSafeLogger = Singleton<Logger, CreateOnFirstUse, ClassLevelLockable, DestroyOnExit>;

// Использование синглтона
ThreadSafeLogger::getInstanceSpecialized().log("Сообщение для логирования");
```

### Выбор различных стратегий

```cpp
// Потокобезопасный синглтон с ленивой инициализацией
using ThreadSafeLogger = Singleton<Logger, CreateOnFirstUse, ClassLevelLockable, DestroyOnExit>;

// Синглтон с оптимизированной многопоточной стратегией Double-Checked Locking
using DCLLogger = Singleton<Logger, CreateOnFirstUse, DoubleCheckedLockable, DestroyOnExit>;

// Синглтон без потокобезопасности для однопоточных приложений
using UnsafeLogger = Singleton<Logger, CreateOnFirstUse, NoThreadSafety, DestroyOnExit>;

// Синглтон, создаваемый при инициализации и никогда не уничтожаемый
using PersistentLogger = Singleton<Logger, CreateOnInit, NoThreadSafety, NeverDestroy>;
```

## Преимущества

1. **Разделение ответственности** - каждая ортогональная политика отвечает только за свой аспект поведения Singleton
2. **Гибкость настройки** - легко комбинировать различные политики для создания Singleton под конкретные требования
3. **Безопасность типов** - ошибки конфигурации выявляются на этапе компиляции
4. **Расширяемость** - легко добавить новые политики, не изменяя основной код

## Требования

- Компилятор с поддержкой C++17 или выше
- Стандартная библиотека C++ (STL) для поддержки многопоточности

## Дополнительная информация

Эта реализация основана на принципах, описанных в книге Андрея Александреску "Modern C++ Design" (глава 6) и библиотеке Loki. Основная идея заключается в применении техники "policy-based design" (проектирование на основе политик), которая позволяет создавать высоко настраиваемые компоненты с нулевыми накладными расходами во время выполнения благодаря использованию шаблонов.

## Важные замечания

- При использовании `DoubleCheckedLockable` рекомендуется использовать `std::memory_order` для более тонкой настройки модели памяти в современных компиляторах
- Библиотека обеспечивает потокобезопасность создания и доступа к экземпляру, но не к методам самого класса
- При необходимости можно расширить библиотеку дополнительными политиками, например:
  - политики логирования
  - политики обработки ошибок
  - политики проверки корректности экземпляра

## Лицензия

Данная библиотека распространяется под лицензией MIT.
