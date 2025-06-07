#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <queue>
#include <utility>

using namespace std;

class AFND {
    public:
        struct Pair_Hash {
            std::size_t operator()(const pair<int,char>& p) const {
                return hash<int>()(p.first) ^ (hash<char>()(p.second) << 1);
            }
        };
    
    private:
        int n_states;
        unordered_set<int> final_states;
        unordered_map<pair<int,char>, vector<int>, Pair_Hash> transitions;
        int start_state;

    
    public:
        /** 
         * Constructor del autómata finito no determinista 
         * @param n_states Número de estados del autómata
         * @param final_states Conjunto de estados finales
         * @param transitions Diccionario de transiciones del autómata
         * @param start_state Estado inicial del autómata
         */
        AFND(int n_states, 
            const unordered_set<int>& final_states, 
            const unordered_map<pair<int,char>, vector<int>, Pair_Hash>& transitions, 
            int start_state = 0): 
                n_states(n_states), 
                start_state(start_state) 
                {
                    for (int s : final_states) {
                        this->final_states.insert(s);
                    }
                    this->transitions = transitions;
                    
                    validate();
                }
        AFND(int n_states, 
            const unordered_set<int>& final_states,
            int start = 0):
                n_states(n_states),
                start_state(start) 
                {
                    for (int s : final_states) {
                        this->final_states.insert(s);
                    }
                    validate();
                }

    private:
        void validate() {
            for (int state: final_states) { 
                if (state < 0 || state >= n_states) {
                    throw invalid_argument(
                        "Estado final fuera de rango: " + to_string(state) +
                        " [0 a " + to_string(n_states - 1) + "]"
                    );

                }
            }
            if (start_state < 0 || start_state >= n_states) {
                throw invalid_argument(
                    "Estado inicial fuera de rango: " + to_string(start_state) +
                    " [0 a " + to_string(n_states - 1) + "]"
                );
            }
            for (const auto& [key,dests] : transitions) {
                int state = key.first;

                if (state < 0 || state >= n_states) {
                    throw invalid_argument(
                        "Estado de transicion fuera de rango: " + to_string(state) +
                        " [0 a " + to_string(n_states - 1) + "]"
                    );
                }
                for (int dest : dests) {
                    if (dest < 0 || dest >= n_states) {
                        throw invalid_argument(
                            "Destino de transicion fuera de rango: " + to_string(dest) +
                            " [0 a " + to_string(n_states - 1) + "]"
                        );
                    }
                }
            }
        }
    
    public:
        void addTransition(int from, char symbol, int to) {
            if (from < 0 || from >= n_states || to < 0 || to >= n_states) throw invalid_argument("Estados fuera del rango valido");
            transitions[{from,symbol}].push_back(to);
        }
        vector<int> getTransitions(int state, char symbol) const {
            auto it = transitions.find({state, symbol});
            if (it != transitions.end()) {
                return it->second;
            } 
            return vector<int>();
        }

        /*
         * Calculo de epsilon-clausura de un conjunto de estados 
         * util si el automata tiene transiciones epsilon  
        */
        unordered_set<int> epsilonClosure(const unordered_set<int>& states) const {
            unordered_set<int> closure = states;
            queue<int> _queue;
            
            for (int state : states) {
                _queue.push(state);
            }

            while (!_queue.empty()) {
                int current = _queue.front();
                _queue.pop();

                vector<int> tmpTransitions = getTransitions(current, '\0'); // Asumiendo que '\0' representa una transición epsilon
                for (int nextState : tmpTransitions) {
                    if (closure.find(nextState) == closure.end()) {
                        closure.insert(nextState);
                        _queue.push(nextState);
                    }
                }
            }
            return closure;
        }

        bool recognize(const string& word) const {
            unordered_set<int> currentStates = epsilonClosure({start_state});

            for (char symbol : word) {
                unordered_set<int> nextStates;
                
                // Para cada estado actual, encontrar todos los estados alcanzables    
                for (int state : currentStates) {
                    vector<int> transitions = getTransitions(state, symbol);
                    for (int nextState : transitions) {
                        nextStates.insert(nextState);
                    }
                }

                currentStates = epsilonClosure(nextStates);
                if (currentStates.empty()) {
                    return false; // No hay estados alcanzables
                }
            }

            for (int state : currentStates) {
                if (final_states.find(state) != final_states.end()) {
                    return true; // Al menos un estado final alcanzado
                }
            }
            return false;
        }
    
        void print() const {
            cout << "AFND con " << n_states << " estados." << endl;
            cout << "Estado inicial: " << start_state << endl;
            cout << "Estados finales: ";
            for (int state : final_states) {
                cout << state << " ";
            }
            cout << endl;

            cout << "Transiciones:" << endl;
            for (const auto& [key, dests] : transitions) {
                cout << "  Desde estado " << key.first << " con simbolo '" 
                     << key.second << "' a estados: ";
                for (int dest : dests) {
                    cout << dest << " ";
                }
                cout << endl;
            }
        }

        bool isFinalState(int state) const { return final_states.find(state) != final_states.end(); }   

        // Getters
        int getNumStates() const { return n_states; }
        int getStartState() const { return start_state; }
        const unordered_set<int>& getFinalStates() const { return final_states; }
        const unordered_map<pair<int,char>, vector<int>, Pair_Hash>& getTransitions() const { return transitions; }
};

class AFD : public AFND {
    public:
        AFD(int n_states, 
            const unordered_set<int>& final_states, 
            const unordered_map<pair<int,char>, vector<int>, AFND::Pair_Hash>& transitions, 
            int start_state = 0):
                AFND(n_states, final_states, transitions, start_state) {
                    validateDeterminism();
            }
        AFD(int n_states, 
            const unordered_set<int>& final_states,
            int start = 0):
                AFND(n_states, final_states, start) {
                }
    
    private:
        void validateDeterminism() {
            for (const auto& [key, dests] : getTransitions()) {
                if (dests.size() != 1) {
                    throw invalid_argument("El AFD debe tener una unica transicion por simbolo desde cada estado.");
                }
            }
        }
    
    public:
        void addTransition(int from, char symbol, int to) {
            if (from < 0 || from >= getNumStates() || to < 0 || to >= getNumStates()) {
                throw invalid_argument("Estados fuera del rango valido");
            }
            if (getTransitions().find({from, symbol}) != getTransitions().end()) {
                throw invalid_argument("Ya existe una transicion para el estado " + to_string(from) + " con el simbolo '" + symbol + "'");
            }
            AFND::addTransition(from, symbol, to);
        }
        bool recognize(const string& word) const {
            int currentState = getStartState();

            for (char symbol : word) {
                vector<int> nextStates = getTransitions(currentState, symbol);
                if (nextStates.empty()) {
                    return false; // No hay transiciones posibles
                }
                currentState = nextStates[0]; // AFD tiene una unica transicion por simbolo
            }

            return isFinalState(currentState); // Verifica si el estado final es un estado de aceptación
        }
        void print() const {
            cout << "AFD con " << getNumStates() << " estados." << endl;
            cout << "Estado inicial: " << getStartState() << endl;
            cout << "Estados finales: ";
            for (int state : getFinalStates()) {
                cout << state << " ";
            }
            cout << endl;

            cout << "Transiciones:" << endl;
            for (const auto& [key, dests] : getTransitions()) {
                cout << "  Desde estado " << key.first << " con simbolo '" 
                     << key.second << "' a estado: ";
                if (!dests.empty()) {
                    cout << dests[0]; // AFD tiene una unica transicion por simbolo
                } else {
                    cout << "Ninguno";
                }
                cout << endl;
            }
        }
    
};





/*
function get-move(NFA, states, symbol) -> set
*/

/*
function NFA-to-DFA(NFA) -> DFA
*/

/*
function UNION
*/

/*
function CONCATENATION
*/

/*
function CLOSURE
*/

/*
function MINIMIZATION
*/





void test1() {
    try {
        unordered_set<int> finals = {1, 2};
        AFND::Pair_Hash pair_hash;
        
        unordered_map<pair<int,char>, vector<int>, AFND::Pair_Hash> transitions;
        transitions[{0, 'a'}] = {1};
        transitions[{1, 'b'}] = {2};
        transitions[{2, 'c'}] = {0};

        AFND automata(3, finals, transitions);
        automata.print();

        vector<string> test_words = {"abc", "ab", "a", "ac", "bca"};
        for (const string& word : test_words) {
            cout << "Reconociendo la palabra '" << word << "': ";
            if (automata.recognize(word)) {
                cout << "Aceptada" << endl;
            } else {
                cout << "No aceptada" << endl;
            }
        }
        
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }
}

void test2() {
    try {
        unordered_set<int> finals = {2};
        unordered_map<pair<int,char>, vector<int>, AFND::Pair_Hash> transitions;

        transitions[{0, 'a'}] = {1};
        transitions[{0, 'b'}] = {0};
        transitions[{1, '\0'}] = {2};
        transitions[{2, 'a'}] = {2};
        transitions[{2, 'b'}] = {2};

        AFND automata(3, finals, transitions);

        automata.print();

        vector<string> test_words = {"", "b", "bb", "ab", "bbab", "aba", "ba"};
        for (const string& word : test_words) {
            cout << "Reconociendo la palabra '" << word << "': ";
            if (automata.recognize(word)) {
                cout << "Aceptada" << endl;
            } else {
                cout << "No aceptada" << endl;
            }
        }
        
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }
}

void test3() {
    // L = { ab^n | n = 2k+1 para k >= 0 }
    try {
        unordered_set<int> finals = {2};
        unordered_map<pair<int,char>, vector<int>, AFND::Pair_Hash> transitions;

        transitions[ {0,'a'} ] = {1};
        transitions[ {0,'b'} ] = {0};
        transitions[ {1,'a'} ] = {1};
        transitions[ {1,'b'} ] = {2};
        transitions[ {2,'b'} ] = {1};
        
        AFD automata(3, finals, transitions);

        automata.print();

        vector<string> test_words = {"", "a", "ab", "abb", "abbb", "abab", "ababb"};
        for (const string& word : test_words) {
            cout << "Reconociendo la palabra '" << word << "': ";
            if (automata.recognize(word)) {
                cout << "Aceptada" << endl;
            } else {
                cout << "No aceptada" << endl;
            }
        }
        
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }
}


int main(int argc, char const *argv[]) {
    //test1();
    //test2();
    test3();
    return 0;
}
