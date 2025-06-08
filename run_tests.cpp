#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <filesystem>
#include <iostream>

#include "./core/grammar.cpp"
#include "./core/automata.cpp"
#include "./core/parsers.cpp"


using namespace std;

// Loads all .hulk test files from ./tests/ directory
vector<pair<string, string>> load_tests() {
    vector<pair<string, string>> tests;
    for (const auto& entry : filesystem::directory_iterator("./test/")) {
        if (entry.path().extension() == ".hulk") {
            ifstream file(entry.path());
            if (file) {
                string content((istreambuf_iterator<char>(file)),
                                istreambuf_iterator<char>());
                tests.push_back(std::make_pair(entry.path().filename().string(), content));
            }
        }
    }
    return tests;
}







void test1() {
    // Definir simbolos terminales
    Symbol plus("+", true);
    Symbol minus("-", true);
    Symbol star("*", true);
    Symbol div("/", true);
    Symbol opar("(", true);
    Symbol cpar(")", true);
    Symbol num("num", true);

    // Definir simbolos no terminales
    Symbol E("E", false);
    Symbol T("T", false);
    Symbol F("F", false);
    Symbol X("X", false);
    Symbol Y("Y", false);

    // Conjuntos de terminales y no terminales
    unordered_set<Symbol> terminals = {plus, minus, star, div, opar, cpar, num};
    unordered_set<Symbol> nonTerminals = {E, T, F, X, Y};

    // Producciones
    vector<Production> productions = {
        // E -> T X
        Production(E, Sentence({T, X})),
        // X -> + T X
        Production(X, Sentence({plus, T, X})),
        // X -> - T X
        Production(X, Sentence({minus, T, X})),
        // X -> ε
        Production(X, Sentence()),
        // T -> F Y
        Production(T, Sentence({F, Y})),
        // Y -> * F Y
        Production(Y, Sentence({star, F, Y})),
        // Y -> / F Y
        Production(Y, Sentence({div, F, Y})),
        // Y -> ε
        Production(Y, Sentence()),
        // F -> num
        Production(F, Sentence({num})),
        // F -> ( E )
        Production(F, Sentence({opar, E, cpar}))
    };

    // Crear la gramatica
    Grammar grammar(terminals, nonTerminals, E, productions);

    // Imprimir la gramatica
    cout << "=== GRAMATICA DE EXPRESIONES ===\n";
    grammar.print();

    auto firsts = computeFirsts(grammar);
    auto follows = computeFollows(grammar, firsts);
    printFirstAndFollow(grammar, firsts, follows);


}


// TODO : falta gramatica

LL1Parser parser (Grammar& G) {
    LL1Parser _parser(G);
    return _parser;
}



int main(int argc, char const *argv[]) {
    //test1();
    //test_LL1Parser();
    // _parser = parser(G);
    // _parser.parse(tokenizer_result);
    return 0;
}

