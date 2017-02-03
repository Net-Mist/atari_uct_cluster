#ifndef __ATARI_HPP__
#define __ATARI_HPP__

#include "uct.hpp"
#include <ale_interface.hpp>
#include <string>
#include <fstream>
#include "atari_images.hpp"


class AtariState : public State {
public:

    AtariState(std::string snapshot) {
        snapshot_ = snapshot;
    }

    virtual bool equal(State *state) {
        const AtariState *other = dynamic_cast<const AtariState *>(state);
        return !((other == NULL) || (snapshot_.compare(other->snapshot_) != 0));
    }

    virtual State *duplicate() {
        return new AtariState(snapshot_);
    }

    virtual void print() const {
        // nothing to do
    }

    ~AtariState() {

    }

    std::string snapshot_;
};


class AtariAction : public SimAction {
public:
    ale::Action act_;

    AtariAction(ale::Action &act) : act_(act) {
    }

    virtual SimAction *duplicate() {
        return new AtariAction(act_);
    }

    virtual void print() const {
        std::cout << act_;
    }

    virtual bool equal(SimAction *other) {
        AtariAction *act = dynamic_cast<AtariAction *>(other);
        return act->act_ == act_;
    }

    ~AtariAction() {

    }
};


class AtariSimulator : public Simulator {
public:

    AtariSimulator(const std::string &romFile, bool updateFrame, bool pseudoGameover, bool scaleReward, int numRepeats)
            :
            updateFrame_(updateFrame),
            pseudoGameover_(pseudoGameover),
            scaleReward_(scaleReward),
            numRepeats_(numRepeats),
            reward_(0) {

        ale_ = new ale::ALEInterface(romFile);
        currentState_ = new AtariState(ale_->getSnapshot());
        actSet_ = ale_->getMinimalActionSet();
        minReward_ = ale_->minReward();
        maxReward_ = ale_->maxReward();
        frameBuffer_ = new FrameBuffer(4);
        lifeLost_ = false;

        for (int i = 0; i < actSet_.size(); ++i) {
            actVect_.push_back(new AtariAction(actSet_[i]));
            act_map[actSet_[i]] = i;
        }

        cout << "Simulator Information: "
             << "\nROM: " << romFile
             << "\nPseudo GameOver: " << (pseudoGameover_ ? "True" : "False")
             << "\nScale Reward: " << (scaleReward_ ? "True" : "False")
             << "\nMax R: " << maxReward_ << "  Min R: " << minReward_
             << "\nAct Size: " << actSet_.size() << "\n";
        for (int i = 0; i < actSet_.size(); ++i) {
            cout << actSet_[i] << " ";
        }
        cout << endl;
    }

    ~AtariSimulator() {
        delete ale_;
        delete currentState_;
        for (int i = 0; i < actVect_.size(); ++i) {
            delete actVect_[i];
        }
        delete frameBuffer_;
    }

    int lives() {
        return ale_->lives();
    }

    virtual void setState(State *state) {
        const AtariState *other = dynamic_cast<const AtariState *> (state);
        currentState_->snapshot_ = other->snapshot_;
        ale_->restoreSnapshot(currentState_->snapshot_);


        lifeLost_ = false;
    }

    virtual State *getState() {
        return currentState_;
    }

    virtual double act(const SimAction *action) {
        reward_ = 0;
        int prevLives = ale_->lives();
        const AtariAction *other = dynamic_cast<const AtariAction *>(action);
        ale::Action act = other->act_;
        for (int i = 0; i < numRepeats_ && !ale_->gameOver(); ++i) {
            reward_ += ale_->act(act);
        }
        // update screen buffer
        if (updateFrame_) {
            frameBuffer_->pushFrame(ale_->getScreen());
        }
        currentState_->snapshot_ = ale_->getSnapshot();
        lifeLost_ = (prevLives != ale_->lives());
        if (scaleReward_) {
            if (reward_ > 0) return 1;
            if (reward_ < 0) return -1;
            return 0;
        }

        return reward_;
    }

    void setUpdateFrame() {
        updateFrame_ = true;
    }

    // minimize impact on other parts
    // return true if next state is not unique
    // conflict with pseudo death
    // solution: fix life counter in xitari
    // currently fixed: MsPacMan
    virtual bool actDiffer() {
        std::string currentSnapshot = currentState_->snapshot_;
        std::string prevSnapshot;
        for (int i = 0; i < actSet_.size(); ++i) {
            ale_->restoreSnapshot(currentSnapshot);
            for (int j = 0; j < numRepeats_ && !ale_->gameOver(); ++j) {
                ale_->act(actSet_[i]);
            }
            std::string snapshot = ale_->getSnapshot();
            if ((i > 0) && (prevSnapshot.compare(snapshot) != 0)) {
                ale_->restoreSnapshot(currentSnapshot);
                return true;
            }
            prevSnapshot = snapshot;
        }
        ale_->restoreSnapshot(currentSnapshot);
        return false;
    }

    // random action
    SimAction *getRandomAct() {
        int index = (int) (rand() % actVect_.size());
        return actVect_[index];
    }

    virtual vector<SimAction *> &getActions() {
        return actVect_;
    }

    virtual bool isTerminal() {
        return ale_->gameOver() || (pseudoGameover_ && lifeLost_);
    }

    virtual void reset() {
        ale_->resetGame();
        currentState_->snapshot_ = ale_->getSnapshot();
        lifeLost_ = false;
    }

    void recordData(string path, int index, const SimAction *action) {
        char buffer[20];
        sprintf(buffer, "/frames/%d.jpg", index);
        string input = path + buffer;
        this->frameBuffer_->writeToFile(input.c_str());

        // Find the action number
        const AtariAction *other = dynamic_cast<const AtariAction *>(action);
        ale::Action act = other->act_;

        //TODO important : il y a peut-être un décalage de numero entre le mappage des actions au niveau du UCT et de
        // l'Atari... A creuser si ça merde du genre violent.... voir le readme pour les vrais numéros ^^
        // int action_number = act_map[act];
        int action_number = act;

        // generate the database
        sprintf(buffer, "/dataset/%d/%d.jpg", action_number, index);
        string label = path + buffer;
        this->frameBuffer_->writeToFile(label.c_str());

        std::cout << label << std::endl;


    }

    bool getPseudoDeath() {
        return lifeLost_;

    }

    // control the condition of terminal states, true then lost live will termintate
    const bool pseudoGameover_;
    // number of action repeats
    const int numRepeats_;
    //
    const bool scaleReward_;
    double minReward_;
    double maxReward_;
    // true if just losing one life
    bool lifeLost_;
    //
    double reward_;
    // true then update frameBuffer_
    bool updateFrame_;
    FrameBuffer *frameBuffer_;
    //
    ale::ALEInterface *ale_;
    // coding constraint: always equal ale state
    AtariState *currentState_;
    // action set
    ale::ActionVect actSet_;
    vector<SimAction *> actVect_;

    map<ale::Action, int> act_map;

};


#endif
