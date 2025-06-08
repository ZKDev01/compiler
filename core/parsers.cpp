#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <stack>
#include <stdexcept>



//#include "grammar.cpp"

using namespace std;

/**
 * Calcula First para una sentencia alpha
 */
ContainerSet computeLocalFirst(const unordered_map<Symbol, 
                               ContainerSet>& firsts, 
                               const Sentence& alpha) {
    ContainerSet first_alpha;
    
    if (alpha.isEpsilon()) {
        first_alpha.setEpsilon();
    } else {
        for (const Symbol& symbol : alpha) {
            auto it = firsts.find(symbol);
            if (it != firsts.end()) {
                first_alpha.update(it->second);
                if (!it->second.containsEpsilon()) {
                    break;
                }
            }
        }
        
        // Si todos los símbolos pueden derivar epsilon, añadir epsilon
        bool all_can_derive_epsilon = true;
        for (const Symbol& symbol : alpha) {
            auto it = firsts.find(symbol);
            if (it == firsts.end() || !it->second.containsEpsilon()) {
                all_can_derive_epsilon = false;
                break;
            }
        }
        
        if (all_can_derive_epsilon) {
            first_alpha.setEpsilon();
        }
    }
    
    return first_alpha;
}

/**
 * Algoritmo principal para calcular First(G) donde G es la Gramatica 
 */
unordered_map<Symbol, ContainerSet> computeFirsts(const Grammar& G) {
    unordered_map<Symbol, ContainerSet> firsts;
    unordered_map<Sentence, ContainerSet> sentence_firsts;
    
    bool change = true;
    
    // Inicializar First(Vt) - cada terminal tiene como First a si mismo
    for (const Symbol& terminal : G.getTerminals()) {
        firsts[terminal] = ContainerSet(terminal);
    }
    
    // Inicializar First(Vn) - cada no terminal empieza con conjunto vacio
    for (const Symbol& nonTerminal : G.getNonTerminals()) {
        firsts[nonTerminal] = ContainerSet();
    }
    
    while (change) {
        change = false;
        
        // Para cada producción X -> alpha
        for (const Production& production : G.getProductions()) {
            const Symbol& X = production.getLeft();
            const Sentence& alpha = production.getRight();
            
            // Obtener First(X) actual
            ContainerSet& first_X = firsts[X];
            
            // Inicializar First(alpha) si no existe
            if (sentence_firsts.find(alpha) == sentence_firsts.end()) {
                sentence_firsts[alpha] = ContainerSet();
            }
            ContainerSet& first_alpha = sentence_firsts[alpha];
            
            // Calcular First local de alpha
            ContainerSet local_first = computeLocalFirst(firsts, alpha);
            
            // Actualizar First(alpha) y First(X)
            change |= first_alpha.hardUpdate(local_first);
            change |= first_X.hardUpdate(local_first);
        }
    }
    
    return firsts;
}


/**
 * Algoritmo principal para calcular Follow(G)
 * Basado en las reglas:
 * 1. Follow(S) contiene $
 * 2. Para B -> αAβ: Follow(A) ∪= (First(β) - {ε})
 * 3. Para B -> αA o B -> αAβ donde ε ∈ First(β): Follow(A) ∪= Follow(B)
 */
unordered_map<Symbol, ContainerSet> computeFollows(const Grammar& G, 
                                                   const unordered_map<Symbol, ContainerSet>& firsts) {
    unordered_map<Symbol, ContainerSet> follows;
    bool change = true;
    
    // Simbolo EOF (final de fichero)
    Symbol EOF_SYMBOL("$", true);
    
    // Inicializar Follow(Vn) - cada no terminal empieza con conjunto vacio
    for (const Symbol& nonTerminal : G.getNonTerminals()) {
        follows[nonTerminal] = ContainerSet();
    }
    
    // Follow(S) contiene $ (simbolo de inicio)
    follows[G.getStartSymbol()].insert(EOF_SYMBOL);
    

    while (change) {
        change = false;
        
        // Para cada producción X -> alpha
        for (const Production& production : G.getProductions()) {
            const Symbol& X = production.getLeft();
            const Sentence& alpha = production.getRight();
            
            // Si la producción es epsilon, continuar
            if (alpha.isEpsilon()) {
                continue;
            }
            
            ContainerSet& follow_X = follows[X];
            size_t n = alpha.size();
            
            // Recorrer todos los símbolos de la parte derecha
            for (size_t i = 0; i < n; ++i) {
                const Symbol& Y = alpha[i];
                
                // Solo procesamos no terminales
                if (!Y.isNonTerminal()) {
                    continue;
                }
                
                ContainerSet& follow_Y = follows[Y];
                
                // Caso: X -> ζ Y β (hay símbolos después de Y)
                if (i < n - 1) {
                    // Crear la cadena β (símbolos después de Y)
                    vector<Symbol> beta_symbols;
                    for (size_t j = i + 1; j < n; ++j) {
                        beta_symbols.push_back(alpha[j]);
                    }
                    Sentence beta(beta_symbols);
                    
                    // Calcular First(β)
                    ContainerSet first_beta = computeLocalFirst(firsts, beta);
                    
                    // Regla S1: Follow(Y) ∪= (First(β) - {ε})
                    ContainerSet first_beta_no_epsilon = first_beta - unordered_set<std::string>{EPSILON, "epsilon"};
                    change |= follow_Y.hardUpdate(first_beta_no_epsilon);
                    
                    // Regla S2: Si ε ∈ First(β), entonces Follow(Y) ∪= Follow(X)
                    if (first_beta.containsEpsilon()) {
                        change |= follow_Y.hardUpdate(follow_X);
                    }
                }
                // Caso: X -> ζ Y (Y es el ultimo simbolo)
                else {
                    // Regla S2: Follow(Y) ∪= Follow(X)
                    change |= follow_Y.hardUpdate(follow_X);
                }
            }
        }
    }
    
    return follows;
}

/**
 * Funcion auxiliar para imprimir conjuntos First y Follow
 */
void printFirstAndFollow(const Grammar& G,
                        const unordered_map<Symbol, ContainerSet>& firsts,
                        const unordered_map<Symbol, ContainerSet>& follows) {
    cout << "\n=== CONJUNTOS FIRST Y FOLLOW ===\n";
    
    auto print_set = [](const ContainerSet& cs) {
        cout << "{ ";
        for (const auto& symbol : cs.getSymbols()) {
            if (symbol.getName() == EPSILON) {
                cout << "epsilon ";
            } else {
                cout << symbol.getName() << " ";
            }
        }
        if (cs.containsEpsilon()) {
            cout << "epsilon ";
        }
        cout << "}";
    };

    cout << "Elementos de FIRST:\n";
    for (const auto& pair : firsts) {
        cout << "First(" << pair.first.getName() << ") = ";
        print_set(pair.second);
        cout << "\n";
    }

    cout << "\nElementos de FOLLOW:\n";
    for (const auto& pair : follows) {
        cout << "Follow(" << pair.first.getName() << ") = ";
        print_set(pair.second);
        cout << "\n";
    }

    // print First 
    cout << "\nConjuntos FIRST:\n";
    for (const Symbol& nonTerminal : G.getNonTerminals()) {
        auto it = firsts.find(nonTerminal);
        if (it != firsts.end()) {
            cout << "First(" << nonTerminal.getName() << ") = ";
            print_set(it->second);
            cout << "\n";
        }
    }
    
    // print Follow 
    cout << "\nConjuntos FOLLOW:\n";
    for (const Symbol& nonTerminal : G.getNonTerminals()) {
        auto it = follows.find(nonTerminal);
        if (it != follows.end()) {
            cout << "Follow(" << nonTerminal.getName() << ") = ";
            print_set(it->second);
            cout << "\n";
        }
    }
}








class ParsingTable {
public:
    struct ParsingLL1_Pair_Hash {
        size_t operator()(const pair<Symbol,Symbol>& p) const {
            return hash<string>()(p.first.getName()) ^ (hash<string>()(p.second.getName()) << 1);
        }
    };

    // Diccionario: key (no terminal, terminal), value: producción
    unordered_map<pair<Symbol, Symbol>, Production, ParsingLL1_Pair_Hash> TABLE;
    Grammar G;
    
    ParsingTable(const Grammar& G) : G(G) {
        buildTable();
    }

private:
    void buildTable() {
        auto first = computeFirsts(G);
        auto follow = computeFollows(G, first);
        
        
    }

};






/**
 * Clase que implementa un parser LL(1)
 */
class LL1Parser {
private:
    Grammar G;
    // Tabla de parsing: M[NonTerminal][Terminal] -> Production
    unordered_map<Symbol, unordered_map<Symbol, Production>> M;
    unordered_map<Symbol, ContainerSet> firsts;
    unordered_map<Symbol, ContainerSet> follows;
    Symbol EOF_SYMBOL;
    
    /**
     * Construye la tabla de análisis LL(1)
     */
    void buildParsingTable() {
        // Calcular conjuntos First y Follow
        firsts = computeFirsts(G);
        follows = computeFollows(G, firsts);
        
        // Inicializar tabla vacía
        M.clear();
        
        // Para cada producción A -> α
        for (const Production& production : G.getProductions()) {
            const Symbol& A = production.getLeft();
            const Sentence& alpha = production.getRight();
            
            // Calcular conjunto de predicción para esta producción
            ContainerSet prediction_set = computePredictionSet(production);
            
            // Para cada terminal a en Pred(A -> α)
            for (const Symbol& terminal : prediction_set.getSymbols()) {
                // Verificar si ya existe una entrada en M[A, a]
                if (M[A].find(terminal) != M[A].end()) {
                    throw runtime_error("La gramática no es LL(1): conflicto en M[" + 
                                      A.getName() + ", " + terminal.getName() + "]");
                }
                M[A][terminal] = production;
            }
            
            // Si epsilon está en el conjunto de predicción
            if (prediction_set.containsEpsilon()) {
                // Para cada símbolo en Follow(A)
                const ContainerSet& follow_A = follows[A];
                for (const Symbol& follow_symbol : follow_A.getSymbols()) {
                    if (M[A].find(follow_symbol) != M[A].end()) {
                        throw runtime_error("La gramática no es LL(1): conflicto en M[" + 
                                          A.getName() + ", " + follow_symbol.getName() + "]");
                    }
                    M[A][follow_symbol] = production;
                }
            }
        }
    }
    
    /**
     * Calcula el conjunto de predicción para una producción A -> α
     */
    ContainerSet computePredictionSet(const Production& production) {
        const Symbol& A = production.getLeft();
        const Sentence& alpha = production.getRight();
        ContainerSet prediction_set;
        
        if (alpha.isEpsilon()) {
            // Si α = ε, entonces Pred(A -> α) = Follow(A)
            prediction_set.update(follows[A]);
        } else {
            // Pred(A -> α) = First(α)
            ContainerSet first_alpha = computeLocalFirst(firsts, alpha);
            prediction_set.update(first_alpha);
            
            // Si ε ∈ First(α), entonces Pred(A -> α) = First(α) ∪ Follow(A)
            if (first_alpha.containsEpsilon()) {
                prediction_set.update(follows[A]);
            }
        }
        
        return prediction_set;
    }
    
public:
    /**
     * Constructor del parser LL(1)
     */
    LL1Parser(const Grammar& grammar) 
        : G(grammar), EOF_SYMBOL("$", true) {
        buildParsingTable();
    }
    
    /**
     * Realiza el análisis sintáctico de una cadena de entrada
     * @param input Cadena de entrada terminada en EOF ($)
     * @return Vector de producciones aplicadas en orden
     */
    vector<Production> parse(const vector<Symbol>& input) {
        vector<Production> output;
        stack<Symbol> parsing_stack;
        size_t cursor = 0;
        
        // Inicializar pila con EOF y símbolo inicial
        parsing_stack.push(EOF_SYMBOL);
        parsing_stack.push(G.getStartSymbol());
        
        while (!parsing_stack.empty()) {
            Symbol top = parsing_stack.top();
            parsing_stack.pop();
            
            // Verificar bounds del cursor
            if (cursor >= input.size()) {
                throw runtime_error("Entrada insuficiente durante el análisis");
            }
            
            Symbol current_input = input[cursor];
            
            if (top.getName() == EPSILON || top.getName() == "epsilon") {
                // Símbolo epsilon - no hacer nada
                continue;
            }
            else if (top.isTerminal()) {
                // Top es terminal
                if (top == current_input) {
                    if (top == EOF_SYMBOL) {
                        // Análisis exitoso
                        break;
                    }
                    cursor++;
                } else {
                    throw runtime_error("Error sintáctico: esperado '" + top.getName() + 
                                      "', encontrado '" + current_input.getName() + "'");
                }
            }
            else {
                // Top es no terminal
                auto it_A = M.find(top);
                if (it_A == M.end()) {
                    throw runtime_error("No terminal no encontrado en tabla: " + top.getName());
                }
                
                auto it_prod = it_A->second.find(current_input);
                if (it_prod == it_A->second.end()) {
                    throw runtime_error("Error sintáctico: no hay entrada en M[" + 
                                      top.getName() + ", " + current_input.getName() + "]");
                }
                
                Production production = it_prod->second;
                output.push_back(production);
                
                // Expandir producción en la pila (en orden inverso)
                const Sentence& right_side = production.getRight();
                if (!right_side.isEpsilon()) {
                    // Apilar símbolos en orden inverso
                    for (int i = right_side.size() - 1; i >= 0; i--) {
                        parsing_stack.push(right_side[i]);
                    }
                }
            }
        }
        
        return output;
    }
    
    /**
     * Imprime la tabla de análisis LL(1)
     */
    void printParsingTable() const {
        cout << "\n=== TABLA DE ANÁLISIS LL(1) ===\n";
        
        // Obtener todos los terminales para las columnas
        unordered_set<Symbol> all_terminals = G.getTerminals();
        all_terminals.insert(EOF_SYMBOL);
        
        // Imprimir encabezados
        cout << setw(12) << "M[A,a]";
        for (const Symbol& terminal : all_terminals) {
            cout << setw(15) << terminal.getName();
        }
        cout << "\n";
        
        // Imprimir filas para cada no terminal
        for (const Symbol& nonTerminal : G.getNonTerminals()) {
            cout << setw(12) << nonTerminal.getName();
            
            for (const Symbol& terminal : all_terminals) {
                auto it_A = M.find(nonTerminal);
                if (it_A != M.end()) {
                    auto it_prod = it_A->second.find(terminal);
                    if (it_prod != it_A->second.end()) {
                        string prod_str = it_prod->second.toString();
                        if (prod_str.length() > 14) {
                            prod_str = prod_str.substr(0, 11) + "...";
                        }
                        cout << setw(15) << prod_str;
                    } else {
                        cout << setw(15) << "ERROR";
                    }
                } else {
                    cout << setw(15) << "ERROR";
                }
            }
            cout << "\n";
        }
    }
    
    /**
     * Verifica si la gramática es LL(1)
     */
    bool isLL1() const {
        try {
            // Si la construcción de la tabla fue exitosa, es LL(1)
            return true;
        } catch (const runtime_error&) {
            return false;
        }
    }
    
    /**
     * Imprime el resultado del análisis
     */
    void printParseResult(const vector<Production>& derivations) const {
        cout << "\n=== DERIVACIONES APLICADAS ===\n";
        for (size_t i = 0; i < derivations.size(); i++) {
            cout << (i + 1) << ". " << derivations[i].toString() << "\n";
        }
    }
};

/**
 * Función auxiliar para crear una cadena de entrada desde un string
 */
vector<Symbol> createInputString(const string& input, const Grammar& G) {
    vector<Symbol> result;
    
    // Tokenizar la entrada (simplificado - asume tokens separados por espacios)
    stringstream ss(input);
    string token;
    
    while (ss >> token) {
        // Buscar el símbolo en los terminales de la gramática
        bool found = false;
        for (const Symbol& terminal : G.getTerminals()) {
            if (terminal.getName() == token) {
                result.push_back(terminal);
                found = true;
                break;
            }
        }
        if (!found) {
            throw runtime_error("Terminal no reconocido: " + token);
        }
    }
    
    // Añadir símbolo EOF
    result.push_back(Symbol("$", true));
    
    return result;
}

// Ejemplo de uso
void demonstrateLL1Parser() {
    // Ejemplo de gramática simple: E -> E + T | T, T -> T * F | F, F -> ( E ) | id
    
    // Definir símbolos terminales
    unordered_set<Symbol> terminals = {
        Symbol("+", true),
        Symbol("*", true), 
        Symbol("(", true),
        Symbol(")", true),
        Symbol("id", true)
    };
    
    // Definir símbolos no terminales
    unordered_set<Symbol> nonTerminals = {
        Symbol("E", false),
        Symbol("T", false),
        Symbol("F", false)
    };
    
    // Definir producciones (gramática LL(1) equivalente)
    vector<Production> productions = {
        Production(Symbol("E", false), Sentence({Symbol("T", false), Symbol("E'", false)})),
        Production(Symbol("E'", false), Sentence({Symbol("+", true), Symbol("T", false), Symbol("E'", false)})),
        Production(Symbol("E'", false), Sentence()),  // epsilon
        Production(Symbol("T", false), Sentence({Symbol("F", false), Symbol("T'", false)})),
        Production(Symbol("T'", false), Sentence({Symbol("*", true), Symbol("F", false), Symbol("T'", false)})),
        Production(Symbol("T'", false), Sentence()),  // epsilon
        Production(Symbol("F", false), Sentence({Symbol("(", true), Symbol("E", false), Symbol(")", true)})),
        Production(Symbol("F", false), Sentence({Symbol("id", true)}))
    };
    
    // Añadir E' y T' a los no terminales
    nonTerminals.insert(Symbol("E'", false));
    nonTerminals.insert(Symbol("T'", false));
    
    // Crear gramática
    Grammar grammar(terminals, nonTerminals, Symbol("E", false), productions);
    
    try {
        // Crear parser
        LL1Parser parser(grammar);
        
        cout << "Gramática creada exitosamente.\n";
        parser.printParsingTable();
        
        // Probar análisis con entrada "id + id * id"
        vector<Symbol> input = {
            Symbol("id", true),
            Symbol("+", true),
            Symbol("id", true),
            Symbol("*", true),
            Symbol("id", true),
            Symbol("$", true)
        };
        
        vector<Production> result = parser.parse(input);
        parser.printParseResult(result);
        
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}












/*
TODO ShiftReduce-Parser
*/

/*
TODO SLR1-Parser(ShiftReduce-Parser)
*/
