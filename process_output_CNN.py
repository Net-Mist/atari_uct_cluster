import numpy as np
from PIL import Image
import glob
import time
import sys

output_dir = '/hpctmp2/e0046667/'
data = []
total_amount_data = 0

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
    print(nb_frames, "frames")
    total_amount_data += nb_frames

    images = np.zeros((nb_frames, 84, 84, 4), dtype='b')
    image_index_to_action_index = np.zeros(nb_frames, dtype='b')
    nb_frames = np.array([nb_frames])

    i = 0
    for action_dir in action_dirs:
        image_paths = glob.glob(action_dir + '/*')
        image_paths.sort()
        for image_path in image_paths:
            # Prepare the image
            image = Image.open(image_path).convert('L')
            image = image.crop((0, 34, 160, 194))
            image = image.resize((84, 84))
            image = np.array(image, dtype='b')

            # Save the image in 4 buffer
            frame_number = int(image_path.split('/')[-1][:-4])
            images[frame_number - 1, :, :, 0] = image
            if frame_number < nb_frames:
                images[frame_number, :, :, 1] = image
            if frame_number + 1 < nb_frames:
                images[frame_number + 1, :, :, 2] = image
            if frame_number + 2 < nb_frames:
                images[frame_number + 2, :, :, 3] = image

            # and save the action
            image_index_to_action_index[frame_number - 1] = i
        i += 1

    np.savez(output_dir + 'data' + str(k), images=images,
             image_index_to_action_index=image_index_to_action_index,
             nb_frames=nb_frames)
print("total :",total_amount_data)

