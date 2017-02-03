#ifndef __UCT_HPP__
#define __UCT_HPP__

#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>
#include <iostream>

using namespace std;

//==================== Simulator Interface =======================
class State {
public:
    // whether two states are the same
    // used in s' ~ (s,a) checking
    virtual bool equal(State *state) = 0;

    // create and return a copy of state
    virtual State *duplicate() = 0;

    virtual void print() const = 0;

    virtual ~State() {};
};

class SimAction {
public:
    // for pruning
    virtual bool equal(SimAction *act) = 0;

    // create and return a copy of action
    virtual SimAction *duplicate() = 0;

    virtual void print() const = 0;

    virtual ~SimAction() {};
};

class Simulator {
public:
    // set the state of simulator
    virtual void setState(State *state) = 0;

    // get current state of simulator, no state is created
    virtual State *getState() = 0;

    // one step simulation
    virtual double act(const SimAction *action) = 0;

    // get all available actions for current state
    // need to handle memory of actions
    virtual vector<SimAction *> &getActions() = 0;

    // return whether current state is terminal state
    virtual bool isTerminal() = 0;

    // reset the state of the simulator
    virtual void reset() = 0;
};

//================================================================

//=========================== UCT parts ==========================
class ActionNode;

class StateNode {
public:
    // parent action node, for updating tree
    ActionNode *parentAct_;
    // s
    State *state_;
    // r = R(sx, ax, s), (sx,ax) is determined in tree structure
    const double reward_;
    // isTerminal(s)
    const bool isTerminal_;
    // n(s)
    int numVisits_;
    // action pointer, point to next new action
    int actPtr_;
    // the first return of monte-carlo sampling from s
    double firstMC_;
    // A(s)
    vector<SimAction *> actVect_;
    // each node for one action in A(s)
    vector<ActionNode *> nodeVect_;

    // for internal rewards
    // vector<double> internalR_;

    // init state node
    StateNode(ActionNode *_parentAct, State *_state, vector<SimAction *> &_actVect, double _reward, bool _isTerminal);

    // free s, A(s) & A(s) nodes
    virtual ~StateNode();

    // return whether all actions are sampled
    bool isFull();

    // create a node for next new action in A(s)
    // return the index of new action in A(s)
    int addActionNode();
};

class ActionNode {
public:
    // parent state s, thus (s,a) is determined in tree structure
    StateNode *const parentState_;
    // keep s' ~ T(s,a)
    vector<StateNode *> stateVect_;
    // Q(s,a)
    double avgReturn_;
    // n(s,a)
    int numVisits_;

    // init action node
    ActionNode(StateNode *_parentState);

    // free state nodes
    ~ActionNode();

    // return whether sx in {s' ~ T(s,a)}
    bool containNextState(State *state);

    // return the state node containing the next state
    StateNode *getNextStateNode(State *state);

    // create a state node for the next state
    // return the new state node
    StateNode *addStateNode(State *_state, vector<SimAction *> &_actVect, double _reward, bool _isTerminal);
};

class UCTPlanner {
public:
    // simulator interfaces
    Simulator *sim_;
    // uct parameters
    int maxDepth_;
    int numRuns_;
    double ucbScalar_;
    double gamma_;
    double leafValue_;
    double endEpisodeValue_;
    // rand seed value
    StateNode *root_;

    UCTPlanner(Simulator *_sim, int _maxDepth, int _numRuns, double _ucbScalar, double _gamma, double _leafValue = 0,
               double _endEpisodeValue = 0) :
            sim_(_sim),
            maxDepth_(_maxDepth),
            numRuns_(_numRuns),
            ucbScalar_(_ucbScalar),
            gamma_(_gamma),
            leafValue_(_leafValue),
            endEpisodeValue_(_endEpisodeValue),
            root_(NULL) {}

    // does not handle sim_
    ~UCTPlanner() {
        clearTree();
    }

    // set the root node in UCT
    void setRootNode(State *_state, vector<SimAction *> _actVect, double _reward, bool _isTerminal) {
        if (root_ != NULL) {
            clearTree();
        }
        root_ = new StateNode(NULL, _state, _actVect, _reward, _isTerminal);
    }

    // start planning
    void plan();

    // get the planned action for root
    // called after planning
    SimAction *getAction() {
        return root_->actVect_[getGreedyBranchIndex()];
    }

    // return the most visited action for root node
    int getMostVisitedBranchIndex();

    // return the most visited action for root node
    int getGreedyBranchIndex();

    // return the index of action in root
    // add a new action node to tree if the action is never sampled
    int getUCTRootIndex(StateNode *node);

    // return the index of action to sample
    // add a new action node to tree if the action is never sampled
    int getUCTBranchIndex(StateNode *node);

    // update the values along the path from leaf to root
    // update the counters
    void updateValues(StateNode *node, double mcReturn);

    // sample trajectory to a depth
    double MC_Sampling(StateNode *node, int depth);

    // sample to the end of an episode
    double MC_Sampling(StateNode *node);

    // modify the reward function
    // currently it does nothing
    double modifyReward(double orig) {
        return orig;
    }

    void printRootValues() {
        int size = root_->nodeVect_.size();
        for (int i = 0; i < size; ++i) {
            double val = root_->nodeVect_[i]->avgReturn_;
            int numVist = root_->nodeVect_[i]->numVisits_;
            cout << "(";
            root_->actVect_[i]->print();
            cout << "," << val << "," << numVist << ") ";
        }
        cout << root_->isTerminal_;
        // cout << endl;
    }

    // release all nodes
    void clearTree() {
        if (root_ != NULL) {
            pruneState(root_);
        }
        root_ = NULL;
    }

    // adjust UCT tree by pruning out all other branches
    bool terminalRoot() {
        return root_->isTerminal_;
    }

    void prune(SimAction *act);

    // prune out state node and its children
    void pruneState(StateNode *state);

    // prune out action node and its children
    void pruneAction(ActionNode *act);

    // test whether the root contain the right information
    bool testRoot(State *_state, double _reward, bool _isTerminal);

    // test
    void testDeterministicProperty();

    bool testDeterministicPropertyState(StateNode *state);

    bool testDeterministicPropertyAction(ActionNode *action);

    // visit number checkings
    // avg value checkings
    void testTreeStructure();

    bool testTreeStructureState(StateNode *state);

    bool testTreeStructureAction(ActionNode *action);
};

#endif
