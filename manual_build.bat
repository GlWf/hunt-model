mkdir .build
mkdir bin

gcc -O2 -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -funsigned-char -funsigned-bitfields -fshort-enums  -c console.c -o .build/console.o
gcc -O2 -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -funsigned-char -funsigned-bitfields -fshort-enums  -c view.c -o .build/view.o
gcc -O2 -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -funsigned-char -funsigned-bitfields -fshort-enums  -c config.c -o .build/config.o
gcc -O2 -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -funsigned-char -funsigned-bitfields -fshort-enums  -c model.c -o .build/model.o
gcc -O2 -std=gnu99 -Wall -Wextra -Wno-unknown-pragmas -funsigned-char -funsigned-bitfields -fshort-enums  -c control.c -o .build/control.o
gcc -static -Xlinker --no-insert-timestamp  .build/console.o .build/view.o .build/config.o .build/model.o .build/control.o -o .build/hunt-model.exe
copy .build\hunt-model.exe bin\hunt-model.exe
