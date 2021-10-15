# MOSTflexiPL
Compiler für die erweiterbare Programmiersprache MOSTflexiPL

## Getting started

Dieses Projekt verwendet CMake für den Build- und Linkprozess.
Nach dem Herunterladen mittels

```bash
git clone git@github.com:r4gus/MOSTflexiPL.git
```

kann innerhalb des `Compiler` Verzeichnisses ein neuer Ordner mit dem
Namen `build` erstellt werden. Innerhalb des `build` Ordners kann diser
mit `cmake ..` initialisiert werden. Danach muss nur noch `make` ausgeführt
werden. Nach erfolgreichem compilieren und linken befinden sich die Executables
in `Compiler/build/bin` und die Bibliotheken in `Compiler/build/lib`.

> **Achtung:** Das Projekt verwendet automatisch Google Test und läd dies
> beim Ausführen von `cmake ..` in den `build` Ordner herunter um es
> später mit dem restlichen Projekt zu compilieren und zu linken. Sollte
> dies nicht gewünscht werden kann CMake mit `cmake -DWITH_TESTS=OFF ..`
> aufgerufen werden.
