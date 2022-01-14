#define FLXC_MAIN 2021'10'15

#include <iostream>


/* Readline
 *
 * stdio.h muss vor readline.h definiert sein
 * um Compiler-Errors zu vermeiden. Nicht entfernen!!!
 */
#ifdef HAS_READLINE_LIB
#include <cstdio>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifdef HAS_CURSES_LIB
#include <ncurses.h>
#endif

#include <cxxopts.hpp>

using namespace std;

#include "scanner.h"
#include "predef.h"
#include "parser.h"
#include "eval.h"
#include "visualizer/main.hpp"

int repl();
void eval_exprs(const seq<Expr>&);

// Hauptprogramm.
int main (int argc, char* argv []) {
  std::ostream::sync_with_stdio(false);
  cxxopts::Options options(argv[0], "The MOSTflexiPL programming language");
  options.add_options()("f,file", "Die auszuführende Datei", cxxopts::value<std::string>())
                       ("debug-parser", "Visualisiert den Parsvorgang")
                       ("h,help", "Zeigt diese Nachricht an");
  options.parse_positional({"file"});
  options.positional_help("[FILE]").show_positional_help();

  cxxopts::ParseResult result;
  try {
     result = options.parse(argc, argv);
  }catch(const cxxopts::OptionException&){
    std::cout << options.help() << std::endl;
    return 1;
  }

  if(result.count("help")) {
    std::cout << options.help() << std::endl;
    return 0;
  }

  if(!result.count("file"))
    return repl();

    // Quelldatei öffnen und einlesen.
    str filename = result["file"].as<std::string>().c_str();
    scan_open(filename);

    // Vordefinierte Operatoren erzeugen.
    seq<Oper> opers = predef();

    // Parser ausführen.
    eval_exprs(parse(opers));
	if(result.count("debug-parser"))
		start_visualizer(scan_str);
}

int repl() {
#ifdef HAS_READLINE_LIB
    static char const* prompt = "flx > ";
    static char const* exit = "exit";
    rl_bind_key('\t', rl_insert);

    std::cout << "MOSTflexiPL REPL\n";
    std::cout << "Type 'exit' to exit\n";
    std::cout << std::flush;

    while(true) {
        char* inputLine = readline(prompt);
        if(!inputLine || std::strncmp(inputLine, exit, strlen(exit)) == 0) {
            delete[] inputLine;
            break;
        }

        add_history(inputLine);
        scan_string(inputLine);
        clear_tab();
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