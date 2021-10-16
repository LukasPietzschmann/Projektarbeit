#define FLXC_MAIN 2021'10'15

#include <readline/readline.h>
#include <iostream>

using namespace std;

#include "scanner.h"
#include "predef.h"
#include "parser.h"
#include "eval.h"

int repl();
void eval_exprs(const seq<Expr>&);

// Hauptprogramm.
int main (int argc, char* argv []) {
    if(argc == 1)
        return repl();

    // Quelldatei öffnen und einlesen.
    str filename = argv[1];
    scan_open(filename);

    // Vordefinierte Operatoren erzeugen.
    seq<Oper> opers = predef();

    // Parser ausführen.
    eval_exprs(parse(opers));
}

int repl() {
#ifdef HAS_READLINE_LIB
    static char const* prompt = "flx > ";
    rl_bind_key('\t', rl_insert);

    std::cout << "MOSTflexiPL REPL\n";
    std::cout << "Type 'exit' to exit\n";
    std::cout << std::flush;

    while(true) {
        char* inputLine = readline(prompt);
        if(!inputLine || std::strcmp(inputLine, "exit") == 0) {
            delete[] inputLine;
            break;
        }

        add_history(inputLine);
        scan_string(inputLine);
        eval_exprs(parse(predef()));

        delete[] inputLine;
  }
#else
    std::cout << "GNU Readline not found" << std::endl;
#endif
  return 0;
}

void eval_exprs(const seq<Expr>& expressions){
    int i = 0;
    // Erkannte Ausdrücke ausgeben und auswerten.
    for (Expr expr : expressions) {
        Value val = exec(expr); // exec statt eval!
        cout << "Expression " << ++i << " with value ";
        if (nat(val)) cout << val(intval_);
        else if (val) cout << "synth";
        else cout << "nil";
        cout << endl;
    }
}