#include "atari.hpp"
#include <gflags/gflags.h>

DEFINE_string(rom_path, "/home/svu/e0046667/atari_uct_cluster/pong.bin", "Game Rom File");
DEFINE_int32(num_traj, 500, "Sample trajectory numbers");
DEFINE_int32(depth, 300, "Planning depth");
DEFINE_double(ucb, 0.1, "Planning scalar");
DEFINE_double(leaf, 0, "Leaf Value");
DEFINE_double(gamma, 0.99, "Discount factor used in UCT");
DEFINE_bool(save_data, true, "True to save state and action pairs");
DEFINE_string(save_path, "output", "Path to save training data pairs");

int main(int argc, char** argv) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	srand(time(0));
	cout << "Rom: " << FLAGS_rom_path << endl;
	cout << "Num of traj: " << FLAGS_num_traj << endl;
	cout << "Depth: " << FLAGS_depth << endl;
	cout << "UCB: " << FLAGS_ucb << endl;
	cout << "Leaf: " << FLAGS_leaf << endl;
	string rom_path(FLAGS_rom_path);
	AtariSimulator* sim = new AtariSimulator(rom_path, true, false, false, 4);
	AtariSimulator* sim_in_plan = new AtariSimulator(rom_path, false, true, true, 4);
	UCTPlanner uct(sim_in_plan, FLAGS_depth, FLAGS_num_traj, FLAGS_ucb, FLAGS_gamma, 0, FLAGS_leaf);
    
	int steps = 0;
	bool prev_planned = false;
	SimAction* prev_action = NULL;
	double r = 0;
	double rwd ;
    int data_index = 0;
    const int max_steps = 20000;
	while (!sim->isTerminal() && steps < max_steps) {
		steps++;
		SimAction* action = NULL;
		if (sim->actDiffer()) {
			if (prev_planned && (!uct.terminalRoot())) {
				uct.prune(prev_action);
			} else {
				uct.setRootNode(sim->getState(), sim->getActions(), rwd, sim->isTerminal());
			}
    		uct.plan();
    		action = uct.getAction();
			prev_planned = true;
			++data_index;
			if(FLAGS_save_data) {
				sim->recordData(FLAGS_save_path, data_index, action);
			}
			
		} else {
			action = sim->getRandomAct();
			prev_planned = false;
		}

		prev_action = action;
		cout << "step: " << steps << " live: " << sim->lives() << " act: ";
		action->print();

		rwd = sim->act(action);
		r += rwd;
		cout << " rwd: " << rwd << " total: " << r << endl;
		// << " time: " << (total_time / (time_count + 1e-8)) << endl;
	}
	cout <<  "steps: " << steps << "\nr: " << r << endl;
	delete sim;
	delete sim_in_plan;
	return 0;
}
