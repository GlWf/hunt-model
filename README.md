# Курсовая работа


## Сборка

Для сборки нужен компилятор gcc mingw-w64 
[https://github.com/niXman/mingw-builds-binaries/releases](https://github.com/niXman/mingw-builds-binaries/releases)
i686-xx.x.x-release-posix-dwarf-msvcrt-rt (где xx.x.x - версия компилятора). 

В качестве системы сборки можно использовать [ninja](https://ninja-build.org) (скрипт `1.bat`) 
или скрипт `manual_build.bat` если по каким-то причинам использовать `ninja` невозможно.

## Запуск

В результате сборки будет создан исполняемый файл `bin\hunt-model.exe`,
который можно запускать напрямую или при помощи скрипта `2.bat` в корне репозитория. 