#include "uct.hpp"

using namespace std;
//=================== state node part ===============

// init state node: shuffle actions as required
StateNode::StateNode(ActionNode *_parentAct, State *_state, vector<SimAction *> &_actVect, double _reward,
                     bool _isTerminal) :
        parentAct_(_parentAct),
        state_(_state->duplicate()),
        reward_(_reward),
        isTerminal_(_isTerminal),
        numVisits_(0),
        actPtr_(0),
        firstMC_(0) {
    // copy _actVect to actVect
    int _size = _actVect.size();
    for (int i = 0; i < _size; ++i) {
        actVect_.push_back(_actVect[i]->duplicate());
    }
    // shuffle actions
    std::random_shuffle(actVect_.begin(), actVect_.end());
}

// free s, A(s) & A(s) nodes
StateNode::~StateNode() {
    delete state_;

    int sizeAct = actVect_.size();
    for (int i = 0; i < sizeAct; ++i) {
        SimAction *tmp = actVect_[i];
        delete tmp;
    }
    actVect_.clear();

    // int sizeNode = nodeVect_.size();
    // for (int i = 0; i < sizeNode; ++i) {
    //   ActionNode* tmp = nodeVect_[i];
    //   delete tmp;
    // }
    // nodeVect_.clear();
}

// return whether all actions are sampled
bool StateNode::isFull() {
    return (actPtr_ == actVect_.size());
}

// create a node for next new action in A(s)
// return the index of new action in A(s)
int StateNode::addActionNode() {
    assert(actPtr_ < actVect_.size());
    nodeVect_.push_back(new ActionNode(this));
    ++actPtr_;
    return actPtr_ - 1;
}
//============ end of state node ====================

//============ action node part ======================
// init action node
ActionNode::ActionNode(StateNode *_parentState) :
        parentState_(_parentState),
        avgReturn_(0),
        numVisits_(0) {}

// free state nodes
ActionNode::~ActionNode() {
    // int sizeNode = stateVect_.size();
    // for (int i = 0; i < sizeNode; ++i) {
    //   StateNode* tmp = stateVect_[i];
    //   delete tmp;
    // }
    // stateVect_.clear();
}

// return whether sx in {s' ~ T(s,a)}
bool ActionNode::containNextState(State *state) {
    int size = stateVect_.size();
    for (int i = 0; i < size; ++i) {
        if (state->equal(stateVect_[i]->state_)) {
            return true;
        }
    }
    return false;
}

// return the state node containing the next state
StateNode *ActionNode::getNextStateNode(State *state) {
    int size = stateVect_.size();
    for (int i = 0; i < size; ++i) {
        if (state->equal(stateVect_[i]->state_)) {
            return stateVect_[i];
        }
    }
    return NULL;
}

// create a state node for the next state
// return the new state node
StateNode *ActionNode::addStateNode(State *_state, vector<SimAction *> &_actVect, double _reward, bool _isTerminal) {
    int index = stateVect_.size();
    stateVect_.push_back(new StateNode(this, _state, _actVect, _reward, _isTerminal));
    return stateVect_[index];
}
// =============== end of action node ================

// ================  uct part =========================
void UCTPlanner::plan() {
    assert(root_ != NULL);
    // to avoid treat root differently than other nodes, when first see a root, just draw a MC sample trajectory
    int rootOffset = root_->numVisits_;
    if (rootOffset == 0) {
        root_->numVisits_++;
        rootOffset++;
    }

    for (int trajectory = rootOffset; trajectory < numRuns_; ++trajectory) {
        StateNode *current = root_;
        double mcReturn = leafValue_;
        int depth = 0;
        while (true) {
            ++depth;
            // cout << "(" << depth << "," << trajectory << ")" << endl;
            //for(int depth = 0; ((maxDepth == -1) || (depth < maxDepth)); ++depth) {
            if (current->isTerminal_) {
                mcReturn = endEpisodeValue_;
                break;
            }

            if (current->isFull()) { //follow the UCT tree
                //sample a node using UCB1
                int uctBranch = 0;
                if (current == root_) {
                    assert (depth == 1);
                    uctBranch = getUCTRootIndex(current);
                } else {
                    uctBranch = getUCTBranchIndex(current);
                }

                sim_->setState(current->state_);
                double r = sim_->act(current->actVect_[uctBranch]);
                State *nextState = sim_->getState();

                //if old node
                if (current->nodeVect_[uctBranch]->containNextState(nextState)) {
                    //follow path
                    current = current->nodeVect_[uctBranch]->getNextStateNode(nextState);
                    continue;
                } else { //new s'
                    //add new state node
                    //then MC sampling
                    StateNode *nextNode = current->nodeVect_[uctBranch]->addStateNode(nextState, sim_->getActions(), r,
                                                                                      sim_->isTerminal());

                    if (-1 == maxDepth_) {
                        mcReturn = MC_Sampling(nextNode);
                    } else {
                        mcReturn = MC_Sampling(nextNode, maxDepth_ - depth);
                    }
                    current = nextNode;
                    break;
                }
            } else { //start MC-Sampling for the new action
                int actID = current->addActionNode();
                sim_->setState(current->state_);
                double r = sim_->act(current->actVect_[actID]);
                StateNode *nextNode = current->nodeVect_[actID]->addStateNode(sim_->getState(), sim_->getActions(), r,
                                                                              sim_->isTerminal());

                if (-1 == maxDepth_) {
                    mcReturn = MC_Sampling(nextNode);
                } else {
                    mcReturn = MC_Sampling(nextNode, maxDepth_ - depth);
                }
                current = nextNode;
                break;
            }
        }// end of for (depth)
        // update tree values
        updateValues(current, mcReturn);
    }// end of for (numRuns)
}

int UCTPlanner::getMostVisitedBranchIndex() {
    assert(root_ != NULL);
    vector<double> maximizer;//maximizer.clear();
    int size = root_->nodeVect_.size();
    for (int i = 0; i < size; ++i) {
        maximizer.push_back(root_->nodeVect_[i]->numVisits_);
    }
    vector<double>::iterator max_it = std::max_element(maximizer.begin(), maximizer.end());
    int index = std::distance(maximizer.begin(), max_it);

    return index;
}

int UCTPlanner::getGreedyBranchIndex() {
    assert(root_ != NULL);
    vector<double> maximizer;//maximizer.clear();
    int size = root_->nodeVect_.size();
    for (int i = 0; i < size; ++i) {
        maximizer.push_back(root_->nodeVect_[i]->avgReturn_);
    }
    vector<double>::iterator max_it = std::max_element(maximizer.begin(), maximizer.end());
    int index = std::distance(maximizer.begin(), max_it);

    return index;

}

int UCTPlanner::getUCTRootIndex(StateNode *node) {
    double det = log((double) node->numVisits_);

    vector<double> maximizer;
    int size = node->nodeVect_.size();
    for (int i = 0; i < size; ++i) {
        //double val = node->nodeVect[i]->avgReturn + node->internalR[i];
        double val = node->nodeVect_[i]->avgReturn_;
        val += ucbScalar_ * sqrt(det / (double) node->nodeVect_[i]->numVisits_);
        maximizer.push_back(val);
    }
    vector<double>::iterator max_it = std::max_element(maximizer.begin(), maximizer.end());
    int index = std::distance(maximizer.begin(), max_it);

    return index;
}

int UCTPlanner::getUCTBranchIndex(StateNode *node) {
    double det = log((double) node->numVisits_);

    vector<double> maximizer;
    int size = node->nodeVect_.size();
    for (int i = 0; i < size; ++i) {
        double val = node->nodeVect_[i]->avgReturn_;
        val += ucbScalar_ * sqrt(det / (double) node->nodeVect_[i]->numVisits_);
        maximizer.push_back(val);
    }
    vector<double>::iterator max_it = std::max_element(maximizer.begin(), maximizer.end());
    int index = std::distance(maximizer.begin(), max_it);

    return index;
}

void UCTPlanner::updateValues(StateNode *node, double mcReturn) {
    double totalReturn(mcReturn);
    if (node->numVisits_ == 0) {
        node->firstMC_ = totalReturn;
    }
    node->numVisits_++;
    while (node->parentAct_ != NULL) { //back until root is reached
        ActionNode *parentAct = node->parentAct_;
        parentAct->numVisits_++;
        totalReturn *= gamma_;
        totalReturn += modifyReward(node->reward_);
        parentAct->avgReturn_ += (totalReturn - parentAct->avgReturn_) / parentAct->numVisits_;
        node = parentAct->parentState_;
        node->numVisits_++;
    }
}

double UCTPlanner::MC_Sampling(StateNode *node, int depth) {
    double mcReturn(leafValue_);
    sim_->setState(node->state_);
    double discnt = 1;
    for (int i = 0; i < depth; i++) {
        if (sim_->isTerminal()) {
            mcReturn += endEpisodeValue_;
            break;
        }
        vector<SimAction *> &actions = sim_->getActions();
        int actID = rand() % actions.size();
        double r = sim_->act(actions[actID]);
        mcReturn += discnt * modifyReward(r);
        discnt *= gamma_;
    }
    return mcReturn;
}


double UCTPlanner::MC_Sampling(StateNode *node) {
    double mcReturn(endEpisodeValue_);
    sim_->setState(node->state_);
    double discnt = 1;
    while (!sim_->isTerminal()) {
        vector<SimAction *> &actions = sim_->getActions();
        int actID = rand() % actions.size();
        double r = sim_->act(actions[actID]);
        mcReturn += discnt * modifyReward(r);
        discnt *= gamma_;
    }
    return mcReturn;
}

// memory management codes
void UCTPlanner::pruneState(StateNode *state) {
    int sizeNode = state->nodeVect_.size();
    for (int i = 0; i < sizeNode; ++i) {
        ActionNode *tmp = state->nodeVect_[i];
        pruneAction(tmp);
    }
    state->nodeVect_.clear();
    delete state;
}

void UCTPlanner::pruneAction(ActionNode *act) {
    int sizeNode = act->stateVect_.size();
    for (int i = 0; i < sizeNode; ++i) {
        StateNode *tmp = act->stateVect_[i];
        pruneState(tmp);
    }
    act->stateVect_.clear();
    delete act;
}

void UCTPlanner::prune(SimAction *act) {// check whether the root is terminal or not
    StateNode *nextRoot = NULL;
    int size = root_->nodeVect_.size();
    for (int i = 0; i < size; ++i) {
        if (act->equal(root_->actVect_[i])) {
            assert(root_->nodeVect_[i]->stateVect_.size() == 1);
            nextRoot = root_->nodeVect_[i]->stateVect_[0];

            ActionNode *tmp = root_->nodeVect_[i];
            delete tmp;
        } else {
            ActionNode *tmp = root_->nodeVect_[i];
            pruneAction(tmp);
        }
    }

    assert(nextRoot != NULL);

    delete root_;
    root_ = nextRoot;
    root_->parentAct_ = NULL;
}

bool UCTPlanner::testRoot(State *_state, double _reward, bool _isTerminal) {
    return root_ != NULL && (root_->reward_ == _reward) && (root_->isTerminal_ == _isTerminal) &&
           root_->state_->equal(_state);
}


void UCTPlanner::testDeterministicProperty() {
    if (testDeterministicPropertyState(root_)) {
        cout << "Deterministic Property Test passed!" << endl;
    } else {
        cout << "Error in Deterministic Property  Test!" << endl;
        exit(0);
    }
}

bool UCTPlanner::testDeterministicPropertyState(StateNode *state) {
    int actSize = state->nodeVect_.size();
    for (int i = 0; i < actSize; ++i) {
        if (!testTreeStructureAction(state->nodeVect_[i])) {
            return false;
        }
    }
    return true;
}

bool UCTPlanner::testDeterministicPropertyAction(ActionNode *action) {

    int stateSize = action->stateVect_.size();
    if (stateSize != 1) {
        cout << "Error in Deterministic Property Test!" << endl;
        return false;
    }

    for (int i = 0; i < stateSize; ++i) {
        if (!testTreeStructureState(action->stateVect_[i])) {
            return false;
        }
    }
    return true;
}


// visit number checkings
// avg value checkings
void UCTPlanner::testTreeStructure() {
    if (testTreeStructureState(root_)) {
        cout << "Tree Structure Test passed!" << endl;
    } else {
        cout << "Error in Tree Structure Test!" << endl;
        exit(0);
    }
}

bool UCTPlanner::testTreeStructureState(StateNode *state) {
    // numVisits testing
    // n(s) = \sum_{a} n(s,a) + 1 (one offset due to first sample)
    int actVisitCounter = 0;
    int actSize = state->nodeVect_.size();
    for (int i = 0; i < actSize; ++i) {
        actVisitCounter += state->nodeVect_[i]->numVisits_;
    }
    if ((actVisitCounter + 1 != state->numVisits_) && (!state->isTerminal_)) {
        cout << "n(s) = sum_{a} n(s,a) + 1 failed !" << "\nDiff: " << (actVisitCounter + 1 - state->numVisits_)
             << "\nact: " << (actVisitCounter + 1) << "\nState: " << state->numVisits_ << "\nTerm: "
             << (state->isTerminal_ ? "true" : "false") << "\nState: " << endl;
        state->state_->print();
        cout << endl;
        return false;
    }
    for (int i = 0; i < actSize; ++i) {
        if (!testTreeStructureAction(state->nodeVect_[i])) {
            return false;
        }
    }
    return true;
}

bool UCTPlanner::testTreeStructureAction(ActionNode *action) {
    // numVisits testing
    // n(s,a) = \sum n(s')
    int stateVisitCounter = 0;
    int stateSize = action->stateVect_.size();
    for (int i = 0; i < stateSize; ++i) {
        stateVisitCounter += action->stateVect_[i]->numVisits_;
    }
    if (stateVisitCounter != action->numVisits_) {
        cout << "n(s,a) = sum n(s') failed !" << endl;
        return false;
    }

    // avg
    // Q(s,a) = E {r(s') + gamma * sum pi(a') Q(s',a')}
    // Q(s,a) = sum_{s'} n(s') / n(s,a) * ( r(s') + gamma * sum_{a'} (n (s',a') * Q(s',a') + first) / n(s'))
    double value = 0;
    for (int i = 0; i < stateSize; ++i) {
        StateNode *next = action->stateVect_[i];
        double w = next->numVisits_ / (double) action->numVisits_;
        double nextValue = next->firstMC_;
        int nextActSize = next->nodeVect_.size();
        for (int j = 0; j < nextActSize; ++j) {
            nextValue += next->nodeVect_[j]->numVisits_ * next->nodeVect_[j]->avgReturn_;
        }
        nextValue = (nextValue) / (double) next->numVisits_ * gamma_;
        nextValue += next->reward_;
        value += w * nextValue;
    }
    if ((action->avgReturn_ - value) * (action->avgReturn_ - value) > 1e-10) {
        cout << "value constraint failed !" << "avgReturn=" << action->avgReturn_ << " value=" << value << endl;
        return false;
    }

    for (int i = 0; i < stateSize; ++i) {
        if (!testTreeStructureState(action->stateVect_[i])) {
            return false;
        }
    }
    return true;
}
