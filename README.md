# Logger Library & Interactive Application

Многопоточная библиотека для записи сообщений в журнал с уровнями важности и интерактивное консольное приложение для работы с ней.

## Особенности
```
- **Три уровня важности**: `DEBUG`, `INFO`, `ERROR`
- **Фильтрация** – сообщения ниже установленного уровня не записываются
- **Многопоточность** – потокобезопасная запись с использованием мьютексов
- **Асинхронная запись** – интерактивное приложение использует очередь команд и отдельный рабочий поток
- **Кроссплатформенность** – работает на Linux и Windows (MinGW / WSL)
- **Только STL** – без сторонних библиотек
- **Обработка ошибок** – корректная реакция на ошибки открытия/записи файла
```
## Требования
```
- Компилятор с поддержкой C++17 (GCC 7+)
- Утилита `make`
- Для Windows: Git Bash + MinGW или WSL (рекомендуется)
```
## Сборка

```bash
make all          # собрать всё (библиотеку, демо, приложение, тесты)
make demo         # запустить демонстрацию работы библиотеки
make app          # запустить интерактивное приложение
make test         # запустить модульные тесты
make clean        # удалить временные файлы и папки build/, lib/, bin/
make distclean    # полная очистка (включая собранные исполняемые файлы)
```


## Пример использования в коде C++:
```
#include "logger.h"

int main() {
    Logger::init("app.log", Logger::LogLevel::INFO);
    Logger::info("Application started");
    Logger::debug("Debug message (не будет записана, так как уровень DEBUG < INFO)");
    Logger::error("Error message");
    Logger::setDefaultLevel(Logger::LogLevel::DEBUG);
    Logger::debug("Теперь DEBUG сообщения записываются");
    Logger::shutdown();
    return 0;
}
```

## Формат записи в журнал:
```
2026-07-20 20:41:45.067 [INFO] Application started
2026-07-20 20:41:45.331 [INFO] User logged in
2026-07-20 20:41:45.346 [ERROR] Database connection lost
```

## Запуск интерактивного приложения:
```
./bin/logger_app -f mylog.log -l DEBUG
```

## Параметры:
```
-f, --file FILE – имя файла журнала (по умолчанию app.log)

-l, --level LEVEL – уровень по умолчанию (DEBUG, INFO, ERROR, по умолчанию INFO)

-h, --help – справка
```

## Интерактивные команды:
```
message – запись с уровнем по умолчанию

DEBUG:message или DEBUG message – запись с уровнем DEBUG

INFO:message или INFO message – запись с уровнем INFO

ERROR:message или ERROR message – запись с уровнем ERROR

help, h, ? – вывод справки

status, s – вывод текущего состояния

clear – очистка экрана

exit, q – выход из приложения
```

## Пример сеанса:
```
> Application started
> INFO:User logged in
> ERROR:Database connection failed
> DEBUG:Processing request #42
> status
> exit
```

## Тестирование:
```
make test
```

## Запускаются модульные тесты, проверяющие инициализацию, фильтрацию по уровню и многопоточную запись. Ожидаемый вывод:
```
=== Running tests ===
Testing init...
  OK
Testing levels...
  OK
Testing multithread...
  OK
All tests passed!
```

## Структура проекта:
```
.
├── include/
│   └── logger.h              # Публичный заголовок библиотеки
├── src/
│   ├── logger.cpp            # Реализация библиотеки
│   ├── logger_app.h          # Заголовок интерактивного приложения
│   ├── logger_app.cpp        # Реализация интерактивного приложения
│   ├── main.cpp              # Демонстрационное приложение
│   └── main_app.cpp          # Точка входа интерактивного приложения
├── tests/
│   └── test_logger.cpp       # Модульные тесты
├── Makefile                  # Система сборки
├── README.md                 # Данный файл
└── .gitignore                # Исключённые файлы
```