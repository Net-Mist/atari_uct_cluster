/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details.
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  ale_interface.hpp
 *
 *  The shared library interface.
 **************************************************************************** */
#include "uct.hpp"

using namespace std;

// --> x
// vv y
// a simple domain
class ToyState : public State {
public:
    int x, y;
    int food;

    ToyState(int _x, int _y, int _food) : x(_x), y(_y), food(_food) {
    }

    virtual bool equal(State *state) {
        const ToyState *other = dynamic_cast<const ToyState *> (state);
        return !((other == NULL) || (other->x != x) || (other->y != y) || (other->food != food));
    }

    virtual State *duplicate() {
        ToyState *other = new ToyState(x, y, food);
        return other;
    }

    virtual void print() const {
        cout << "(" << x << "," << y << "," << food << ")";
    }
};

class ToyAction : public SimAction {
public:
    int id;

    ToyAction(int _id) : id(_id) {
    }

    virtual SimAction *duplicate() {
        ToyAction *other = new ToyAction(id);

        return other;
    }

    virtual void print() const {
        cout << id;
    }

    virtual bool equal(SimAction *other) {
        ToyAction *act = dynamic_cast<ToyAction *>(other);
        return act->id == id;
    }
};

// deterministic food and shelter
class ToySimulator : public Simulator {
public:
    ToyState *current;
    vector<SimAction *> actVect;
    double reward;

    ToySimulator() {
        // construct actVect
        // construct current
        actVect.push_back(new ToyAction(0));
        actVect.push_back(new ToyAction(1));
        actVect.push_back(new ToyAction(2));
        actVect.push_back(new ToyAction(3));
        current = new ToyState(0, 2, 0);
    }

    ~ToySimulator() {
        // free actVect
        // free current
        delete actVect[0];
        delete actVect[1];
        delete actVect[2];
        delete actVect[3];
        delete current;
    }

    virtual void setState(State *state) {
        const ToyState *other = dynamic_cast<const ToyState *> (state);
        if (other == NULL) {
            return;
        }
        current->x = other->x;
        current->y = other->y;
        current->food = other->food;
    }

    virtual State *getState() {
        return current;
    }

    // 0 up
    // 1 down
    // 2 left
    // 3 right
    virtual double act(const SimAction *action) {
        assert(!isTerminal());

        const ToyAction *act = dynamic_cast<const ToyAction *> (action);
        if (act == NULL) {
            return 0;
        }
        int id = act->id;
        if (rand() / (double) RAND_MAX < 0.1) {
            id = rand() % 4;
            // cout << "random happen!" << endl;
        }
        switch (id) {
            case 0:
                if ((current->x == 0) && (current->y > 0)) {
                    current->y--;
                }
                break;
            case 1:
                if ((current->x == 0) && (current->y < 4)) {
                    current->y++;
                }
                break;
            case 2:
                if (current->x > 0) {
                    current->x--;
                }
                break;
            case 3:
                if (current->x < 4) {
                    current->x++;
                }
                break;
            default:
                break;
        }
        return ((current->x == 4) && (current->y == current->food)) ? 10 : 0;
    }

    virtual vector<SimAction *> &getActions() {
        return actVect;
    }

    virtual bool isTerminal() {
        if ((current->x != 4) || (current->y != current->food)) {
            return false;
        }
        return true;
    }

    virtual void reset() {
        current->x = 0;
        current->y = 2;
        current->food = rand() % 5;
    }
};


// then test

int main(int argc, char **argv) {
    ToySimulator *sim = new ToySimulator();
    ToySimulator *sim2 = new ToySimulator();
    UCTPlanner uct(sim2, -1, 100, 1, 0.95);
    int numGames = 100;

    for (int i = 0; i < numGames; ++i) {
        int steps = 0;
        double r = 0;
        // sim->getState()->print();
        while (!sim->isTerminal()) {
            steps++;
            uct.setRootNode(sim->getState(), sim->getActions(), r, sim->isTerminal());
            uct.plan();
            SimAction *action = uct.getAction();

            // cout << "-" ;
            // action->print();
            // cout << "->";
            // uct.testTreeStructure();

            r = sim->act(action);
            // sim->getState()->print();
            // cout << endl;
            // sim->getState()->print();
        }
        sim->reset();
        cout << "Game:" << i << "  steps: " << steps << "  r: " << r << endl;
    }
}

