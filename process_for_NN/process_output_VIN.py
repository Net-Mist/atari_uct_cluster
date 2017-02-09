import numpy as np
from PIL import Image
import glob
import time
import sys
from image_preprocess import *

output_dir = '/hpctmp2/e0046667/'
data = []

first_simu = int(sys.argv[1])
last_simu = int(sys.argv[2])


for k in range(first_simu, last_simu + 1):
    simulation_path = output_dir + "output"+str(k)
    action_dirs = glob.glob(simulation_path + '/dataset/*')
    action_dirs.sort()
    nb_actions = len(action_dirs)

    # Compute the number of frame
    nb_frames = 0
    for action_dir in action_dirs:
        nb_frames += len(glob.glob(action_dir + '/*'))
    print("k = ",k,", ",nb_frames, "frames")

    # Prepare the storage of the data
    states = np.zeros((nb_frames, 4 + 2 * 160), dtype='B')
    state_index_to_action_index = np.zeros(nb_frames, dtype='B')
    nb_frames = np.array([nb_frames])

    i = 0
    for action_dir in action_dirs:
        print(i)
        image_paths = glob.glob(action_dir + '/*')
        image_paths.sort()
        for image_path in image_paths:
            # Prepare the image
            image = Image.open(image_path)
            x_b, y_b, player_distribution, opponent_distribution = process(image)

            # Save the data
            frame_number = int(image_path.split('/')[-1][:-4])
            states[frame_number - 1, 0] = x_b
            states[frame_number - 1, 1] = y_b
            if frame_number < nb_frames:
                states[frame_number, 2] = x_b
                states[frame_number, 3] = y_b
            states[frame_number - 1, 4:164] = player_distribution
            states[frame_number - 1, 164:] = opponent_distribution
                
            # and save the action
            image_index_to_action_index[frame_number - 1] = i
        i += 1
    print('save')
    np.savez(output_dir + 'VIN/data' + str(k), states=states,
             image_index_to_action_index=image_index_to_action_index,
             nb_frames=nb_frames)

