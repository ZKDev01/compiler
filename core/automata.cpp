#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <stdexcept>
#include <queue>

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
                        "Estado de transición fuera de rango: " + to_string(state) +
                        " [0 a " + to_string(n_states - 1) + "]"
                    );
                }
                for (int dest : dests) {
                    if (dest < 0 || dest >= n_states) {
                        throw invalid_argument(
                            "Destino de transición fuera de rango: " + to_string(dest) +
                            " [0 a " + to_string(n_states - 1) + "]"
                        );
                    }
                }
            }
        }
};

int main(int argc, char const *argv[])
{
    try {
        unordered_set<int> finals = {1, 2};
        unordered_map<pair<int,char>, vector<int>, AFND::Pair_Hash> transitions = {
            {{0, 'a'}, {1}},
            {{1, 'b'}, {2}},
            {{2, 'c'}, {0}}
        };
        AFND automata(3, finals, transitions);
        cout << "Automata finito no determinista creado correctamente." << endl;
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}


/*
class NFA
    states: int 
    finals: List[int]
    transitions: Dict[ (int,str), List[int] ]
    start: int = 0

function epsilon-transitions 

function move 

function recognize 

function epsilon-clousure
*/


/*
class DFA(NFA)
*/

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

